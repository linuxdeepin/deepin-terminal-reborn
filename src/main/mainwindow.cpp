/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangpeili<wangpeili@uniontech.com>
 *
 * Maintainer:wangpeili<wangpeili@uniontech.com>
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
#include "mainwindow.h"

#include "mainwindowplugininterface.h"
#include "settings.h"
#include "shortcutmanager.h"
#include "tabbar.h"
#include "termproperties.h"
#include "termwidgetpage.h"
#include "termwidget.h"
#include "titlebar.h"
#include "operationconfirmdlg.h"
#include "encodepanelplugin.h"
#include "customcommandplugin.h"
#include "serverconfigmanager.h"
#include "utils.h"
#include "dbusmanager.h"
#include "windowsmanager.h"
#include "service.h"
#include "terminalapplication.h"

#include <DSettings>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DThemeManager>
#include <DTitlebar>
#include <DFileDialog>
#include <DLog>

#include <QApplication>
#include <QDesktopWidget>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QtDBus>

#include <QVBoxLayout>
#include <QMap>
#include <DImageButton>

#include <fstream>
using std::ifstream;
using std::ofstream;

/******** Add by ut001000 renfeixiang 2020-08-07:将最小宽度和高度设置成全局变量***************/
const int MainWindow::m_MinWidth = 450;
const int MainWindow::m_MinHeight = 250;

DWIDGET_USE_NAMESPACE

// 定义雷神窗口边缘,接近边缘光标变化图标
#define QUAKE_EDGE 5

MainWindow::MainWindow(TermProperties properties, QWidget *parent)
    : DMainWindow(parent),
      m_menu(new QMenu(this)),
      m_tabbar(nullptr),
      m_centralWidget(new QWidget(this)),
      m_centralLayout(new QVBoxLayout(m_centralWidget)),
      m_termStackWidget(new QStackedWidget),
      m_properties(properties),
      m_isQuakeWindow(properties[QuakeMode].toBool()),
      m_winInfoConfig(new QSettings(getWinInfoConfigPath(), QSettings::IniFormat, this))
{
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 Begin***************/
    m_menu->setObjectName("MainWindowQMenu");
    m_centralWidget->setObjectName("MainWindowCentralQWidget");
    m_centralLayout->setObjectName("MainWindowCentralLayout");
    m_termStackWidget->setObjectName("MainWindowTermQStackedWidget");
    m_winInfoConfig->setObjectName("MainWindowWindowQSettings");
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 End***************/
    static int id = 0;
    m_MainWindowID = ++id;
    // 首先到 “此刻的共享内存”里面找，如果找不到内容，说明这个不是子进程
    m_ReferedAppStartTime = Service::instance()->getSubAppStartTime();
    if (m_ReferedAppStartTime == 0) {
        // 主进程的启动时间存在APP中
        TerminalApplication *app = static_cast<TerminalApplication *>(qApp);
        m_ReferedAppStartTime = app->getStartTime();
        qDebug() << "[main app] Start Time = "
                 << QDateTime::fromMSecsSinceEpoch(m_ReferedAppStartTime).toString("yyyy-MM-dd hh:mm:ss:zzz");
    } else {
        qDebug() << "[sub app] Start Time = "
                 << QDateTime::fromMSecsSinceEpoch(m_ReferedAppStartTime).toString("yyyy-MM-dd hh:mm:ss:zzz");
    }
    m_CreateWindowTime = Service::instance()->getEntryTime();
    qDebug() << "MainWindow Create Time = "
             << QDateTime::fromMSecsSinceEpoch(m_CreateWindowTime).toString("yyyy-MM-dd hh:mm:ss:zzz");
}

/*******************************************************************************
 1. @函数:    initUI
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类初始化UI界面
*******************************************************************************/
void MainWindow::initUI()
{
    initWindow();
    // Plugin may need centralWidget() to work so make sure initPlugin() is after setCentralWidget()
    // Other place (eg. create titlebar menu) will call plugin method so we should create plugins before init other
    // parts.
    initPlugins();
    initTabBar();
    initTitleBar();
    initWindowAttribute();


    qApp->installEventFilter(this);
}
/*******************************************************************************
 1. @函数:    initWindow
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类初始化窗口
*******************************************************************************/
void MainWindow::initWindow()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(m_MinWidth, m_MinHeight);
    setEnableBlurWindow(Settings::instance()->backgroundBlur());
    setWindowIcon(QIcon::fromTheme("deepin-terminal"));

    // Init layout
    m_centralLayout->setMargin(0);
    m_centralLayout->setSpacing(0);
    m_centralLayout->addWidget(m_termStackWidget);
    setCentralWidget(m_centralWidget);
}

/*******************************************************************************
 1. @函数:    initTabBar
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类初始化标签栏
*******************************************************************************/
void MainWindow::initTabBar()
{
    m_tabbar = new TabBar(this);
    m_tabbar->setFocusPolicy(Qt::NoFocus);

    connect(m_tabbar, &DTabBar::tabBarClicked, this, [ = ](int index) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(index));
        TermWidget *term = tabPage->currentTerminal();
        bool bIdle = !(term->hasRunningProcess());

        int currSessionId = term->getSessionId();
        if (bIdle && isTabChangeColor(currSessionId)) {
            m_tabVisitMap.insert(currSessionId, true);
            m_tabChangeColorMap.insert(currSessionId, false);
            m_tabbar->removeNeedChangeTextColor(index);
        }
    });

    // 点击TAB上页触发
    connect(m_tabbar,
            &DTabBar::currentChanged,
            this,
    [this](int index) {
        focusPage(m_tabbar->identifier(index));
    },
    Qt::QueuedConnection);
    // 点击TAB上的＂＋＂触发
    connect(m_tabbar, &DTabBar::tabAddRequested, this, [this]() {
        createNewWorkspace();
    }, Qt::QueuedConnection);

    // 点击TAB上的＂X＂触发
    connect(m_tabbar,
            &DTabBar::tabCloseRequested,
            this,
    [this](int index) {
        closeTab(m_tabbar->identifier(index));
    },
    Qt::QueuedConnection);

    // TAB菜单发来的关闭请求
    connect(m_tabbar, &TabBar::menuCloseTab, this, [ = ](QString Identifier) {
        closeTab(Identifier);
        return;
    });

    // TAB菜单发来的关闭其它窗口请求,需要逐一关闭
    connect(m_tabbar, &TabBar::menuCloseOtherTab, this, [ = ](QString Identifier) {
        closeOtherTab(Identifier);
        return;
    });

    addTab(m_properties);
}

/*******************************************************************************
 1. @函数:    initOptionButton
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    普通模式下，option button需要在全屏时切换控件
*******************************************************************************/
void MainWindow::initOptionButton()
{
    // 全屏退出按钮
    // DTK的全屏按钮不能满足UI要求，隐去DTK最右侧的全屏
    QWidget *dtkbutton = titlebar()->findChild<QWidget *>("DTitlebarDWindowQuitFullscreenButton");
    if (dtkbutton != nullptr) {
        dtkbutton->hide();
    } else {
        qDebug() << "can not found DTitlebarDWindowQuitFullscreenButton in DTitlebar";
    }


    // option button
    DIconButton *optionBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton");
    if (optionBtn != nullptr) {
        //optionBtn->setFocusPolicy(Qt::TabFocus);
        // mainwindow的设置按钮触发
        connect(titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton"), &DIconButton::pressed, this, [this]() {
            showPlugin(PLUGIN_TYPE_NONE);
            // 判断是否超过最大数量限制
            QList<QAction *> actionList = m_menu->actions();
            for (auto item : actionList) {
                if (item->text() == tr("New window")) {
                    // 菜单根据数量自动设置true or false
                    item->setEnabled(Service::instance()->isCountEnable());
                }
            }

        });
    } else {
        qDebug() << "can not found DTitlebarDWindowOptionButton in DTitlebar";
    }
}
/*******************************************************************************
 1. @函数:    initOptionMenu
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    option menu初始化
*******************************************************************************/
void MainWindow::initOptionMenu()
{
    titlebar()->setMenu(m_menu);
    /******** Modify by m000714 daizhengwen 2020-04-03: 新建窗口****************/
    // 防止创建新窗口的action被多次触发
    QTimer *createTimer = new QTimer(this);
    // 设置定时器,等待菜单消失,触发一次,防止多次被触发
    createTimer->setSingleShot(true);
    QAction *newWindowAction(new QAction(tr("New window"), this));
    connect(createTimer, &QTimer::timeout, this, [ = ]() {
        // 创建新的窗口
        qDebug() << "menu click new window";

        TermWidgetPage *tabPage = currentPage();
        TermWidget *term = tabPage->currentTerminal();
        QString currWorkingDir = term->workingDirectory();
        emit newWindowRequest(currWorkingDir);
    });
    connect(newWindowAction, &QAction::triggered, this, [ = ]() {
        // 等待菜单消失  此处影响菜单创建窗口的性能
        createTimer->start(50);
    });
    m_menu->addAction(newWindowAction);
    /********************* Modify by m000714 daizhengwen End ************************/
    for (auto &plugin : m_plugins) {
        QAction *pluginMenu = plugin->titlebarMenu(this);
        // 取消Encoding插件的菜单展示
        if (plugin->getPluginName() == PLUGIN_TYPE_ENCODING) {
            continue;
        }
        if (pluginMenu) {
            m_menu->addAction(pluginMenu);
        }
    }

    QAction *settingAction(new QAction(tr("Settings"), this));
    m_menu->addAction(settingAction);
    m_menu->addSeparator();
    connect(settingAction, &QAction::triggered, Service::instance(), [ = ] {
        Service::instance()->showSettingDialog(this);
    });
}

/*******************************************************************************
 1. @函数:    initPlugins
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类初始化插件
*******************************************************************************/
void MainWindow::initPlugins()
{
    // Todo: real plugin loader and plugin support.
    // ThemePanelPlugin *testPlugin = new ThemePanelPlugin(this);
    // testPlugin->initPlugin(this);
    EncodePanelPlugin *encodePlugin = new EncodePanelPlugin(this);
    encodePlugin->initPlugin(this);

    customCommandPlugin = new CustomCommandPlugin(this);
    customCommandPlugin->initPlugin(this);

    remoteManagPlugin = new RemoteManagementPlugin(this);
    remoteManagPlugin->initPlugin(this);

    m_plugins.append(encodePlugin);
    m_plugins.append(customCommandPlugin);
    m_plugins.append(remoteManagPlugin);
}

MainWindow::~MainWindow()
{
}

/*******************************************************************************
 1. @函数:    setDefaultLocation
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    设置默认位置，最大化，全屏还原使用
*******************************************************************************/
void MainWindow::setDefaultLocation()
{
    resize(WINDOW_DEFAULT_SIZE);
    singleFlagMove();
}
/*******************************************************************************
 1. @函数:    singleFlagMove
 2. @作者:    ut000439 王培利
 3. @日期:    2020-06-02
 4. @说明:    首个普通窗口，需要居中显示
*******************************************************************************/
void MainWindow::singleFlagMove()
{
    if (m_properties[SingleFlag].toBool()) {
        Dtk::Widget::moveToCenter(this);
        qDebug() << "SingleFlag move" ;
    }
}

/*******************************************************************************
 1. @函数:    isTabVisited
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类判断标签是否访问了
*******************************************************************************/
bool MainWindow::isTabVisited(int tabSessionId)
{
    return m_tabVisitMap.value(tabSessionId);
}

/*******************************************************************************
 1. @函数:    isTabChangeColor
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类判断是否是标签更改颜色
*******************************************************************************/
bool MainWindow::isTabChangeColor(int tabSessionId)
{
    return m_tabChangeColorMap.value(tabSessionId);
}

