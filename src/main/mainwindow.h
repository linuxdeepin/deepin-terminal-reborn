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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "termproperties.h"
#include "titlebar.h"
#include "termwidgetpage.h"
#include "remotemanagementplugn.h"
#include "utils.h"
#include "define.h"
#include "customthemesettingdialog.h"

// dtk
#include <DMainWindow>
#include <DWidgetUtil>
#include <DToolButton>
#include <DIconButton>
#include <DPushButton>

// qt
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QSettings>
#include <QMouseEvent>
#include <QTimer>
#include <QMenu>
#include <QDebug>
#include <QShortcut>

#include <functional>


DWIDGET_USE_NAMESPACE

class TabBar;
class TermWidgetPage;
class TermProperties;
class ShortcutManager;
class MainWindowPluginInterface;
class CustomCommandPlugin;


/*******************************************************************************
 1. @类名:    SwitchThemeMenu
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-10-28
 4. @说明:    主题菜单的快捷键项在鼠标离开悬浮时，触发主题还原
*******************************************************************************/
class SwitchThemeMenu : public QMenu
{
    Q_OBJECT
public:
    SwitchThemeMenu(const QString &title, QWidget *parent = nullptr);
    /**
     * @brief 捕获鼠标离开主题项事件
     * @author ut000125 sunchengxi
     */
    void leaveEvent(QEvent *) override;
    /**
     * @brief 主题菜单栏隐藏时触发
     * @author ut000125 sunchengxi
     */
    void hideEvent(QHideEvent *) override;
    /**
     * @brief 捕获鼠标进入主题项事件
     * @author ut000125 sunchengxi
     * @param event 鼠标进入主题项事件
     */
    void enterEvent(QEvent *event) override;
    /**
     * @brief 处理键盘主题项左键按下离开事件
     * @author ut000125 sunchengxi
     * @param event 键盘主题项左键按下离开事件
     */
    void keyPressEvent(QKeyEvent *event) override;

signals:
    //主题项在鼠标停靠离开时触发的信号
    void mainWindowCheckThemeItemSignal();
    //主题菜单隐藏时设置主题信号
    void menuHideSetThemeSignal();
public:
    //鼠标悬殊主题记录，防止频繁刷新，鼠标再次进入主题列表负责刷新预览
    QString hoveredThemeStr = "";
};


