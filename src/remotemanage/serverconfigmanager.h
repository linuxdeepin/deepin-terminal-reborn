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
 1. @类名:    ServerConfigManager
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    远程管理的数据管理类
             负责初始化配置文件的数据
             负责对配置文件数据的增删改操作
             负责管理对数据操作的对话框，负责同步对话框的数据
*******************************************************************************/
#ifndef SERVERCONFIGMANAGER_H
#define SERVERCONFIGMANAGER_H

#include "settingio.h"

#include <QList>
#include <QMap>
#include <QObject>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QMutex>

struct ServerConfig {
    QString m_serverName;
    QString m_address;
    QString m_userName;
    QString m_password;
    QString m_privateKey;
    QString m_port;
    QString m_group;
    QString m_path;
    QString m_command;
    QString m_encoding;
    QString m_backspaceKey;
    QString m_deleteKey;
};

// 旧版本groupName的count
#define OLD_VERTIONCOUNT 3
// 新版本groupName的count
#define NEW_VERTIONCOUNT 4


class ServerConfigOptDlg;
class ListView;
class ServerConfigManager : public QObject
{
    Q_OBJECT
public:

    // 面板类型
    enum PanelType {
        // 初始界面
        PanelType_Manage = 0,
        // 分组界面
        PanelType_Group,
        // 搜索界面
        PanelType_Search
    };

    static ServerConfigManager *instance();
    ~ServerConfigManager();
    /**
     * @brief 初始化服务器配置
     * @author ut000610 daizhengwen
     */
    void initServerConfig();
    /**
     * @brief 保存服务器配置
     * @author ut000610 daizhengwen
     * @param config 服务器配置
     */
    void saveServerConfig(ServerConfig *config);
    /**
     * @brief 删除服务器配置
     * @author ut000610 daizhengwen
     * @param config 服务器配置
     */
    void delServerConfig(ServerConfig *config);
    /**
     * @brief 修改服务器配置
     * @author ut000610 daizhengwen
     * @param newConfig 新配置
     * @param oldConfig 老配置
     */
    void modifyServerConfig(ServerConfig *newConfig, ServerConfig *oldConfig);
    /**
     * @brief 获取服务器配置
     * @author ut000610 daizhengwen
     * @return
     */
    QMap<QString, QList<ServerConfig *>> &getServerConfigs();

    /**
     * @brief refreshServerList
     * @author ut000610 戴正文
     * @param type 初始界面 PanelType_Manage 分组界面；PanelType_Group 搜索界 PanelType_Search
     * @param listview 所需填充的列表
     * @param filter 搜索界面的搜索条件
     * @param group 分组界面的组名
     */
    void refreshServerList(PanelType type, ListView *listview, const QString &filter = "", const QString &group = "");
    /**
     * @brief 服务器设置修改对话框
     * @author ut000610 daizhengwen
     * @param key
     * @param dlg 服务器设置修改对话框
     */
    void setModifyDialog(QString key, ServerConfigOptDlg *dlg);
    /**
     * @brief 从m_serverConfigDialogMap中将dlg数据删除
     * @author ut000610 戴正文
     * @param dlg
     */
    void removeDialog(ServerConfigOptDlg *dlg);
    /**
     * @brief 同步弹窗数据
     * @author ut000610 daizhengwen
     * @param key
     * @param newConfig 新配置
     */
    void SyncData(QString key, ServerConfig *newConfig);
    /**
     * @brief 删除数据，关闭所有同类弹窗
     * @author ut000610 daizhengwen
     * @param key
     */
    void closeAllDialog(QString key);
    /**
     * @brief 根据组名获得组内服务器个数
     * @author ut000610 戴正文
     * @param strGroupName 组名
     * @return
     */
    int getServerCount(const QString &strGroupName);
    /**
     * @brief 根据关键值获取远程配置信息
     * @author ut000610 戴正文
     * @param key 关键值
     * @return
     */
    ServerConfig *getServerConfig(const QString &key);

    // 获取密码 => 查找密码
    /**
     * @brief 远程管理获得密码(异步方法)
     * @author ut000610 戴正文
     * @param userName 用户名
     * @param address 地址
     * @param port 端口
     * @param key
     */
    void remoteGetSecreats(const QString &userName, const QString &address, const QString &port, const QString &key);
    /**
     * @brief 保存密码
     * @author ut000610 戴正文
     * @param config
     */
    void remoteStoreSecreats(ServerConfig *config);
    /**
     * @brief 删除密码
     * @author ut000610 戴正文
     * @param userName 用户名
     * @param address 地址
     * @param port 端口
     */
    void remoteClearSecreats(const QString &userName, const QString &address, const QString &port);

    /**
     * @brief 转换数据
     * @author ut000610 戴正文
     */
    void ConvertData();

public slots:
    /**
     * @brief 处理查询密码操作的结果写入map
     * @author ut000610 戴正文
     * @param key
     * @param password 密码
     */
    void onLookupFinish(const QString &key, const QString &password);

Q_SIGNALS:
    // 刷新列表
    void refreshList();
    // 获取查询到的密码
    void lookupSerceats(const QString &key, const QString &password);

private:
    explicit ServerConfigManager(QObject *parent = nullptr);

    static ServerConfigManager *m_instance;

    // 避免重复数据用的数据结构,兼容旧版本
    QMap<QString, ServerConfig *> m_remoteConfigs;
    // 避免名称重复用的数据结构
    QMap<QString, int> m_remoteName;
    // 配置文件数据
    QMap<QString, QList<ServerConfig *>> m_serverConfigs;
    // 展示的弹窗的键值及弹窗的指针 <服务器名，同类服务器弹窗列表> 弹窗只需存储修改弹窗
    QMap<QString, QList<ServerConfigOptDlg *>> m_serverConfigDialogMap;
    // 添加服务器弹窗
    /**
     * @brief 写服务器配置文件
     * @author m000714 戴正文
     * @param commandsSettings
     * @param strGroupName 组名
     * @param config 服务器配置
     */
    inline void settServerConfig(USettings &commandsSettings, const QString &strGroupName, ServerConfig *config);
    /**
     * @brief 初始化主界面，将数据填充进去
     * @author ut000610 戴正文
     * @param listview 需要填充的列表
     */
    void fillManagePanel(ListView *listview);
    /**
     * @brief 填充搜索界面
     * @author ut000610 戴正文
     * @param listview 需要填充的列表
     * @param filter 需要过滤的条件
     * @param group 需要过滤的组：有组,组内搜索；没组，全局搜索
     */
    void fillSearchPanel(ListView *listview, const QString &filter, const QString &group = "");
    /**
     * @brief 根据分组填充列表
     * @author ut000610 戴正文
     * @param listview 列表
     * @param group 组名
     */
    void fillGroupPanel(ListView *listview, const QString &group);
    // 查找密码的个数
    int m_lookupCount = 0;
    // 是否重写配置文件
    bool m_rewriteConfig = false;
};

typedef QMap<QString, QList<ServerConfigOptDlg *>>::Iterator serverConfigDialogMapIterator;

#endif  // SERVERCONFIGMANAGER_H
