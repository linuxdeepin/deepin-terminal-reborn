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
#include "operationconfirmdlg.h"
#include "utils.h"
#include "service.h"
#include "windowsmanager.h"

#include <DDesktopServices>
#include <DInputDialog>
#include <DApplicationHelper>
#include <DLog>
#include <DDialog>

#include <QApplication>
#include <QKeyEvent>
#include <QDesktopServices>
#include <QMenu>
#include <QVBoxLayout>
#include <QTime>
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>

DWIDGET_USE_NAMESPACE
using namespace Konsole;
TermWidget::TermWidget(TermProperties properties, QWidget *parent) : QTermWidget(0, parent), m_properties(properties)
{
    Utils::set_Object_Name(this);
    // 窗口数量加1
    WindowsManager::instance()->terminalCountIncrease();
    //qDebug() << " TermWidgetparent " << parentWidget();
    m_page = static_cast<TermWidgetPage *>(parentWidget());
    setContextMenuPolicy(Qt::CustomContextMenu);

    setHistorySize(5000);

    // set shell program
    QString shell{ getenv("SHELL") };
    setShellProgram(shell.isEmpty() ? "/bin/bash" : shell);
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
    theme=Settings::instance()->colorScheme();
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
        QTimer::singleShot(10, this, [this]() {
            QFont font = getTerminalFont();
            font.setFamily(Settings::instance()->fontName());
            font.setPointSize(Settings::instance()->fontSize());
            setTerminalFont(font);
        });
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

    // 输出滚动，会在每个输出判断是否设置了滚动，即时设置
    connect(this, &QTermWidget::receivedData, this, [this](QString value) {
        Q_UNUSED(value)
        // 获取是否允许输出时滚动
        if (getIsAllowScroll()) {
            // 允许,则滚动到最新位置
            setTrackOutput(Settings::instance()->OutputtingScroll());
        } else {
            // 不允许,则不滚动
            // 将标志位置位
            setIsAllowScroll(true);
        }
    });

    // 接收到输出
    connect(this, &TermWidget::receivedData, this, [this](QString value) {
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
        if (value.contains("Connection to") && value.contains(" closed.")) {
            QTimer::singleShot(100, this, [&]() {
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
            });
        }
    });

    connect(this, &QTermWidget::urlActivated, this, [](const QUrl & url, bool fromContextMenu) {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier || fromContextMenu) {
            QDesktopServices::openUrl(url);
        }
    });

    connect(this, &QWidget::customContextMenuRequested, this, &TermWidget::customContextMenuCall);

    connect(DApplicationHelper::instance(),
            &DApplicationHelper::themeTypeChanged,
            this,
    [ = ](DGuiApplicationHelper::ColorType builtInTheme) {
        qDebug() << "themeChanged" << builtInTheme;
        // ThemePanelPlugin *plugin = qobject_cast<ThemePanelPlugin *>(getPluginByName("Theme"));
        QString theme = "Dark";
        /************************ Mod by sunchengxi 2020-09-16:Bug#48226#48230#48236#48241 终端默认主题色应改为深色修改引起的系列问题修复 Begin************************/
        //Mod by sunchengxi 2020-09-17:Bug#48349 主题色选择跟随系统异常
        if (builtInTheme == DGuiApplicationHelper::LightType) {
            theme = "Light";
        }
        /************************ Mod by sunchengxi 2020-09-16:Bug#48226#48230#48236#48241 终端默认主题色应改为深色修改引起的系列问题修复 End ************************/
        setColorScheme(theme);
        Settings::instance()->setColorScheme(theme);
    });

    // 未找到搜索的匹配结果
    connect(this, &QTermWidget::sig_noMatchFound, this, [this]() {
        parentPage()->setMismatchAlert(true);
    });
    // 找到搜索匹配的结果 => 记录查找时间 => 打印日志，方便性能测试
    connect(this, &QTermWidget::sig_matchFound, this, [this](){
        parentPage()->printSearchCostTime();
    });
    /********************* Modify by n014361 wangpeili End ************************/

    connect(this, &QTermWidget::isTermIdle, this, [this](bool bIdle) {
        emit termIsIdle(getSessionId(), bIdle);
    });

    TermWidgetPage *parentPage = qobject_cast<TermWidgetPage *>(parent);
    //qDebug() << parentPage << endl;
    connect(this, &QTermWidget::uninstallTerminal, parentPage, &TermWidgetPage::uninstallTerminal);
    /******** Modify by ut000610 daizhengwen 2020-06-11: 启动成功，则允许下一个窗口创建****************/
