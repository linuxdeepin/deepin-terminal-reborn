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
#include "termwidget.h"
#include "define.h"
#include "settings.h"
#include "termproperties.h"
#include "mainwindow.h"
#include "shortcutmanager.h"
#include "utils.h"
#include "service.h"
#include "windowsmanager.h"
#include "serverconfigmanager.h"

#include <DDesktopServices>
#include <DInputDialog>
#include <DApplicationHelper>
#include <DLog>
#include <DDialog>
#include <DFloatingMessage>
#include <DMessageManager>

#include <QApplication>
#include <QKeyEvent>
#include <QDesktopServices>
#include <QMenu>
#include <QVBoxLayout>
#include <QHostInfo>
#include <QTime>
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>

DWIDGET_USE_NAMESPACE
using namespace Konsole;
TermWidget::TermWidget(const TermProperties &properties, QWidget *parent) : QTermWidget(0, parent), m_properties(properties)
{
    Utils::set_Object_Name(this);
    // 窗口数量加1
    WindowsManager::instance()->terminalCountIncrease();
    initTabTitle();
    //qDebug() << " TermWidgetparent " << parentWidget();
    m_page = static_cast<TermWidgetPage *>(parentWidget());
    setContextMenuPolicy(Qt::CustomContextMenu);

    setHistorySize(5000);

    QString strShellPath = Settings::instance()->shellPath();
    // set shell program
    qDebug() << "set shell program : " << strShellPath;
    setShellProgram(strShellPath);
    /******** Modify by ut000610 daizhengwen 2020-07-08:初始化透明度 Begin***************/
    // 若没有窗口特效，则不生效
    // 若有窗口特效，生效

    // 此方法会丢失焦点（show hide方法丢失焦点）
    // setTermOpacity(Settings::instance()->opacity());

    // 底层方法，设置当前窗口的透明度
    if (Service::instance()->isWindowEffectEnabled()) {
        // 判断当前是否有窗口特效
        setTerminalOpacity(Settings::instance()->opacity());
    }
    /********************* Modify by ut000610 daizhengwen End ************************/
    //setScrollBarPosition(QTermWidget::ScrollBarRight);//commend byq nyq

    /******** Modify by n014361 wangpeili 2020-01-13:              ****************/
    // theme
    QString theme = "Dark";
    /************************ Mod by sunchengxi 2020-09-16:Bug#48226#48230#48236#48241 终端默认主题色应改为深色修改引起的系列问题修复 Begin************************/
    theme = Settings::instance()->colorScheme();
    //if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
    //    theme = "Light";
    //}
    /************************ Mod by sunchengxi 2020-09-16:Bug#48226#48230#48236#48241 终端默认主题色应改为深色修改引起的系列问题修复 End ************************/
    setColorScheme(theme);
    Settings::instance()->setColorScheme(theme);

    // 这个参数启动为默认值UTF-8
    setTextCodec(QTextCodec::codecForName("UTF-8"));

    /******** Modify by n014361 wangpeili 2020-03-04: 增加保持打开参数控制，默认自动关闭**/
    setAutoClose(!m_properties.contains(KeepOpen));
    /********************* Modify by n014361 wangpeili End ************************/

    // WorkingDir
    if (m_properties.contains(WorkingDir)) {
        setWorkingDirectory(m_properties[WorkingDir].toString());
    }

    if (m_properties.contains(Execute)) {
        //QString args = m_properties[Execute].toString();
        //QStringList argList = args.split(QRegExp(QStringLiteral("\\s+")), QString::SkipEmptyParts);
        QStringList argList = m_properties[Execute].toStringList();
        qDebug() << "Execute args:" << argList;
        if (argList.count() > 0) {
            setShellProgram(argList.at(0));
            argList.removeAt(0);
            if (argList.count()) {
                setArgs(argList);
            }
        }
    }

    // 字体和字体大小, 8#字体立即设置的时候会有BUG显示，做个延迟生效就好了。
    if (Settings::instance()->fontSize() == 8) {
        QTimer::singleShot(10, this, &TermWidget::onSetTerminalFont);
    } else {
        QFont font = getTerminalFont();
        font.setFamily(Settings::instance()->fontName());
        font.setPointSize(Settings::instance()->fontSize());
        setTerminalFont(font);
    }

    // 光标形状
    setKeyboardCursorShape(static_cast<QTermWidget::KeyboardCursorShape>(Settings::instance()->cursorShape()));
    // 光标闪烁
    setBlinkingCursor(Settings::instance()->cursorBlink());

    // 按键滚动
    setPressingScroll(Settings::instance()->PressingScroll());

    /******** Modify by ut000439 wangpeili 2020-07-27: fix bug 39371: 分屏线可以拉到边****/
    // 以最小mainwindow分4屏为标准的最小大小
    /******** Modify by ut001000 renfeixiang 2020-08-07:修改成根据全局变量m_MinWidth，m_MinHeight计算出term的最小高度和宽度***************/
    setMinimumSize(MainWindow::m_MinWidth / 2, (MainWindow::m_MinHeight - WIN_TITLE_BAR_HEIGHT) / 2);
    /********************* Modify by n014361 wangpeili End ************************/

    QString currentEnvLanguage = Utils::getCurrentEnvLanguage();
    // 判断是维吾尔语或者藏语时
    if (QStringLiteral("bo_CN") == currentEnvLanguage || QStringLiteral("ug_CN") == currentEnvLanguage) {
        // 启用双向文本渲染支持
        setBidiEnabled(true);
    }

    // fix bug#67979 Shell配置中设置无效shell程序名后，新增窗口无悬浮框提示
    initConnections();

    // 启动shell
    startShellProgram();

    // 增加可以自动运行脚本的命令，不需要的话，可以删除
    if (m_properties.contains(Script)) {
        QString args = m_properties[Script].toString();
        qDebug() << "run cmd:" << args;
        args.append("\n");
        if (!m_properties.contains(KeepOpen)) {
            args.append("exit\n");
        }
        sendText(args);
    }

    setFocusPolicy(Qt::NoFocus);

    TermWidgetPage *parentPage = qobject_cast<TermWidgetPage *>(parent);
    //qDebug() << parentPage << endl;
    connect(this, &QTermWidget::uninstallTerminal, parentPage, &TermWidgetPage::uninstallTerminal);
}

