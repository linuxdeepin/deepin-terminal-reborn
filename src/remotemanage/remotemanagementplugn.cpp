/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  daizhengwen<daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen<daizhengwen@uniontech.com>
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
#include "remotemanagementplugn.h"
#include "mainwindow.h"
#include "utils.h"
#include "service.h"

#include "../views/termwidget.h"//added by nyq

#include <QTextCodec>
#include <QDebug>

RemoteManagementPlugin::RemoteManagementPlugin(QObject *parent) : MainWindowPluginInterface(parent)
{
    Utils::set_Object_Name(this);
    m_pluginName = "Remote Management";
}

/*******************************************************************************
 1. @函数:    initPlugin
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    远程管理初始化插件
*******************************************************************************/
void RemoteManagementPlugin::initPlugin(MainWindow *mainWindow)
{
    qDebug() << "RemoteManagementPlugin init Plugin.";
    m_mainWindow = mainWindow;
    //initRemoteManagementTopPanel();
    connect(m_mainWindow, &MainWindow::showPluginChanged,  this, [ = ](const QString name, bool bSetFocus) {
        if (MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT != name) {
            // 判断窗口是否已经显示
            if (m_isShow) {
                // 若显示，则隐藏
                getRemoteManagementTopPanel()->hideAnim();
                m_isShow = false;
            }

        } else {
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，远程连接界面使用不方便，将雷神窗口变大适应正常的远程连接界面 Begin***************/
            if (m_mainWindow->isQuakeMode() && m_mainWindow->height() < LISTMINHEIGHT) {
                m_mainWindow->resize(m_mainWindow->width(), LISTMINHEIGHT); //首先设置雷神界面的大小
                m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);//重新打开远程连接界面，当前流程结束
                return;
            }
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，远程连接界面使用不方便，将雷神窗口变大适应正常的远程连接界面 End***************/
            getRemoteManagementTopPanel()->show();
            m_isShow = true;
            if (bSetFocus) {
                // 若焦点不在终端上，焦点落入平面
                getRemoteManagementTopPanel()->setFocusInPanel();
            } else {
                // 若焦点在终端上，焦点落回终端
                m_mainWindow->focusCurrentPage();
            }
        }
    });
    connect(m_mainWindow, &MainWindow::quakeHidePlugin, this, [ = ]() {
        // 焦点在列表上，隐藏时，焦点现在当前窗口上
        if (m_mainWindow->isFocusOnList()) {
            m_mainWindow->focusCurrentPage();
            qDebug() << "focus on remote list, hide remote list and set foucs on terminal";
        }
        getRemoteManagementTopPanel()->hide();
    });
}

/*******************************************************************************
 1. @函数:    titlebarMenu
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    远程管理标题栏菜单
*******************************************************************************/
QAction *RemoteManagementPlugin::titlebarMenu(MainWindow *mainWindow)
{
    QAction *remoteManagementAction(new QAction(tr("Remote management"), mainWindow));

    connect(remoteManagementAction, &QAction::triggered, mainWindow, [mainWindow]() {
        mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);
    });
    return remoteManagementAction;
}

/*******************************************************************************
 1. @函数:    getRemoteManagementTopPanel
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    获取远程管理顶部面板
*******************************************************************************/
RemoteManagementTopPanel *RemoteManagementPlugin::getRemoteManagementTopPanel()
{
    if (nullptr == m_remoteManagementTopPanel) {
        initRemoteManagementTopPanel();
    }
    return m_remoteManagementTopPanel;
}

/*******************************************************************************
 1. @函数:    initRemoteManagementTopPanel
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    初始化远程管理顶部面板
*******************************************************************************/
void RemoteManagementPlugin::initRemoteManagementTopPanel()
{
    qDebug() << __FUNCTION__;
    m_remoteManagementTopPanel = new RemoteManagementTopPanel(m_mainWindow->centralWidget());
    m_remoteManagementTopPanel->setObjectName("RemoteManagementTopPanel");
    connect(m_remoteManagementTopPanel,
            &RemoteManagementTopPanel::doConnectServer,
            this,
            &RemoteManagementPlugin::doCennectServer, Qt::QueuedConnection);
}

