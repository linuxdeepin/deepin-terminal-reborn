/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     sunchengxi <sunchengxi@uniontech.com>
 *
 * Maintainer: sunchengxi <sunchengxi@uniontech.com>
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
#include "customcommandplugin.h"
#include "customcommandpanel.h"
#include "customcommandtoppanel.h"
#include "mainwindow.h"
#include "termwidgetpage.h"
#include "settings.h"
#include "shortcutmanager.h"
#include "utils.h"

//qt
#include <QDebug>

CustomCommandPlugin::CustomCommandPlugin(QObject *parent) : MainWindowPluginInterface(parent)
{
    Utils::set_Object_Name(this);
    m_pluginName = "Custom Command";
}

void CustomCommandPlugin::initPlugin(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    //initCustomCommandTopPanel();
    connect(m_mainWindow, &MainWindow::showPluginChanged,  this, [ = ](const QString name, bool bSetFocus) {
        if (MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND != name) {
            // 若插件已经显示，则隐藏
            if (m_isShow) {
                qDebug() << "Command top panel hide";
                getCustomCommandTopPanel()->hideAnim();
                m_isShow = false;
            }
        } else {
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，自定义界面使用不方便，将雷神窗口变大适应正常的自定义界面 Begin***************/
            if (m_mainWindow->isQuakeMode() && m_mainWindow->height() < LISTMINHEIGHT) {
                //因为拉伸函数设置了FixSize，导致自定义界面弹出时死循环，然后崩溃的问题
                QuakeWindow *quakeWindow = qobject_cast<QuakeWindow *>(m_mainWindow);
                quakeWindow->switchEnableResize(true);
                m_mainWindow->resize(m_mainWindow->width(), LISTMINHEIGHT); //首先设置雷神界面的大小
                m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);//重新打开自定义界面，当前流程结束
                //窗口弹出后，重新判断雷神窗口是否需要有拉伸属性
                quakeWindow->switchEnableResize();
                return;
            }
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，自定义界面使用不方便，将雷神窗口变大适应正常的自定义界面 End***************/
            getCustomCommandTopPanel()->show(bSetFocus);
            m_isShow = true;
            //getCustomCommandTopPanel()->setFocus();
        }
    });
    connect(m_mainWindow, &MainWindow::quakeHidePlugin, this, [ = ]() {
        getCustomCommandTopPanel()->hide();
    });
}

QAction *CustomCommandPlugin::titlebarMenu(MainWindow *mainWindow)
{
    QAction *customCommandAction(new QAction(tr("Custom commands"), mainWindow));

    connect(customCommandAction, &QAction::triggered, mainWindow, [mainWindow]() {
        mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
    });

    return customCommandAction;
}

void CustomCommandPlugin::initCustomCommandTopPanel()
{
    qDebug() << __FUNCTION__;
    m_customCommandTopPanel = new CustomCommandTopPanel(m_mainWindow->centralWidget());
    m_customCommandTopPanel->setObjectName("CustomCustomCommandTopPanel");//Add by ut001000 renfeixiang 2020-08-14
    connect(m_customCommandTopPanel,
            &CustomCommandTopPanel::handleCustomCurCommand,
            this,
            &CustomCommandPlugin::doCustomCommand);
}

CustomCommandTopPanel *CustomCommandPlugin::getCustomCommandTopPanel()
{
    if (!m_customCommandTopPanel) {
        initCustomCommandTopPanel();
    }

    return m_customCommandTopPanel;
}

void CustomCommandPlugin::doCustomCommand(const QString &strTxt)
{
    qDebug() <<  __FUNCTION__ << ",[" << strTxt << "]";
    if (!strTxt.isEmpty()) {
        m_mainWindow->currentPage()->sendTextToCurrentTerm(strTxt);
        m_mainWindow->focusCurrentPage();
        /******** Add by nt001000 renfeixiang 2020-05-28:增加 使用mainwindow的hideplugin函数隐藏自定义窗口bug#21992 Begin***************/
        m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_NONE);
        /******** Add by nt001000 renfeixiang 2020-05-28:增加 使用mainwindow的hideplugin函数隐藏自定义窗口bug#21992 End***************/
    }
    emit doHide();
}


