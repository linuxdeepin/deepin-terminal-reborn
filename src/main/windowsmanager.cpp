#include "windowsmanager.h"
#include "dbusmanager.h"
#include "utils.h"
#include "service.h"

#include <QDebug>

WindowsManager *WindowsManager::pManager = new WindowsManager();
WindowsManager *WindowsManager::instance()
{
    return  pManager;
}

void WindowsManager::runQuakeWindow(TermProperties properties)
{
    if (m_quakeWindow == nullptr) {
        qDebug() << "runQuakeWindow :create";
        m_quakeWindow = new QuakeWindow(properties);
        m_quakeWindow->show();
        // 雷神创建的第一个时候，m_quakeWindow仍为null，需要在这里更正一下．
        Service::instance()->updateShareMemoryCount(m_quakeWindow == nullptr ? m_widgetCount : m_widgetCount - 1);
        return;
    }
    // Alt+F2的显隐功能实现点
    quakeWindowShowOrHide();
}

void WindowsManager::quakeWindowShowOrHide()
{
    //MainWindow *mainWindow = getMainWindow();
    qDebug() << "ShowOrHide" << m_quakeWindow->winId();

    // 没有显示，就显示．
    if (!m_quakeWindow->isVisible()) {
        qDebug() << "!mainWindow  isVisible now show !" << m_quakeWindow->winId();
        m_quakeWindow->show();
    }

    // 没有激活就激活
    if (!m_quakeWindow->isActiveWindow()) {
        qDebug() << "QuakeWindow is activate, now activateWindow" << m_quakeWindow->winId();
        int index = DBusManager::callKDECurrentDesktop();
        if (index != -1 && m_quakeWindow->getDesktopIndex() != index) {
            // 不在同一个桌面
            DBusManager::callKDESetCurrentDesktop(m_quakeWindow->getDesktopIndex());
            // 选择拉伸方式，因为此时不知道鼠标位置
            m_quakeWindow->switchEnableResize();
        }
        m_quakeWindow->activateWindow();
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
    m_quakeWindow->hide();
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