void TermWidget::initConnections()
{
    // 输出滚动，会在每个输出判断是否设置了滚动，即时设置
    connect(this, &QTermWidget::receivedData, this, &TermWidget::onQTermWidgetReceivedData);

    // 接收到输出
    connect(this, &TermWidget::receivedData, this, &TermWidget::onTermWidgetReceivedData);

    // 接收到拖拽文件的Urls数据
    connect(this, &QTermWidget::sendUrlsToTerm, this, &TermWidget::onDropInUrls);

    connect(this, &QTermWidget::urlActivated, this, &TermWidget::onUrlActivated);

    connect(this, &QWidget::customContextMenuRequested, this, &TermWidget::customContextMenuCall);

    connect(DApplicationHelper::instance(),
            &DApplicationHelper::themeTypeChanged,
            this,
            &TermWidget::onThemeTypeChanged);

    // 未找到搜索的匹配结果
    connect(this, &QTermWidget::sig_noMatchFound, this, &TermWidget::onSig_noMatchFound);
    // 找到搜索匹配的结果 => 记录查找时间 => 打印日志，方便性能测试
    connect(this, &QTermWidget::sig_matchFound, this, &TermWidget::onSig_matchFound);
    /********************* Modify by n014361 wangpeili End ************************/

    connect(this, &QTermWidget::isTermIdle, this, &TermWidget::onTermIsIdle);

    connect(this, &QTermWidget::shellWarningMessage, this, &TermWidget::onShellMessage);

    connect(this, &QTermWidget::titleChanged, this, &TermWidget::onTitleChanged);

    // 标题参数变化
    connect(this, &QTermWidget::titleArgsChange, this, &TermWidget::onTitleArgsChange);
    connect(this, &TermWidget::copyAvailable, this, &TermWidget::onCopyAvailable);

    connect(Settings::instance(), &Settings::terminalSettingChanged, this, &TermWidget::onSettingValueChanged);

    //窗口特效开启则使用设置的透明度，窗口特效关闭时直接把窗口置为不透明
    connect(Service::instance(), &Service::onWindowEffectEnabled, this, &TermWidget::onWindowEffectEnabled);

    // 接收触控板事件
    connect(Service::instance(), &Service::touchPadEventSignal, this, &TermWidget::onTouchPadSignal);

    connect(Service::instance(), &Service::hostnameChanged, this, &TermWidget::onHostnameChanged);
}

inline void TermWidget::onSetTerminalFont()
{
    QFont font = getTerminalFont();
    font.setFamily(Settings::instance()->fontName());
    font.setPointSize(Settings::instance()->fontSize());
    setTerminalFont(font);
}

inline void TermWidget::onSig_matchFound()
{
    parentPage()->printSearchCostTime();
}

inline void TermWidget::onSig_noMatchFound()
{
    parentPage()->setMismatchAlert(true);
}

inline void TermWidget::onQTermWidgetReceivedData(QString value)
{
    Q_UNUSED(value)
    // 完善终端输出滚动相关功能，默认设置为"智能滚动"(即滚动条滑到最底下时自动滚动)
    if (!Settings::instance()->OutputtingScroll()) {
        setIsAllowScroll(true);
        return;
    }

    // 获取是否允许输出时滚动
    if (getIsAllowScroll()) {
        // 允许,则滚动到最新位置
        setTrackOutput(Settings::instance()->OutputtingScroll());
    } else {
        // 不允许,则不滚动
        // 将标志位置位
        setIsAllowScroll(true);
    }
}

inline void TermWidget::onTermWidgetReceivedData(QString value)
{
    /******** Modify by ut000610 daizhengwen 2020-05-25: quit download****************/
    if (value.contains("Transfer incomplete")) {
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier);
        QApplication::sendEvent(focusWidget(), &keyPress);
    }
    if (value.endsWith("\b \b #")) {                     // 结束的时候有乱码的话，将它清除
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_U, Qt::ControlModifier);
        QApplication::sendEvent(focusWidget(), &keyPress);
    }
    /********************* Modify by ut000610 daizhengwen End ************************/
    // 退出远程后，设置成false
    if ((value.contains("Connection to") && value.contains(" closed.")) || value.contains("Permission denied")) {
        QTimer::singleShot(100, this, &TermWidget::onExitRemoteServer);
    }
}

inline void TermWidget::onExitRemoteServer()
{
    // 判断是否此时退出远程
    if (!isInRemoteServer()) {
        qDebug() << "exit remote";
        setIsConnectRemote(false);
        // 还原编码
        setTextCodec(QTextCodec::codecForName(encode().toLocal8Bit()));
        qDebug() << "current encode " << encode();
        setBackspaceMode(m_backspaceMode);
        setDeleteMode(m_deleteMode);
        emit Service::instance()->checkEncode(encode());
    }
}

