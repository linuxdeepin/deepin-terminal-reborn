/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangpeili <wangpeili@uniontech.com>
 *
 * Maintainer: wangpeili <wangpeili@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "termwidgetpage.h"
#include "termwidget.h"
#include "settings.h"
#include "utils.h"
#include "service.h"
#include "windowsmanager.h"
#include "mainwindow.h"
#include "define.h"

#include <DLog>
#include <DDialog>

#include <QVBoxLayout>
#include <QApplication>

TermWidgetPage::TermWidgetPage(const TermProperties &properties, QWidget *parent)
    : QWidget(parent), m_findBar(new PageSearchBar(this))
{
    Utils::set_Object_Name(this);
    //qDebug() << "parentTermWidgetPage" << parentWidget();
    m_MainWindow = static_cast<MainWindow *>(parentWidget());
    setFocusPolicy(Qt::NoFocus);
    setProperty("TAB_CUSTOM_NAME_PROPERTY", false);

    //fix bug 55961 UOS SP2 OEM版本的系统下，一个窗口同时打开几个终端，各终端简无法切换
    //Qt自带API接口QUuid::createUuid()在浪潮服务器上会生成重复的id：ffffffff-ffff-4fff-bfff-ffffffffffff
    QString pageId = QString::number(reinterpret_cast<qulonglong>(this));
    // 生成唯一 pageID
    setProperty("TAB_IDENTIFIER_PROPERTY", pageId);

    TermWidget *w = createTerm(properties);
    m_layout = new QVBoxLayout(this);
    m_layout->setObjectName("TermPageLayout");//Add by ut001000 renfeixiang 2020-08-13
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(w);
    setLayout(m_layout);
    //qDebug() << "w->parent()" << w->parent();

    // Init find bar.
    connect(m_findBar, &PageSearchBar::findNext, this, &TermWidgetPage::handleFindNext);
    connect(m_findBar, &PageSearchBar::findPrev, this, &TermWidgetPage::handleFindPrev);
    connect(m_findBar, &PageSearchBar::keywordChanged, this, &TermWidgetPage::handleKeywordChanged);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, &TermWidgetPage::handleThemeTypeChanged);

    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 2020-05-28 优化代码 Begin***************/
    connect(this, &TermWidgetPage::uninstallTerminal, this, &TermWidgetPage::handleUninstallTerminal);
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/

    m_currentTerm = w;
}

inline void TermWidgetPage::handleKeywordChanged(QString keyword)
{
    handleUpdateSearchKeyword(keyword);
}

inline void TermWidgetPage::handleThemeTypeChanged()
{
    applyTheme();
}

inline bool TermWidgetPage::handleUninstallTerminal(QString commandname)
{
    //MainWindow *mainWindow = qobject_cast<MainWindow *>(parent);
    //构造函数中已经获取了mainwindow窗口，无需在获取一遍
    if (m_MainWindow->hasRunningProcesses()) {
        if (!Utils::showExitUninstallConfirmDialog()) {
            return false;
        }
    }
    return Utils::showUnistallConfirmDialog(commandname);
}

MainWindow *TermWidgetPage::parentMainWindow()
{
    //qDebug()<<"parentMainWindow" <<parentWidget();
    return m_MainWindow;
}

void TermWidgetPage::setParentMainWindow(MainWindow *mainWin)
{
    m_MainWindow = mainWin;
}

void TermWidgetPage::setSplitStyle(DSplitter *splitter)
{
    splitter->setHandleWidth(1);
    QSplitterHandle *handle = splitter->handle(1);

    if (handle) {
        //分割线颜色暂时设置为Highlight颜色，需要和UI确认下
        //此处代码暂时保留  //DPalette pa = DApplicationHelper::instance()->palette(handle);
        //bug#57044 中的分割线颜色，保留的代码对默认主题，和十个内置主题的颜色是正确获取，但是在自定义的颜色获取存在异常，采取如下方式获取
        DPalette pa = DApplicationHelper::instance()->applicationPalette();
        QColor splitBrush = pa.color(DPalette::Highlight);
        pa.setBrush(DPalette::Background, splitBrush);
        handle->setPalette(pa);
        handle->setBackgroundRole(QPalette::Background);
        handle->setAutoFillBackground(true);
    }
}