/*******************************************************************************
 1. @类名:    MainWindow
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    主窗口
             基类，不可直接生成对象，派生普测窗口和雷神窗口
             分发插件显隐信号
             控制TAB-PAGE增减
             捆绑全局快捷键
*******************************************************************************/
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(TermProperties properties, QWidget *parent = nullptr);
    ~MainWindow() override;

    /**
     * @brief 基类增加标签
     * @author ut000439 wangpeili
     * @param properties 标签页属性
     * @param activeTab
     */
    void addTab(TermProperties properties, bool activeTab = false);
    /**
     * @brief 拖拽成功后，将之前窗口的标签插入当前MainWindow窗口
     * @author ut000438 王亮
     * @param tabName 标签名称
     * @param activeTab 活动标签
     * @param isVirtualAdd 拖拽过程中存在一种标签预览模式，此时不需要真实添加
     * @param page 终端工作区
     * @param insertIndex 插入标签索引
     */
    void addTabWithTermPage(const QString &tabName, bool activeTab, bool isVirtualAdd, TermWidgetPage *page, int insertIndex = -1);
    /**
     * @brief 判断是否超出最大允许控件数量，如果超出则不允许新建标签
     * @author ut000438 王亮
     * @return
     */
    bool beginAddTab();
    /**
     * @brief 结束添加标签
     * @author ut000438 王亮
     * @param termPage 终端工作区
     * @param activeTab 是否拖拽过程中
     * @param index 索引
     * @param startTime 开始时间
     */
    void endAddTab(TermWidgetPage *termPage, bool activeTab, int index, qint64 startTime);

    /******** Modify by n014361 wangpeili 2020-01-07:  关闭其它标签页功能 ************/
    // 点击，右键，快捷键以及被调用．
    /**
     * @brief closeTab 关闭标签页
     * @author n014361 王培利
     * @param identifier 标签标识符
     * @param hasConfirmed 是否提示
     */
    void closeTab(const QString &identifier, bool hasConfirmed = false);
    /**
     * @brief 根据identifier获取对应的TermWidgetPage
     * @author ut000438 王亮
     * @param identifier 标识符
     * @return
     */
    TermWidgetPage *getTermPage(const QString &identifier);
    /**
     * @brief 根据identifier移除对应的TermPage
     * @author ut000438 王亮
     * @param identifier 标识符
     * @param isDelete
     */
    void removeTermWidgetPage(const QString &identifier, bool isDelete);
    // Tab右键或者快捷键
    /**
     * @brief 关闭其它窗口
     * @author n014361 王培利
     * @param identifier 标识符
     * @param hasConfirmed 是否确认
     */
    void closeOtherTab(const QString &identifier, bool hasConfirmed = false);
    /**
     * @brief 关闭所有标签页功能
     * @author n014361 王培利
     */
    void closeAllTab();
    /********************* Modify by n014361 wangpeili End ************************/
    /**
     * @brief showExitConfirmDialog 关闭窗口、终端时，显示确认提示框
     * @author ut000439 王培利
     * @param type 类型
     * @param count 数量
     * @param parent
     */
    void showExitConfirmDialog(Utils::CloseType type, int count = 1, QWidget *parent = nullptr);

    /************************ Mod by sunchengxi 2020-04-30:分屏修改标题异常问题 Begin************************/
    /**
     * @brief 获取当前标签的标题
     * @author ut000439 wangpeili
     * @return
     */
    QString getCurrTabTitle();
    /************************ Mod by sunchengxi 2020-04-30:分屏修改标题异常问题 End  ************************/
    /**
     * @brief 焦点是否在列表上
     * @author ut000610 戴正文
     * @return
     */
    bool isFocusOnList();

    /**
     * @brief 远程连接执行上传文件：1) 支持远程时拖拽文件自动上传2) 支持上传指定的文件
     * @author ut000610 戴正文
     * @param strFileNames 文件名称
     */
    void remoteUploadFile(QString strFileNames);
    /**
      * @brief 焦点切换到某个PAGE页
      * @author ut000439 王培利
      * @param identifier 标识符
      */
    void focusPage(const QString &identifier);
    /**
     * @brief 基类聚焦到当前页面
     * @author ut000439 wangpeili
     */
    void focusCurrentPage();
    /**
     * @brief 基类获取当前页面
     * @author ut000439 wangpeili
     * @return
     */
    TermWidgetPage *currentPage();
    void setCurrentPage(TermWidgetPage *page);
    /**
     * @brief 基类通过标识符获取页面
     * @author ut000439 wangpeili
     * @param identifier 标识符
     * @return
     */
    TermWidgetPage *getPageByIdentifier(const QString &identifier);

    /**
     * @brief 基类终端是否闲置响应函数(闲置即没有程序运行)
     * @author ut000439 wangpeili
     * @param tabIdentifier 标签标识符
     * @param bIdle
     */
    void onTermIsIdle(QString tabIdentifier, bool bIdle);

    /**
     * @brief sz命令之后，等待输入文件信息，并下载文件
     * @author ut000439 王培利
     */
    void executeDownloadFile();

    /**
     * @brief 基类按下Ctrl+@函数
     * @author ut000439 wangpeili
     */
    void pressCtrlAt();
    /**
     * @brief 基类按下Ctrl+U函数
     * @author ut000439 wangpeili
     */
    void pressCtrlU();
    /**
     * @brief 基类睡眠函数
     * @author ut000439 wangpeili
     * @param msec
     */
    void sleep(int msec);

    /**
     * @brief 由mainwindow统一指令当前显示哪个插件
     * @author n014361 王培利
     * @param name 插件名称
     */
    void showPlugin(const QString &name);
    /**
     * @brief 快速隐藏所有插件，resize专用的。
     * @author n014361 王培利
     */
    void hidePlugin();
    /**
     * @brief 基类创建新工作区域
     * @author ut000439 wangpeili
     */
    void createNewTab();
    /**
     * @brief 由mainwindow统一获取当前选择的文本。
     * @author ut000439 wangpeili
     * @return
     */
    QString selectedText();

    /**
     * @brief 创建新窗口需要的时间
     * @author ut000610 戴正文
     * @return
     */
    qint64 createNewMainWindowTime();

    /**
     * @brief 是否有正在执行的进程
     * @author ut000439 wangpeili
     * @return
     */
    bool hasRunningProcesses();
    /**
     * @brief 基类判断是否是雷神窗口
     * @author ut000439 wangpeili
     * @return
     */
    bool isQuakeMode();
    /**
     * @brief 用于标记当前tab是否为雷神窗口的tab
     * @author ut000438 王亮
     * @param isQuakeWindowTab 当前tab是否为雷神窗口的tab
     */
    void setIsQuakeWindowTab(bool isQuakeWindowTab);

    static constexpr const char *PLUGIN_TYPE_SEARCHBAR = "Search Bar";
    static constexpr const char *PLUGIN_TYPE_THEME = "Theme";
    static constexpr const char *PLUGIN_TYPE_CUSTOMCOMMAND = "Custom Command";
    static constexpr const char *PLUGIN_TYPE_REMOTEMANAGEMENT = "Remote Management";
    static constexpr const char *PLUGIN_TYPE_ENCODING = "Encoding";
    static constexpr const char *PLUGIN_TYPE_NONE = "None";
    /******** Add by nt001000 renfeixiang 2020-05-25:增加 定义 Begin***************/
    static constexpr const char *CONFIG_WINDOW_WIDTH = "window_width";
    static constexpr const char *CONFIG_WINDOW_HEIGHT = "window_height";
    //Modify by ut001000 renfeixiang 2020-06-18:修改quake_window_Height为quake_window_height bug#33779
    static constexpr const char *CONFIG_QUAKE_WINDOW_HEIGHT = "quake_window_height";
    /******** Add by nt001000 renfeixiang 2020-05-25:增加 定义 End***************/
    static constexpr const char *QKEYSEQUENCE_FOCUSOUT_TIMINAL = "Meta+Tab";
    static constexpr const char *QKEYSEQUENCE_PASTE_BUILTIN = "Shift+Ins";
    static constexpr const char *QKEYSEQUENCE_COPY_BUILTIN = "Ctrl+Ins";

    /******** Add by ut001000 renfeixiang 2020-08-07:将m_MinWidth和m_MinHeight设置成全局变量***************/
    // 窗口最小宽度
    static const int m_MinWidth;
    // 窗口最小高度
    static const int m_MinHeight;

    //主题菜单
    static constexpr const char *THEME                          = "Theme";
    //浅色主题
    static constexpr const char *THEME_SYSTEN_LIGHT             = "Light Theme";
    //深色主题
    static constexpr const char *THEME_SYSTEN_DARK              = "Dark Theme";
    //跟随系统
    static constexpr const char *THEME_SYSTEN                   = "System Theme";

    //浅色主题
    static constexpr const char *THEME_LIGHT                    = "Light";
    //深色主题
    static constexpr const char *THEME_DARK                     = "Dark";
    //无内置主题
    static constexpr const char *THEME_NO                       = "";
    //内置主题1
    static constexpr const char *THEME_ONE                      = "Theme1";
    static constexpr const char *THEME_ONE_NAME                 = "Elementary";
    //内置主题2
    static constexpr const char *THEME_TWO                      = "Theme2";
    static constexpr const char *THEME_TWO_NAME                 = "Empathy";
    //内置主题3
    static constexpr const char *THEME_THREE                    = "Theme3";
    static constexpr const char *THEME_THREE_NAME               = "Tomorrow night blue";
    //内置主题4
    static constexpr const char *THEME_FOUR                     = "Theme4";
    static constexpr const char *THEME_FOUR_NAME                = "Bim";
    //内置主题5
    static constexpr const char *THEME_FIVE                     = "Theme5";
    static constexpr const char *THEME_FIVE_NAME                = "Freya";
    //内置主题6
    static constexpr const char *THEME_SIX                      = "Theme6";
    static constexpr const char *THEME_SIX_NAME                 = "Hybrid";
    //内置主题7
    static constexpr const char *THEME_SEVEN                    = "Theme7";
    static constexpr const char *THEME_SEVEN_NAME               = "Ocean dark";
    //内置主题8
    static constexpr const char *THEME_EIGHT                    = "Theme8";
    static constexpr const char *THEME_EIGHT_NAME               = "Deepin";
    //内置主题9
    static constexpr const char *THEME_NINE                     = "Theme9";
    static constexpr const char *THEME_NINE_NAME                = "Ura";
    //内置主题10
    static constexpr const char *THEME_TEN                      = "Theme10";
    static constexpr const char *THEME_TEN_NAME                 = "One light";
    //自定义主题
    static constexpr const char *THEME_CUSTOM                   = "Custom Theme";


    // /etc/hostname 文件路径
    static constexpr const char *HOSTNAME_PATH                 = "/etc/hostname";

    /**
     * @brief 基类获取桌面索引
     * @author ut000439 wangpeili
     * @return
     */
    int getDesktopIndex() const;

    //Add by ut001000 renfeixiang 2020-11-16 标记是否动画效果，true : 不动画 false : 动画
    bool isNotAnimation = true;
    //雷神窗口动画时间计算公式：quakeAnimationBaseTime加上(quakeAnimationHighDistributionTotalTime乘以当前雷神高度除以雷神最大高度)所得时间
    //雷神窗口动画的基础时间
    static constexpr const int quakeAnimationBaseTime = 300;
    //雷神窗口动画按高度分配的总时间
    static constexpr const int quakeAnimationHighDistributionTotalTime = 150;
    //雷神窗口的tabbar高度,用于计算page的最小高度值
    static constexpr const int tabbarHeight = 60;

