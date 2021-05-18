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

void WindowsManager::runQuakeWindow(TermProperties properties)
{
    if (nullptr == m_quakeWindow) {
        qDebug() << "runQuakeWindow :create";
        m_quakeWindow = new QuakeWindow(properties);
        //Add by ut001000 renfeixiang 2020-11-16 设置开始雷神动画效果标志
        m_quakeWindow->setAnimationFlag(false);
        m_quakeWindow->show();
        //Add by ut001000 renfeixiang 2020-11-16 开始从上到下的动画
        m_quakeWindow->topToBottomAnimation();
        m_quakeWindow->activateWindow();
        // 雷神创建的第一个时候，m_quakeWindow仍为null，需要在这里更正一下．
        Service::instance()->updateShareMemoryCount(m_quakeWindow == nullptr ? m_widgetCount : m_widgetCount - 1);
        return;
    }
    // Alt+F2的显隐功能实现点
    quakeWindowShowOrHide();
}

void WindowsManager::quakeWindowShowOrHide()
{
    //隐藏 则 显示终端
    if (!m_quakeWindow->isVisible()) {
        m_quakeWindow->setAnimationFlag(false);
        m_quakeWindow->show();
        m_quakeWindow->topToBottomAnimation();
        m_quakeWindow->activateWindow();
        return;
    }

    //不在当前桌面显示时，切换到终端所在桌面
    int curDesktopIndex = DBusManager::callKDECurrentDesktop();
    if(curDesktopIndex != -1 && curDesktopIndex != m_quakeWindow->getDesktopIndex()) {
        //切换 桌面
        DBusManager::callKDESetCurrentDesktop(m_quakeWindow->getDesktopIndex());
        //选择拉伸方式，因为此时不知道鼠标位置
        m_quakeWindow->switchEnableResize();
        m_quakeWindow->activateWindow();
        m_quakeWindow->focusCurrentPage();
        return;
    }
    // 终端的普通对话框,不处理
    if (Service::instance()->getIsDialogShow()) {
        return;
    }

    //若终端和设置界面同时存在，则隐藏终端和设置界面
    if (Service::instance()->isSettingDialogVisible() && Service::instance()->getSettingOwner() == m_quakeWindow) {
        Service::instance()->hideSettingDialog();
        m_quakeWindow->hideQuakeWindow();
        return;
    }
    //终端未激活则激活
    if(!m_quakeWindow->isActiveWindow()) {
        m_quakeWindow->activateWindow();
        m_quakeWindow->focusCurrentPage();
        return;
    }
    //隐藏终端
    m_quakeWindow->hideQuakeWindow();
}

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
    qDebug() << qPrintable(strNewMainWindowTime);
}

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

WindowsManager::WindowsManager(QObject *parent) : QObject(parent)
{
    Utils::set_Object_Name(this);
}

int WindowsManager::widgetCount() const
{
    if (nullptr == WindowsManager::instance()->getQuakeWindow()) {
        return m_widgetCount;
    } else {
        // 不将雷神统计在内
        return m_widgetCount - 1;
    }

}

void WindowsManager::terminalCountIncrease()
{
    ++m_widgetCount;
    // 雷神首次创建的时候m_quakeWindow　= nullptr,　统计数据会多出来一个,后面流程会修正．
    Service::instance()->updateShareMemoryCount(m_quakeWindow == nullptr ? m_widgetCount : m_widgetCount - 1);
    qDebug() << "++ Terminals Count : " << m_widgetCount;
}

void WindowsManager::terminalCountReduce()
{
    --m_widgetCount;
    Service::instance()->updateShareMemoryCount(m_quakeWindow == nullptr ? m_widgetCount : m_widgetCount - 1);
    qDebug() << "-- Terminals Count : " << m_widgetCount;
}