//    connect(this, &TermWidget::processStarted, Service::instance(), [ = ]() {
//        QTimer::singleShot(200, this, [ = ]() {
//            Service::instance()->setMemoryEnable(true);
//        });
//    });
    /********************* Modify by ut000610 daizhengwen End ************************/

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

    connect(this, &QTermWidget::titleChanged, this, [this] { emit termTitleChanged(TermWidget::title()); });
    connect(this, &TermWidget::copyAvailable, this, [this](bool enable) {
        if (Settings::instance()->IsPasteSelection() && enable) {
            qDebug() << "hasCopySelection";
            QString strSelected = selectedText();
            QApplication::clipboard()->setText(strSelected, QClipboard::Clipboard);
        }
    });
    connect(Settings::instance(), &Settings::terminalSettingChanged, this, &TermWidget::onSettingValueChanged);

    //窗口特效开启则使用设置的透明度，窗口特效关闭时直接把窗口置为不透明
    connect(Service::instance(), &Service::onWindowEffectEnabled, this, [this](bool isWinEffectEnabled) {
        if (isWinEffectEnabled) {
            this->setTermOpacity(Settings::instance()->opacity());
        } else {
            this->setTermOpacity(1.0);
        }
    });

    // 接收触控板事件
    connect(Service::instance(), &Service::touchPadEventSignal, this, &TermWidget::onTouchPadSignal);

    setFocusPolicy(Qt::NoFocus);
}

TermWidget::~TermWidget()
{

    // 窗口减1
    WindowsManager::instance()->terminalCountReduce();
}

/*******************************************************************************
 1. @函数:    parentPage
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    获取父页面
*******************************************************************************/
TermWidgetPage *TermWidget::parentPage()
{
    //qDebug() << "parentPage" << parentWidget();
    return  m_page;
}

/*******************************************************************************
 1. @函数:    handleTermIdle
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    处理终端空闲
*******************************************************************************/
void TermWidget::handleTermIdle(bool bIdle)
{
    emit termIsIdle(this->getSessionId(), bIdle);
}