TermWidget *TermWidgetPage::currentTerminal()
{
    return m_currentTerm;
}

QString TermWidgetPage::getCurrentTerminalTitle()
{
    if (nullptr == m_currentTerm)
        return "";
    return m_currentTerm->getTabTitle();
}

void TermWidgetPage::split(Qt::Orientation orientation)
{
    parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_NONE);
    TermWidget *term = m_currentTerm;
    if (getTerminalCount() == 1) {
        qDebug() << "first split";
        QSplitter *firstSplit = createSubSplit(term, orientation);
        m_layout->addWidget(firstSplit);
        //return ;
    } else {
        qDebug() << "not first split";
        QSplitter *upSplit = qobject_cast<QSplitter *>(term->parent());
        int index = upSplit->indexOf(term);
        QList<int> parentSizes = upSplit->sizes();

        // 用新的Split分割布局替换原来的位置
        QSplitter *subSplit = createSubSplit(term, orientation);
        upSplit->insertWidget(index, subSplit);
        upSplit->setSizes(parentSizes);
        setSplitStyle(upSplit);
    }

    /******** Add by ut001000 renfeixiang 2020-08-07:新增分屏时改变大小，bug#41436***************/
    parentMainWindow()->updateMinHeight();
    return ;
}

DSplitter *TermWidgetPage::createSubSplit(TermWidget *term, Qt::Orientation orientation)
{
    TermProperties properties(term->workingDirectory());
    term->setParent(nullptr);
    TermWidget *newTerm  = createTerm(properties);

    // 意义与名称是相反的
    DSplitter *subSplit = new DSplitter(orientation == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal,
                                        this);

    subSplit->setFocusPolicy(Qt::NoFocus);
    subSplit->insertWidget(0, term);
    subSplit->insertWidget(1, newTerm);
    subSplit->setSizes({ 1, 1 });
    setSplitStyle(subSplit);
    setCurrentTerminal(newTerm);
    /******** Modify by ut000439 wangpeili 2020-07-27: fix bug 39371: 分屏线可以拉到边****/
    subSplit->setChildrenCollapsible(false);
    /********************* Modify by n014361 wangpeili End ************************/

    return subSplit;
}

void TermWidgetPage::closeSplit(TermWidget *term, bool hasConfirmed)
{
    qDebug() << "TermWidgetPage::closeSplit:" << term->getSessionId();
    if (getTerminalCount() > 1) {
        if (!hasConfirmed && term->hasRunningProcess()) {
            showExitConfirmDialog(Utils::CloseType_Terminal, 1, parentMainWindow());
            return;
        }
        QSplitter *upSplit = qobject_cast<QSplitter *>(term->parent());
        term->setParent(nullptr);

        // 另一个兄弟也可能是终端，也可能是split,
        QWidget *brother = upSplit->widget(0);
        TermWidget *nextTerm =  upSplit->findChild<TermWidget *>();
        // 如果上级是分屏
        if ("QSplitter" == QString(upSplit->parent()->metaObject()->className())) {
            QSplitter *upupSplit = qobject_cast<QSplitter *>(upSplit->parent());
            //兄弟替换parent split
            upupSplit->replaceWidget(upupSplit->indexOf(upSplit), brother);
        }
        // 上级不是分屏控件，就是布局在控制了
        else {
            qDebug() << "TermWidgetPage only one term exist!";
            m_layout->addWidget(brother);
        }

        // 子控件的变化会引起焦点的变化，控制焦点要放在最后
        if (nextTerm != nullptr) {
            qDebug() << "nextTerm change" << m_currentTerm->getSessionId();
            nextTerm->setFocus();
        } else {
            qDebug() << "can not found nextTerm in TermWidget";
        }

        // 释放控件
        term->deleteLater();
        upSplit->setParent(nullptr);
        upSplit->deleteLater();
        qDebug() << "page terminal count =" << getTerminalCount();
        /******** Add by ut001000 renfeixiang 2020-08-07:关闭分屏时改变大小，bug#41436***************/
        parentMainWindow()->updateMinHeight();
        return;
    }
    parentMainWindow()->closeTab(identifier());
    return;
}

