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
 1. @类名:    ServerConfigGroupPanel
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    远程管理分组界面
             主要有返回按钮，搜索框和分组列表构成
             搜索框只有两个以上的选项时才显示
             搜索结果为组内搜索
*******************************************************************************/
#ifndef SERVERCONFIGGROUPPANEL_H
#define SERVERCONFIGGROUPPANEL_H

#include "commonpanel.h"
#include "serverconfigmanager.h"

#include <QWidget>

class ListView;
class ServerConfigGroupPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit ServerConfigGroupPanel(QWidget *parent = nullptr);
    /**
     * @brief 服务器配置组面板刷新数据
     * @author ut000610 daizhengwen
     * @param groupName 分组名
     */
    void refreshData(const QString &groupName);
    /**
     * @brief 设置焦点返回的位置：position -1 搜索框；其他列表位置；返回键按键不是这边设置
     * @author ut000610 戴正文
     */
    void setFocusBack();
    /**
     * @brief 清除所有焦点
     * @author ut000610 戴正文
     */
    void clearAllFocus();

signals:
    // 显示搜索结果，搜索框有焦点
    void showSearchPanel(const QString &strFilter);
    void doConnectServer(ServerConfig *curServer);
    // 返回前一个界面
    void rebackPrevPanel();

public slots:
    /**
     * @brief 服务器配置组面板处理显示搜索到的结果
     * @author ut000610 daizhengwen
     */
    void handleShowSearchResult();
    /**
     * @brief 服务器配置组面板刷新搜索状态
     * @author ut000610 daizhengwen
     */
    void refreshSearchState();
    /**
     * @brief 列表项被点击， 连接远程
     * @author ut000610 戴正文
     * @param key 列表项
     */
    void onItemClicked(const QString &key);

    void onRefreshList();
    void onListViewFocusOut(Qt::FocusReason type);

private:
    /**
     * @brief 服务器配置组面板初始化UI界面
     * @author ut000610 daizhengwen
     */
    void initUI();

private:
    ListView *m_listWidget = nullptr;
    QString m_groupName;
};

#endif  // SERVERCONFIGGROUPPANEL_H