signals:
    void newWindowRequest(const QString &directory);
    // !这两个信号被封装了，请不要单独调用！
    void showPluginChanged(const QString &name, bool bSetFocus = false);
    void quakeHidePlugin();
    void mainwindowClosed(MainWindow *);
    // 编码方式，仅当前mainwindow有效．
    void changeEncodeSig(const QString &name);

public slots:
    /**
     * @brief 参数修改统一接口
     * @author n014361 王培利
     * @param keyName 参数名
     */
    void onWindowSettingChanged(const QString &keyName);
    /**
     * @brief 基类快捷键设置变化响应函数
     * @author ut000439 wangpeili
     * @param keyName 快捷键名称
     */
    void onShortcutSettingChanged(const QString &keyName);

    /**
     * @brief (远程）上传文件到服务器端
     * @author ut000439 王培利
     */
    void remoteUploadFile();
    /**
     * @brief 基类从远程服务下载文
     * @author ut000610 daizhengwen
     */
    void remoteDownloadFile();
    /**
     * @brief 处理雷神窗口自动隐藏功能以及window+D一起显示的问题
     * @author n014361 王培利
     * @param state 状态
     */
    void onApplicationStateChanged(Qt::ApplicationState state);
    /**
     * @brief 基类增加自定义命令响应函数
     * @author ut000125 sunchengxi
     * @param 新的自定义名命令
     */
    void addCustomCommandSlot(QAction *newAction);
    /**
     * @brief 基类删除自定义命令响应函数
     * @author ut000125 sunchengxi
     * @param 自定义命令
     */
    void removeCustomCommandSlot(QAction *newAction);

    /******** Add by ut001000 renfeixiang 2020-06-03:增加 Begin***************/
    /**
     * @brief 处理关闭类型的参数
     * @author ut001000 任飞翔
     * @param result
     * @param type 关闭类型
     */
    void OnHandleCloseType(int result, Utils::CloseType type);
    /******** Add by ut001000 renfeixiang 2020-06-03:增加 End***************/

    /**
     * @brief  QAction触发对应的自定义命令槽函数
     * @author ut000438 王亮
     */
    void onCommandActionTriggered();

    /**
     * @brief 上传文件对话框关闭处理
     * @author ut000438 王亮
     * @param code
     */
    void onUploadFileDialogFinished(int code);

