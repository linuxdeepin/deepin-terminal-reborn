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

#ifndef WINDOWSMANAGER_H
#define WINDOWSMANAGER_H
/*******************************************************************************
 1. @类名:    WindowsManager
 2. @作者:    ut000439 王培利
 3. @日期:    2020-05-19
 4. @说明:    全局窗口管理，维护mainwindow列表．
　　　　　　　　普通窗口的创建，关闭．
　　　　　　　　雷神窗口的创建，关闭，显与隐
*******************************************************************************/

#include "termproperties.h"
#include "mainwindow.h"

#include <QObject>

// 窗口最大数量
#define MAXWIDGETCOUNT 190

class WindowsManager : public QObject
{
    Q_OBJECT
public:
    static WindowsManager *instance();
    /**
     * @brief 运行雷神窗口
     * @author ut000439 wangpeili
     * @param properties 属性
     */
    void runQuakeWindow(TermProperties properties);
    /**
     * @brief 雷神窗口显示或者隐藏
     * @author ut000439 wangpeili
     */
    void quakeWindowShowOrHide();
    /**
     * @brief 创建普通窗口
     * @author ut000439 wangpeili
     * @param properties 属性
     */
    void createNormalWindow(TermProperties properties);

    /**
     * @brief 终端界面计数增加
     * @author ut000439 wangpeili
     */
    void terminalCountIncrease();
    /**
     * @brief 终端界面计数减少
     * @author ut000439 wangpeili
     */
    void terminalCountReduce();
    /**
     * @brief 终端界面计数
     * @author ut000439 wangpeili
     * @return
     */
    int widgetCount() const;
    // 获取雷神窗口
    MainWindow *getQuakeWindow()
    {
        return m_quakeWindow;
    }
    // 获取普通窗口列表
    QList<MainWindow *> getNormalWindowList()
    {
        return m_normalWindowList;
    }

signals:

public slots:
    /**
     * @brief 主窗口关闭响应函数
     * @author ut000439 wangpeili
     */
    void onMainwindowClosed(MainWindow *);
private:
    QList<MainWindow *> m_normalWindowList;
    QuakeWindow *m_quakeWindow = nullptr;
    TermWidgetPage *m_currentPage = nullptr;
private:
    /**
     * @brief 窗口管理，空函数
     * @author ut000439 wangpeili
     * @param parent
     */
    explicit WindowsManager(QObject *parent = nullptr);
    static WindowsManager *pManager;
    // 窗口数量
    int m_widgetCount = 0;
};

#endif // WINDOWSMANAGER_H
