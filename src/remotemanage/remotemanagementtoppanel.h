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
 1. @类名:    RemoteManagementTopPanel
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    远程管理界面管理类
             主要负责界面切换和动画效果
*******************************************************************************/
#ifndef REMOTEMANAGEMENTTOPPANEL_H
#define REMOTEMANAGEMENTTOPPANEL_H

#include "rightpanel.h"
#include "commonpanel.h"
#include "remotemanagementpanel.h"
#include "serverconfiggrouppanel.h"
#include "remotemanagementsearchpanel.h"
#include "serverconfigmanager.h"

#include <QPropertyAnimation>
#include <QWidget>
#include <QStack>

// 记录平面的状态
struct PanelState {
    // 平面的类型
    ServerConfigManager::PanelType m_type;
    // 平面的返回是否需要有焦点
    bool m_isFocusOn = false;
    // 若返回需要有焦点，则记录index
    int m_currentListIndex = -1;
};

class RemoteManagementTopPanel : public RightPanel
{
    Q_OBJECT
public:
    explicit RemoteManagementTopPanel(QWidget *parent = nullptr);
    /**
     * @brief 远程管理顶部面板显示
     * @author ut000610 daizhengwen
     */
    void show();
    /**
     * @brief 根据插件给的条件值设置焦点,键盘进入设置焦点
     * @author ut000610 戴正文
     */
    void setFocusInPanel();

public slots:
    /**
     * @brief 显示搜索界面：若当前界面为主界面 => 全局搜索;若当前界面为分组界面 => 组内搜索
     * @author ut000610 戴正文
     * @param strFilter
     */
    void showSearchPanel(const QString &strFilter);
    /**
     * @brief 显示分组界面
     * @author ut000610 戴正文
     * @param strGroupName 组名
     * @param isFocusOn 是否有焦点
     */
    void showGroupPanel(const QString &strGroupName, bool isFocusOn);
    /**
     * @brief 显示前一个界面（返回）
     * @author ut000610 戴正文
     */
    void showPrevPanel();

signals:
    void focusOut();
    void doConnectServer(ServerConfig *curServer);

private:
    // 远程主界面
    RemoteManagementPanel *m_remoteManagementPanel = nullptr;
    // 远程分组界面
    ServerConfigGroupPanel *m_serverConfigGroupPanel = nullptr;
    // 远程搜索界面
    RemoteManagementSearchPanel *m_remoteManagementSearchPanel = nullptr;
    // 搜索条件
    QStack<QString> m_filterStack;
    // 分组条件
    QString m_group;
    // 当前界面
    ServerConfigManager::PanelType m_currentPanelType;
    // 记录前一个界面
    QStack<PanelState> m_prevPanelStack;

    /**
     * @brief 设置平面显示状态
     * @author ut000610 戴正文
     * @param panelType
     */
    void setPanelShowState(ServerConfigManager::PanelType panelType);
    /**
     * @brief 远程管理顶部面板从左到右动画
     * @author ut000610 daizhengwen
     * @param animation
     * @param animation1
     */
    void panelLeftToRight(QPropertyAnimation *animation, QPropertyAnimation *animation1);
    /**
     * @brief 远程管理顶部面板从右到左动画
     * @author ut000610 daizhengwen
     * @param animation
     * @param animation1
     */
    void panelRightToLeft(QPropertyAnimation *animation, QPropertyAnimation *animation1);
};

#endif  // REMOTEMANAGEMENTTOPPANEL_H