protected:
    /**
     * @brief 按键响应事件，是空函数
     * @author ut000439 wangpeili
     * @param event
     */
    void keyPressEvent(QKeyEvent *event) override;
    /**
     * @brief 基类关闭事件
     * @author ut000439 wangpeili
     * @param event
     */
    void closeEvent(QCloseEvent *event) override;
    /**
     * @brief 基类调整大小事件
     * @author ut000439 wangpeili
     * @param event
     */
    void resizeEvent(QResizeEvent *event) override;
    /**
     * @brief 基类事件过滤器
     * @author ut000439 wangpeili
     * @param watched
     * @param event
     * @return
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    //------------------------------------------------------------

    /**
     * @brief 选中当前的内置主题项
     * @author ut000125 sunchengxi
     * @param expandThemeStr 内置主题项
     * @param action
     */
    void checkExtendThemeItem(const QString &expandThemeStr, QAction *&action);
    /**
     * @brief 选中当前的主题项
     * @author ut000125 sunchengxi
     */
    void checkThemeItem();
    /**
     * @brief 增加主题菜单
     * @author ut000125 sunchengxi
     */
    void addThemeMenuItems();
public:
    /**
     * @brief 选择主题项
     * @author ut000125 sunchengxi
     */
    void switchThemeAction(QAction *);
    /**
     * @brief 选择内置主题项
     * @author ut000125 sunchengxi
     * @param action
     * @param themeNameStr 主题项
     */
    void switchThemeAction(QAction *&action, const QString &themeNameStr);


