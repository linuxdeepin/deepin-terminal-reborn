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

/*******************************************************************************
 1. @类名:    RemoteManagementPlugn
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    远程管理插件管理类
             该类负责和主窗口交互，初始化远程管理的界面管理类
             初始化菜单按钮
             隐藏和显示窗口
             设置编码，退格键，删除键格式
*******************************************************************************/
#ifndef REMOTEMANAGEMENTPLUGN_H
#define REMOTEMANAGEMENTPLUGN_H

#include "mainwindowplugininterface.h"
#include "remotemanagementtoppanel.h"

#include <QObject>
#include <QWidget>

class TermWidget;
class MainWindow;
class RemoteManagementPlugin : public MainWindowPluginInterface
{
    Q_OBJECT
public:
    explicit RemoteManagementPlugin(QObject *parent = nullptr);
    /**
     * @brief 远程管理初始化插件
     * @author ut000610 daizhengwen
     * @param mainWindow 主窗口
     */
    void initPlugin(MainWindow *mainWindow) override;
    /**
     * @brief 远程管理标题栏菜单
     * @author ut000610 daizhengwen
     * @param mainWindow 主窗口
     * @return
     */
    QAction *titlebarMenu(MainWindow *mainWindow) override;

    /**
     * @brief 获取远程管理顶部面板
     * @author ut000610 daizhengwen
     * @return
     */
    RemoteManagementTopPanel *getRemoteManagementTopPanel();
    /**
     * @brief 初始化远程管理顶部面板
     * @author ut000610 daizhengwen
     */
    void initRemoteManagementTopPanel();
    /**
     * @brief 创建连接远程的临时shell文件
     * @author ut000610 戴正文
     * @param curServer
     * @return
     */
    QString createShellFile(ServerConfig *curServer);

    /**
     * @brief 设置远程管理编码
     * @author ut000610 daizhengwen
     * @param encode 编码
     */
    void setRemoteEncode(QString encode);
    /**
     * @brief 设置退格键的模式
     * @author ut000610 戴正文
     * @param term
     * @param backspaceKey
     */
    void setBackspaceKey(TermWidget *term, QString backspaceKey);
    /**
     * @brief 设置删除键的模式
     * @author ut000610 戴正文
     * @param term
     * @param deleteKey
     */
    void setDeleteKey(TermWidget *term, QString deleteKey);
signals:
    void doHide();

public slots:
    /**
     * @brief 远程管理做连接服务器
     * @author ut000610 daizhengwen
     * @param curServer
     */
    void doCennectServer(ServerConfig *curServer);

private:
    /**
     * @brief 把字符串123转成形如313233的ASCII编码字符串
     * @author ut000438 王亮
     * @param strSrc 把字符串
     * @return
     */
    QString convertStringToAscii(const QString &strSrc);

    MainWindow *m_mainWindow = nullptr;
    RemoteManagementTopPanel *m_remoteManagementTopPanel = nullptr;
};

#endif  // REMOTEMANAGEMENTPLUGN_H