inline void TermWidget::onUrlActivated(const QUrl &url, bool fromContextMenu)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier || fromContextMenu) {
        QDesktopServices::openUrl(url);
    }
}

inline void TermWidget::onThemeTypeChanged(DGuiApplicationHelper::ColorType builtInTheme)
{
    qDebug() << "themeChanged" << builtInTheme;
    // ThemePanelPlugin *plugin = qobject_cast<ThemePanelPlugin *>(getPluginByName("Theme"));
    QString theme = "Dark";
    /************************ Mod by sunchengxi 2020-09-16:Bug#48226#48230#48236#48241 终端默认主题色应改为深色修改引起的系列问题修复 Begin************************/
    //Mod by sunchengxi 2020-09-17:Bug#48349 主题色选择跟随系统异常
    if (builtInTheme == DGuiApplicationHelper::LightType) {
        theme = "Light";
    }
    /************************ Mod by sunchengxi 2020-09-16:Bug#48226#48230#48236#48241 终端默认主题色应改为深色修改引起的系列问题修复 End ************************/
    //setColorScheme(theme);
    //Settings::instance()->setColorScheme(theme);
    QString  expandThemeStr = "";
    expandThemeStr = Settings::instance()->extendColorScheme();
    if (expandThemeStr.isEmpty()) {
        if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::LightType) {
            theme = "Light";
        }
        setColorScheme(theme);
        Settings::instance()->setColorScheme(theme);
    } else {
        setColorScheme(expandThemeStr, Settings::instance()->m_customThemeModify);
        Settings::instance()->m_customThemeModify = false;
    }
}

inline void TermWidget::onTermIsIdle(bool bIdle)
{
    emit termIsIdle(m_page->identifier(), bIdle);
}

inline void TermWidget::onTitleChanged()
{
    // 解析shell传来的title 用户名 主机名 地址/目录
    QString tabTitle = TermWidget::title();
    // %w shell设置的窗口标题
    m_tabArgs[SHELL_TITLE] = tabTitle;
    m_remoteTabArgs[SHELL_TITLE] = tabTitle;
    // 将标题参数变更信息传出
    emit termTitleChanged(getTabTitle());
}

inline void TermWidget::onCopyAvailable(bool enable)
{
    if (Settings::instance()->IsPasteSelection() && enable) {
        qDebug() << "hasCopySelection";
        QString strSelected = selectedText();
        QApplication::clipboard()->setText(strSelected, QClipboard::Clipboard);
    }
}

inline void TermWidget::onWindowEffectEnabled(bool isWinEffectEnabled)
{
    if (isWinEffectEnabled) {
        this->setTermOpacity(Settings::instance()->opacity());
    } else {
        this->setTermOpacity(1.0);
    }
}

TermWidget::~TermWidget()
{
    // 窗口减1
    WindowsManager::instance()->terminalCountReduce();
}

TermWidgetPage *TermWidget::parentPage()
{
    //qDebug() << "parentPage" << parentWidget();
    return  m_page;
}

void TermWidget::onTitleArgsChange(QString key, QString value)
{
    // tab获取参数
    m_tabArgs[key] = value;
    // 获取当前目录短 由长到短
    if (DIR_L == key) {
        QString dir = value;
        // 当前目录短（若有优化方法请及时告知）
        if (dir == "~" || dir.startsWith(QDir::homePath())) {
            // 出现家目录~的情况
            QString homePath = QDir::homePath();
            QStringList pathList = homePath.split("/");
            // 当前目录短
            m_tabArgs[DIR_S] = pathList.last();
            // 当前目录长对于~的处理 => 传过来的不是~但要填进去~和提示符保持一致
            dir.replace(homePath, "~");
            m_tabArgs[DIR_L] = dir;
        } else if (dir == "/") {
            // 出现根目录/的情况
            m_tabArgs[DIR_S] = "/";
        } else {
            // 一般情况
            QStringList pathList = dir.split("/");
            m_tabArgs[DIR_S] = pathList.last();
        }
    }

    // 更改标签标题
    emit termTitleChanged(getTabTitle());
}

void TermWidget::onHostnameChanged()
{
    // 主机名变化
    QString hostName = QHostInfo::localHostName();
    m_tabArgs[LOCAL_HOST_NAME] = hostName;
    // 发送标签标题变化
    emit termTitleChanged(getTabTitle());
}

inline void TermWidget::onCopy()
{
    copyClipboard();
}

inline void TermWidget::onPaste()
{
    pasteClipboard();
}

inline void TermWidget::onOpenFileInFileManager()
{
    //DDesktopServices::showFolder(QUrl::fromLocalFile(workingDirectory()));

    //打开文件夹的方式 和  打开文件夹 并勾选文件的方式 如下
    //dde-file-manager -n /data/home/lx777/my-wjj/git/2020-08/18-zoudu/build-deepin-terminal-unknown-Debug
    //dde-file-manager --show-item a.pdf

    QProcess process;
    //未选择内容
    if (selectedText().isEmpty()) {
        process.startDetached("dde-file-manager -n " + workingDirectory());
        return;
    }

    QFileInfo fi(workingDirectory() + "/" + selectedText());
    //选择的内容是文件或者文件夹
    if (fi.isFile() || fi.isDir()) {
        process.startDetached("dde-file-manager --show-item " + workingDirectory() + "/" + selectedText());
        return;
    }
    //选择的文本不是文件也不是文件夹
    process.startDetached("dde-file-manager -n " + workingDirectory());
}