/*******************************************************************************
 1. @函数:    doCennectServer
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    远程管理做连接服务器
*******************************************************************************/
void RemoteManagementPlugin::doCennectServer(ServerConfig *curServer)
{
    qDebug() << "RemoteManagementPlugin do connect server.";
    if (nullptr != curServer) {

        QString shellFile = createShellFile(curServer);
        QString strTxt = "expect -f " + shellFile + "\n";
        //--added by qinyaning(nyq) to solve the probelm which Connecting to the remote server
        /*does not connect to the remote server directly in the new TAB. time: 2020.4.13 18:15
         * */
        if (m_mainWindow->currentPage()->currentTerminal()->hasRunningProcess()) {
            m_mainWindow->addTab(m_mainWindow->currentPage()->createCurrentTerminalProperties(), true);
        }
        //--------------------------------//
        /******** Modify by m000714 daizhengwen 2020-04-30: 将当前还没执行的命令清空****************/
        m_mainWindow->focusCurrentPage();
        m_mainWindow->pressCtrlU();
        m_mainWindow->sleep(100);
        /********************* Modify by m000714 daizhengwen End ************************/
        m_mainWindow->currentPage()->sendTextToCurrentTerm(strTxt);
        // 等待连接 100ms等待命令发过去正常立即执行，100ms足够，一下的信号槽只是判断是否开启另一个程序去连接
        // 若有程序去连接，则判断已连接，若连接失败，则判断为断开连接
        QTimer::singleShot(100, this, [ = ]() {
            TermWidget *term = m_mainWindow->currentPage()->currentTerminal();
            if (!term) {
                // 若term为空
                qDebug() << "current terminal is null";
            }
            // 判断是否连接服务器
            if (!term->isInRemoteServer()) {
                // 没有连接上
                qDebug() << "disconnect to server";
                return;
            }
            // 标记此term连接远程
            term->setIsConnectRemote(true);
            qDebug() << "connect to server";
            // 编码
            setRemoteEncode(curServer->m_encoding);
            // 退格键
            setBackspaceKey(term, curServer->m_backspaceKey);
            // 删除键
            setDeleteKey(term, curServer->m_deleteKey);
        });

    }
    /******** Modify by ut000610 daizhengwen 2020-06-04: 点击连接服务器后，隐藏列表，焦点回到主窗口****************/
    m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_NONE);
    // 隐藏列表后，将焦点设置到主窗口 100ms足够
    QTimer::singleShot(100, this, [&]() {
        if (m_mainWindow->isActiveWindow()) {
            m_mainWindow->focusCurrentPage();
        }
    });
    /********************* Modify by ut000610 daizhengwen End ************************/
}