/*******************************************************************************
 1. @函数:    addTab
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类增加标签
*******************************************************************************/
void MainWindow::addTab(TermProperties properties, bool activeTab)
{
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    /***add by ut001121 zhangmeng 修复BUG#24452 点击“+”按钮新建工作区，自定义命令/编码/远程管理插件未消失***/
    showPlugin(PLUGIN_TYPE_NONE);

    if (WindowsManager::instance()->widgetCount() >= MAXWIDGETCOUNT) {
        // 没有雷神，且是雷神让通过，不然不让通过
        if (!(nullptr == WindowsManager::instance()->getQuakeWindow() && m_isQuakeWindow)) {
            // 非雷神窗口不得超过MAXWIDGETCOUNT
            qDebug() << "addTab failed, can't create number more than " << MAXWIDGETCOUNT;
            return;
        }
    }

    TermWidgetPage *termPage = new TermWidgetPage(properties, this);
    setNewTermPage(termPage, activeTab);

    // pageID存在 tab中，所以page增删改操作都要由tab发起。
    int index = m_tabbar->addTab(termPage->identifier(), m_tabbar->tabText(m_tabbar->currentIndex()));
    qDebug() << "addTab index" << index;
    if (activeTab) {
        m_tabbar->setCurrentIndex(index);
        m_tabbar->removeNeedChangeTextColor(index);
    }

    TermWidget *term = termPage->currentTerminal();
    m_tabbar->saveSessionIdWithTabIndex(term->getSessionId(), index);
    m_tabbar->saveSessionIdWithTabId(term->getSessionId(), termPage->identifier());
    connect(termPage, &TermWidgetPage::termTitleChanged, this, &MainWindow::onTermTitleChanged);
    connect(termPage, &TermWidgetPage::tabTitleChanged, this, &MainWindow::onTabTitleChanged);
    connect(termPage, &TermWidgetPage::lastTermClosed, this, [this](const QString & identifier) {
        closeTab(identifier);
    });

    connect(this, &MainWindow::showPluginChanged, termPage, [ = ](const QString name) {
        // 判断是否是当前页，不是当前页不用管
        if (this->currentPage() == termPage) {
            // 显示和隐藏搜索框
            if (PLUGIN_TYPE_SEARCHBAR == name) {
                // 显示搜索框
                termPage->showSearchBar(SearchBar_Show);
            } else if (PLUGIN_TYPE_NONE == name) {
                // 隐藏搜索框，焦点落回终端
                termPage->showSearchBar(SearchBar_FocusOut);
            } else {
                // 隐藏搜索框
                termPage->showSearchBar(SearchBar_Hide);
            }
        }
    });

    // 快速隐藏搜索框
    connect(this, &MainWindow::quakeHidePlugin, termPage, [ = ]() {
        // 判断是否是当前页，不是当前页不用管
        if (this->currentPage() == termPage) {
            // 隐藏搜索框
            termPage->showSearchBar(SearchBar_Hide);
        }
    });

    connect(termPage->currentTerminal(), &TermWidget::termIsIdle, this, &MainWindow::onTermIsIdle);
    qint64 endTime = QDateTime::currentMSecsSinceEpoch();
    QString strNewTabTime = GRAB_POINT + LOGO_TYPE + CREATE_NEW_TAB_TIME + QString::number(endTime - startTime);
    qInfo() << qPrintable(strNewTabTime);
}

/*******************************************************************************
 1. @函数:    hasRunningProcesses
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    是否有正在执行的进程
*******************************************************************************/
bool MainWindow::hasRunningProcesses()
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        /******** Modify by nt001000 renfeixiang 2020-05-28:修改 判断当前tab中是否有其它分屏正在执行 bug#28910 Begin***************/
        //没有校验当前tab中是否有其它正在执行的分屏
        //TermWidget *term = tabPage->currentTerminal();
        if (tabPage->runningTerminalCount() != 0) {
            /******** Modify by nt001000 renfeixiang 2020-05-28:修改 判断当前tab中是否有其它分屏正在执行 End***************/
            qDebug() << "here are processes running in this terminal tab... " << tabPage->identifier() << endl;
            return true;
        } else {
            qDebug() << "no processes running in this terminal tab... " << tabPage->identifier() << endl;
        }
    }

    return false;
}
/*******************************************************************************
 1. @函数:    closeTab
 2. @作者:    n014361 王培利
 3. @日期:    2020-05-07
 4. @说明:    一个tab只提示一次, 检测后需要重入，hasCheck=true
             或者不需要提示，hasCheck=true即可
*******************************************************************************/
void MainWindow::closeTab(const QString &identifier, bool hasConfirmed)
{
    /***add by ut001121 zhangmeng 20200508 修复BUG#24457 点击标签栏“x”按钮，右键菜单关闭工作区，关闭其它工作区，自定义命令/编码/远程管理插件未消失***/
    showPlugin(PLUGIN_TYPE_NONE);

    TermWidgetPage *tabPage = getPageByIdentifier(identifier);
    if (tabPage == nullptr) {
        return;
    }
    // 关闭前必须要切换过去先
    if (m_tabbar->currentIndex() != m_tabbar->getIndexByIdentifier(identifier)) {
        m_tabbar->setCurrentIndex(m_tabbar->getIndexByIdentifier(identifier));
    }
    // 默认每个窗口关闭都提示一次．
    if (!hasConfirmed && tabPage->runningTerminalCount() != 0) {
        showExitConfirmDialog(Utils::CloseType_Tab, tabPage->runningTerminalCount(), this);
        return;
    }
    qDebug() << "Tab closed" << identifier;
    int currSessionId = tabPage->currentTerminal()->getSessionId();
    m_tabVisitMap.remove(currSessionId);
    m_tabChangeColorMap.remove(currSessionId);
    m_tabbar->removeTab(identifier);
    m_termStackWidget->removeWidget(tabPage);
    tabPage->deleteLater();

    /******** Add by ut001000 renfeixiang 2020-08-07:关闭tab时改变大小，bug#41436***************/
    updateMinHeight();

    if (m_tabbar->count() != 0) {
        updateTabStatus();
        focusCurrentPage();
        return;
    }
    qDebug() << "mainwindow close";
    close();
}

/*******************************************************************************
 1. @函数:    updateTabStatus
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    更新标签页状态
*******************************************************************************/
void MainWindow::updateTabStatus()
{
    for (int i = 0; i < m_tabbar->count(); i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        TermWidget *term = tabPage->currentTerminal();
        bool bIdle = !(term->hasRunningProcess());
        int currSessionId = term->getSessionId();

        if (bIdle) {
            if (isTabVisited(currSessionId)) {
                m_tabVisitMap.insert(currSessionId, false);
                m_tabChangeColorMap.insert(currSessionId, false);
                m_tabbar->removeNeedChangeTextColor(i);
            } else if (isTabChangeColor(currSessionId)) {
                m_tabbar->setChangeTextColor(i);
            } else {
                m_tabbar->removeNeedChangeTextColor(i);
            }
        } else {
            m_tabbar->removeNeedChangeTextColor(i);
        }
    }
}

/*******************************************************************************
 1. @函数:    getCurrTabTitle
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    获取当前便签的标题
*******************************************************************************/
QString MainWindow::getCurrTabTitle()
{
    return m_tabbar->tabText(m_tabbar->currentIndex());
}

/*******************************************************************************
 1. @函数:    isFocusOnList
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-31
 4. @说明:    焦点是否在列表上
*******************************************************************************/
bool MainWindow::isFocusOnList()
{
    bool isFocus = true;
    DIconButton *button = m_tabbar->findChild<DIconButton *>("AddButton");
    // 判断是否找到
    if (button != nullptr) {
        // 判断按钮是否有焦点
        if (button->hasFocus()) {
            isFocus = false;
            qDebug() << "focus on AddButton";
        }
    } else {
        qDebug() << "can not found AddButton in DIconButton";
    }

    QList<QString> buttonList = {"DTitlebarDWindowOptionButton", "DTitlebarDWindowMinButton", "DTitlebarDWindowMaxButton", "DTitlebarDWindowCloseButton"};
    for (const QString &objectName : buttonList) {
        DIconButton *button = titlebar()->findChild<DIconButton *>(objectName);
        // 判断是否找到
        if (button != nullptr) {
            // 判断按钮是否有焦点
            if (button->hasFocus()) {
                isFocus = false;
                qDebug() << "focus on " << objectName;
            }
        } else {
            qDebug() << "can not found objectName in DIconButton";
        }
    }
    qDebug() << "focus on list : " << isFocus;
    return isFocus;

}
/*******************************************************************************
 1. @函数:    closeOtherTab
 2. @作者:    n014361 王培利
 3. @日期:    2020-05-07
 4. @说明:    关闭其它窗口
*******************************************************************************/
void MainWindow::closeOtherTab(const QString &identifier, bool hasConfirmed)
{
    int runningCount = 0;
    QList<QString> closeTabIdList;
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage && tabPage->identifier() != identifier) {
            closeTabIdList.append(tabPage->identifier());
            runningCount += tabPage->runningTerminalCount();
        }
    }

    if (!hasConfirmed && runningCount != 0) {
        // 全部关闭时，仅提示一次．
        showExitConfirmDialog(Utils::CloseType_OtherTab, runningCount, this);
        return;
    }

    // 关闭其它窗口，需要检测
    for (QString &id : closeTabIdList) {
        closeTab(id, true);
        qDebug() << " close" << id;
    }

    //如果是不关闭当前页的，最后回到当前页来．
    m_tabbar->setCurrentIndex(m_tabbar->getIndexByIdentifier(identifier));

    return;
}
/*******************************************************************************
 1. @函数:     closeOtherTab
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     关闭所有标签页功能
*******************************************************************************/
void MainWindow::closeAllTab()
{
    QList<QString> closeTabIdList;
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        closeTabIdList.append(tabPage->identifier());
    }

    // 全部关闭时，不再检测了，
    for (QString &id : closeTabIdList) {
        closeTab(id, true);
        qDebug() << " close" << id;
    }

    return;
}
/*******************************************************************************
 1. @函数:    showExitConfirmDialog
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    关闭窗口、终端时，显示确认提示框
*******************************************************************************/
void MainWindow::showExitConfirmDialog(Utils::CloseType type, int count, QWidget *parent)
{
    // count < 1 不提示
    if (count < 1) {
        return;
    }
    QString title;
    QString txt;
    Utils::CloseType temtype = type;
    // 关闭窗口的时候，如果只有一个tab,提示的内容要为终端．
    if (Utils::CloseType_Window == type && m_tabbar->count() == 1) {
        temtype = Utils::CloseType_Terminal;
    }
    Utils::getExitDialogText(temtype, title, txt, count);

    /******** Modify by ut000439 wangpeili 2020-07-27:  bug 39643  ****************/
    // 关闭确认窗口前确认焦点位置是否在close button上，并且发起了关闭窗口
    bool closeBtnHasfocus = false;
    DIconButton *closeBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
    if (closeBtn != nullptr && closeBtn->hasFocus() && type == Utils::CloseType_Window) {
        closeBtnHasfocus = true;
        qDebug() << "before close window, focus widget is close button. ";
    }

    if (closeBtn == nullptr) {
        qDebug() << "can not found DTitlebarDWindowCloseButton in DTitlebar";
    }
    /********************* Modify by n014361 wangpeili End ************************/


    // 有弹窗显示
    Service::instance()->setIsDialogShow(this, true);

    DDialog *dlg = new DDialog(title, txt, parent);
    dlg->setIcon(QIcon::fromTheme("deepin-terminal"));
    dlg->addButton(QString(tr("Cancel")), false, DDialog::ButtonNormal);
    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close Begin***************/
    dlg->addButton(QString(tr("Close")), true, DDialog::ButtonWarning);
    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close End***************/
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
    dlg->setProperty("type", type);

    /******** Modify by ut000439 wangpeili 2020-07-27:  bug 39643  ****************/
    if (closeBtnHasfocus) {
        dlg->setProperty("focusCloseBtn",  true);
    } else {
        dlg->setProperty("focusCloseBtn",  false);
    }
    /********************* Modify by n014361 wangpeili End ************************/

    /******** Modify by ut001000 renfeixiang 2020-06-03:修改 将dlg的槽函数修改为OnHandleCloseType，处理全部在OnHandleCloseType函数中 Begin***************/
    connect(dlg, &DDialog::finished, this, [this](int result) {
        OnHandleCloseType(result, Utils::CloseType(qobject_cast<DDialog *>(sender())->property("type").toInt()));
        /******** Modify by ut000439 wangpeili 2020-07-27:  bug 39643  ****************/
        if (result != 1 && qobject_cast<DDialog *>(sender())->property("focusCloseBtn").toBool())        {
            DIconButton *closeBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
            if (closeBtn != nullptr) {
                closeBtn->setFocus();
                qDebug() << "close button setFocus";
            } else {
                qDebug() << "can not found DTitlebarDWindowCloseButton in DTitlebar";
            }
        }
        /********************* Modify by n014361 wangpeili End ************************/
    });

    return ;
}
/*******************************************************************************
 1. @函数:    focusPage
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    焦点切换到某个PAGE页
*******************************************************************************/
void MainWindow::focusPage(const QString &identifier)
{
    TermWidgetPage *tabPage = getPageByIdentifier(identifier);
    if (tabPage) {
        m_termStackWidget->setCurrentWidget(tabPage);
        tabPage->focusCurrentTerm();
        return;
    }
    qDebug() << "focusTab nullptr identifier" << identifier;
}