protected slots:
    /**
     * @brief 基类终端标题变化响应函数
     * @author ut000439 wangpeili
     * @param title 标题
     */
    void onTermTitleChanged(QString title);
    /**
     * @brief 基类创建新窗口响应函数
     * @author ut000439 wangpeili
     * @param 工作路径
     */
    void onCreateNewWindow(QString workingDir);

    /**
     * @brief 鼠标选择时主题切换触发的槽函数
     * @author ut000125 sunchengxi
     */
    void themeActionTriggeredSlot(QAction *);
    /**
     * @brief 鼠标悬浮主题项时切换触发的槽函数
     * @author ut000125 sunchengxi
     */
    void themeActionHoveredSlot(QAction *);
    /**
     * @brief 设置选中的主题项的槽函数
     * @author ut000125 sunchengxi
     */
    void setThemeCheckItemSlot();
    /**
     * @brief 主题菜单隐藏时设置主题槽函数
     * @author ut000125 sunchengxi
     */
    void menuHideSetThemeSlot();

    void slotShowRenameTabDialog(QString Identifier);
    void slotMenuCloseOtherTab(QString Identifier);
    void slotMenuCloseTab(QString Identifier);
    void slotTabBarClicked(int index, QString tabIdentifier);
    void slotTabCloseRequested(int index);
    void slotTabAddRequested();
    void slotTabCurrentChanged(int index);

    void slotNewWindowActionTriggered();
    void slotClickNewWindowTimeout();
    void slotOptionButtonPressed();

    void slotFileChanged();
    void slotLastTermClosed(const QString &identifier);
    void slotDDialogFinished(int result);

    void slotShortcutBuiltinCopy();
    void slotShortcutBuiltinPaste();
    void slotShortcutFocusOut();
    void slotShortcutRemoteManage();
    void slotShortcutCustomCommand();
    void slotShortcutDisplayShortcuts();
    void slotShortcutRenameTitle();
    void slotShortcutSwitchFullScreen();
    void slotShortcutSelectAll();
    void slotShortcutDefaultSize();
    void slotShortcutZoomOut();
    void slotShortcutZoomIn();
    void slotShortcutFind();
    void slotShortcutPaste();
    void slotShortcutCopy();
    void slotShortcutCloseOtherWorkspaces();
    void slotShortcutCloseWorkspace();
    void slotShortcutSelectRightWorkspace();
    void slotShortcutSelectLeftWorkspace();
    void slotShortcutSelectLowerWorkspace();
    void slotShortcutSelectUpperWorkspace();
    void slotShortcutVerticalSplit();
    void slotShortcutHorizonzalSplit();
    void slotShortcutNextTab();
    void slotShortcutPreviousTab();
    void slotShortcutCloseOtherTabs();
    void slotShortcutCloseTab();
    void slotShortcutNewTab();
    void slotShortcutSwitchActivated();

    void slotDialogSelectFinished(int code);
    void slotCustomCommandActionTriggered();

protected:
    /**
     * @brief 基类初始化UI界面
     * @author ut000439 wangpeili
     */
    void initUI();
    /**
     * @brief 基类初始化窗口
     * @author ut000439 wangpeili
     */
    void initWindow();
    /**
     * @brief 设置默认位置，最大化，全屏还原使用
     * @author n014361 王培利
     */
    void setDefaultLocation();
    /**
     * @brief 首个普通窗口，需要居中显示
     * @author ut000439 王培利
     */
    void singleFlagMove();
    /**
     * @brief 基类获取配置窗口状态
     * @author ut000439 wangpeili
     * @return
     */
    QString getConfigWindowState();
    /**
     * @brief 获取半屏大小（高度-1,如果不-1,最大后无法正常还原）
     * @author n014361 王培利
     * @return
     */
    QSize halfScreenSize();
    /**
     * @brief 基类初始化插件
     * @author ut000439 wangpeili
     */
    void initPlugins();
    /**
     * @brief 基类初始化快捷键
     * @author ut000439 wangpeili
     */
    void initShortcuts();
    /**
     * @brief 基类初始化连接
     * @author ut000439 wangpeili
     */
    void initConnections();
    /**
     * @brief 基类初始化标签栏
     * @author ut000439 wangpeili
     */
    void initTabBar();
    /**
     * @brief 普通模式下，option button需要在全屏时切换控件
     * @author n014361 王培利
     */
    void initOptionButton();
    /**
     * @brief option menu初始化
     * @author n014361 王培利
     */
    void initOptionMenu();
    /**
     * @brief 初始化文件检测
     * @author ut000442 zhaogongqiang
     */
    void initFileWatcher();

    /**
     * @brief 基类设置新终端页面
     * @author ut000439 wangpeili
     * @param termPage 终端页面
     * @param activePage
     */
    void setNewTermPage(TermWidgetPage *termPage, bool activePage = true);

    /**
     * @brief 基类获取窗口信息配置路径
     * @author ut000439 wangpeili
     * @return
     */
    QString getWinInfoConfigPath();
    void handleTitleBarMenuFocusPolicy();

    /**
     * @brief  基类判断标签是否访问了
     * @author ut000439 wangpeili
     * @param tabIdentifier 标签识别码
     * @return
     */
    bool isTabVisited(const QString &tabIdentifier);
    /**
     * @brief 基类判断是否是标签更改颜色
     * @author ut000439 wangpeili
     * @param tabIdentifier 标签识别码
     * @return
     */
    bool isTabChangeColor(const QString &tabIdentifier);
    /**
     * @brief 更新标签页状态
     * @author ut000439 wangpeili
     */
    void updateTabStatus();

    // 终端退出保护
    bool closeConfirm();

    /**
     * @brief 显示快捷键列表信息
     * @author n014361 王培利
     */
    void displayShortcuts();
    /**
     * @brief 创建Json组信息
     * @author n014361 王培利
     * @param keyCategory 类别
     * @param jsonGroups Json组信息
     */
    void createJsonGroup(const QString &keyCategory, QJsonArray &jsonGroups);
    /**
     * @brief 创建内置快捷键管理
     * @author n014361 王培利
     * @param key 快捷键
     * @param AutoRepeat 是否可以连续触发快捷键, 默认可以
     * @return
     */
    QShortcut *createNewShotcut(const QString &key, bool AutoRepeat = true);
    //--added by qinyaning(nyq) 2020.4.13 18:21 解决无法从服务器下载文件的问题//
    /**
     * @brief 基类按下Enter键函数
     * @author ut000439 wangpeili
     * @param text
     */
    void pressEnterKey(const QString &text);

    /**
     * @brief mainwindow创建结束记录
     * @author ut000439 王培利
     */
    void createWindowComplete();
    /**
     * @brief 首个终端创建成功结束, 统计各个时间
     * @author ut000439 王培利
     */
    void firstTerminalComplete();