/*** 修复 bug 28162 鼠标左右键一起按终端会退出 ***/
/*******************************************************************************
 1. @函数:    addMenuActions
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    添加菜单操作
*******************************************************************************/
void TermWidget::addMenuActions(const QPoint &pos)
{
    bool isRemoting = isInRemoteServer();

    QList<QAction *> termActions = filterActions(pos);
    for (QAction *&action : termActions) {
        m_menu->addAction(action);
    }

    if (!m_menu->isEmpty()) {
        m_menu->addSeparator();
    }

    // add other actions here.
    if (!selectedText().isEmpty()) {

        m_menu->addAction(tr("Copy"), this, [this] { copyClipboard(); });
    }
    if (!QApplication::clipboard()->text(QClipboard::Clipboard).isEmpty()) {
        m_menu->addAction(tr("Paste"), this, [this] { pasteClipboard(); });
    }
    /******** Modify by n014361 wangpeili 2020-02-26: 添加打开(文件)菜单功能 **********/
    if (!isRemoting && !selectedText().isEmpty()) {
        QFileInfo tempfile(workingDirectory() + "/" + selectedText());
        if (tempfile.exists()) {
            m_menu->addAction(tr("Open"), this, [this] {
                QString file = workingDirectory() + "/" + selectedText();
                QString cmd = QString("xdg-open ") + file;
                //在linux下，可以通过system来xdg-open命令调用默认程序打开文件；
                system(cmd.toStdString().c_str());
                // qDebug() << file << " open";
            });
        }
    }
    /********************* Modify by n014361 wangpeili End ************************/

    m_menu->addAction(tr("Open in file manager"), this, [this] {
        //DDesktopServices::showFolder(QUrl::fromLocalFile(workingDirectory()));

        //打开文件夹的方式 和  打开文件夹 并勾选文件的方式 如下
        //dde-file-manager -n /data/home/lx777/my-wjj/git/2020-08/18-zoudu/build-deepin-terminal-unknown-Debug
        //dde-file-manager --show-item a.pdf

        QProcess process;
        //未选择内容
        if (selectedText().isEmpty())
        {
            process.startDetached("dde-file-manager -n " + workingDirectory());
            return;
        }

        QFileInfo fi(workingDirectory() + "/" + selectedText());
        //选择的内容是文件或者文件夹
        if (fi.isFile() || fi.isDir())
        {
            process.startDetached("dde-file-manager --show-item " + workingDirectory() + "/" + selectedText());
            return;
        }
        //选择的文本不是文件也不是文件夹
        process.startDetached("dde-file-manager -n " + workingDirectory());

    });

    m_menu->addSeparator();


    Qt::Orientation orientation = static_cast<DSplitter *>(parentWidget())->orientation();
    int layer = getTermLayer();

    if (layer == 1 || (layer == 2 && orientation == Qt::Horizontal)) {
        m_menu->addAction(tr("Horizontal split"), this, [this] {
            getTermLayer();
            // menu关闭与分屏同时进行时，会导致QT计算光标位置异常。
            QTimer::singleShot(10, this, [ = ]()
            {
                parentPage()->split(Qt::Horizontal);
            });

        });
    }
    if (layer == 1 || (layer == 2 && orientation == Qt::Vertical)) {
        m_menu->addAction(tr("Vertical split"), this, [this] {
            getTermLayer();
            // menu关闭与分屏同时进行时，会导致QT计算光标位置异常。
            QTimer::singleShot(10, this, [ = ]()
            {
                parentPage()->split(Qt::Vertical);
            });
        });
    }

    /******** Modify by n014361 wangpeili 2020-02-21: 增加关闭窗口和关闭其它窗口菜单    ****************/
    m_menu->addAction(tr("Close window"), this, [this] {
        parentPage()->closeSplit(parentPage()->currentTerminal());
    });
    //m_menu->addAction(tr("Close Window"), this, [this] { ((TermWidgetPage *)m_Page)->close();});
    if (parentPage()->getTerminalCount() > 1) {
        m_menu->addAction(tr("Close other windows"), this, [this] {
            parentPage()->closeOtherTerminal();
        });
    };

    /********************* Modify by n014361 wangpeili End ************************/
    m_menu->addSeparator();
    m_menu->addAction(tr("New workspace"), this, [this] {
        parentPage()->parentMainWindow()->createNewWorkspace();
    });

    m_menu->addSeparator();

    if (!parentPage()->parentMainWindow()->isQuakeMode()) {
        bool isFullScreen = this->window()->windowState().testFlag(Qt::WindowFullScreen);
        if (isFullScreen) {
            m_menu->addAction(tr("Exit fullscreen"), this, [this] {
                parentPage()->parentMainWindow()->switchFullscreen();
            });
        } else {
            m_menu->addAction(tr("Fullscreen"), this, [this] {
                parentPage()->parentMainWindow()->switchFullscreen();
            });
        }
    }

    m_menu->addAction(tr("Find"), this, [this] {
        parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_SEARCHBAR);
    });
    m_menu->addSeparator();

    if (!selectedText().isEmpty()) {
        DMenu *search = new DMenu(tr("Search"), this);

        search->addAction("Bing", this, [this] {
            QString strurl = "https://cn.bing.com/search?q=" + selectedText();
            QDesktopServices::openUrl(QUrl(strurl));
        });
        search->addAction("Baidu", this, [this] {
            QString strurl = "https://www.baidu.com/s?wd=" + selectedText();
            QDesktopServices::openUrl(QUrl(strurl));
        });
        search->addAction("Github", this, [this] {
            QString strurl = "https://github.com/search?q=" + selectedText();
            QDesktopServices::openUrl(QUrl(strurl));
        });
        search->addAction("Stack Overflow", this, [this] {
            QString strurl = "https://stackoverflow.com/search?q=" + selectedText();
            QDesktopServices::openUrl(QUrl(strurl));
        });
        m_menu->addMenu(search);
        m_menu->addSeparator();
    }

    m_menu->addAction(tr("Rename title"), this, [this] {
        //QString currTabTitle = this->property("currTabTitle").toString();
        /************************ Mod by sunchengxi 2020-04-30:分屏修改标题异常问题 Begin************************/
        QString currTabTitle = parentPage()->parentMainWindow()->getCurrTabTitle();
        if (currTabTitle.isEmpty())
        {
            currTabTitle = this->property("currTabTitle").toString();
        }
        /************************ Mod by sunchengxi 2020-04-30:分屏修改标题异常问题 End  ************************/
        if (currTabTitle.isNull())
        {
            currTabTitle = this->title();
        }
        qDebug() << "currTabTitle" << currTabTitle << endl;
        parentPage()->showRenameTitleDialog(currTabTitle);
    });

    m_menu->addAction(tr("Encoding"), this, [this] {
        parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);
    });

    m_menu->addAction(tr("Custom commands"), this, [this] {
        parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
    });

    m_menu->addAction(tr("Remote management"), this, [this] {
        parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);
    });

    if (isInRemoteServer()) {
        m_menu->addSeparator();
        m_menu->addAction(tr("Upload file"), this, [this] {
            parentPage()->parentMainWindow()->remoteUploadFile();
        });
        m_menu->addAction(tr("Download file"), this, [this] {
            parentPage()->parentMainWindow()->remoteDownloadFile();
        });
    }

    m_menu->addSeparator();

    m_menu->addAction(tr("Settings"), this, [ = ] {
        Service::instance()->showSettingDialog(parentPage()->parentMainWindow());
    });

    // 判断是否到达最大数量
    if (!Service::instance()->isCountEnable()) {
        QList<QAction *> actionList = m_menu->actions();
        for (auto item : actionList) {
            if (item->text() == tr("New workspace") || item->text() == tr("Horizontal split") || item->text() == tr("Vertical split")) {
                // 无法点击的菜单项置灰
                item->setEnabled(false);
            }
        }
    }
}