/*******************************************************************************
 1. @函数:    focusCurrentPage
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类聚焦到当前页面
*******************************************************************************/
void MainWindow::focusCurrentPage()
{
    focusPage(m_tabbar->identifier(m_tabbar->currentIndex()));
}

/*******************************************************************************
 1. @函数:    currentPage
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类获取当前页面
*******************************************************************************/
TermWidgetPage *MainWindow::currentPage()
{
    return qobject_cast<TermWidgetPage *>(m_termStackWidget->currentWidget());
}

/*******************************************************************************
 1. @函数:    getPageByIdentifier
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类通过标识符获取页面
*******************************************************************************/
TermWidgetPage *MainWindow::getPageByIdentifier(const QString &identifier)
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage && tabPage->identifier() == identifier) {
            return tabPage;
        }
    }
    qDebug() << "getPageByIdentifier nullptr identifier" << identifier;
    return nullptr;
}

/*******************************************************************************
 1. @函数:    forAllTabPage
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类对所有标签页执行func
*******************************************************************************/
void MainWindow::forAllTabPage(const std::function<void(TermWidgetPage *)> &func)
{
    for (int i = 0, count = m_termStackWidget->count(); i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage) {
            func(tabPage);
        }
    }
}

/*******************************************************************************
 1. @函数:    onTermIsIdle
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类终端是否闲置响应函数
*******************************************************************************/
void MainWindow::onTermIsIdle(int currSessionId, bool bIdle)
{
    int tabIndex = m_tabbar->queryIndexBySessionId(currSessionId);

    //如果标签被点过，移除标签颜色
    if (isTabVisited(currSessionId) && bIdle) {
        m_tabVisitMap.insert(currSessionId, false);
        m_tabChangeColorMap.insert(currSessionId, false);
        m_tabbar->removeNeedChangeTextColor(tabIndex);
        return;
    }

    if (bIdle) {
        //空闲状态如果标签被标记变色，则改变标签颜色
        if (m_tabbar->isNeedChangeTextColor(tabIndex)) {
            m_tabChangeColorMap.insert(currSessionId, true);
            m_tabbar->setChangeTextColor(tabIndex);
        }
    } else {

        //如果当前标签是活动标签，移除变色请求
        int activeTabIndex = m_tabbar->currentIndex();
        if (activeTabIndex == tabIndex) {
            m_tabVisitMap.insert(currSessionId, false);
            m_tabChangeColorMap.insert(currSessionId, false);
            m_tabbar->removeNeedChangeTextColor(tabIndex);
            return;
        }

        //标记变色，发起请求，稍后等空闲状态变色
        m_tabChangeColorMap.insert(currSessionId, false);
        DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
        DPalette pa = appHelper->standardPalette(appHelper->themeType());
        m_tabbar->setNeedChangeTextColor(tabIndex, pa.color(DPalette::Highlight));
    }
}

/*******************************************************************************
 1. @函数:    resizeEvent
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类调整大小事件
*******************************************************************************/
void MainWindow::resizeEvent(QResizeEvent *event)
{
    // 保存窗口位置
    saveWindowSize();
    // 通知隐藏插件
    hidePlugin();

    DMainWindow::resizeEvent(event);
}

/*******************************************************************************
 1. @函数:    closeEvent
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类关闭事件
*******************************************************************************/
void MainWindow::closeEvent(QCloseEvent *event)
{
    saveWindowSize();

    // 注销和关机时不需要确认了
    if (qApp->isSavingSession()) {
        DMainWindow::closeEvent(event);
    }
    // 一页一页退出，当全部退出以后，mainwindow自然关闭．
    event->ignore();

    int runningCount = 0;
    QList<QString> closeTabIdList;
    int count = m_termStackWidget->count();
    for (int i = 0 ; i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        closeTabIdList.append(tabPage->identifier());
        runningCount += tabPage->runningTerminalCount();
    }

    if (!m_hasConfirmedClose && runningCount != 0) {
        // 如果不能马上关闭，并且还在没有最小化．
        if (runningCount != 0  && isMinimized()) {
            qDebug() << "isMinimized........... " << endl;
            setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        }

        // 全部关闭时，仅提示一次．
        showExitConfirmDialog(Utils::CloseType_Window, runningCount, this);
        return;
    }
    closeAllTab();


    if (m_tabbar->count() == 0) {
        DMainWindow::closeEvent(event);
        emit mainwindowClosed(this);
    }
}

/*******************************************************************************
 1. @函数:    isQuakeMode
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类判断是否是雷神窗口
*******************************************************************************/
bool MainWindow::isQuakeMode()
{
    return  m_isQuakeWindow;
}

/*******************************************************************************
 1. @函数:    onTermTitleChanged
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类终端标题变化响应函数
*******************************************************************************/
void MainWindow::onTermTitleChanged(QString title)
{
    TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(sender());
    const bool customName = tabPage->property("TAB_CUSTOM_NAME_PROPERTY").toBool();
    if (!customName) {
        m_tabbar->setTabText(tabPage->identifier(), title);
    }

    // 判定第一次修改标题的时候，认为终端已经创建成功
    // 以此认为第一次打开终端窗口结束，记录时间
    if (!hasCreateFirstTermialComplete) {
        Service::instance()->setMemoryEnable(true);
        firstTerminalComplete();
        hasCreateFirstTermialComplete = true;
    }
}

/*******************************************************************************
 1. @函数:    onTabTitleChanged
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类标签标题变化响应函数
*******************************************************************************/
void MainWindow::onTabTitleChanged(QString title)
{
    TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(sender());
    TermWidget *term = tabPage->currentTerminal();
    term->setProperty("currTabTitle", QVariant::fromValue(title));
    m_tabbar->setTabText(tabPage->identifier(), title);
}

/*******************************************************************************
 1. @函数:    getConfigWindowState
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类获取配置窗口状态
*******************************************************************************/
QString MainWindow::getConfigWindowState()
{
    QString windowState =
        Settings::instance()->settings->option("advanced.window.use_on_starting")->value().toString();

    // 启动参数配置的状态值优先于 内部配置的状态值
    if (m_properties.contains(StartWindowState)) {
        QString state = m_properties[StartWindowState].toString();
        qDebug() << "use line state set:" << state;
        if (state == "maximum") {
            windowState = "window_maximum";
        } else if (state == "splitscreen") {
            windowState = "split_screen";
        } else if (state == "normal") {
            windowState = "window_normal";
        } else if (state == "fullscreen") {
            windowState = state;
        } else {
            qDebug() << "error line state set:" << state << "ignore it!";
        }
    }
    return  windowState;
}

/*******************************************************************************
 1. @函数:    halfScreenSize
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    获取半屏大小（高度-1,如果不-1,最大后无法正常还原）
*******************************************************************************/
QSize MainWindow::halfScreenSize()
{
    QDesktopWidget w;
    int x = w.availableGeometry().width() / 2;
    int y = w.availableGeometry().height() - 1;
    QSize size(x, y);
    //qDebug() << "halfScreenSize:" << size;
    return size;
}

/*******************************************************************************
 1. @函数:    getWinInfoConfigPath
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类获取窗口信息配置路径
*******************************************************************************/
QString MainWindow::getWinInfoConfigPath()
{
    QDir winInfoPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!winInfoPath.exists()) {
        winInfoPath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString winInfoFilePath(winInfoPath.filePath("wininfo-config.conf"));
    return winInfoFilePath;
}

/*******************************************************************************
 1. @函数:    initWindowPosition
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类初始化窗口位置
*******************************************************************************/
void MainWindow::initWindowPosition(MainWindow *mainwindow)
{
    int m_WindowNumber = executeCMD(cmd);
    if (m_WindowNumber == 1) {
        mainwindow->move((QApplication::desktop()->width() - width()) / 2, (QApplication::desktop()->height() - height()) / 2);
    }
}