protected:
    // 初始化标题栏
    virtual void initTitleBar() = 0;
    // 初始化窗口属性
    virtual void initWindowAttribute() = 0;

public:
    // 保存窗口尺寸
    virtual void saveWindowSize() = 0;
    // 切换全屏
    virtual void switchFullscreen(bool forceFullscreen = false) = 0;
    // 显示快捷键功能
    virtual QPoint calculateShortcutsPreviewPoint() = 0;
    // 处理雷神窗口丢失焦点自动隐藏功能
    virtual void onAppFocusChangeForQuake() = 0;
    // 根据字体和字体大小设置最小高度
    virtual void setWindowMinHeightForFont() = 0;
    /******** Add by ut001000 renfeixiang 2020-08-07:基类中定义更新最小高度函数的纯虚函数***************/
    virtual void updateMinHeight() = 0;

protected:
    QMenu *m_menu = nullptr;
    TabBar *m_tabbar = nullptr;
    QWidget *m_centralWidget = nullptr;
    QVBoxLayout *m_centralLayout = nullptr;
    QStackedWidget *m_termStackWidget = nullptr;
    QList<MainWindowPluginInterface *> m_plugins;
    TermProperties m_properties;
    TitleBar *m_titleBar = nullptr;
    bool m_isQuakeWindow = false;
    QMap<QString, bool> m_tabVisitMap;
    QMap<QString, bool> m_tabChangeColorMap;

    /******** Modify by n014361 wangpeili 2020-02-20: 内置快捷键集中管理 ****************/
    QMap<QString, QShortcut *> m_builtInShortcut;
    /********************* Modify by n014361 wangpeili End ************************/
    QString downloadFilePath = "";
    RemoteManagementPlugin *remoteManagPlugin = nullptr;
    CustomCommandPlugin *customCommandPlugin = nullptr;
    QString m_strWindowId;
    const char *cmd = "ps -elf|grep deepin-terminal$ |wc -l";

    /******** Modify by m000714 daizhengwen 2020-03-29: 终端窗口配置 ****************/
    QSettings *m_winInfoConfig = nullptr;
    /********************* Modify by m000714 daizhengwen End ************************/
    QString m_CurrentShowPlugin = PLUGIN_TYPE_NONE;

protected:
    // 是否需要保存位置开关，雷神窗口不关心这个参数
    bool m_IfUseLastSize = false;
    // 雷神终端所在桌面
    int m_desktopIndex = 0;
    bool m_hasConfirmedClose = false;

    // 对应的程序启动时间，主进程或子进程
    qint64 m_ReferedAppStartTime = 0;
    // MainWindow开始创建的时间
    qint64 m_CreateWindowTime = 0;
    // MainWindow创建结束的时间
    qint64 m_WindowCompleteTime = 0;
    // 第一个Terminal创建完成时间
    qint64 m_FirstTerminalCompleteTime = 0;

    // ID
    int m_MainWindowID = 0;
    // 创建第一个终端完成时，需要记录
    bool hasCreateFirstTermialComplete = false;