void TermWidgetPage::showExitConfirmDialog(Utils::CloseType type, int count, QWidget *parent)
{
    /******** Modify by ut001000 renfeixiang 2020-06-03:修改 直接调用MainWindow中的showExitConfirmDialog函数中 Begin***************/
    parentMainWindow()->showExitConfirmDialog(type, count, parent);
    /******** Modify by ut001000 renfeixiang 2020-06-03:修改 直接调用MainWindow中的showExitConfirmDialog函数中 End***************/
//    // count < 1 不提示
//    if (count < 1) {
//        return;
//    }
//    QString title;
//    QString txt;
//    Utils::getExitDialogText(type, title, txt, count);

//    // 有弹窗显示
//    Service::instance()->setIsDialogShow(window(), true);

//    DDialog *dlg = new DDialog(title, txt, parent);
//    dlg->setIcon(QIcon::fromTheme("deepin-terminal"));
//    dlg->addButton(QString(tr("Cancel")), false, DDialog::ButtonNormal);
//    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close Begin***************/
//    dlg->addButton(QString(tr("Close")), true, DDialog::ButtonWarning);
//    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close End***************/
//    dlg->setWindowModality(Qt::WindowModal);
//    setAttribute(Qt::WA_ShowModal);
//    dlg->show();


//    if (type == Utils::CloseType_Terminal) {
//        connect(dlg, &DDialog::finished, this, [this](int result) {
//            qDebug() << result;
//            // 有弹窗消失
//            Service::instance()->setIsDialogShow(window(), false);
//            if (result == 1) {
//                //接口二次重入
//                closeSplit(currentTerminal(), true);
//            }
//        });
//    }

//    if (type == Utils::CloseType_OtherTerminals) {
//        connect(dlg, &DDialog::finished, this, [this](int result) {
//            qDebug() << result;
//            // 有弹窗消失
//            Service::instance()->setIsDialogShow(window(), false);
//            if (result == 1) {
//                //接口二次重入
//                closeOtherTerminal(true);
//            }
//        });
//    }

    return ;
}

const QString TermWidgetPage::identifier()
{
    return property("TAB_IDENTIFIER_PROPERTY").toString();
}

void TermWidgetPage::focusCurrentTerm()
{
    m_currentTerm->setFocus();
}

void TermWidgetPage::closeOtherTerminal(bool hasConfirmed)
{
    int runningCount = runningTerminalCount() - static_cast<int>(currentTerminal()->hasRunningProcess());
    if (!hasConfirmed && runningCount != 0) {
        showExitConfirmDialog(Utils::CloseType_OtherTerminals, runningCount, parentMainWindow());
        return;
    }

    QList<TermWidget *> termList = findChildren<TermWidget *>();
    // 终端数量小于2,执行关闭其他窗口操作
    if (termList.count() < 2) {
        qDebug() << "current window doesn't have other terminal, can't close other terminals.";
        return;
    }

    int currSessionId = m_currentTerm->getSessionId();
    //exit protection
    for (TermWidget *term : qAsConst(termList)) {
        if (term->getSessionId() != currSessionId) {
            // 前面已经检测过了，不用重复检测
            closeSplit(term, true);
        }
    }

    setTerminalOpacity(Settings::instance()->opacity());
    m_MainWindow->focusCurrentPage();
}