/*******************************************************************************
 1. @函数:    initShortcuts
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类初始化快捷键
*******************************************************************************/
void MainWindow::initShortcuts()
{
    ShortcutManager::instance()->initConnect(this);
    connect(ShortcutManager::instance(), &ShortcutManager::addCustomCommandSignal, this, &MainWindow::addCustomCommandSlot);
    connect(ShortcutManager::instance(), &ShortcutManager::removeCustomCommandSignal, this, &MainWindow::removeCustomCommandSlot);

    /******** Modify by n014361 wangpeili 2020-01-10: 增加设置的各种快捷键修改关联***********×****/
    // new_workspace
    connect(createNewShotcut("shortcuts.workspace.new_workspace", false), &QShortcut::activated, this, [this]() {
        this->addTab(currentPage()->createCurrentTerminalProperties(), true);
    });

    // close_workspace
    connect(createNewShotcut("shortcuts.workspace.close_workspace"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            closeTab(page->identifier());
        }
    });

    // Close_other_workspaces
    connect(createNewShotcut("shortcuts.workspace.close_other_workspace"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            closeOtherTab(page->identifier());
        }
    });

    // previous_workspace
    connect(createNewShotcut("shortcuts.workspace.previous_workspace"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            int index = m_tabbar->currentIndex();
            index -= 1;
            if (index < 0) {
                index = m_tabbar->count() - 1;
            }
            m_tabbar->setCurrentIndex(index);
        }
    });

    // next_workspace
    connect(createNewShotcut("shortcuts.workspace.next_workspace"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            int index = m_tabbar->currentIndex();
            index += 1;
            if (index == m_tabbar->count()) {
                index = 0;
            }
            m_tabbar->setCurrentIndex(index);
        }
    });

    // horionzal_split
    connect(createNewShotcut("shortcuts.workspace.horionzal_split", false), &QShortcut::activated, this, [this]() {
        // 判读数量是否允许分屏
        if (Service::instance()->isCountEnable()) {
            TermWidgetPage *page = currentPage();
            if (page) {
                if (page->currentTerminal()) {
                    int layer = page->currentTerminal()->getTermLayer();
                    Qt::Orientation orientation = static_cast<DSplitter *>(page->currentTerminal()->parentWidget())->orientation();
                    if (layer == 1 || (layer == 2 && orientation == Qt::Horizontal)) {
                        page->split(Qt::Horizontal);
                        return ;
                    }
                }
            }
        }
        qDebug() << "can't split vertical  again";
    });

    // vertical_split
    connect(createNewShotcut("shortcuts.workspace.vertical_split", false), &QShortcut::activated, this, [this]() {
        // 判读数量是否允许分屏
        if (Service::instance()->isCountEnable()) {
            TermWidgetPage *page = currentPage();
            if (page) {
                if (page->currentTerminal()) {
                    int layer = page->currentTerminal()->getTermLayer();
                    Qt::Orientation orientation = static_cast<DSplitter *>(page->currentTerminal()->parentWidget())->orientation();
                    if (layer == 1 || (layer == 2 && orientation == Qt::Vertical)) {
                        page->split(Qt::Vertical);
                        return ;
                    }
                }
            }
        }
        qDebug() << "can't split vertical  again";
    });

    // select_upper_window
    connect(createNewShotcut("shortcuts.workspace.select_upper_window"), &QShortcut::activated, this, [this]() {
        qDebug() << "Alt+k";
        TermWidgetPage *page = currentPage();
        if (page) {
            page->focusNavigation(Qt::TopEdge);
        }
    });
    // select_lower_window
    connect(createNewShotcut("shortcuts.workspace.select_lower_window"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->focusNavigation(Qt::BottomEdge);
        }
    });
    // select_left_window
    connect(createNewShotcut("shortcuts.workspace.select_left_window"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->focusNavigation(Qt::LeftEdge);
        }
    });
    // select_right_window
    connect(createNewShotcut("shortcuts.workspace.select_right_window"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->focusNavigation(Qt::RightEdge);
            // QMouseEvent e(QEvent::MouseButtonPress, ) QApplication::sendEvent(focusWidget(), &keyPress);
        }
    });

    // close_window
    connect(createNewShotcut("shortcuts.workspace.close_window"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            qDebug() << "CloseWindow";
            page->closeSplit(page->currentTerminal());
        }
    });

    // close_other_windows
    connect(createNewShotcut("shortcuts.workspace.close_other_windows"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->closeOtherTerminal();
        }
    });

    // copy
    connect(createNewShotcut("shortcuts.terminal.copy"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->copyClipboard();
        }
    });

    // paste
    connect(createNewShotcut("shortcuts.terminal.paste"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->pasteClipboard();
        }
    });

    // search
    connect(createNewShotcut("shortcuts.terminal.find"), &QShortcut::activated, this, [this]() {
        showPlugin(PLUGIN_TYPE_SEARCHBAR);
    });

    // zoom_in
    connect(createNewShotcut("shortcuts.terminal.zoom_in"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->zoomInCurrentTierminal();
        }
        /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 Begin***************/
        setWindowMinHeightForFont();
        /******** Add by ut001000 renfeixiang 2020-08-07:zoom_in时改变大小，bug#41436***************/
        updateMinHeight();
        /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 End***************/
    });

    // zoom_out
    connect(createNewShotcut("shortcuts.terminal.zoom_out"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->zoomOutCurrentTerminal();
        }
        /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 Begin***************/
        setWindowMinHeightForFont();
        /******** Add by ut001000 renfeixiang 2020-08-07:zoom_out时改变大小，bug#41436***************/
        updateMinHeight();
        /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 End***************/
    });

    // default_size
    connect(createNewShotcut("shortcuts.terminal.default_size"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            page->setFontSize(Settings::instance()->fontSize());
        }
        /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 Begin***************/
        setWindowMinHeightForFont();
        /******** Add by ut001000 renfeixiang 2020-08-07:default_size时改变大小，bug#41436***************/
        updateMinHeight();
        /******** Add by nt001000 renfeixiang 2020-05-20:增加 雷神窗口根据字体大小设置最小高度函数 End***************/
    });

    // select_all
    connect(createNewShotcut("shortcuts.terminal.select_all"), &QShortcut::activated, this, [this]() {
        TermWidgetPage *page = currentPage();
        if (page) {
            qDebug() << "selectAll";
            page->selectAll();
        }
    });

    // switch_fullscreen
    connect(createNewShotcut("shortcuts.advanced.switch_fullscreen"), &QShortcut::activated, this, [this]() {
        switchFullscreen();
    });

    // rename_tab
    connect(createNewShotcut("shortcuts.advanced.rename_title"), &QShortcut::activated, this, [this]() {
        showPlugin(PLUGIN_TYPE_NONE);
        TermWidgetPage *page = currentPage();
        if (page) {
            QString currTabTitle = m_tabbar->tabText(m_tabbar->currentIndex());
            page->showRenameTitleDialog(currTabTitle);
        }
    });

    // display_shortcuts
    connect(createNewShotcut("shortcuts.advanced.display_shortcuts"), &QShortcut::activated, this, [this]() {
        qDebug() << "displayShortcuts";
        displayShortcuts();
    });

    // custom_command
    connect(createNewShotcut("shortcuts.advanced.custom_command"), &QShortcut::activated, this, [this]() {
        if (m_CurrentShowPlugin == PLUGIN_TYPE_CUSTOMCOMMAND) {
            showPlugin(PLUGIN_TYPE_NONE);
        } else {
            showPlugin(PLUGIN_TYPE_CUSTOMCOMMAND);
        }
    });

    // remote_management
    connect(createNewShotcut("shortcuts.advanced.remote_management"), &QShortcut::activated, this, [this]() {
        if (m_CurrentShowPlugin == PLUGIN_TYPE_REMOTEMANAGEMENT) {
            showPlugin(PLUGIN_TYPE_NONE);
        } else {
            showPlugin(PLUGIN_TYPE_REMOTEMANAGEMENT);
        }
    });
    /********************* Modify by n014361 wangpeili End ************************/

    /******** Modify by ut000439 wangpeili 2020-07-17:              ****************/
    QShortcut *shortcutFoucusOut = new QShortcut(QKeySequence(QKEYSEQUENCE_FOCUSOUT_TIMINAL), this);
    connect(shortcutFoucusOut, &QShortcut::activated, this, [this]() {
        qDebug() << "focusout timinal is activated!" << QKEYSEQUENCE_FOCUSOUT_TIMINAL;
        DIconButton *addButton = m_tabbar->findChild<DIconButton *>("AddButton");
        if (addButton != nullptr) {
            addButton->setFocus();
        } else {
            qDebug() << "can not found AddButton in DIconButton";
        }
    });
    /********************* Modify by n014361 wangpeili End ************************/
    /******** Modify by ut000439 wangpeili 2020-07-27: bug 39494   ****************/
    QShortcut *shortcutBuiltinPaste = new QShortcut(QKeySequence(QKEYSEQUENCE_PASTE_BUILTIN), this);
    connect(shortcutBuiltinPaste, &QShortcut::activated, this, [this]() {
        qDebug() << "built in paste shortcut is activated!" << QKEYSEQUENCE_PASTE_BUILTIN;
        TermWidgetPage *page = currentPage();
        if (page) {
            page->pasteClipboard();
        }
    });

    /******** Modify by ut001000 renfeixiang 2020-08-28:修改 bug 44477***************/
    QShortcut *shortcutBuiltinCopy = new QShortcut(QKeySequence(QKEYSEQUENCE_COPY_BUILTIN), this);
    connect(shortcutBuiltinCopy, &QShortcut::activated, this, [this]() {
        qDebug() << "built in copy shortcut is activated!" << QKEYSEQUENCE_COPY_BUILTIN;
        TermWidgetPage *page = currentPage();
        if (page) {
            page->copyClipboard();
        }
    });
    /********************* Modify by n014361 wangpeili End ************************/

    for (int i = 1; i <= 9; i++) {
        QString shortCutStr = QString("ctrl+shift+%1").arg(i);
        //qDebug() << shortCutStr;
        QShortcut *switchTabSC = new QShortcut(QKeySequence(shortCutStr), this);
        connect(switchTabSC, &QShortcut::activated, this, [this, i]() {
            TermWidgetPage *page = currentPage();
            if (page) {
                if (9 == i && m_tabbar->count() > 9) {
                    m_tabbar->setCurrentIndex(m_tabbar->count() - 1);
                    return;
                }

                if (i - 1 >= m_tabbar->count()) {
                    qDebug() << "i - 1 > tabcount" << i - 1 << m_tabbar->count() << endl;
                    return;
                }

                qDebug() << "index" << i - 1 << endl;
                m_tabbar->setCurrentIndex(i - 1);
                return;
            }
            qDebug() << "currentPage nullptr ??";
        });
    }
}

/*******************************************************************************
 1. @函数:    initConnections
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类初始化连接
*******************************************************************************/
void MainWindow::initConnections()
{
    connect(this, &MainWindow::mainwindowClosed, WindowsManager::instance(), &WindowsManager::onMainwindowClosed);
    connect(Settings::instance(), &Settings::windowSettingChanged, this, &MainWindow::onWindowSettingChanged);
    connect(Settings::instance(), &Settings::shortcutSettingChanged, this, &MainWindow::onShortcutSettingChanged);
    connect(this, &MainWindow::newWindowRequest, this, &MainWindow::onCreateNewWindow);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, [ = ]() {
        //变成自动变色的图标以后，不需要来回变了。
        // applyTheme();
    });
    connect(qApp, &QGuiApplication::applicationStateChanged, this, &MainWindow::onApplicationStateChanged);
}
/*******************************************************************************
 1. @函数:    executeCMD
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    运行终端命令
*******************************************************************************/
int MainWindow::executeCMD(const char *cmd)
{
    char *result;
    char buf_ps[1024] = {0};
    FILE *ptr;

    ptr = popen(cmd, "r");
    result = fgets(buf_ps, 1024, ptr);

    QString qStr(result);
    int num = qStr.toInt() ;
    pclose(ptr);
    return  num;
}

/*******************************************************************************
 1. @函数:    showPlugin
 2. @作者:    n014361 王培利
 3. @日期:    2020-03-26
 4. @说明:   由mainwindow统一指令当前显示哪个插件
*******************************************************************************/
void MainWindow::showPlugin(const QString &name)
{
    bool bSetFocus = false;
    // 当焦点不在终端时，调用插件，并直接进入焦点
    if (qApp->focusWidget() != nullptr) {
        if (QString(qApp->focusWidget()->metaObject()->className()) != TERM_WIDGET_NAME) {
            bSetFocus = true;
        }
    }

    if (m_CurrentShowPlugin == name && m_CurrentShowPlugin == PLUGIN_TYPE_NONE) {
        // 目前没有列表显示，直接返回
        qDebug() << "no plugin show!";
        return;
    }

    m_CurrentShowPlugin = name;
    if (name != PLUGIN_TYPE_NONE) {
        qDebug() << "show Plugin" << name << bSetFocus;
    }

    emit showPluginChanged(name, bSetFocus);
}
/*******************************************************************************
 1. @函数:    hidePlugin
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    快速隐藏所有插件，resize专用的。
*******************************************************************************/
void MainWindow::hidePlugin()
{
    if (m_CurrentShowPlugin == PLUGIN_TYPE_NONE) {
        return;
    }
    qDebug() << "hide Plugin" << m_CurrentShowPlugin;
    m_CurrentShowPlugin = PLUGIN_TYPE_NONE;
    emit quakeHidePlugin();
}

/*******************************************************************************
 1. @函数:    selectedText
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类选择文本
*******************************************************************************/
QString MainWindow::selectedText(bool preserveLineBreaks)
{
    TermWidgetPage *page = currentPage();
    if (page) {
        if (page->currentTerminal()) {
            return page->currentTerminal()->selectedText(preserveLineBreaks);
        }
    }
    qDebug() << "not point terminal??";
    return  "";
}

