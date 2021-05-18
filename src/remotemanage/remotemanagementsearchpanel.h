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
 1. @类名:    RemoteManagementSearchPanel
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    远程管理搜索界面
             主要由返回按钮， 搜索条件显示框和搜索结果列表构成
*******************************************************************************/
#ifndef REMOTEMANAGEMENTSEARCHPANEL_H
#define REMOTEMANAGEMENTSEARCHPANEL_H

#include "commonpanel.h"
#include "serverconfigmanager.h"

#include <DGuiApplicationHelper>

#include <QWidget>

DGUI_USE_NAMESPACE

class ListView;
class RemoteManagementSearchPanel : public CommonPanel
{
    Q_OBJECT
public:

    explicit RemoteManagementSearchPanel(QWidget *parent = nullptr);
    /**
     * @brief 远程管理搜索面板按组和过滤器刷新数据
     * @author ut000610 daizhengwen
     * @param strGroup 组
     * @param strFilter 过滤器
     */
    void refreshDataByGroupAndFilter(const QString &strGroup, const QString &strFilter);
    /**
     * @brief 远程管理搜索面板按过滤器刷新数据
     * @author ut000610 daizhengwen
     * @param strFilter 过滤器
     */
    void refreshDataByFilter(const QString &strFilter);
    /**
     * @brief 清除界面所有的焦点
     * @author ut000610 戴正文
     */
    void clearAllFocus();
    /**
     * @brief 设置焦点返回，从分组返回
     * @author ut000610 戴正文
     * @param strGroup 组
     * @param isFocusOn
     * @param prevIndex
     */
    void setFocusBack(const QString &strGroup, bool isFocusOn, int prevIndex);
    /**
     * @brief 获取列表当前的焦点
     * @author ut000610 戴正文
     * @return
     */
    int getListIndex();

signals:
    // 连接远程
    void doConnectServer(ServerConfig *curItemServer);
    // 显示远程分组界面
    void showGroupPanel(const QString &strGroup, bool isFocusOn);
    // 返回前一个页面
    void rebackPrevPanel();

public slots:
    /**
     * @brief 远程项被点击，连接远程
     * @author ut000610 戴正文
     * @param key
     */
    void onItemClicked(const QString &key);
    /**
     * @brief 焦点从列表中出的事件
     * @author ut000610 戴正文
     * @param type 类型
     */
    void onFocusOutList(Qt::FocusReason type);
    /**
     * @brief 处理刷新列表信号
     * @author ut000610 戴正文
     */
    void onRefreshList();

    void handleThemeTypeChanged(DGuiApplicationHelper::ColorType themeType);
    void handleListViewFocusOut(Qt::FocusReason type);

private:
    /**
     * @brief 远程管理搜索面板初始化UI界面
     * @author ut000610 daizhengwen
     */
    void initUI();
    /**
     * @brief 设置远程管理搜索的过滤器
     * @author ut000610 daizhengwen
     * @param filter 过滤器
     */
    void setSearchFilter(const QString &filter);

private:
    ListView *m_listWidget = nullptr;
    QString m_strGroupName;
    QString m_strFilter;
    // 判断是组内搜索还是组外搜索
    bool m_isGroupOrNot = false;
};

#endif  // REMOTEMANAGEMENTSEARCHPANEL_H