/*** 修复 bug 28162 鼠标左右键一起按终端会退出 ***/
void TermWidget::addMenuActions(const QPoint &pos)
{
    bool isRemoting = isConnectRemote();

    QList<QAction *> termActions = filterActions(pos);
    for (QAction *&action : termActions) {
        m_menu->addAction(action);
    }

    if (!m_menu->isEmpty()) {
        m_menu->addSeparator();
    }

    // add other actions here.
    if (!selectedText().isEmpty()) {

        m_menu->addAction(tr("Copy"), this, &TermWidget::onCopy);
    }
    if (!QApplication::clipboard()->text(QClipboard::Clipboard).isEmpty()) {
        m_menu->addAction(tr("Paste"), this, &TermWidget::onPaste);
    }
    /******** Modify by n014361 wangpeili 2020-02-26: 添加打开(文件)菜单功能 **********/
    if (!isRemoting && !selectedText().isEmpty()) {
        QString fileName = getFormatFileName(selectedText());
        QString filePath = getFilePath(fileName);
        QUrl fileUrl = QUrl::fromLocalFile(filePath);
        QFileInfo tempfile(fileUrl.path());
        if (fileName.length() > 0 && tempfile.exists()) {
            m_menu->addAction(tr("Open"), this, &TermWidget::onOpenFile);
        }
    }
    /********************* Modify by n014361 wangpeili End ************************/

    m_menu->addAction(tr("Open in file manager"), this, &TermWidget::onOpenFileInFileManager);

    m_menu->addSeparator();


    Qt::Orientation orientation = static_cast<DSplitter *>(parentWidget())->orientation();
    int layer = getTermLayer();

    if (layer == 1 || (layer == 2 && orientation == Qt::Horizontal)) {
        m_menu->addAction(tr("Horizontal split"), this, &TermWidget::onHorizontalSplit);
    }
    if (layer == 1 || (layer == 2 && orientation == Qt::Vertical)) {
        m_menu->addAction(tr("Vertical split"), this, &TermWidget::onVerticalSplit);
    }

    /******** Modify by n014361 wangpeili 2020-02-21: 增加关闭窗口和关闭其它窗口菜单    ****************/
    m_menu->addAction(QObject::tr("Close workspace"), this, &TermWidget::onCloseCurrWorkSpace);
    //m_menu->addAction(tr("Close Window"), this, [this] { ((TermWidgetPage *)m_Page)->close();});
    if (parentPage()->getTerminalCount() > 1) {
        m_menu->addAction(QObject::tr("Close other workspaces"), this, &TermWidget::onCloseOtherWorkSpaces);
    };

    /********************* Modify by n014361 wangpeili End ************************/
    m_menu->addSeparator();
    m_menu->addAction(tr("New tab"), this, &TermWidget::onCreateNewTab);

    m_menu->addSeparator();

    if (!parentPage()->parentMainWindow()->isQuakeMode()) {
        bool isFullScreen = this->window()->windowState().testFlag(Qt::WindowFullScreen);
        if (isFullScreen) {
            m_menu->addAction(tr("Exit fullscreen"), this, &TermWidget::onSwitchFullScreen);
        } else {
            m_menu->addAction(tr("Fullscreen"), this, &TermWidget::onSwitchFullScreen);
        }
    }

    m_menu->addAction(tr("Find"), this, &TermWidget::onShowSearchBar);
    m_menu->addSeparator();

    if (!selectedText().isEmpty()) {
        DMenu *search = new DMenu(tr("Search"), this);

        search->addAction("Bing", this, &TermWidget::openBing);
        search->addAction("Baidu", this, &TermWidget::openBaidu);
        search->addAction("Github", this, &TermWidget::openGithub);
        search->addAction("Stack Overflow", this, &TermWidget::openStackOverflow);
        m_menu->addMenu(search);
        m_menu->addSeparator();
    }

    m_menu->addAction(tr("Encoding"), this, &TermWidget::onShowEncoding);

    m_menu->addAction(tr("Custom commands"), this, &TermWidget::onShowCustomCommands);

    m_menu->addAction(tr("Remote management"), this, &TermWidget::onShowRemoteManagement);

    if (isConnectRemote()) {
        m_menu->addSeparator();
        m_menu->addAction(tr("Upload file"), this, &TermWidget::onUploadFile);
        m_menu->addAction(tr("Download file"), this, &TermWidget::onDownloadFile);
    }

    m_menu->addSeparator();

    m_menu->addAction(tr("Settings"), this, &TermWidget::onShowSettings);

    // 判断是否到达最大数量
    if (!Service::instance()->isCountEnable()) {
        QList<QAction *> actionList = m_menu->actions();
        for (auto item : actionList) {
            if (item->text() == tr("New tab") || item->text() == tr("Horizontal split") || item->text() == tr("Vertical split")) {
                // 无法点击的菜单项置灰
                item->setEnabled(false);
            }
        }
    }
}

inline void TermWidget::onHorizontalSplit()
{
    getTermLayer();
    // menu关闭与分屏同时进行时，会导致QT计算光标位置异常。
    QTimer::singleShot(10, this, &TermWidget::splitHorizontal);
}