/*******************************************************************************
 1. @函数:    onCreateNewWindow
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类创建新窗口响应函数
*******************************************************************************/
void MainWindow::onCreateNewWindow(QString workingDir)
{
    Q_UNUSED(workingDir);
    // 调用一个新的进程，开启终端
    QProcess process;
    process.startDetached(QCoreApplication::applicationFilePath());
}

/*******************************************************************************
 1. @函数:    eventFilter
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类事件过滤器
*******************************************************************************/
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        // 当前的终端进行操作
        TermWidget *term = currentPage()->currentTerminal();
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        // 键盘事件 按下回车
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            // 判断是否进行下载输入完要下载的文件后按下回车且当前窗口是激活窗口
            if (term->enterSzCommand() && term->isActiveWindow()) {
                //--added by qinyaning(nyq) to slove Unable to download file from server, time: 2020.4.13 18:21--//
                // 先用sz获取输入的文件
                pressEnterKey("\nsz \"${files[@]}\"");
                //-------------------------------------
                // 执行下载操作
                executeDownloadFile();
                // 下载完成,将是否进行下载的标志位设置为false
                term->setEnterSzCommand(false);
            }
        }
        if ((keyEvent->modifiers() == Qt::ControlModifier) && (keyEvent->key() == Qt::Key_C || keyEvent->key() == Qt::Key_D)) {
            // 当点击ctrl+c或者ctrl+d时判断,当前窗口是否正在等待下载输入
            if (term->enterSzCommand()) {
                // 若是则将是否进行下载的标志位设置为false
                term->setEnterSzCommand(false);
            }
        }
        /******** Modify by ut000610 daizhengwen 2020-09-07:焦点在推出全屏上,点击Enter或Space Begin***************/
        // fix#bug 46680
        if ((watched->metaObject()->className() == QStringLiteral("Dtk::Widget::DWindowQuitFullButton"))
                && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Space)) {
            // 退出全屏
            switchFullscreen(false);
            return true;
        }
        /********************* Modify by ut000610 daizhengwen End ************************/
    }

    //if (watched == this) {
    if (event->type() == QEvent::KeyPress) {
        //将事件转化为键盘事件
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        //按下Tab键执行焦点切换事件，如果个别控件需要特殊处理TAB的话，在这里加代码
#if 0
        if (key_event->key() == Qt::Key_Tab) {
            bool realm_edit_focus = false;  //= realm_line_edit->hasFocus();
            bool user_edit_focus = false;   // user_line_edit->hasFocus();
            focusNextChild();
            if (realm_edit_focus) {
                // user_line_edit->setFocus();
            } else {
                // password_line_edit->setFocus();
            }

            return true;
        }
#endif
        // 全局按下ESC键返回终端，过滤掉个别情况
        if (key_event->key() == Qt::Key_Escape) {
            QString filterReason; // 过滤原因
            do {
                // 不干扰其它对话框使用ESC（一般框会遮盖enable false)
                if (!this->isEnabled()) {
                    filterReason = "mainwidow is disable";
                    break;
                }
                // 未激活不生效
                if (!this->isActiveWindow()) {
                    filterReason = "mainwidow is not active";
                    break;
                }
                // 不干扰终端使用ESC
                if (QString(qApp->focusWidget()->metaObject()->className()) == TERM_WIDGET_NAME) {
                    filterReason = "focusWidget is terminnal";
                    /***add by ut001121 zhangmeng 20200922 修复BUG48960***/
                    showPlugin(PLUGIN_TYPE_NONE);
                    break;
                }
                // 如果有菜单出现的时候，ESC无效
                for (QWidget *top : qApp->topLevelWidgets()) {
                    if (QString(top->metaObject()->className()) == "QMenu" && top->isVisible()) {
                        filterReason = QString(top->metaObject()->className()) + " is display";
                        break;
                    }
                }
            } while (false);

            if (!filterReason.isEmpty()) {
                qDebug() << "Esc is not effect, reason:" << filterReason;
            } else {
                focusCurrentPage();
                showPlugin(PLUGIN_TYPE_NONE);
                return  true;
            }
        }
    }

    return DMainWindow::eventFilter(watched, event);
}


/*******************************************************************************
 1. @函数:    onSettingValueChanged
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-19
 4. @说明:    参数修改统一接口
*******************************************************************************/
void MainWindow::onWindowSettingChanged(const QString &keyName)
// void MainWindow::onSettingValueChanged(const int &keyIndex, const QVariant &value)
{
    if (keyName == "advanced.window.blurred_background") {
        setEnableBlurWindow(Settings::instance()->backgroundBlur());
        return;
    }

    // use_on_starting重启生效
    if (keyName == "advanced.window.use_on_starting") {
        QString state = Settings::instance()->settings->option("advanced.window.use_on_starting")->value().toString();
        if ("window_normal" == state) {
            m_IfUseLastSize = true;
            /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height Begin***************/
            m_winInfoConfig->setValue(CONFIG_WINDOW_WIDTH, WINDOW_DEFAULT_WIDTH);
            m_winInfoConfig->setValue(CONFIG_WINDOW_HEIGHT, WINDOW_DEFAULT_HEIGHT);
            /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height End***************/
            qDebug() << "change value change value change value";
        } else {
            m_IfUseLastSize = false;
        }
        qDebug() << "settingValue[" << keyName << "] changed to " << state
                 << ", auto effective when next start!";
        return;
    }
    // auto_hide_raytheon_window在使用中自动读取生效
    if ((keyName == "advanced.window.auto_hide_raytheon_window") || (keyName == "advanced.window.use_on_starting")) {
        qDebug() << "settingValue[" << keyName << "] changed to " << Settings::instance()->OutputtingScroll()
                 << ", auto effective when happen";
        /***mod begin by ut001121 zhangmeng 20200528 修复BUG28920***/
        //onApplicationStateChanged(QApplication::applicationState());
        onAppFocusChangeForQuake();
        /***mod end by ut001121***/
        return;
    }

    qDebug() << "settingValue[" << keyName << "] changed is not effective";
}

/*******************************************************************************
 1. @函数:    onShortcutSettingChanged
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类快捷键设置变化响应函数
*******************************************************************************/
void MainWindow::onShortcutSettingChanged(const QString &keyName)
{
    qDebug() << "Shortcut[" << keyName << "] changed";
    if (m_BuiltInShortcut.contains(keyName)) {
        QString value = Settings::instance()->settings->option(keyName)->value().toString();
        m_BuiltInShortcut[keyName]->setKey(QKeySequence(value));
        return;
    }

    qDebug() << "Shortcut[" << keyName << "] changed is unknown!";
}

/*******************************************************************************
 1. @函数:    setNewTermPage
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类设置新终端页面
*******************************************************************************/
void MainWindow::setNewTermPage(TermWidgetPage *termPage, bool activePage)
{
    m_termStackWidget->addWidget(termPage);
    if (activePage) {
        m_termStackWidget->setCurrentWidget(termPage);
    }
}

/*******************************************************************************
 1. @函数:    createNewWorkspace
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类创建新工作区域
*******************************************************************************/
void MainWindow::createNewWorkspace()
{
    addTab(currentPage()->createCurrentTerminalProperties(), true);
}
/*******************************************************************************
 1. @函数:    applyTheme
 2. @作者:    n014361 王培利
 3. @日期:    2020-03-09
 4. @说明:    非DTK控件手动匹配系统主题的修改
*******************************************************************************/
void MainWindow::applyTheme()
{
    return;
}

/*******************************************************************************
 1. @函数:    getPluginByName
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类通过名字获取面板
*******************************************************************************/
MainWindowPluginInterface *MainWindow::getPluginByName(const QString &name)
{
    for (int i = 0; i < m_plugins.count(); i++) {
        if (m_plugins.at(i)->getPluginName() == name) {
            return m_plugins.at(i);
        }
    }
    return nullptr;
}

/*******************************************************************************
 1. @函数:void MainWindow::displayShortcuts()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     显示快捷键列表信息
*******************************************************************************/
void MainWindow::displayShortcuts()
{
    QPoint pos = calculateShortcutsPreviewPoint();

    QJsonArray jsonGroups;
    createJsonGroup("terminal", jsonGroups);
    createJsonGroup("workspace", jsonGroups);
    createJsonGroup("advanced", jsonGroups);
    QJsonObject shortcutObj;
    shortcutObj.insert("shortcut", jsonGroups);

    QJsonDocument doc(shortcutObj);

    QStringList shortcutString;
    QByteArray array = doc.toJson();
    QString param1 = "-j=" + QString(array.data());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    QProcess *shortcutViewProcess = new QProcess();
    shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProcess, SIGNAL(finished(int)), shortcutViewProcess, SLOT(deleteLater()));
}

/*******************************************************************************
 1. @函数: void MainWindow::createJsonGroup(const QString &keyCategory, QJsonArray &jsonGroups)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     创建JsonGroup组
*******************************************************************************/
void MainWindow::createJsonGroup(const QString &keyCategory, QJsonArray &jsonGroups)
{
    qDebug() << keyCategory;

    QString strGroupName = "";
    if (keyCategory == "workspace") {
        strGroupName =  QObject::tr("Workspace");
    } else if (keyCategory == "terminal") {
        strGroupName =  QObject::tr("Terminal");
    } else if (keyCategory == "advanced") {
        strGroupName =  QObject::tr("Others");
    } else {
        return;
    }
    QString groupname = "shortcuts." + keyCategory;

    QJsonArray JsonArry;
    for (auto &opt :
            Settings::instance()->settings->group(groupname)->options()) {  // Settings::instance()->settings->keys())
        QJsonObject jsonItem;
        QString name = QObject::tr(opt->name().toUtf8().data());
        /***del begin by ut001121 zhangmeng 修复BUG#23269 快捷键菜单“切换全屏”显示与文案不一致***/
        /*if (opt->name() == "Fullscreen")
            name = tr("Toggle fullscreen");
        */
        /***del end by ut001121 zhangmeng***/
        jsonItem.insert("name", name);
        jsonItem.insert("value", opt->value().toString());
        JsonArry.append(jsonItem);
    }

    if (keyCategory == "workspace") {
        QJsonObject jsonItem;
        jsonItem.insert("name", tr("Select workspace"));
        jsonItem.insert("value", "Ctrl+Shift+1~9");
        JsonArry.append(jsonItem);
    }

    /************************ Add by sunchengxi 2020-06-08:json重新排序，快捷键显示顺序调整 Begin************************/
    //default-config.json 文件增加的跟此处相关字段，此处相应添加，保证显示。
    //用三个条件分开，清晰，方便后续调整扩展维护，代码稍微多点。
    if (keyCategory == "terminal") {
        QStringList strList;
        strList << QObject::tr("Copy")  << QObject::tr("Paste") << QObject::tr("Find") << QObject::tr("Zoom in") << QObject::tr("Zoom out") << QObject::tr("Default size") << QObject::tr("Select all");
        QJsonArray newJsonArry;
        for (int i = 0; i < strList.size(); i++) {
            for (int j = 0; j < JsonArry.size(); j++) {
                QJsonObject jsonItem = JsonArry[j].toObject();
                if (jsonItem.value("name") == strList[i]) {
                    newJsonArry.append(jsonItem);
                    break;
                }
            }
        }
        JsonArry = newJsonArry;
    }
    if (keyCategory == "workspace") {
        QStringList strList;
        strList << QObject::tr("New workspace") << QObject::tr("Close workspace") << QObject::tr("Close other workspaces") << QObject::tr("Previous workspace") << QObject::tr("Next workspace")
                << QObject::tr("Select workspace") << QObject::tr("Vertical split") << QObject::tr("Horizontal split") << QObject::tr("Select upper window") << QObject::tr("Select lower window")
                << QObject::tr("Select left window") << QObject::tr("Select right window") << QObject::tr("Close window") << QObject::tr("Close other windows");
        QJsonArray newJsonArry;
        for (int i = 0; i < strList.size(); i++) {
            for (int j = 0; j < JsonArry.size(); j++) {
                QJsonObject jsonItem = JsonArry[j].toObject();
                if (jsonItem.value("name") == strList[i]) {
                    newJsonArry.append(jsonItem);
                    break;
                }
            }
        }
        JsonArry = newJsonArry;
    }
    if (keyCategory == "advanced") {
        QStringList strList;
        strList << QObject::tr("Fullscreen") << QObject::tr("Rename title") << QObject::tr("Display shortcuts") << QObject::tr("Custom commands") << QObject::tr("Remote management");
        QJsonArray newJsonArry;
        for (int i = 0; i < strList.size(); i++) {
            for (int j = 0; j < JsonArry.size(); j++) {
                QJsonObject jsonItem = JsonArry[j].toObject();
                if (jsonItem.value("name") == strList[i]) {
                    newJsonArry.append(jsonItem);
                    break;
                }
            }
        }
        JsonArry = newJsonArry;
    }
    /************************ Add by sunchengxi 2020-06-08:json重新排序，快捷键显示顺序调整   End************************/

    QJsonObject JsonGroup;
    JsonGroup.insert("groupName", strGroupName);
    JsonGroup.insert("groupItems", JsonArry);
    jsonGroups.append(JsonGroup);
}
/*******************************************************************************
 1. @函数:    createNewShotcut
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-20
 4. @说明:    创建内置快捷键管理,
             AutoRepeat 是否可以连续触发快捷键, 默认可以
*******************************************************************************/
QShortcut *MainWindow::createNewShotcut(const QString &key, bool AutoRepeat)
{
    QString value = Settings::instance()->settings->option(key)->value().toString();
    QShortcut *shortcut = new QShortcut(QKeySequence(value), this);
    m_BuiltInShortcut[key] = shortcut;
    shortcut->setAutoRepeat(AutoRepeat);
    // qDebug() << "createNewShotcut" << key << value;
    return shortcut;
}



