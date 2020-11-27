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
 1. @类名:    RemoteManagementPanel
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    远程管理初始界面，依次显示搜索框，远程列表，添加服务器按钮
             搜索框只有远程列表里的项超过两个才显示
             远程列表里的项分两类：
             1）分组项：该项内有同一分组的所有远程项
             2）远程项：该项内可以点击连接远程服务器
*******************************************************************************/
#ifndef REMOTEMANAGEMENTPANEL_H
#define REMOTEMANAGEMENTPANEL_H

#include "commonpanel.h"
#include "serverconfigoptdlg.h"
#include "listview.h"

#include <QWidget>

class RemoteManagementPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit RemoteManagementPanel(QWidget *parent = nullptr);
    void refreshPanel();
    // 设置焦点进入控件
    void setFocusInPanel();
    // 设置焦点从分组界面返回时的位置
    void setFocusBack(const QString &strGroup);
    // 清除
    void clearListFocus();
    // 获取列表中当前焦点的位置
    int getListIndex();

signals:
    // 显示搜索界面
    void showSearchPanel(const QString &strFilter);
    // 显示分组,根据布尔值判断是否有焦点
    void showGroupPanel(const QString &strGroup, bool isFocusOn);
    // 连接远程
    void doConnectServer(ServerConfig *curItemServer);

public slots:
    void showCurSearchResult();
    void showAddServerConfigDlg();
    void refreshSearchState();
    // 处理连接远程操作
    void onItemClicked(const QString &key);

private:
    void initUI();

private:
    ListView *m_listWidget = nullptr;
};

#endif  // REMOTEMANAGEMENTPANEL_H