public:
    //主题菜单
    SwitchThemeMenu *switchThemeMenu        = nullptr;
    //浅色主题快捷键
    QAction         *lightThemeAction       = nullptr;
    //深色主题快捷键
    QAction         *darkThemeAction        = nullptr;
    //跟随系统主题快捷键
    QAction         *autoThemeAction        = nullptr;

    //内置主题1快捷键
    QAction         *themeOneAction         = nullptr;
    //内置主题2快捷键
    QAction         *themeTwoAction         = nullptr;
    //内置主题3快捷键
    QAction         *themeThreeAction       = nullptr;
    //内置主题4快捷键
    QAction         *themeFourAction        = nullptr;
    //内置主题5快捷键
    QAction         *themeFiveAction        = nullptr;
    //内置主题6快捷键
    QAction         *themeSixAction         = nullptr;
    //内置主题7快捷键
    QAction         *themeSevenAction       = nullptr;
    //内置主题8快捷键
    QAction         *themeEightAction       = nullptr;
    //内置主题9快捷键
    QAction         *themeNineAction        = nullptr;
    //内置主题10快捷键
    QAction         *themeTenAction         = nullptr;
    //自定义主题快捷键
    QAction        *themeCustomAction      = nullptr;

    //主题快捷键组
    QActionGroup    *group                  = nullptr;

    //当前勾选的菜单主题快捷键
    QAction         *currCheckThemeAction   = nullptr;

    //用于保存identifier/TermWidgetPage键值对的map
    QMap<QString, TermWidgetPage *> m_termWidgetPageMap;

    bool m_isInitialized = false;

    QTimer *m_createTimer = nullptr;
};

/*******************************************************************************
 1. @类名:    NormalWindow
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    普通终端窗口
*******************************************************************************/
class NormalWindow : public MainWindow
{
    Q_OBJECT

public:
    explicit NormalWindow(TermProperties properties, QWidget *parent = nullptr);
    ~NormalWindow() override;
    /******** Add by ut001000 renfeixiang 2020-08-07:普通窗口不做处理***************/
    virtual void updateMinHeight() override {return;}

protected:
    /**
     * @brief 普通终端窗口初始化标题栏
     * @author ut001121 zhangmeng
     */
    virtual void initTitleBar() override;
    /**
     * @brief 初始化窗口属性,标准模式的窗口设置
     * @author n014361 王培利
     */
    virtual void initWindowAttribute() override;
    /**
     * @brief 普通窗口保存窗口大小
     * @author ut001121 zhangmeng
     */
    virtual void saveWindowSize() override;
    /**
     * @brief 普通窗口全屏切换
     * @author ut001121 zhangmeng
     * @param forceFullscreen
     */
    virtual void switchFullscreen(bool forceFullscreen = false) override;
    /**
     * @brief 普通窗口计算快捷预览显示坐标
     * @author ut000439 wangpeili
     * @return
     */
    virtual QPoint calculateShortcutsPreviewPoint() override;
    /**
     * @brief 普通窗口处理雷神窗口丢失焦点自动隐藏功能，普通窗口不用该函数
     * @author ut001121 zhangmeng
     */
    virtual void onAppFocusChangeForQuake() override;
    // 根据字体和字体大小设置最小高度
    virtual void setWindowMinHeightForFont() override {return;}

protected:
    /**
     * @brief 普通窗口变化事件
     * @author ut001121 zhangmeng
     * @param event 窗口变化事件
     */
    void changeEvent(QEvent *event) override;
};

/*******************************************************************************
 1. @类名:    QuakeWindow
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    雷神终端窗口
*******************************************************************************/
class QuakeWindow : public MainWindow
{
    Q_OBJECT
    //之前修改代码误删了这个属性，导致雷神动画失效，而且会导致雷神窗口属性异常(第一次雷神不可关闭，隐藏/显示后可关闭)
    //fix bug:63057 dock栏雷神终端缩略图右上角无关闭x按钮
    Q_PROPERTY(int height READ height WRITE setHeight)
    // 雷神resize是否拖拽的状态
    enum Quake_Resize_State {
        // 不进行resize
        Quake_NoResize = 0,
        // 正在resize
        Quake_Resize = 1,
        // 准备resize
        Quake_Prepare_Resize = 2
    };

public:
    explicit QuakeWindow(TermProperties properties, QWidget *parent = nullptr);
    ~QuakeWindow() override;

    /******** Add by ut001000 renfeixiang 2020-08-07:用于雷神窗口增加和减少横向分屏时，对雷神窗口的自小高进行修改，bug#41436***************/
    /**
     * @brief 用于雷神窗口增加和减少横向分屏时，对雷神窗口的自小高进行修改，bug#41436
     * @author ut001000 任飞翔
     */
    virtual void updateMinHeight() override;