/*******************************************************************************
 1. @函数:    remoteUploadFile
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    (远程）上传文件到服务器端
*******************************************************************************/
void MainWindow::remoteUploadFile()
{
    QStringList fileName = Utils::showFilesSelectDialog(this);
    if (!fileName.isEmpty()) {
        pressCtrlAt();
        sleep(100);
        QString strTxt = "sz ";
        for (QString &str : fileName) {
            strTxt += str + " ";
        }
        currentPage()->sendTextToCurrentTerm(strTxt);
        currentPage()->sendTextToCurrentTerm("\n");
    }
}

/**
 * Download file from remote server
 */
/*******************************************************************************
 1. @函数:    remoteDownloadFile
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    基类从远程服务下载文件
*******************************************************************************/
void MainWindow::remoteDownloadFile()
{
    TermWidget *term = currentPage()->currentTerminal();
    downloadFilePath = Utils::showDirDialog(this);

    if (!downloadFilePath.isNull() && !downloadFilePath.isEmpty()) {
        //QString strTxt = "read -e -a files -p \"" + tr("Type path to download file") + ": \"; sz \"${files[@]}\"\n";
        //currentTab()->sendTextToCurrentTerm(strTxt);
        //--added by qinyaning(nyq) to slove Unable to download file from server, time: 2020.4.13 18:21--//
        QString strTxt = QString("read -e -a files -p \"%1: \"").arg(tr("Type path to download file"));
        pressEnterKey(strTxt);
        currentPage()->sendTextToCurrentTerm("\n");
        //-------------------
        term->setEnterSzCommand(true);
        //sleep(100);//
    }
}

/*******************************************************************************
 1. @函数:    onApplicationStateChanged
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-28
 4. @说明:    当雷神窗口处于非激动状态自动隐藏
*******************************************************************************/
void MainWindow::onApplicationStateChanged(Qt::ApplicationState state)
{
    Q_UNUSED(state)
    return;
}

/*******************************************************************************
 1. @函数:    addCustomCommandSlot
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-08-11
 4. @说明:    基类增加自定义命令响应函数
*******************************************************************************/
void MainWindow::addCustomCommandSlot(QAction *newAction)
{
    qDebug() << " MainWindow::addCustomCommandSlot";

    QAction *action = newAction;
    addAction(action);

    connect(action, &QAction::triggered, this, [this, action]() {
        if (!this->isActiveWindow()) {
            return ;
        }
        QString command = action->data().toString();
        if (!command.endsWith('\n')) {
            command.append('\n');
        }
        currentPage()->sendTextToCurrentTerm(command);
    });

}

/*******************************************************************************
 1. @函数:    removeCustomCommandSlot
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-08-11
 4. @说明:    基类删除自定义命令响应函数
*******************************************************************************/
void MainWindow::removeCustomCommandSlot(QAction *newAction)
{
    qDebug() << " MainWindow::removeCustomCommandSlot";
    removeAction(newAction);
}

/*******************************************************************************
 1. @函数:    OnHandleCloseType
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-06-03
 4. @说明:    处理CloseType的关闭窗口
*******************************************************************************/
void MainWindow::OnHandleCloseType(int result, Utils::CloseType type)
{
    qDebug() << "OnHandleCloseType type is" << type;
    // 弹窗隐藏或消失
    Service::instance()->setIsDialogShow(this, false);
    if (result != 1) {
        qDebug() << "user cancle close";
        return;
    }

    TermWidgetPage *page = currentPage();
    if (page == nullptr) {
        qDebug() << "null pointer of currentPage ???";
        return;
    }

    //以下所有接口二次重入
    switch (type) {
    case Utils::CloseType_Window:
        m_hasConfirmedClose = true;
        close();
        break;
    case Utils::CloseType_Tab:
        closeTab(page->identifier(), true);
        break;
    case Utils::CloseType_OtherTab:
        closeOtherTab(page->identifier(), true);
        break;
    case Utils::CloseType_Terminal://Terminal相关的关闭弹框操作
        page->closeSplit(page->currentTerminal(), true);
        break;
    case Utils::CloseType_OtherTerminals:
        page->closeOtherTerminal(true);
        break;
    }
}

/*******************************************************************************
 1. @函数:    keyPressEvent
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    按键响应事件，是空函数
*******************************************************************************/
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

/*******************************************************************************
 1. @函数:    executeDownloadFile
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    sz命令之后，等待输入文件信息，并下载文件
*******************************************************************************/
void MainWindow::executeDownloadFile()
{
    //--modified by qinyaning(nyq) to slove Unable to download file from server, time: 2020.4.13 18:21--//
    // 前一条命令执行
    currentPage()->sendTextToCurrentTerm("\r\n");
    // 执行玩等待1s直到文件传输结束
    sleep(1000);
    // ctrl+@ 进入当前机器
    pressCtrlAt();
    // cd 到当前机器的制定文件夹下
    sleep(100);
    QString strCd = "cd " + downloadFilePath;
    currentPage()->sendTextToCurrentTerm(strCd);
    // 将文件下载下来
    QString strRz = "\r\nrz -be";
    // 执行下载命令
    currentPage()->sendTextToCurrentTerm(strRz);
    // 将下载路径置空
    downloadFilePath = "";
    //-------------------------------------------
}

/*******************************************************************************
 1. @函数:    pressCtrlAt
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类按下Ctrl+@函数
*******************************************************************************/
void MainWindow::pressCtrlAt()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_At, Qt::ControlModifier);
    QApplication::sendEvent(focusWidget(), &keyPress);
}

/*******************************************************************************
 1. @函数:    pressCtrlU
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类按下Ctrl+U函数
*******************************************************************************/
void MainWindow::pressCtrlU()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_U, Qt::ControlModifier);
    QApplication::sendEvent(focusWidget(), &keyPress);
}

/*******************************************************************************
 1. @函数:    pressCtrlC
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类按下Ctrl+C函数
*******************************************************************************/
void MainWindow::pressCtrlC()
{
    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier);
    QApplication::sendEvent(focusWidget(), &keyPress);
}

/*******************************************************************************
 1. @函数:    sleep
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类睡眠函数
*******************************************************************************/
void MainWindow::sleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

//--added by qinyaning(nyq) to slove Unable to download file from server, time: 2020.4.13 18:21--//
/*******************************************************************************
 1. @函数:    pressEnterKey
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类按下Enter键函数
*******************************************************************************/
void MainWindow::pressEnterKey(const QString &text)
{
    QKeyEvent event(QEvent::KeyPress, 0, Qt::NoModifier, text);
    QApplication::sendEvent(focusWidget(), &event);  // expose as a big fat keypress event
}
/*******************************************************************************
 1. @函数:    createWindowComplete
 2. @作者:    ut000439 王培利
 3. @日期:    2020-08-08
 4. @说明:    mainwindow创建结束记录
*******************************************************************************/
void MainWindow::createWindowComplete()
{
    m_WindowCompleteTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    // qDebug()<<"MainWindow Create complete, Time use"<<m_WindowCompleteTime - m_CreateWindowTime;
}
/*******************************************************************************
 1. @函数:    firstTerminalComplete
 2. @作者:    ut000439 王培利
 3. @日期:    2020-08-08
 4. @说明:    首个终端创建成功结束, 统计各个时间
*******************************************************************************/
void MainWindow::firstTerminalComplete()
{
    m_FirstTerminalCompleteTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qDebug() << "app create all complete," << "MainWindowID = " << m_MainWindowID << ",all time use" << m_FirstTerminalCompleteTime - m_ReferedAppStartTime << "ms";
    qDebug() << "before entry use" << m_CreateWindowTime - m_ReferedAppStartTime << "ms";
    // 创建mainwidow时间，这个时候terminal并没有创建好，不能代表什么。
    //qDebug() << "create mainwidow use " << m_WindowCompleteTime - m_CreateWindowTime << "ms";
    qDebug() << "cretae first Terminal use" << m_FirstTerminalCompleteTime - m_CreateWindowTime << "ms";
}

/*******************************************************************************
 1. @函数:    createNewMainWindowTime
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-09-21
 4. @说明:    创建新窗口需要的时间
*******************************************************************************/
qint64 MainWindow::createNewMainWindowTime()
{
    // 当前时间减去创建时间
    return (QDateTime::currentDateTime().toMSecsSinceEpoch() - m_ReferedAppStartTime);
}