/*******************************************************************************
 1. @函数:    RemoteEncode
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    获取远程管理的编码
*******************************************************************************/
QString TermWidget::RemoteEncode() const
{
    return m_remoteEncode;
}

/*******************************************************************************
 1. @函数:    setRemoteEncode
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置远程管理的编码
*******************************************************************************/
void TermWidget::setRemoteEncode(const QString &RemoteEncode)
{
    m_remoteEncode = RemoteEncode;
}

/*******************************************************************************
 1. @函数:    setBackspaceMode
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-06-02
 4. @说明:    根据设置的模式改变退格模式
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    setDeleteMode
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-06-02
 4. @说明:    根据设置的模式改变删除模式
*******************************************************************************/
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
/*******************************************************************************
 1. @函数:    getTermLayer
 2. @作者:    ut000439 王培利
 3. @日期:    2020-06-06
 4. @说明:    获取当前terminal距离page的层次．用于限定分屏．
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    encode
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    获取编码
*******************************************************************************/
QString TermWidget::encode() const
{
    return m_encode;
}

/*******************************************************************************
 1. @函数:    setEncode
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置编码
*******************************************************************************/
void TermWidget::setEncode(const QString &encode)
{
    m_encode = encode;
}

/*******************************************************************************
 1. @函数:    isConnectRemote
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    是否连接远程管理
*******************************************************************************/
bool TermWidget::isConnectRemote() const
{
    return m_isConnectRemote;
}

/*******************************************************************************
 1. @函数:    setIsConnectRemote
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置连接远程管理
*******************************************************************************/
void TermWidget::setIsConnectRemote(bool isConnectRemote)
{
    m_isConnectRemote = isConnectRemote;
}

/*******************************************************************************
 1. @函数:    enterSzCommand
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    是否进行下载
*******************************************************************************/
bool TermWidget::enterSzCommand() const
{
    return m_enterSzCommand;
}

