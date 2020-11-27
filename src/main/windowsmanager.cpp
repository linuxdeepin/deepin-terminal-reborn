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
#include "windowsmanager.h"
#include "dbusmanager.h"
#include "utils.h"
#include "service.h"
#include "define.h"

#include <QDebug>

WindowsManager *WindowsManager::pManager = new WindowsManager();
WindowsManager *WindowsManager::instance()
{
    return  pManager;
}

/*******************************************************************************
 1. @函数:    runQuakeWindow
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    运行雷神窗口
*******************************************************************************/
void WindowsManager::runQuakeWindow(TermProperties properties)
{
    if (m_quakeWindow == nullptr) {
        qDebug() << "runQuakeWindow :create";
        m_quakeWindow = new QuakeWindow(properties);
        m_quakeWindow->show();
        m_quakeWindow->activateWindow();
        // 雷神创建的第一个时候，m_quakeWindow仍为null，需要在这里更正一下．
        Service::instance()->updateShareMemoryCount(m_quakeWindow == nullptr ? m_widgetCount : m_widgetCount - 1);
        return;
    }
    // Alt+F2的显隐功能实现点
    quakeWindowShowOrHide();
}

/*******************************************************************************
 1. @函数:    quakeWindowShowOrHide
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    雷神窗口显示或者隐藏
*******************************************************************************/
void WindowsManager::quakeWindowShowOrHide()
{
    //MainWindow *mainWindow = getMainWindow();
    qDebug() << "ShowOrHide" << m_quakeWindow->winId();

    // 没有显示，就显示．
    if (!m_quakeWindow->isVisible()) {
        qDebug() << "!mainWindow  isVisible now show !" << m_quakeWindow->winId();
        // 判断终端在原来窗口是否隐藏.原来窗口隐藏的情况下,显示
        if (!m_quakeWindow->isShowOnCurrentDesktop()) {
            m_quakeWindow->show();
        }
    }

    // 没有激活就激活
    if (!m_quakeWindow->isActiveWindow()) {
        qDebug() << "QuakeWindow is activate, now activateWindow" << m_quakeWindow->winId();
        int index = DBusManager::callKDECurrentDesktop();
        if (index != -1 && m_quakeWindow->getDesktopIndex() != index) {
            // 不在同一个桌面
            DBusManager::callKDESetCurrentDesktop(m_quakeWindow->getDesktopIndex());
        }
        m_quakeWindow->activateWindow();
        // fix#42497 防止隐藏显示后"+"号高亮
        m_quakeWindow->focusCurrentPage();
        return;
    }

    // 如果已经激活，那么就隐藏
    qDebug() << "isWinVisible mainWindow->isActiveWindow() : start hide" << m_quakeWindow->winId();
    // 雷神的普通对话框,不处理
    if (Service::instance()->getIsDialogShow()) {
        return;
    }
    // 雷神设置框显示,不处理
    if (Service::instance()->isSettingDialogVisible() && Service::instance()->getSettingOwner() == m_quakeWindow) {
        if (m_quakeWindow->isActiveWindow()) {
            Service::instance()->showSettingDialog(m_quakeWindow);
        }
        return;
    }
    // 隐藏雷神
    m_quakeWindow->hideQuakeWindow();

}

/*******************************************************************************
 1. @函数:    createNormalWindow
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    创建普通窗口
*******************************************************************************/
void WindowsManager::createNormalWindow(TermProperties properties)
{
    TermProperties newProperties = properties;
    if (m_normalWindowList.count() == 0) {
        newProperties[SingleFlag] = true;
    }
    MainWindow *newWindow = new NormalWindow(newProperties);
    m_normalWindowList << newWindow;
    qDebug() << "create NormalWindow, current count =" << m_normalWindowList.count()
             << ", SingleFlag" << newProperties[SingleFlag].toBool();
    newWindow->show();
    qint64 newMainWindowTime = newWindow->createNewMainWindowTime();
    QString strNewMainWindowTime = GRAB_POINT + LOGO_TYPE + CREATE_NEW_MAINWINDOE + QString::number(newMainWindowTime);
    qInfo() << qPrintable(strNewMainWindowTime);
}

/*******************************************************************************
 1. @函数:    onMainwindowClosed
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    主窗口关闭响应函数
*******************************************************************************/
void WindowsManager::onMainwindowClosed(MainWindow *window)
{
    /***add begin by ut001121 zhangmeng 20200527 关闭终端窗口时重置设置框所有者 修复BUG28636***/
    if (window == Service::instance()->getSettingOwner()) {
        Service::instance()->resetSettingOwner();
    }
    /***add end by ut001121 zhangmeng***/

    /***mod begin by ut001121 zhangmeng 20200617 应用程序主动控制退出 修复BUG33541***/
    if (window == m_quakeWindow) {
        Q_ASSERT(window->isQuakeMode() == true);
        m_quakeWindow = nullptr;
    } else if (m_normalWindowList.contains(window)) {
        Q_ASSERT(window->isQuakeMode() == false);
        m_normalWindowList.removeOne(window);
    } else {
        //Q_ASSERT(false);
        qDebug() << "unkown windows closed " << window;
    }

    window->deleteLater();

    // 程序退出判断 add by ut001121
    if (m_normalWindowList.size() == 0 && m_quakeWindow == nullptr) {
        qApp->quit();
    }
    /***mod end by ut001121***/
}

/*******************************************************************************
 1. @函数:    WindowsManager
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    窗口管理，空函数
*******************************************************************************/
WindowsManager::WindowsManager(QObject *parent) : QObject(parent)
{
    Utils::set_Object_Name(this);
}

/*******************************************************************************
 1. @函数:    widgetCount
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    终端界面计数
*******************************************************************************/
int WindowsManager::widgetCount() const
{
    if (nullptr == WindowsManager::instance()->getQuakeWindow()) {
        return m_widgetCount;
    } else {
        // 不将雷神统计在内
        return m_widgetCount - 1;
    }

}

/*******************************************************************************
 1. @函数:    terminalCountIncrease
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    终端界面计数增加
*******************************************************************************/
void WindowsManager::terminalCountIncrease()
{
    ++m_widgetCount;
    // 雷神首次创建的时候m_quakeWindow　= nullptr,　统计数据会多出来一个,后面流程会修正．
    Service::instance()->updateShareMemoryCount(m_quakeWindow == nullptr ? m_widgetCount : m_widgetCount - 1);
    qDebug() << "++ Terminals Count : " << m_widgetCount;
}

/*******************************************************************************
 1. @函数:    terminalCountReduce
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    终端界面计数减少
*******************************************************************************/
void WindowsManager::terminalCountReduce()
{
    --m_widgetCount;
    Service::instance()->updateShareMemoryCount(m_quakeWindow == nullptr ? m_widgetCount : m_widgetCount - 1);
    qDebug() << "-- Terminals Count : " << m_widgetCount;
}