inline void TermWidget::onVerticalSplit()
{
    getTermLayer();
    // menu关闭与分屏同时进行时，会导致QT计算光标位置异常。
    QTimer::singleShot(10, this, &TermWidget::splitVertical);
}

inline void TermWidget::splitHorizontal()
{
    parentPage()->split(Qt::Horizontal);
    //分屏时切换到当前选中主题方案
    switchThemeOnSplitScreen();
}

inline void TermWidget::splitVertical()
{
    parentPage()->split(Qt::Vertical);
    //分屏时切换到当前选中主题方案
    switchThemeOnSplitScreen();
}

inline void TermWidget::onCloseCurrWorkSpace()
{
    parentPage()->closeSplit(parentPage()->currentTerminal());
}

inline void TermWidget::onCloseOtherWorkSpaces()
{
    parentPage()->closeOtherTerminal();
}

inline void TermWidget::onCreateNewTab()
{
    parentPage()->parentMainWindow()->createNewTab();
}

inline void TermWidget::onSwitchFullScreen()
{
    parentPage()->parentMainWindow()->switchFullscreen();
}

inline void TermWidget::openBing()
{
    QString strUrl = "https://cn.bing.com/search?q=" + selectedText();
    openUrl(strUrl);
}

inline void TermWidget::openBaidu()
{
    QString strUrl = "https://www.baidu.com/s?wd=" + selectedText();
    openUrl(strUrl);
}

inline void TermWidget::openGithub()
{
    QString strUrl = "https://github.com/search?q=" + selectedText();
    openUrl(strUrl);
}

inline void TermWidget::openStackOverflow()
{
    QString strUrl = "https://stackoverflow.com/search?q=" + selectedText();
    openUrl(strUrl);
}

inline void TermWidget::onShowSearchBar()
{
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_SEARCHBAR);
}

inline void TermWidget::onShowEncoding()
{
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);
}

inline void TermWidget::onShowCustomCommands()
{
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
}

inline void TermWidget::onShowRemoteManagement()
{
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);
}

inline void TermWidget::onUploadFile()
{
    parentPage()->parentMainWindow()->remoteUploadFile();
}

inline void TermWidget::onDownloadFile()
{
    parentPage()->parentMainWindow()->remoteDownloadFile();
}

inline void TermWidget::onShowSettings()
{
    Service::instance()->showSettingDialog(parentPage()->parentMainWindow());
}

inline void TermWidget::openUrl(QString strUrl)
{
    QDesktopServices::openUrl(QUrl(strUrl));
}

inline QString TermWidget::getFormatFileName(QString selectedText)
{
    QString fileName = selectedText.trimmed();
    if ((fileName.startsWith("'") && fileName.endsWith("'"))
            || (fileName.startsWith("\"") && fileName.endsWith("\""))) {
        fileName = fileName.remove(0, 1);
        fileName = fileName.remove(fileName.length() - 1, 1);
        qDebug() << "fileName is :" << fileName;
    }

    return fileName;
}

inline QString TermWidget::getFilePath(QString fileName)
{
    //如果fileName本身已经是一个文件路径
    if (fileName.startsWith("/")) {
        return fileName;
    }

    return workingDirectory() + "/" + fileName;
}

inline void TermWidget::onOpenFile()
{
    QString fileName = getFormatFileName(selectedText());
    QString filePath = getFilePath(fileName);
    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    QDesktopServices::openUrl(fileUrl);
}

QString TermWidget::RemoteEncode() const
{
    return m_remoteEncode;
}

void TermWidget::setRemoteEncode(const QString &RemoteEncode)
{
    m_remoteEncode = RemoteEncode;
}

void TermWidget::setBackspaceMode(const EraseMode &backspaceMode)
{
    char ch;
    int length;
    switch (backspaceMode) {
    case EraseMode_Auto:
        ch = getErase();
        if (ch == 0) {
            ch = '\010';
        }
        length = 1;
        break;
    case EraseMode_Control_H:
        ch = '\010';
        length = 1;
        break;
    case EraseMode_Ascii_Delete:
        ch = '\177';
        length = 1;
        break;
    case EraseMode_TTY:
        ch = getErase();
        length = 1;
        qDebug() << "tty erase : " << QByteArray(&ch, length).toHex();
        break;
    case EraseMode_Escape_Sequeue:
        length = 4;
        char sequeue[] = "\e[3~";
        QTermWidget::setBackspaceMode(sequeue, 4);
        return;
    }
    QTermWidget::setBackspaceMode(&ch, length);
}

void TermWidget::setDeleteMode(const EraseMode &deleteMode)
{
    char ch;
    int length;
    switch (deleteMode) {
    case EraseMode_Control_H:
        ch = '\010';
        length = 1;
        break;
    case EraseMode_Ascii_Delete:
        ch = '\177';
        length = 1;
        break;
    case EraseMode_TTY:
        ch = getErase();
        length = 1;
        qDebug() << "tty erase : " << QByteArray(&ch, length).toHex();
        break;
    case EraseMode_Auto:
    case EraseMode_Escape_Sequeue:
        length = 4;
        char sequeue[] = "\e[3~";
        QTermWidget::setDeleteMode(sequeue, 4);
        return;
    }
    QTermWidget::setDeleteMode(&ch, length);
}

int TermWidget::getTermLayer()
{
    int layer = 1;
    QWidget *currentW = this;
    while (currentW->parentWidget() != parentPage()) {
        layer++;
        currentW = currentW->parentWidget();
    }
    qDebug() << "getTermLayer = " << layer;
    return  layer;
}