/*******************************************************************************
 1. @函数:    setEnterSzCommand
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置进行下载
*******************************************************************************/
void TermWidget::setEnterSzCommand(bool enterSzCommand)
{
    m_enterSzCommand = enterSzCommand;
}

/*******************************************************************************
 1. @函数:    customContextMenuCall
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-08-11
 4. @说明:    自定义上下文菜单调用
*******************************************************************************/
void TermWidget::customContextMenuCall(const QPoint &pos)
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

/*******************************************************************************
 1. @函数:    isInRemoteServer
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    是否在远程服务器中
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    setTermOpacity
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置不透明度
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::setTerminalFont(const QString &fontName)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     设置字体
*******************************************************************************/
void TermWidget::setTermFont(const QString &fontName)
{
    QFont font = getTerminalFont();
    font.setFamily(fontName);
    setTerminalFont(font);
}

/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::setTerminalFontSize(const int fontSize)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     设置字体大小
*******************************************************************************/
void TermWidget::setTermFontSize(const int fontSize)
{
    QFont font = getTerminalFont();
    font.setFixedPitch(true);
    font.setPointSize(fontSize);
    setTerminalFont(font);
}


/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::skipToNextCommand()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:    跳转到下一个命令（这个功能没找到库的接口，现在是暂时是以虚拟键形式实现）
*******************************************************************************/
void TermWidget::skipToNextCommand()
{
    qDebug() << "skipToNextCommand";
}

/*******************************************************************************
 1. @函数:  void TermWidgetWrapper::skipToPreCommand()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:   跳转到前一个命令（这个功能没找到库的接口，现在是暂时是以虚拟键形式实现）
*******************************************************************************/
void TermWidget::skipToPreCommand()
{
    qDebug() << "skipToPreCommand";
}

/*******************************************************************************
 1. @函数:  void TermWidgetWrapper::setCursorShape(int shape)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     设置光标形状
*******************************************************************************/
void TermWidget::setCursorShape(int shape)
{
    Konsole::Emulation::KeyboardCursorShape cursorShape = Konsole::Emulation::KeyboardCursorShape(shape);
    setKeyboardCursorShape(cursorShape);
}

/*******************************************************************************
 1. @函数:    setPressingScroll
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    设置按键时是否滚动
*******************************************************************************/
void TermWidget::setPressingScroll(bool enable)
{
    if (enable) {
        setMotionAfterPasting(2);
    } else {
        setMotionAfterPasting(0);
    }
}

/*******************************************************************************
 1. @函数:    selectEncode
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    选择编码
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::pasteSelection()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     粘贴选择内容
*******************************************************************************/
void TermWidget::wpasteSelection()
{
    int x1, y1, x2, y2;
    getSelectionStart(x1, y1);
    qDebug() << x1 << y1;
    getSelectionEnd(x2, y2);
    qDebug() << x2 << y2;

    pasteSelection();
}

/*******************************************************************************
 1. @函数:    onSettingValueChanged
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-20
 4. @说明:    Terminal的各项设置生效
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    onTouchPadSignal
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-19
 4. @说明:    处理触控板事件
 1) 参数 name      : 触控板事件类型(手势或者触摸类型) pinch 捏 tap 敲 swipe 右键单击 单键
 2) 参数 direction : 手势方向 触控板上 up 触控板下 down 左 left 右 right 无 none 向内 in 向外 out
                    触控屏上 top 触摸屏下 bot
 3) 参数 fingers   : 手指数量 (1,2,3,4,5)
 注意libinput接收到触摸板事件后将接收到的数据通过Event广播出去
*******************************************************************************/
void TermWidget::onTouchPadSignal(QString name, QString direction, int fingers)
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

/*******************************************************************************
 1. @函数:    wheelEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-09-08
 4. @说明:    支持Ctrl + 滚轮上下事件
 Ctrl+滚轮上 放大
 Ctrl+滚轮下 缩小
*******************************************************************************/
void TermWidget::wheelEvent(QWheelEvent *event)
{
    // 当前窗口被激活,且有焦点
    if (isActiveWindow() && hasFocus()) {
        if (event->modifiers() == Qt::ControlModifier) {
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