/*******************************************************************************
 1. @函数:    getDesktopIndex
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    基类获取桌面索引
*******************************************************************************/
int MainWindow::getDesktopIndex() const
{
    return m_desktopIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 普通终端窗口
*/
NormalWindow::NormalWindow(TermProperties properties, QWidget *parent): MainWindow(properties, parent)
{
    Q_ASSERT(m_isQuakeWindow == false);
    setObjectName("NormalWindow");
    initUI();
    initConnections();
    initShortcuts();
    createWindowComplete();
}

NormalWindow::~NormalWindow()
{

}

/*******************************************************************************
 1. @函数:    initTitleBar
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    普通终端窗口初始化标题栏
*******************************************************************************/
void NormalWindow::initTitleBar()
{
    // titleba在普通模式和雷神模型不一样的功能
    m_titleBar = new TitleBar(this, false);
    m_titleBar->setObjectName("NormalWindowTitleBar");//Add by ut001000 renfeixiang 2020-08-14
    m_titleBar->setTabBar(m_tabbar);

    titlebar()->setCustomWidget(m_titleBar);
    // titlebar()->setAutoHideOnFullscreen(true);
    titlebar()->setTitle("");

    //设置titlebar焦点策略为不抢占焦点策略，防止点击titlebar后终端失去输入焦点
    titlebar()->setFocusPolicy(Qt::NoFocus);
    initOptionButton();
    initOptionMenu();

    //fix bug 17566 正常窗口下，新建和关闭窗口菜单栏会高亮
    //handleTitleBarMenuFocusPolicy();

    /******** Modify by ut000439 wangpeili 2020-07-22:  SP3.1 DTK TAB控件 ****************/
    // 清理titlebar、titlebar所有控件不可获取焦点
    Utils::clearChildrenFocus(titlebar());
    Utils::clearChildrenFocus(m_tabbar);
    // 重新设置可见控件焦点
    DIconButton *addButton = m_tabbar->findChild<DIconButton *>("AddButton");
    if (addButton != nullptr) {
        addButton->setFocusPolicy(Qt::TabFocus);
    } else {
        qDebug() << "can not found AddButton in DIconButton";
    }
    DIconButton *optionBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton");
    if (optionBtn != nullptr) {
        optionBtn->setFocusPolicy(Qt::TabFocus);
    } else {
        qDebug() << "can not found DTitlebarDWindowOptionButton in DTitlebar";
    }
    QWidget *quitFullscreenBtn = titlebar()->findChild<QWidget *>("DTitlebarDWindowQuitFullscreenButton");
    if (quitFullscreenBtn != nullptr) {
        quitFullscreenBtn->setFocusPolicy(Qt::TabFocus);
    } else {
        qDebug() << "can not found DTitlebarDWindowQuitFullscreenButton in DTitlebar";
    }
    DIconButton *minBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMinButton");
    if (minBtn != nullptr) {
        minBtn->setFocusPolicy(Qt::TabFocus);
    } else {
        qDebug() << "can not found DTitlebarDWindowMinButton in DTitlebar";
    }
    DIconButton *maxBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMaxButton");
    if (maxBtn != nullptr) {
        maxBtn->setFocusPolicy(Qt::TabFocus);
    } else {
        qDebug() << "can not found DTitlebarDWindowMaxButton in DTitlebar";
    }
    DIconButton *closeBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
    if (closeBtn != nullptr) {
        closeBtn->setFocusPolicy(Qt::TabFocus);
    } else {
        qDebug() << "can not found DTitlebarDWindowCloseButton in DTitlebar";
    }
    if (addButton != nullptr && optionBtn != nullptr && quitFullscreenBtn != nullptr && minBtn != nullptr && maxBtn != nullptr && closeBtn != nullptr) {
        QWidget::setTabOrder(addButton, optionBtn);
        QWidget::setTabOrder(optionBtn, quitFullscreenBtn);
        QWidget::setTabOrder(quitFullscreenBtn, minBtn);
        QWidget::setTabOrder(minBtn, maxBtn);
        QWidget::setTabOrder(maxBtn, closeBtn);
    }

    /********************* Modify by n014361 wangpeili End ************************/
}

/*******************************************************************************
 1. @函数:    initWindowAttribute copy from setNormalWindow
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    标准模式的窗口设置
*******************************************************************************/
void NormalWindow::initWindowAttribute()
{
    // init window state.
    QString windowState = getConfigWindowState();
    if (windowState == "window_maximum") {
        show();
        setDefaultLocation();
        showMaximized();
    } else if (windowState == "fullscreen") {
        setDefaultLocation();
        switchFullscreen(true);
    } else if (windowState == "split_screen") {
        setWindowRadius(0);
        resize(halfScreenSize());
    } else {
        m_IfUseLastSize = true;
        int saveWidth = m_winInfoConfig->value(CONFIG_WINDOW_WIDTH).toInt();
        int saveHeight = m_winInfoConfig->value(CONFIG_WINDOW_HEIGHT).toInt();
        qDebug() << "load window_width: " << saveWidth;
        qDebug() << "load window_height: " << saveHeight;
        // 如果配置文件没有数据
        if (saveWidth == 0 || saveHeight == 0) {
            saveWidth = WINDOW_DEFAULT_WIDTH;
            saveHeight = WINDOW_DEFAULT_HEIGHT;
        }
        resize(QSize(saveWidth, saveHeight));
        singleFlagMove();
    }
}

/*******************************************************************************
 1. @函数:    saveWindowSize
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    普通窗口保存窗口大小
*******************************************************************************/
void NormalWindow::saveWindowSize()
{
    // 过滤普通模式的特殊窗口
    if (!m_IfUseLastSize) {
        return;
    }

    // (真.假)半屏窗口大小时就不记录了
    /******** Modify by ut001000 renfeixiang 2020-07-03:fix# 36482 ***************/
    // 1.高度-1,如果不-1,最大后无法正常还原
    // 2.+ QSize(0, 1) 适应原始高度
    // 3.- QSize(0, 1) 适应关闭窗口特效， 半屏后无法还原
    if ((size() == halfScreenSize()) || (size() == (halfScreenSize() + QSize(0, 1))) || (size() == (halfScreenSize() - QSize(0, 1)))) {
        return;
    }

    if (windowState() == Qt::WindowNoState) {
        /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height Begin***************/
        // 记录最后一个正常窗口的大小
        m_winInfoConfig->setValue(CONFIG_WINDOW_WIDTH, width());
        m_winInfoConfig->setValue(CONFIG_WINDOW_HEIGHT, height());
        qDebug() << "save windows size:" << width() << height();
        /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换window_width，window_height End***************/
    }
}

/*******************************************************************************
 1. @函数:    switchFullscreen
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    普通窗口全屏切换
*******************************************************************************/
void NormalWindow::switchFullscreen(bool forceFullscreen)
{
    if (forceFullscreen || !window()->windowState().testFlag(Qt::WindowFullScreen)) {
        window()->setWindowState(windowState() | Qt::WindowFullScreen);
    } else {
        window()->setWindowState(windowState() & ~Qt::WindowFullScreen);
    }

    // 全屏和取消全屏后，都将焦点设置回终端
    focusCurrentPage();
}

/*******************************************************************************
 1. @函数:    calculateShortcutsPreviewPoint
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    普通窗口计算快捷方式预览点
*******************************************************************************/
QPoint NormalWindow::calculateShortcutsPreviewPoint()
{
    QRect rect = window()->geometry();
    return QPoint(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
}

/*******************************************************************************
 1. @函数:    onAppFocusChangeForQuake
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    普通窗口处理雷神窗口丢失焦点自动隐藏功能，普通窗口不用该函数
*******************************************************************************/
void NormalWindow::onAppFocusChangeForQuake()
{
    return;
}

/*******************************************************************************
 1. @函数:    changeEvent
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    普通窗口变化事件
*******************************************************************************/
void NormalWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 雷神终端窗口
*/
QuakeWindow::QuakeWindow(TermProperties properties, QWidget *parent): MainWindow(properties, parent),
    // 初始化雷神resize的定时器
    m_resizeTimer(new QTimer(this))
{
    Q_ASSERT(m_isQuakeWindow == true);
    setObjectName("QuakeWindow");
    initUI();
    initConnections();
    initShortcuts();
    createWindowComplete();
    // 设置雷神resize定时器属性
    m_resizeTimer->setSingleShot(true);
    // 绑定信号槽
    connect(m_resizeTimer, &QTimer::timeout, this, &QuakeWindow::onResizeWindow);
}

QuakeWindow::~QuakeWindow()
{
    // 析构resize定时器
    if (nullptr != m_resizeTimer) {
        delete m_resizeTimer;
    }
}

/*******************************************************************************
 1. @函数:    initTitleBar
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    雷神窗口初始化标题栏
*******************************************************************************/
void QuakeWindow::initTitleBar()
{
    // titleba在普通模式和雷神模型不一样的功能
    m_titleBar = new TitleBar(this, true);
    m_titleBar->setObjectName("QuakeWindowTitleBar");//Add by ut001000 renfeixiang 2020-08-14
    m_titleBar->setTabBar(m_tabbar);

    /** add by ut001121 zhangmeng 20200723 for sp3 keyboard interaction */
    //雷神终端设置系统标题栏为禁用状态,使其不获取焦点,不影响键盘交互操作.
    titlebar()->setEnabled(false);
    titlebar()->setFixedHeight(0);
    m_centralLayout->addWidget(m_titleBar);

    /** del by ut001121 zhangmeng 20200723 for sp3 keyboard interaction*/
#if 0
    //设置titlebar焦点策略为不抢占焦点策略
    titlebar()->setFocusPolicy(Qt::NoFocus);

    // titlebar所有控件不可获取焦点
    Utils::clearChildrenFocus(titlebar());
    Utils::clearChildrenFocus(m_tabbar);

    /******** Modify by ut000439 wangpeili 2020-07-22:  SP3.1 DTK TAB控件 ****************/
    DIconButton *addButton = m_tabbar->findChild<DIconButton *>("AddButton");
    if (addButton != nullptr) {
        addButton->setFocusPolicy(Qt::TabFocus);
    } else {
        qDebug() << "can not found AddButton in DIconButton";
    }

    // 雷神下其它控件一律没有焦点
    DIconButton *optionBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton");
    if (optionBtn != nullptr) {
        optionBtn->setFocusPolicy(Qt::NoFocus);
    } else {
        qDebug() << "can not found DTitlebarDWindowOptionButton in DTitlebar";
    }
    DIconButton *minBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMinButton");
    if (minBtn != nullptr) {
        minBtn->setFocusPolicy(Qt::NoFocus);
    } else {
        qDebug() << "can not found DTitlebarDWindowMinButton in DTitlebar";
    }
    DIconButton *maxBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMaxButton");
    if (maxBtn != nullptr) {
        maxBtn->setFocusPolicy(Qt::NoFocus);
    } else {
        qDebug() << "can not found DTitlebarDWindowMaxButton in DTitlebar";
    }
    DIconButton *closeBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
    if (closeBtn != nullptr) {
        closeBtn->setFocusPolicy(Qt::NoFocus);
    } else {
        qDebug() << "can not found DTitlebarDWindowCloseButton in DTitlebar";
    }

    /*QWidget::setTabOrder(addButton, optionBtn);
    QWidget::setTabOrder(optionBtn, minBtn);
    QWidget::setTabOrder(minBtn, maxBtn);
    QWidget::setTabOrder(maxBtn, closeBtn);*/
    /********************* Modify by n014361 wangpeili End ************************/
#endif
}

/*******************************************************************************
 1. @函数:    initWindowAttribute copy from setQuakeWindow
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-22
 4. @说明:    雷神窗口的特殊设置
*******************************************************************************/
void QuakeWindow::initWindowAttribute()
{
    /************************ Add by m000743 sunchengxi 2020-04-27:雷神窗口任务栏移动后位置异常问题 Begin************************/
    setWindowRadius(0);
    //QRect deskRect = QApplication::desktop()->availableGeometry();//获取可用桌面大小
    QDesktopWidget *desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry(); //获取设备屏幕大小
    Qt::WindowFlags windowFlags = this->windowFlags();
    setWindowFlags(windowFlags | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint /*| Qt::BypassWindowManagerHint*/ /*| Qt::Dialog*/);

    //add a line by ut001121 zhangmeng 2020-04-27雷神窗口禁用移动(修复bug#22975)
    setEnableSystemMove(false);//    setAttribute(Qt::WA_Disabled, true);

    /******** Modify by m000714 daizhengwen 2020-03-26: 窗口高度超过２／３****************/
    setMinimumSize(screenRect.size().width(), 60);
    setMaximumHeight(screenRect.size().height() * 2 / 3);
    /********************* Modify by m000714 daizhengwen End ************************/
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFixedWidth(QApplication::desktop()->availableGeometry().width());
    connect(desktopWidget, &QDesktopWidget::workAreaResized, this, [this]() {
        qDebug() << "workAreaResized" << QApplication::desktop()->availableGeometry();
        /******** Modify by nt001000 renfeixiang 2020-05-20:修改成只需要设置雷神窗口宽度,根据字体高度设置雷神最小高度 Begin***************/
        setMinimumWidth(QApplication::desktop()->availableGeometry().width());
        setWindowMinHeightForFont();
        /******** Add by ut001000 renfeixiang 2020-08-07:workAreaResized时改变大小，bug#41436***************/
        updateMinHeight();
        /******** Modify by nt001000 renfeixiang 2020-05-20:修改成只需要设置雷神窗口宽度,根据字体高度设置雷神最小高度 End***************/
        move(QApplication::desktop()->availableGeometry().x(), QApplication::desktop()->availableGeometry().y());
        qDebug() << "size" << size();
        setFixedWidth(QApplication::desktop()->availableGeometry().width());
        return ;
    });

    /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换quake_window_Height Begin***************/
    int saveHeight = m_winInfoConfig->value(CONFIG_QUAKE_WINDOW_HEIGHT).toInt();
    /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换quake_window_Height End***************/
    qDebug() << "quake_window_Height: " << saveHeight;
    qDebug() << "quake_window_Height: " << minimumSize();
    // 如果配置文件没有数据
    if (saveHeight == 0) {
        saveHeight = screenRect.size().height() / 3;
    }
    int saveWidth = screenRect.size().width();
    resize(QSize(saveWidth, saveHeight));
    // 记录雷神高度
    m_quakeWindowHeight = saveHeight;
    move(0, 0);
    /************************ Add by m000743 sunchengxi 2020-04-27:雷神窗口任务栏移动后位置异常问题 End  ************************/

    /******** Add by nt001000 renfeixiang 2020-05-20:增加setQuakeWindowMinHeight函数，设置雷神最小高度 Begin***************/
    setWindowMinHeightForFont();
    /******** Add by nt001000 renfeixiang 2020-05-20:增加setQuakeWindowMinHeight函数，设置雷神最小高度 End***************/
}

/*******************************************************************************
 1. @函数:    saveWindowSize
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    雷神窗口保存雷神窗口大小
*******************************************************************************/
void QuakeWindow::saveWindowSize()
{
    // 记录最后一个正常窗口的大小
    /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换quake_window_Height Begin***************/
    m_winInfoConfig->setValue(CONFIG_QUAKE_WINDOW_HEIGHT, height());
    /******** Modify by nt001000 renfeixiang 2020-05-25: 文件wininfo-config.conf中参数,使用定义更换quake_window_Height End***************/
    qDebug() << "save quake_window_Height:" << height();
}

/*******************************************************************************
 1. @函数:    switchFullscreen
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    全屏切换，雷神窗口不用
*******************************************************************************/
void QuakeWindow::switchFullscreen(bool forceFullscreen)
{
    Q_UNUSED(forceFullscreen)
    return;
}

/*******************************************************************************
 1. @函数:    calculateShortcutsPreviewPoint
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    雷神窗口计算快捷键预览位置
*******************************************************************************/
QPoint QuakeWindow::calculateShortcutsPreviewPoint()
{
    //--added by qinyaning(nyq) to solve the problem of can't show center--//
    QRect rect = QApplication::desktop()->availableGeometry();
    //---------------------------------------------------------------------//
    return QPoint(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
}

/*******************************************************************************
 1. @函数:    onAppFocusChangeForQuake
 2. @作者:    ut001121 张猛
 3. @日期:    2020-05-22
 4. @说明:    处理雷神窗口丢失焦点自动隐藏功能
*******************************************************************************/
void QuakeWindow::onAppFocusChangeForQuake()
{
    // 开关关闭，不处理
    if (!Settings::instance()->settings->option("advanced.window.auto_hide_raytheon_window")->value().toBool()) {
        return;
    }

    // 雷神窗口隐藏，不处理
    if (!isVisible()) {
        return;
    }

    // 雷神的普通对话框,不处理
    if (Service::instance()->getIsDialogShow()) {
        return;
    }
    // 雷神设置框显示,不处理
    if (Service::instance()->isSettingDialogVisible() && Service::instance()->getSettingOwner() == this) {
        return;
    }

    // 处于激活状态,不处理
    if (isActiveWindow()) {
        return;
    }

    hideQuakeWindow();
}

/*******************************************************************************
 1. @函数:    setQuakeWindowMinHeight
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-05-20
 4. @说明:    雷神窗口根据字体和字体大小设置最小高度
*******************************************************************************/
/******** Add by nt001000 renfeixiang 2020-05-20:增加雷神窗口根据字体和字体大小设置最小高度函数 Begin***************/
void QuakeWindow::setWindowMinHeightForFont()
{
    int height = 0;
    //根据内部term的最小高度设置雷神终端的最小高度, (m_MinHeight-50)/2是内部term的最小高度，50是雷神窗口的标题栏高度
    //add by ut001000 renfeixiang 2020-08-07
    height = (m_MinHeight - WIN_TITLE_BAR_HEIGHT) / 2 + 60;
    setMinimumHeight(height);
}
/******** Add by nt001000 renfeixiang 2020-05-20:增加雷神窗口根据字体和字体大小设置最小高度函数 End***************/

/*******************************************************************************
 1. @函数:    updateMinHeight
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-08-07
 4. @说明:    用于雷神窗口增加和减少横向分屏时，对雷神窗口的自小高进行修改，bug#41436
*******************************************************************************/
void QuakeWindow::updateMinHeight()
{
    qDebug() << "start updateMinHeight";
    bool hasHorizontalSplit = false;
    int count = m_termStackWidget->count();
    for (int i = 0; i < count; i++) {
        TermWidgetPage *tabPage = qobject_cast<TermWidgetPage *>(m_termStackWidget->widget(i));
        if (tabPage->hasHasHorizontalSplit()) {
            hasHorizontalSplit = true;
            break;
        }
    }
    if (hasHorizontalSplit) {
        if (minimumHeight() != m_MinHeight + 10) {
            qDebug() << "set has Vertical split MinHeight";
            setMinimumHeight(m_MinHeight + 10);
        }
    } else {
        if (minimumHeight() == m_MinHeight + 10) {
            qDebug() << "set not has Vertical split MinHeight";
            setWindowMinHeightForFont();
        }
    }
}

/*******************************************************************************
 1. @函数:    isShowOnCurrentDesktop
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-25
 4. @说明:    给出当前桌面雷神是否显示
*******************************************************************************/
bool QuakeWindow::isShowOnCurrentDesktop()
{
    return m_desktopMap[m_desktopIndex];
}

/*******************************************************************************
 1. @函数:    hideQuakeWindow
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-25
 4. @说明:    隐藏雷神终端
*******************************************************************************/
void QuakeWindow::hideQuakeWindow()
{
    // 隐藏雷神
    hide();
    // 记录雷神在当前窗口的状态
    m_desktopMap[m_desktopIndex] = false;
}

/*******************************************************************************
 1. @函数:    onResizeWindow
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-24
 4. @说明:    处理resize信号,延迟一段时间处理,避免处理过快,界面显示有延迟
*******************************************************************************/
void QuakeWindow::onResizeWindow()
{
    resize(width(), m_quakeWindowHeight);
}

/*******************************************************************************
 1. @函数:    changeEvent
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    雷神窗口变化事件
*******************************************************************************/
void QuakeWindow::changeEvent(QEvent *event)
{
    // 不是激活事件,不处理
    if (event->type() == QEvent::ActivationChange) {
        onAppFocusChangeForQuake();
    }

    return QMainWindow::changeEvent(event);
}

/*******************************************************************************
 1. @函数:    showEvent
 2. @作者:    ut001121 张猛
 3. @日期:    2020-05-28
 4. @说明:    窗口显示事件
*******************************************************************************/
void QuakeWindow::showEvent(QShowEvent *event)
{
    /***add begin by ut001121 zhangmeng 20200528 重新获取桌面索引 修复BUG29082***/
    m_desktopIndex = DBusManager::callKDECurrentDesktop();
    /***add end by ut001121***/
    // 记录当前桌面的index为显示状态
    m_desktopMap[m_desktopIndex] = true;

    /***add by ut001121 zhangmeng 20200606 切换窗口拉伸属性 修复BUG24430***/
    switchEnableResize();

    DMainWindow::showEvent(event);
}

/*******************************************************************************
 1. @函数:    event
 2. @作者:    ut001121 张猛
 3. @日期:    2020-06-06
 4. @说明:    窗口事件
*******************************************************************************/
bool QuakeWindow::event(QEvent *event)
{
    /***add begin by ut001121 zhangmeng 20200606 切换窗口拉伸属性 修复BUG24430***/
    if (event->type() == QEvent::HoverMove) {
        switchEnableResize();
    }
    /***add end by ut001121***/

    return MainWindow::event(event);
}

/*******************************************************************************
 1. @函数:    __FUNCTION__
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-21
 4. @说明:    事件过滤器,处理雷神相关的事件
*******************************************************************************/
bool QuakeWindow::eventFilter(QObject *watched, QEvent *event)
{
#if 0
    // 由于MainWindow是qApp注册的时间过滤器,所以这里需要判断
    // 只处理雷神的事件 QuakeWindowWindow是Qt内置用来管理QuakeWindow的Mouse事件的object
    if (watched->objectName() != "QuakeWindowWindow") {
        return MainWindow::eventFilter(watched, event);
    }

    // 鼠标点击事件
    if (event->type() == QEvent::MouseButtonPress) {
        // 鼠标点击窗口后激活窗口,因为不走窗管
        activateWindow();
        // 判断是否可以resize
        if (Quake_Prepare_Resize == m_resizeState) {
            // 若可以设置状态为resize,等待用户move鼠标
            m_resizeState = Quake_Resize;
        }
    }

    // 鼠标释放事件
    if (event->type() == QEvent::MouseButtonRelease) {
        // 松开鼠标 将状态值恢复默认
        m_resizeState = Quake_NoResize;
    }
    // 鼠标移动事件
    if (event->type() == QEvent::MouseMove) {
        // 将事件装换为鼠标事件
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event) ;
        int mousePosY = mouseEvent->screenPos().y();
        int margin = mousePosY - height();
        switch (m_resizeState) {
        // 若此时已经准备好resize
        case Quake_Resize:
            // 记录当前的雷神窗口高度
            m_quakeWindowHeight = height() + margin;
            // 延时发送,避免resize过于频繁,导致titlebar抖动
            m_resizeTimer->start(5);
            break;
        // 若没到边缘或者到边缘还没点击
        default:
            // 判断鼠标位置
            if (qAbs(margin) < QUAKE_EDGE) {
                // 若小于定义的边缘大小,将光标变化为指定形状
                setCursor(Qt::SizeVerCursor);
                m_resizeState = Quake_Prepare_Resize;
            } else {
                // 若大于,则还原
                setCursor(Qt::ArrowCursor);
                m_resizeState = Quake_NoResize;
            }
            break;
        }
    }
#endif
    return MainWindow::eventFilter(watched, event);

}

/*******************************************************************************
 1. @函数:    switchEnableResize
 2. @作者:    ut001121 张猛
 3. @日期:    2020-06-06
 4. @说明:    切换窗口拉伸属性
*******************************************************************************/
void QuakeWindow::switchEnableResize()
{
    // 如果(桌面光标Y坐标)>(雷神窗口Y坐标+雷神高度的1/2),则启用拉伸属性.否则禁用拉伸属性
    setEnableSystemResize(QCursor::pos().y() > pos().y() + height() / 2);
}