void TermWidget::setTabFormat(const QString &tabFormat)
{
    // 非全局设置优先级更高
    if (m_tabFormat.isGlobal != false && m_tabFormat.currentTabFormat != tabFormat) {
        m_tabFormat.currentTabFormat = tabFormat;
        // 有变化，就将变化发出，但此时可能显示的不是变化的项
        emit termTitleChanged(getTabTitle());
    }
}

void TermWidget::setRemoteTabFormat(const QString &remoteTabFormat)
{
    // 非全局设置优先级更高
    if (m_tabFormat.isGlobal != false && m_tabFormat.remoteTabFormat != remoteTabFormat) {
        m_tabFormat.remoteTabFormat = remoteTabFormat;
        // 有变化，就将变化发出，但此时可能显示的不是变化的项
        emit termTitleChanged(getTabTitle());
    }
}

void TermWidget::renameTabFormat(const QString &tabFormat, const QString &remoteTabFormat)
{
    // 重命名优先级高
    m_tabFormat.currentTabFormat = tabFormat;
    m_tabFormat.remoteTabFormat = remoteTabFormat;
    m_tabFormat.isGlobal = false;
    emit termTitleChanged(getTabTitle());
}

QString TermWidget::getTabTitle()
{
    QString strTabName;
    // 判断当前是否连接远程
    if (isConnectRemote() /*|| getForegroundProcessName() == "ssh"*/) {
        // 连接远程 远程标签标题格式
        strTabName = getTabTitle(m_remoteTabArgs, m_tabFormat.remoteTabFormat);
    } else {
        // 当前标签标题格式
        strTabName = getTabTitle(m_tabArgs, m_tabFormat.currentTabFormat);
    }

    // 没有内容则给Terminal作为默认标题
    if (strTabName.trimmed().isEmpty()) {
        strTabName = DEFAULT_TAB_TITLE;
    }
    return strTabName;
}

QString TermWidget::getTabTitle(QMap<QString, QString> format, QString TabFormat)
{
    // 遍历参数列表
    for (QString key : format.keys()) {
        // 判断是否包含改参数
        if (TabFormat.contains(key)) {
            // 包含的话，把参数替换为参数对应的内容
            TabFormat.replace(key, format[key]);
        }
    }
    return TabFormat;
}

void TermWidget::switchThemeOnSplitScreen()
{
    QString  expandThemeStr = "";
    expandThemeStr = Settings::instance()->extendColorScheme();
    if (!expandThemeStr.isEmpty()) {
        emit DApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::instance()->themeType());
    }
}

void TermWidget::initTabTitle()
{
    static ushort sessionNumber = 0;
    // 初始化标签标题参数
    initTabTitleArgs();
    // 会话编号赋值
    m_sessionNumber = ++sessionNumber;
    m_tabArgs[TAB_NUM] = QString::number(m_sessionNumber);
    m_remoteTabArgs[TAB_NUM] = QString::number(m_sessionNumber);
    // 初始化hostName
    m_tabArgs[LOCAL_HOST_NAME] = QHostInfo::localHostName();
    // 设置标签标题格式
    setTabFormat(Settings::instance()->tabTitleFormat());
    setRemoteTabFormat(Settings::instance()->remoteTabTitleFormat());
    // 标签标题变化，则每个term变化
    connect(Settings::instance(), &Settings::tabFormatChanged, this, &TermWidget::setTabFormat);
    connect(Settings::instance(), &Settings::remoteTabFormatChanged, this, &TermWidget::setRemoteTabFormat);
}

void TermWidget::initTabTitleArgs()
{
    QStringList strTabArgs = TAB_ARGS.split(" ");
    // 填充标签标题参数
    for (QString arg : strTabArgs) {
        m_tabArgs.insert(arg, " ");
    }
    // 填充远程标题标签参数
    QStringList strRemoteTabArgs = REMOTE_ARGS.split(" ");
    for (QString arg : strRemoteTabArgs) {
        m_remoteTabArgs.insert(arg, " ");
    }
    qDebug() << "Tab args init! tab title count : " << m_tabArgs.count() << " remote title count : " << m_remoteTabArgs.count();
}

QString TermWidget::getTabTitleFormat()
{
    return m_tabFormat.currentTabFormat;
}

QString TermWidget::getRemoteTabTitleFormat()
{
    return m_tabFormat.remoteTabFormat;
}

QString TermWidget::getCurrentTabTitleFormat()
{
    // 连接远程
    if (isConnectRemote()) {
        return m_tabFormat.remoteTabFormat;
    }

    // 未连接远程
    return m_tabFormat.currentTabFormat;
}

QString TermWidget::encode() const
{
    return m_encode;
}

void TermWidget::setEncode(const QString &encode)
{
    m_encode = encode;
}

bool TermWidget::isConnectRemote() const
{
    return m_isConnectRemote;
}

void TermWidget::setIsConnectRemote(bool isConnectRemote)
{
    m_isConnectRemote = isConnectRemote;
}

void TermWidget::modifyRemoteTabTitle(ServerConfig remoteConfig)
{
    // 远程主机名
    m_remoteTabArgs[REMOTE_HOST_NAME] = remoteConfig.m_address;
    // 用户名 %u
    m_remoteTabArgs[USER_NAME] = remoteConfig.m_userName;
    // 用户名@ %U
    m_remoteTabArgs[USER_NAME_L] = remoteConfig.m_userName + QString("@");

    // 标签标题变化
    emit termTitleChanged(getTabTitle());
}

