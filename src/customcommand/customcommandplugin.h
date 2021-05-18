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

/*******************************************************************************
 1. @类名:    CustomCommandPlugin
 2. @作者:    ut000125 孙成熙
 3. @日期:    2020-08-05
 4. @说明:    自定义插件的具体实现类
*******************************************************************************/
#ifndef CUSTOMCOMMANDPLUGIN_H
#define CUSTOMCOMMANDPLUGIN_H

#include "customcommandtoppanel.h"
#include "mainwindowplugininterface.h"

#include <QObject>
#include <QWidget>

class CustomCommandSearchRstPanel;
class CustomCommandPanel;
class MainWindow;
class CustomCommandPlugin : public MainWindowPluginInterface
{
    Q_OBJECT
public:
    explicit CustomCommandPlugin(QObject *parent = nullptr);

    /**
     * @brief 初始化自定义命令插件
     * @author sunchengxi
     * @param mainWindow 主窗口
     */
    void initPlugin(MainWindow *mainWindow) override;
    /**
     * @brief 初始化窗口菜单栏里的自定义命令快捷键
     * @author sunchengxi
     * @param mainWindow 主窗口
     * @return
     */
    QAction *titlebarMenu(MainWindow *mainWindow) override;

    /**
     * @brief 获取当前插件的自定义命令面板指针
     * @author sunchengxi
     * @return
     */
    CustomCommandTopPanel *getCustomCommandTopPanel();
    /**
     * @brief 初始化自定义命令面板
     * @author sunchengxi
     */
    void initCustomCommandTopPanel();
signals:
    void doHide();

public slots:
    /**
     * @brief 执行自定义命令，并隐藏右侧插件面板
     * @author sunchengxi
     * @param strTxt 自定义命令
     */
    void doCustomCommand(const QString &strTxt);

private:
    MainWindow *m_mainWindow = nullptr;
    CustomCommandTopPanel *m_customCommandTopPanel = nullptr;
};

#endif  // CUSTOMCOMMANDPLUGIN_H