    /**
     * @brief 给出当前桌面雷神是否显示
     * @author ut000610 戴正文
     * @return
     */
    bool isShowOnCurrentDesktop();
    /**
     * @brief 隐藏雷神终端
     * @author ut000610 戴正文
     */
    void hideQuakeWindow();
    /**
     * @brief 切换窗口拉伸属性
     * @author ut001121 张猛
     */
    void switchEnableResize();
    /**
     * @brief 是否开放窗口拉伸属性
     * @author ut001121 张猛
     * @param isEnable true 可以拉伸;false 不可以拉伸
     */
    void switchEnableResize(bool isEnable);

    /******** Add by ut001000 renfeixiang 2020-11-16:增加 雷神窗口动画效果函数 Begin***************/
    //设置雷神动画效果是否正在执行
    void setAnimationFlag(bool flag) {isNotAnimation = flag;}
    /**
     * @brief 雷神窗口从上而下的动画效果函数
     * @author ut001000 任飞翔
     */
    void topToBottomAnimation();
    /**
     * @brief 雷神窗口从下而上的动画效果函数
     * @author ut001000 任飞翔
     */
    void bottomToTopAnimation();
    /******** Add by ut001000 renfeixiang 2020-11-16 End***************/

public slots:
    /**
     * @brief 处理resize信号,延迟一段时间处理,避免处理过快,界面显示有延迟
     * @author ut000610 戴正文
     */
    void onResizeWindow();

    void slotWorkAreaResized();

    /**
     * @brief 雷神窗口从上而下的动画结束的处理
     * @author ut000438 王亮
     */
    void onTopToBottomAnimationFinished();

    /**
     * @brief 雷神窗口从下而上的动画结束的处理
     * @author ut000438 王亮
     */
    void onBottomToTopAnimationFinished();

protected:
    /**
     * @brief 雷神窗口初始化标题栏
     * @author ut001121 zhangmeng
     */
    virtual void initTitleBar() override;
    /**
     * @brief 初始化窗口属性,雷神窗口的特殊设置
     * @author n014361 王培利
     */
    virtual void initWindowAttribute() override;
    /**
     * @brief 雷神窗口保存雷神窗口大小
     * @author ut001121 zhangmeng
     */
    virtual void saveWindowSize() override;
    // 切换全屏
    /**
     * @brief 全屏切换，雷神窗口不用
     * @author ut001121 zhangmeng
     * @param forceFullscreen
     */
    virtual void switchFullscreen(bool forceFullscreen = false) override;
    /**
     * @brief 雷神窗口计算快捷键预览位置
     * @author ut001121 zhangmeng
     * @return
     */
    virtual QPoint calculateShortcutsPreviewPoint() override;
    /**
     * @brief 处理雷神窗口丢失焦点自动隐藏功能
     * @author ut001121 张猛
     */
    virtual void onAppFocusChangeForQuake() override;
    /**
     * @brief 雷神窗口根据字体和字体大小设置最小高度
     * @author ut001000 任飞翔
     */
    virtual void setWindowMinHeightForFont() override;

protected:
    /**
     * @brief 雷神窗口变化事件
     * @author ut001121 zhangmeng
     * @param event 雷神窗口变化事件
     */
    void changeEvent(QEvent *event) override;
    /**
     * @brief 窗口显示事件
     * @author ut001121 张猛
     * @param event 窗口显示事件
     */
    void showEvent(QShowEvent *event) override;
    /**
     * @brief 窗口事件
     * @author ut001121 张猛
     * @param event 窗口事件
     * @return
     */
    bool event(QEvent *event) override;
    /**
     * @brief 事件过滤器,处理雷神相关的事件resize
     * @author ut000610 戴正文
     * @param watched
     * @param event
     * @return
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    // 雷神窗口resize状态 默认noresize
    Quake_Resize_State m_resizeState = Quake_NoResize;
    // 雷神窗口的高度
    int m_quakeWindowHeight;
    // 雷神resize的定时器
    QTimer *m_resizeTimer = nullptr;
    // 记录雷神是否当前桌面显示
    QMap<int, bool> m_desktopMap;

    //Add by ut001000 renfeixiang 2020-11-16
    //获取配置文件中保存的雷神窗口高度
    int getQuakeHeight() {return m_winInfoConfig->value(CONFIG_QUAKE_WINDOW_HEIGHT).toInt();}
    /**
     * @brief 动画效果使用的设置雷神窗口高度的函数
     * @author ut001000 任飞翔
     * @param h 雷神窗口高度
     */
    void setHeight(int h);
    /**
     * @brief 计算雷神动画时间函数
     * @author ut001000 任飞翔
     * @return
     */
    int getQuakeAnimationTime();
};

#endif  // MAINWINDOW_H