bool TermWidget::enterSzCommand() const
{
    return m_enterSzCommand;
}

void TermWidget::setEnterSzCommand(bool enterSzCommand)
{
    m_enterSzCommand = enterSzCommand;
}

inline void TermWidget::customContextMenuCall(const QPoint &pos)
{
    /***add by ut001121 zhangmeng 20200514 右键获取焦点, 修复BUG#26003***/
    setFocus();

    // 右键清理插件
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_NONE);

    /*** 修复 bug 28162 鼠标左右键一起按终端会退出 ***/
    if (nullptr == m_menu) {
        m_menu = new DMenu(this);
        m_menu->setObjectName("TermMenu");//Add by ut001000 renfeixiang 2020-08-13
    }

    m_menu->clear();
    addMenuActions(pos);

    // display the menu.
    m_menu->exec(mapToGlobal(pos));
}

bool TermWidget::isInRemoteServer()
{
    int pid = getForegroundProcessId();
    if (pid <= 0) {
        return false;
    }
    QString pidFilepath = "/proc/" + QString::number(pid) + "/comm";
    QFile pidFile(pidFilepath);
    if (pidFile.exists()) {
        pidFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QString commString(pidFile.readLine());
        pidFile.close();
        if ("expect" == commString.trimmed()) {
            return true;
        }
    }
    return false;
}

void TermWidget::setTermOpacity(qreal opacity)
{
    //这里再次判断一遍，因为刚启动时，还是需要判断一次当前是否开启了窗口特效
    qreal termOpacity = opacity;
    if (!Service::instance()->isWindowEffectEnabled()) {
        termOpacity = 1.0;
    }

    setTerminalOpacity(termOpacity);
    /******* Modify by n014361 wangpeili 2020-01-04: 修正实时设置透明度问题************/
    hide();
    show();
    /********************* Modify by n014361 wangpeili End ************************/
}

void TermWidget::setTermFont(const QString &fontName)
{
    QFont font = getTerminalFont();
    font.setFamily(fontName);
    setTerminalFont(font);
}

void TermWidget::setTermFontSize(const int fontSize)
{
    QFont font = getTerminalFont();
    font.setFixedPitch(true);
    font.setPointSize(fontSize);
    setTerminalFont(font);
}


void TermWidget::skipToNextCommand()
{
    qDebug() << "skipToNextCommand";
}

void TermWidget::skipToPreCommand()
{
    qDebug() << "skipToPreCommand";
}

void TermWidget::setCursorShape(int shape)
{
    Konsole::Emulation::KeyboardCursorShape cursorShape = Konsole::Emulation::KeyboardCursorShape(shape);
    setKeyboardCursorShape(cursorShape);
}

void TermWidget::setPressingScroll(bool enable)
{
    if (enable) {
        setMotionAfterPasting(2);
    } else {
        setMotionAfterPasting(0);
    }
}

void TermWidget::selectEncode(QString encode)
{
    // 直接设置终端
    setTextCodec(QTextCodec::codecForName(encode.toUtf8()));
    // 是否连接远程
    if (!isConnectRemote()) {
        // 记录当前的encode
        setEncode(encode);
        qDebug() << "current encode " << encode;
    } else {
        // 记录远程的encode
        setRemoteEncode(encode);
        qDebug() << "Remote encode " << encode;
    }
}

void TermWidget::onSettingValueChanged(const QString &keyName)
{
    qDebug() << "onSettingValueChanged:" << keyName;
    if (keyName == "basic.interface.opacity") {
        setTermOpacity(Settings::instance()->opacity());
        return;
    }

    if (keyName == "basic.interface.font") {
        setTermFont(Settings::instance()->fontName());
        /******** Add by nt001000 renfeixiang 2020-05-20:增加字体变化时设置雷神窗口最小高度 Begin***************/
        m_page->parentMainWindow()->setWindowMinHeightForFont();
        /******** Add by ut001000 renfeixiang 2020-08-07:字体改变时改变大小，bug#41436***************/
        m_page->parentMainWindow()->updateMinHeight();
        /******** Add by nt001000 renfeixiang 2020-05-20:增加字体变化时设置雷神窗口最小高度 End***************/
        return;
    }

    if (keyName == "basic.interface.font_size") {
        setTermFontSize(Settings::instance()->fontSize());
        /******** Add by nt001000 renfeixiang 2020-05-20:增加字体大小变化时设置雷神窗口最小高度 Begin***************/
        m_page->parentMainWindow()->setWindowMinHeightForFont();
        /******** Add by ut001000 renfeixiang 2020-08-07:字体大小改变时改变大小，bug#41436***************/
        m_page->parentMainWindow()->updateMinHeight();
        /******** Add by nt001000 renfeixiang 2020-05-20:增加字体大小变化时设置雷神窗口最小高度 End***************/
        return;
    }

    if (keyName == "advanced.cursor.cursor_shape") {
        setCursorShape(Settings::instance()->cursorShape());
        return;
    }

    if (keyName == "advanced.cursor.cursor_blink") {
        setBlinkingCursor(Settings::instance()->cursorBlink());
        return;
    }

    if (keyName == "advanced.scroll.scroll_on_key") {
        setPressingScroll(Settings::instance()->PressingScroll());
        return;
    }

    if (keyName == "basic.interface.theme") {
        return;
    }
    // 这里只是立即生效一次，真正生效起作用的地方在初始的connect中
    if (keyName == "advanced.cursor.auto_copy_selection") {
        if (Settings::instance()->IsPasteSelection()) {
            copyClipboard();
        } else {
            QApplication::clipboard()->clear(QClipboard::Clipboard);
        }

        return;
    }

    if (keyName == "advanced.scroll.scroll_on_output") {
        qDebug() << "settingValue[" << keyName << "] changed to " << Settings::instance()->OutputtingScroll()
                 << ", auto effective when happen";
        return;
    }

    qDebug() << "settingValue[" << keyName << "] changed is not effective";
}