/******** Modify by ut000439 wangpeili 2020-01-07:  修改了计算上下左右判断的方法 ********×****/
void TermWidgetPage::focusNavigation(Qt::Edge dir)
{
    QPoint comparPoint = GetComparePoint(currentTerminal(), dir);
    TermWidget *dst = nullptr;
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    //QMap<TermWidget *, QRect> mapTermRect;
    for (TermWidget *term : qAsConst(termList)) {
        if (GetRect(term).contains(comparPoint)) {
            qDebug() << "yes!" << comparPoint.x() << comparPoint.y();
            dst = term;
            break;
        }
    }
    if (dst) {
        dst->setFocus();
    }
}

int TermWidgetPage::getTerminalCount()
{
    return findChildren<TermWidget *>().size();
}

bool TermWidgetPage::hasHasHorizontalSplit()
{
    qDebug() << "start hasHasHorizontalSplit";
    QList<QSplitter *> splitList = findChildren<QSplitter *>();
    for (QSplitter *split : splitList) {
        if (split->orientation() == Qt::Vertical) {
            return  true;
        }
    }
    return  false;
}
/********************* Modify by ut000439 wangpeili End ************************/
QRect TermWidgetPage::GetRect(TermWidget *term)
{
    QPoint leftTop = term->mapTo(term->window(), QPoint(0, 0));
    QPoint rightBottom = term->mapTo(term->window(), QPoint(term->width(), term->height()));
    QRect rec(leftTop, rightBottom);
    qDebug() << "leftTop: " << leftTop.x() << leftTop.y() << "rightBottom: " << rightBottom.x() << rightBottom.y();
    return rec;
}

QPoint TermWidgetPage::GetComparePoint(TermWidget *term, Qt::Edge dir)
{
    QPoint leftTop = term->mapTo(term->window(), QPoint(0, 0));
    QPoint leftBottom = term->mapTo(term->window(), QPoint(0, term->height()));
    QPoint rightTop = term->mapTo(term->window(), QPoint(term->width(), 0));
    QPoint ret;
    switch (dir) {
    case Qt::LeftEdge:
        ret = leftTop + QPoint(-1, 1);
        break;
    case Qt::RightEdge:
        ret = rightTop + QPoint(1, 1);
        break;
    case Qt::TopEdge:
        ret = leftTop + QPoint(1, -1);
        break;
    case Qt::BottomEdge:
        ret = leftBottom + QPoint(1, 1);
        break;
    }
    return ret;
}

int TermWidgetPage::runningTerminalCount()
{
    int count = 0;
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        if (term->hasRunningProcess()) {
            count++;
        }
    }
    return count;
}

TermProperties TermWidgetPage::createCurrentTerminalProperties()
{
    TermProperties properties;

    TermWidget *term = currentTerminal();
    if (term) {
        properties[WorkingDir] = currentTerminal()->workingDirectory();
    }

    return properties;
}

void TermWidgetPage::setTerminalOpacity(qreal opacity)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setTermOpacity(opacity);
    }
}

void TermWidgetPage::setColorScheme(const QString &name)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setColorScheme(name);
    }
}

void TermWidgetPage::sendTextToCurrentTerm(const QString &text, bool isRemoteConnect)
{
    TermWidget *term = currentTerminal();
    if (term) {
        //标记当前命令是代码中通过sendText发给终端的(而不是用户手动输入的命令)
        term->setProperty("isSendByRemoteManage", QVariant(isRemoteConnect));
        term->sendText(text);
    }
}

void TermWidgetPage::copyClipboard()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->copyClipboard();
    }
}

void TermWidgetPage::pasteClipboard()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->pasteClipboard();
    }
}

void TermWidgetPage::toggleShowSearchBar()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->toggleShowSearchBar();
    }
}

void TermWidgetPage::zoomInCurrentTierminal()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->zoomIn();
    }
}

void TermWidgetPage::zoomOutCurrentTerminal()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->zoomOut();
    }
}

