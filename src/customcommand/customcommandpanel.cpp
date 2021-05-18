/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     sunchengxi <sunchengxi@uniontech.com>
 *
 * Maintainer: sunchengxi <sunchengxi@uniontech.com>
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
#include "customcommandpanel.h"
#include "customcommandoptdlg.h"
#include "shortcutmanager.h"
#include "service.h"

#include <DGroupBox>
#include <DVerticalLine>
#include <DLog>
#include <DDialog>

#include <QPixmap>
#include <QEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

CustomCommandPanel::CustomCommandPanel(QWidget *parent) : CommonPanel(parent)
{
    Utils::set_Object_Name(this);
    initUI();
}
CustomCommandPanel::~CustomCommandPanel()
{
    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }
}

void CustomCommandPanel::showCurSearchResult()
{
    QString strTxt = m_searchEdit->text();
    if (strTxt.isEmpty()) {
        return;
    }
    emit showSearchResult(strTxt);
}

void CustomCommandPanel::showAddCustomCommandDlg()
{
    qDebug() <<  __FUNCTION__ << __LINE__;

    if (m_pushButton->hasFocus()) {
        qDebug() << "------------------------hasFocus";
        m_bpushButtonHaveFocus = true;
    } else {
        qDebug() << "------------------------ not         hasFocus";
        m_bpushButtonHaveFocus = false;
    }

    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }

    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);

    m_pdlg = new CustomCommandOptDlg(CustomCommandOptDlg::CCT_ADD, nullptr, this);
    m_pdlg->setObjectName("CustomAddCommandDialog");//Add by ut001000 renfeixiang 2020-08-14
    connect(m_pdlg, &CustomCommandOptDlg::finished, this, [ &](int result) {
        // 弹窗隐藏或消失
        Service::instance()->setIsDialogShow(window(), false);

        qDebug() << "finished" << result;

        if (QDialog::Accepted == result) {
            qDebug() << "Accepted";
            QAction *newAction = m_pdlg->getCurCustomCmd();
            //m_cmdListWidget->addItem(ItemFuncType_Item, newAction->text(), newAction->shortcut().toString());
            // 新增快捷键 => 显示在列表中使用大写 down2up dzw 20201215
            m_cmdListWidget->addItem(ItemFuncType_Item, newAction->text(), Utils::converDownToUp(newAction->shortcut().toString()));
            /************************ Add by m000743 sunchengxi 2020-04-20:解决自定义命令无法添加 Begin************************/
            ShortcutManager::instance()->addCustomCommand(*newAction);
            /************************ Add by m000743 sunchengxi 2020-04-20:解决自定义命令无法添加 End  ************************/

            emit Service::instance()->refreshCommandPanel("", "");

            refreshCmdSearchState();
            /******** Modify by m000714 daizhengwen 2020-04-10: 滚动条滑至最底端****************/
            int index = m_cmdListWidget->indexFromString(newAction->text());
            m_cmdListWidget->setScroll(index);
            /********************* Modify by m000714 daizhengwen End ************************/

        }

        if (m_bpushButtonHaveFocus) {
            m_pushButton->setFocus(Qt::TabFocusReason);
        }

    });
    m_pdlg->show();
}

void CustomCommandPanel::doCustomCommand(const QString &key)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    QAction *item = ShortcutManager::instance()->findActionByKey(key);

    QString strCommand = item->data().toString();
    if (!strCommand.endsWith('\n')) {
        strCommand.append('\n');
    }
    emit handleCustomCurCommand(strCommand);
}

void CustomCommandPanel::onFocusOut(Qt::FocusReason type)
{
    if ((Qt::TabFocusReason == type) || (Qt::NoFocusReason == type)) {
        // 下一个 或 列表为空， 焦点定位到添加按钮上
        m_pushButton->setFocus();
        m_cmdListWidget->clearIndex();
        qDebug() << "set focus on add pushButton";
    } else if (Qt::BacktabFocusReason == type) {
        // 判断是否可见，可见设置焦点
        if (m_searchEdit->isVisible()) {
            m_searchEdit->lineEdit()->setFocus();
            m_cmdListWidget->clearIndex();
            qDebug() << "set focus on add search edit";
        }
    }
}

void CustomCommandPanel::refreshCmdPanel()
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    clearSearchInfo();
    ShortcutManager::instance()->fillCommandListData(m_cmdListWidget);
    refreshCmdSearchState();
}

void CustomCommandPanel::refreshCmdSearchState()
{
    qDebug() << __FUNCTION__ << m_cmdListWidget->count() << endl;
    if (m_cmdListWidget->count() >= 2) {
        /************************ Add by m000743 sunchengxi 2020-04-22:自定义命令搜索显示异常 Begin************************/
        m_searchEdit->clearEdit();
        /************************ Add by m000743 sunchengxi 2020-04-22:自定义命令搜索显示异常  End ************************/
        m_searchEdit->show();
    } else {
        m_searchEdit->hide();
    }
}

void CustomCommandPanel::setFocusInPanel()
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    if (m_searchEdit->isVisible()) {
        // 搜索框在
        m_searchEdit->lineEdit()->setFocus();
    } else if (m_cmdListWidget->isVisible() && m_cmdListWidget->count() != 0) {
        // 列表数据不为0
        m_cmdListWidget->setFocus();
    } else if (m_pushButton->isVisible()) {
        // 添加按钮下
        m_pushButton->setFocus();
    } else {
        qDebug() << "focus error unkown reason";
    }
}

void CustomCommandPanel::initUI()
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    m_searchEdit = new DSearchEdit(this);
    //fix bug#64976 按tab键循环切换到右上角X按钮时继续按tab键，焦点不能切换到搜索框
    m_searchEdit->setFocusPolicy(Qt::StrongFocus);
    m_searchEdit->setFocusProxy(m_searchEdit->lineEdit());
    m_searchEdit->setObjectName("CustomSearchEdit");//Add by ut001000 renfeixiang 2020-08-13
    m_searchEdit->setClearButtonEnabled(true);
    DFontSizeManager::instance()->bind(m_searchEdit, DFontSizeManager::T6);

    m_cmdListWidget = new ListView(ListType_Custom, this);
    m_cmdListWidget->setObjectName("CustomCommandListWidget");//Add by ut001000 renfeixiang 2020-08-13

    m_pushButton = new DPushButton(this);
    m_pushButton->setObjectName("CustomAddCommandButton");//Add by ut001000 renfeixiang 2020-08-13
    m_pushButton->setFixedHeight(36);
    m_pushButton->setText(tr("Add Command"));

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addSpacing(10);
    btnLayout->addWidget(m_pushButton);
    btnLayout->addSpacing(10);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addSpacing(10);
    hLayout->addWidget(m_searchEdit);
    hLayout->addSpacing(10);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    m_vlayout = vLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(10);
    vLayout->addSpacing(10);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_cmdListWidget);
    vLayout->addLayout(btnLayout);
    vLayout->addSpacing(10);
    setLayout(vLayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &CustomCommandPanel::showCurSearchResult);
    connect(m_pushButton, &DPushButton::clicked, this, &CustomCommandPanel::showAddCustomCommandDlg);
    connect(m_cmdListWidget, &ListView::itemClicked, this, &CustomCommandPanel::doCustomCommand);
    connect(m_cmdListWidget, &ListView::listItemCountChange, this, &CustomCommandPanel::refreshCmdSearchState);
    connect(m_cmdListWidget, &ListView::focusOut, this, &CustomCommandPanel::onFocusOut);
}