void TermWidget::onDropInUrls(const char *urls)
{
    QString strUrls = QString::fromLocal8Bit(urls);
    qDebug() << "recv urls:" << strUrls;
    if (isConnectRemote()) {
        // 远程管理连接中
        QString strTxt = "sz ";
        strTxt += strUrls;
        // 上传文件
        parentPage()->parentMainWindow()->remoteUploadFile(strTxt);
    } else {
        // 发送字符
        sendText(strUrls);
    }
}


inline void TermWidget::onTouchPadSignal(QString name, QString direction, int fingers)
{
    qDebug() << __FUNCTION__;
    qDebug() << name << direction << fingers;
    // 当前窗口被激活,且有焦点
    if (isActiveWindow() && hasFocus()) {
        if (name == "pinch" && fingers == 2) {
            if (direction == "in") {
                // 捏合 in是手指捏合的方向 向内缩小
                zoomOut();  // zoom out 缩小
            } else if (direction == "out") {
                // 捏合 out是手指捏合的方向 向外放大
                zoomIn();   // zoom in 放大
            }
        }
    }
}

void TermWidget::onShellMessage(QString currentShell, bool isSuccess)
{
    if (isSuccess) {
        // 替换了shell提示
        QString strSetShell = Settings::instance()->shellPath();
        QString strShellWarning = QObject::tr("Could not find \"%1\", starting \"%2\" instead. Please check your shell profile.").arg(strSetShell).arg(currentShell);
        showShellMessage(strShellWarning);
    } else {
        // 启动shell失败
        QString strShellNoFound = QObject::tr("Could not find \"%1\", unable to run it").arg(currentShell);
        showShellMessage(strShellNoFound);
    }
}

void TermWidget::wheelEvent(QWheelEvent *event)
{
    // 当前窗口被激活,且有焦点
    if (isActiveWindow() && hasFocus()) {
        if (Qt::ControlModifier == event->modifiers()) {
            int directionY = event->angleDelta().y();
            if (directionY < 0) {
                // 向下缩小
                zoomOut();  // zoom out 缩小
            } else {
                // 向上放大
                zoomIn();   // zoom in 放大
            }
            return;
        }
    }
    QTermWidget::wheelEvent(event);
}

bool TermWidget::eventFilter(QObject *o, QEvent *e)
{
    if (m_messageTextMap.contains(o) && e->type() == QEvent::Resize) {
        QLabel *label = static_cast<QLabel *>(o);
        QString orgText = m_messageTextMap[o];
        QString elideText = Utils::getElidedText(label->font(), orgText, label->width(), Qt::ElideRight);
        if (elideText != label->text())
            label->setText(elideText);
    }
    return QTermWidget::eventFilter(o, e);
}

void TermWidget::showFlowMessage(bool show)
{
    if (nullptr == m_flowMessage) {
        m_flowMessage = new DFloatingMessage(DFloatingMessage::ResidentType, this);
        QList<QLabel *> lst = m_flowMessage->findChildren<QLabel *>();
        for (auto label : lst) {
            label->setWordWrap(false);
        }
        m_flowMessage->setIcon(QIcon(":icons/deepin/builtin/warning.svg"));
        QString strText = QObject::tr("Output has been suspended by pressing Ctrl+S. Pressing Ctrl+Q to resume.");
        installEventMessageText(m_flowMessage, strText);
        DMessageManager::instance()->sendMessage(this, m_flowMessage);
    }
    m_flowMessage->setVisible(show);
}

void TermWidget::showShellMessage(QString strWarnings)
{
    // 初始化悬浮框
    DFloatingMessage *shellWarningsMessage = new DFloatingMessage(DFloatingMessage::ResidentType, this);
    QList<QLabel *> lst = shellWarningsMessage->findChildren<QLabel *>();
    for (auto label : lst) {
        label->setWordWrap(false);
    }
    // 关闭悬浮框时销毁
    connect(shellWarningsMessage, &DFloatingMessage::closeButtonClicked, shellWarningsMessage, &DFloatingMessage::deleteLater);
    // 设置icon和文字
    shellWarningsMessage->setIcon(QIcon(":icons/deepin/builtin/warning.svg"));
    installEventMessageText(shellWarningsMessage, strWarnings);
    // 调用DTK的方法关闭悬浮框
    DMessageManager::instance()->sendMessage(this, shellWarningsMessage);
}

void TermWidget::installEventMessageText(DFloatingMessage *widget, const QString &text)
{
    widget->setMessage("the sentence for searching MessageLabel of DFloatingMessage");
    for (auto label : widget->findChildren<QLabel *>()) {
        if (label->text() == "the sentence for searching MessageLabel of DFloatingMessage") {
            label->installEventFilter(this);
            m_messageTextMap[label] = text;
            break;
        }
    }
    widget->setMessage(text);
}