void TermWidgetPage::setFontSize(int fontSize)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setTermFontSize(fontSize);
    }
}

void TermWidgetPage::setFont(QString fontName)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setTermFont(fontName);
    }
}

void TermWidgetPage::selectAll()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->setSelectionAll();
    }
}

void TermWidgetPage::skipToNextCommand()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->skipToNextCommand();
    }
}

void TermWidgetPage::skipToPreCommand()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->skipToPreCommand();
    }
}

void TermWidgetPage::setcursorShape(int shape)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setCursorShape(shape);
    }
}

void TermWidgetPage::setBlinkingCursor(bool enable)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setBlinkingCursor(enable);
    }
}

void TermWidgetPage::setPressingScroll(bool enable)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setPressingScroll(enable);
    }
}

void TermWidgetPage::showSearchBar(int state)
{
    /******** Modify by ut001000 renfeixiang 2020-08-28:修改bug 45227,SearchBar没有显示，且不需要显示时，return Begin***************/
    // 沒显示，且不要显示
    if (!m_findBar->isVisible() && state != SearchBar_Show) {
        return;
    }
    /******** Modify by ut001000 renfeixiang 2020-08-28***************/
    if (SearchBar_Show == state) {
        /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，查询界面使用不方便，将雷神窗口变大适应正常的查询界面 Begin***************/
        if (m_MainWindow->isQuakeMode() && m_MainWindow->height() < LISTMINHEIGHT) {
            //因为拉伸函数设置了FixSize，导致自定义界面弹出时死循环，然后崩溃的问题
            QuakeWindow *quakeWindow = qobject_cast<QuakeWindow *>(m_MainWindow);
            if (nullptr == quakeWindow)
                return;
            quakeWindow->switchEnableResize(true);
            m_MainWindow->resize(m_MainWindow->width(), LISTMINHEIGHT); //首先设置雷神界面的大小
            m_MainWindow->showPlugin(MainWindow::PLUGIN_TYPE_SEARCHBAR);//重新打开查询界面，当前流程结束
            //窗口弹出后，重新判断雷神窗口是否需要有拉伸属性
            quakeWindow->switchEnableResize();
            return;
        }
        /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，查询界面使用不方便，将雷神窗口变大适应正常的查询界面 End***************/
        m_findBar->raise();
        m_findBar->clearHoldContent();
        m_findBar->show();
        //Add by ut001000 renfeixiang 2020-12-02 在搜索框弹出时，添加设置Term的m_bHasSelect为false函数
        if (m_currentTerm != nullptr) {
            m_currentTerm->setNoHasSelect();
        }
        m_findBar->move(width() - SEARCHBAR_RIGHT_MARGIN, 0);
        qDebug() << __FUNCTION__ << "show search bar!";
        QTimer::singleShot(10, this, [ = ] { m_findBar->focus(); });
    } else if (SearchBar_Hide == state) {
        m_findBar->hide();
        qDebug() << __FUNCTION__ << "hide search bar!";
    } else if (SearchBar_FocusOut == state) {

        qDebug() << __FUNCTION__ << "hide search bar! focus in term!";
        /******** Modify by ut001000 renfeixiang 2020-08-28:修改bug 45227,焦点只有在m_findBar上时，才将焦点设置到CurrentPage Begin***************/
        if (Utils::getMainWindow(this)->isFocusOnList()) {
            Utils::getMainWindow(this)->focusCurrentPage();
        }
        m_findBar->hide();
        /******** Modify by ut001000 renfeixiang 2020-08-28 End***************/
    }
}

