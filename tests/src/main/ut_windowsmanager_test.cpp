/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangliang <wangliang@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
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

#include "ut_windowsmanager_test.h"

#include "service.h"
#include "mainwindow.h"
#include "tabbar.h"
#include "termwidget.h"
#include "windowsmanager.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QApplication>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

UI_WindowsManager_Test::UI_WindowsManager_Test()
{
}

void UI_WindowsManager_Test::SetUp()
{
    m_service = Service::instance();
    //Service的init初始化函数只能执行一次，否则会crash
    if (!m_service->property("isServiceInit").toBool()) {
        m_service->init();
        m_service->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;

    m_quakeTermProperty[QuakeMode] = true;
}

void UI_WindowsManager_Test::TearDown()
{
}

#ifdef UI_WINDOWSMANAGER_TEST

TEST_F(UI_WindowsManager_Test, runQuakeWindow)
{
    WindowsManager *winManager = WindowsManager::instance();
    winManager->runQuakeWindow(m_quakeTermProperty);
    EXPECT_EQ((winManager->m_quakeWindow != nullptr), true);
}

TEST_F(UI_WindowsManager_Test, getQuakeWindow)
{
    WindowsManager *winManager = WindowsManager::instance();
    MainWindow *mainWin = winManager->getQuakeWindow();
    EXPECT_EQ((mainWin != nullptr), true);
}

TEST_F(UI_WindowsManager_Test, createNormalWindow)
{
    WindowsManager *winManager = WindowsManager::instance();
    winManager->createNormalWindow(m_normalTermProperty);
    EXPECT_EQ((winManager->m_normalWindowList.size() > 0), true);
}

TEST_F(UI_WindowsManager_Test, terminalCountIncrease)
{
    WindowsManager *winManager = WindowsManager::instance();
    int widgetCount = winManager->widgetCount();
    winManager->terminalCountIncrease();
    EXPECT_EQ(winManager->widgetCount(), widgetCount + 1);
}

TEST_F(UI_WindowsManager_Test, terminalCountReduce)
{
    WindowsManager *winManager = WindowsManager::instance();
    int widgetCount = winManager->widgetCount();
    winManager->terminalCountReduce();
    EXPECT_EQ(winManager->widgetCount(), widgetCount - 1);
}

/*******************************************************************************
 1. @函数:    runQuakeWindow
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-09
 4. @说明:    运行雷神窗口的显隐测试
*******************************************************************************/
TEST_F(UI_WindowsManager_Test, runQuakeWindow2)
{
    // 若雷神还在就关闭
    if (WindowsManager::instance()->getQuakeWindow()) {
        WindowsManager::instance()->getQuakeWindow()->closeAllTab();
        WindowsManager::instance()->m_quakeWindow = nullptr;
    }

    // 启动雷神
    WindowsManager::instance()->runQuakeWindow(m_quakeTermProperty);
    WindowsManager::instance()->m_quakeWindow->setAnimationFlag(true);
    // 雷神存在
    EXPECT_NE(WindowsManager::instance()->getQuakeWindow(), nullptr);
    // 相当于再次Alt+F2
    WindowsManager::instance()->runQuakeWindow(m_quakeTermProperty);

    // 关闭雷神窗口
    WindowsManager::instance()->getQuakeWindow()->closeAllTab();
    WindowsManager::instance()->m_quakeWindow = nullptr;
}

/*******************************************************************************
 1. @函数:    onMainwindowClosed
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-09
 4. @说明:    测试窗口关闭
*******************************************************************************/
TEST_F(UI_WindowsManager_Test, onMainwindowClosed)
{
    // 新建一个窗口
    MainWindow *newWindow = new NormalWindow(m_normalTermProperty);
    // 关闭当前的窗口 => 测试是否正常执行
    WindowsManager::instance()->onMainwindowClosed(newWindow);

    // 没有雷神窗口
    if (!WindowsManager::instance()->getQuakeWindow()) {
        // 创建雷神窗口
        WindowsManager::instance()->runQuakeWindow(m_quakeTermProperty);
    }
    // 关闭雷神窗口
    WindowsManager::instance()->getQuakeWindow()->closeAllTab();
    if (WindowsManager::instance()->getQuakeWindow()) {
        WindowsManager::instance()->onMainwindowClosed(WindowsManager::instance()->getQuakeWindow());
    }
    EXPECT_EQ(WindowsManager::instance()->getQuakeWindow(), nullptr);
}

TEST_F(UI_WindowsManager_Test, quakeWindowShowOrHide)
{
    // 若雷神还在就关闭
    if (WindowsManager::instance()->getQuakeWindow()) {
        WindowsManager::instance()->getQuakeWindow()->closeAllTab();
        WindowsManager::instance()->m_quakeWindow = nullptr;
    }

    // 启动雷神
    WindowsManager::instance()->runQuakeWindow(m_quakeTermProperty);
    WindowsManager::instance()->m_quakeWindow->setAnimationFlag(true);
    // 雷神存在
    EXPECT_NE(WindowsManager::instance()->getQuakeWindow(), nullptr);
    // 雷神显示
    EXPECT_EQ(WindowsManager::instance()->getQuakeWindow()->isVisible(), true);
    WindowsManager::instance()->getQuakeWindow()->setVisible(false);
     WindowsManager::instance()->m_quakeWindow->hideQuakeWindow();
    WindowsManager::instance()->quakeWindowShowOrHide();
    // 相当于再次Alt+F2
    WindowsManager::instance()->runQuakeWindow(m_quakeTermProperty);
    // 雷神隐藏 => 又开始了新一轮的动画
    EXPECT_EQ(WindowsManager::instance()->getQuakeWindow()->isNotAnimation, false);

    // 关闭雷神窗口
    WindowsManager::instance()->getQuakeWindow()->closeAllTab();
    WindowsManager::instance()->m_quakeWindow = nullptr;
}
#endif
