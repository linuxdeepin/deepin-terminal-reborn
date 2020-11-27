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
#include "mainwindow.h"
#include "environments.h"
#include "dbusmanager.h"
#include "service.h"
#include "utils.h"
#include "terminalapplication.h"
#include "define.h"


#include <DApplication>
#include <DApplicationSettings>
#include <DLog>

#include <QDir>
#include <QDebug>
#include <QCommandLineParser>
#include <QTranslator>
#include <QTime>

DWIDGET_USE_NAMESPACE
/******** Modify by n014361 wangpeili 2020-01-10:增加日志需要 ***********×****/
DCORE_USE_NAMESPACE
/********************* Modify by n014361 wangpeili End ************************/
int main(int argc, char *argv[])
{
    // 应用计时
    QTime useTime;
    useTime.start();
    //为了更精准，起动就度量时间
    qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    // 启动应用
    TerminalApplication app(argc, argv);
    app.setStartTime(startTime);
    DApplicationSettings set(&app);

    // 系统日志
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    // 参数解析
    TermProperties properties;
    Utils::parseCommandLine(app.arguments(), properties, true);

    qDebug() << endl << endl << endl;
    qDebug() << "new terminal start run";
    DBusManager manager;
    if (!manager.initDBus()) {
        // 初始化失败，则已经注册过dbus
        // 判断是否能创建新的的窗口
        // 不是雷神且正在创建
        if (!properties[QuakeMode].toBool() && !Service::instance()->getEnable(startTime)) {
            qDebug() << "[sub app] Server can't create, drop this create request! time use "
                     << useTime.elapsed() << "ms";
            return 0;
        }

        // 调用entry接口
        /******** Modify by ut000610 daizhengwen 2020-05-25: 在终端中打开****************/
        QStringList args = app.arguments();
        bool isCurrentPaht = false;
        for (QString &arg : args) {
            // 若已有-w和--work-directory参数，直接将参数传给主进程执行
            if (arg == "-w" || arg == "--work-directory") {
                isCurrentPaht = true;
                break;
            }
        }
        if (!isCurrentPaht) {
            args += "-w";
            args += QDir::currentPath();
        }
        /********************* Modify by ut000610 daizhengwen End ************************/
        qDebug() << "[sub app] start to call main terminal entry! app args " << args;
        DBusManager::callTerminalEntry(args);
        qDebug() << "[sub app] task complete! sub app quit, time use "
                 << useTime.elapsed() << "ms";
        return 0;
    }
    // 这行不要删除
    qputenv("TERM", "xterm-256color");

    // 主进程
    Service *service = Service::instance();
    service->connect(&manager, &DBusManager::entryArgs, service, &Service::Entry);
    // 初始化数据
    service->init();
    // 创建窗口
    service->Entry(app.arguments());
    qDebug() << "First Terminal Window create complete! time use " << useTime.elapsed() << "ms";
    QString strInitAppTime = GRAB_POINT + LOGO_TYPE + INIT_APP_TIME + QString::number(useTime.elapsed());
    qInfo() << qPrintable(strInitAppTime);
    // 监听触控板事件
    manager.listenTouchPadSignal();

    // 监听桌面切换事件
    // manager.listenDesktopSwitched();

    return app.exec();
}