void TermWidgetPage::showRenameTitleDialog()
{
    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);
    if (nullptr == m_renameDlg) {
        m_renameDlg = new TabRenameDlg(this);
        m_renameDlg->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QString tabTitleFormat = currentTerminal()->getTabTitleFormat();
        QString remoteTabTitleFormat = currentTerminal()->getRemoteTabTitleFormat();
        m_renameDlg->setText(tabTitleFormat, remoteTabTitleFormat);
        m_renameDlg->setFocusPolicy(Qt::NoFocus);
        m_renameDlg->setAttribute(Qt::WA_DeleteOnClose);

        //bug 53463 ut000442 判断当前是否为远程链接状态，根据状态设置对应行的选中
        //设置全选效果
        m_renameDlg->setFocusOnEdit(currentTerminal()->isConnectRemote());

        m_renameDlg->open();

        connect(m_renameDlg, &TabRenameDlg::finished, this, &TermWidgetPage::handleTabRenameDlgFinished);

        connect(m_renameDlg, &TabRenameDlg::tabTitleFormatRename, this, &TermWidgetPage::tabTitleFormatChanged);
    }
}

inline void TermWidgetPage::handleTabRenameDlgFinished()
{
    // 弹窗隐藏或消失
    Service::instance()->setIsDialogShow(window(), false);
    m_renameDlg = nullptr;
}

void TermWidgetPage::printSearchCostTime()
{
    qint64 costTime = m_findBar->searchCostTime();
    if (costTime != -1) {
        QString strSearchTime = GRAB_POINT + LOGO_TYPE + SEARCH_TIME + QString::number(costTime);
        qDebug() << qPrintable(strSearchTime);
    }
}

void TermWidgetPage::onTermRequestRenameTab(QString newTabName)
{
    if (newTabName.isEmpty()) {
        setProperty("TAB_CUSTOM_NAME_PROPERTY", false);
        emit termTitleChanged(m_currentTerm->title());
    } else {
        // Mark it as we renamed it.
        setProperty("TAB_CUSTOM_NAME_PROPERTY", true);
        // Yeah, TermWidgetPage doesn't store the tab name, only the tab bar did it.
        emit tabTitleChanged(newTabName);
    }
}

void TermWidgetPage::onTermTitleChanged(QString title)
{
    TermWidget *term = qobject_cast<TermWidget *>(sender());
    // 标题内容没变化的话不发，不是当前终端改变，不发
    if (m_currentTerm == term && m_tabTitle != title) {
        m_tabTitle = title;
        emit termTitleChanged(title);
    }
}

void TermWidgetPage::onTermGetFocus()
{
    TermWidget *term = qobject_cast<TermWidget *>(sender());
    setCurrentTerminal(term);
    emit Service::instance()->currentTermChange(m_currentTerm);
    qDebug() << "onTermGetFocus" << m_currentTerm->getSessionId();
    emit termGetFocus();
}

void TermWidgetPage::onTermClosed()
{
    TermWidget *w = qobject_cast<TermWidget *>(sender());
    if (!w) {
        qDebug() << "TermWidgetPage::onTermClosed: Unknown object to handle" << w;
        return;
    }
    closeSplit(w);
}

void TermWidgetPage::handleFindNext()
{
    qDebug() << m_findBar->searchKeytxt();
    setMismatchAlert(false);
    m_currentTerm->search(m_findBar->searchKeytxt(), true, true);
}

void TermWidgetPage::handleFindPrev()
{
    setMismatchAlert(false);
    m_currentTerm->search(m_findBar->searchKeytxt(), false, false);
}

/*******************************************************************************
 1. @函数:    slotFindbarClose
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    空函数，待处理
*******************************************************************************/
//void TermWidgetPage::slotFindbarClose()
//{
//    EditWrapper *wrapper = currentWrapper();
//    if (wrapper->bottomBar()->isHidden())
//    {
//        wrapper->bottomBar()->show();
//    }
//}

/*******************************************************************************
 1. @函数:    handleRemoveSearchKeyword
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    空函数，待处理
*******************************************************************************/
//void TermWidgetPage::handleRemoveSearchKeyword()
//{
//    currentWrapper()->textEditor()->removeKeywords();
//}