/*******************************************************************************
 1. @函数:    createShellFile
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-31
 4. @说明:    创建连接远程的的临时shell文件
*******************************************************************************/
QString RemoteManagementPlugin::createShellFile(ServerConfig *curServer)
{
    qDebug() << "RemoteManagementPlugin create temporary shell file.";
    // 首先读取通用模板
    QFile sourceFile(":/other/ssh_login.sh");
    QString fileString;
    // 打开文件
    if (sourceFile.open(QIODevice::ReadOnly)) {
        //读取文件
        fileString = sourceFile.readAll();
        // 关闭文件
        sourceFile.close();
    }

    QString strArgs = QString(" <<USER>> <<SERVER>> <<PORT>> <<PRIVATE_KEY>> <<PASSWORD>>");
    // 用远程管理数据替换文件内的关键字
    strArgs.replace("<<USER>>", curServer->m_userName);
    strArgs.replace("<<SERVER>>", curServer->m_address.trimmed());
    strArgs.replace("<<PORT>>", curServer->m_port);
    // 根据是否有证书，替换关键字 // pass_way仅仅是占位
    if (curServer->m_privateKey.isNull() || curServer->m_privateKey.isEmpty()) {
        fileString.replace("<<AUTHENTICATION>>", "no");
        strArgs.replace("<<PRIVATE_KEY>>", "NoPrivateKeyPath");
        strArgs.replace("<<PASSWORD>>", curServer->m_password);
    } else {
        fileString.replace("<<AUTHENTICATION>>", "yes");
        strArgs.replace("<<PRIVATE_KEY>>", curServer->m_privateKey);
        strArgs.replace("<<PASSWORD>>", "");
    }

    QString path = curServer->m_path;
    QString command = curServer->m_command;
    // 添加远程提示
    QString remote_command = "echo " + tr("Make sure that rz and sz commands have been installed in the server before right clicking to upload and download files.") + " && ";
    if (!path.isNull() && !path.isEmpty()) {
        remote_command = remote_command + "cd " + path + " && ";
    }
    if (!command.isNull() && !command.isEmpty()) {
        remote_command = remote_command + command + " && ";
    }
    fileString.replace("<<REMOTE_COMMAND>>", remote_command);

    // 创建临时文件和执行脚本所需要的参数
    QString toFileStr = "/tmp/terminal-" + Utils::getRandString();
    QFile toFile(toFileStr);
    toFile.open(QIODevice::WriteOnly | QIODevice::Text);
    // 写文件
    toFile.write(fileString.toUtf8());
    // 退出文件
    toFile.close();
    return QString(toFileStr + strArgs);
}

/*******************************************************************************
 1. @函数:    setRemoteEncode
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置远程管理编码
*******************************************************************************/
void RemoteManagementPlugin::setRemoteEncode(QString encode)
{
    TermWidget *term = m_mainWindow->currentPage()->currentTerminal();
    if (!encode.isNull() && !encode.isEmpty()) {
        // 设置当前窗口的编码
        term->setTextCodec(QTextCodec::codecForName(encode.toLocal8Bit()));
        qDebug() << "Remote encode " << encode;
    }
    // 记录远程编码
    term->setRemoteEncode(encode);
    // 切换编码列表的编码
    emit Service::instance()->checkEncode(encode);
}

/*******************************************************************************
 1. @函数:    setBackspaceKey
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-31
 4. @说明:    设置退格键的模式
*******************************************************************************/
void RemoteManagementPlugin::setBackspaceKey(TermWidget *term, QString backspaceKey)
{
    if (backspaceKey == "control-h") {
        term->setBackspaceMode(EraseMode_Control_H);
    } else if (backspaceKey == "auto") {
        term->setBackspaceMode(EraseMode_Auto);
    } else if (backspaceKey == "escape-sequence") {
        term->setBackspaceMode(EraseMode_Escape_Sequeue);
    } else if (backspaceKey == "ascii-del") {
        term->setBackspaceMode(EraseMode_Ascii_Delete);
    } else if (backspaceKey == "tty") {
        term->setBackspaceMode(EraseMode_TTY);
    }
    qDebug() << "backspace mode " << backspaceKey;
}

/*******************************************************************************
 1. @函数:    setDeleteKey
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-31
 4. @说明:    设置删除键的模式
*******************************************************************************/
void RemoteManagementPlugin::setDeleteKey(TermWidget *term, QString deleteKey)
{
    if (deleteKey == "control-h") {
        term->setDeleteMode(EraseMode_Control_H);
    } else if (deleteKey == "auto") {
        term->setDeleteMode(EraseMode_Auto);
    } else if (deleteKey == "escape-sequence") {
        term->setDeleteMode(EraseMode_Escape_Sequeue);
    } else if (deleteKey == "ascii-del") {
        term->setDeleteMode(EraseMode_Ascii_Delete);
    } else if (deleteKey == "tty") {
        term->setDeleteMode(EraseMode_TTY);
    }
    qDebug() << "delete mode " << deleteKey;
}