void TermWidgetPage::handleUpdateSearchKeyword(const QString &keyword)
{
    setMismatchAlert(false);
    if (keyword.isEmpty()) {
        m_currentTerm->clearSelection();
    } else {
        // 输入时直接查找，被禁用
        // m_currentTerm->search(m_findBar->SearchKeytxt(), true, false);
    }
}

void TermWidgetPage::applyTheme()
{
    updateSplitStyle();
}

void TermWidgetPage::updateSplitStyle()
{
    QList<DSplitter *> splitList = findChildren<DSplitter *>();
    for (DSplitter *splitter : qAsConst(splitList)) {
        setSplitStyle(splitter);
    }
}

void TermWidgetPage::slotShowPluginChanged(const QString name)
{
    MainWindow *mainWindow = qobject_cast<MainWindow *>(sender());
    // 判断是否是当前页，不是当前页不用管
    if (mainWindow && mainWindow->currentPage() == this) {
        // 显示和隐藏搜索框
        if (MainWindow::PLUGIN_TYPE_SEARCHBAR == name) {
            // 显示搜索框
            this->showSearchBar(SearchBar_Show);
        } else if (MainWindow::PLUGIN_TYPE_NONE == name) {
            // 隐藏搜索框，焦点落回终端
            this->showSearchBar(SearchBar_FocusOut);
        } else {
            // 隐藏搜索框
            this->showSearchBar(SearchBar_Hide);
        }
    }
}

void TermWidgetPage::slotQuakeHidePlugin()
{
    MainWindow *mainWindow = qobject_cast<MainWindow *>(sender());
    // 判断是否是当前页，不是当前页不用管
    if (mainWindow && mainWindow->currentPage() == this) {
        // 隐藏搜索框
        this->showSearchBar(SearchBar_Hide);
    }
}

void TermWidgetPage::setCurrentTerminal(TermWidget *term)
{
    TermWidget *oldTerm = m_currentTerm;
    m_currentTerm = term;
    if (oldTerm != m_currentTerm) {
        // 当前界面切换
        qDebug() << "m_currentTerm change" << m_currentTerm->getSessionId();
        QString tabTitle = term->getTabTitle();
        // 当前标签为空，标签格式不为空 => 未得到term参数，暂不上传数据
        if ((tabTitle == DEFAULT_TAB_TITLE) && !term->getCurrentTabTitleFormat().trimmed().isEmpty()) {
            return;
        }
        m_tabTitle = tabTitle;
        // 当前窗口变化修改标签标题
        emit termTitleChanged(m_tabTitle);
    }
}

TermWidget *TermWidgetPage::createTerm(TermProperties properties)
{
    TermWidget *term = new TermWidget(properties, this);
    connect(term, &TermWidget::termRequestRenameTab, this, &TermWidgetPage::onTermRequestRenameTab);
    connect(term, &TermWidget::termTitleChanged, this, &TermWidgetPage::onTermTitleChanged);
    connect(term, &TermWidget::termGetFocus, this, &TermWidgetPage::onTermGetFocus);
    connect(term, &TermWidget::leftMouseClick, this, &TermWidgetPage::handleLeftMouseClick);

    connect(term, &TermWidget::finished, this, &TermWidgetPage::onTermClosed);
    qDebug() << "create Terminal, sessionId = " << term->getSessionId();
    // 对标签页重命名设置
    connect(this, &TermWidgetPage::tabTitleFormatChanged, term, &TermWidget::renameTabFormat);
    return term;
}

inline void TermWidgetPage::handleLeftMouseClick()
{
    parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_NONE);
}

void TermWidgetPage::setTextCodec(QTextCodec *codec)
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->setTextCodec(codec);
    }
}

void TermWidgetPage::setMismatchAlert(bool alert)
{
    m_findBar->setNoMatchAlert(alert);
}

void TermWidgetPage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    //qDebug() << "resizeEvent" << x() << y();
    this->m_findBar->move(width() - SEARCHBAR_RIGHT_MARGIN, 0);
}
