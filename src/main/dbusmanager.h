/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  daizhengwen<daizhengwen@uniontech.com>
 *
 * Maintainer:daizhengwen<daizhengwen@uniontech.com>
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

#ifndef DBUSMANAGER_H
#define DBUSMANAGER_H
#include "service.h"

#include <QObject>
#include <QList>
#include <QVariant>

// deepin-terminal
#define TERMINALSERVER "com.deepin.terminal.reborn"
#define TERMINALINTERFACE "/window"

// kwin dbus
#define KWinDBusService "org.kde.KWin"
#define KWinDBusPath "/KWin"

// deepin Appearance
#define AppearanceService "com.deepin.daemon.Appearance"
#define AppearancePath "/com/deepin/daemon/Appearance"

// sound effect 音效服务
#define SOUND_EFFECT_SERVICE    "com.deepin.daemon.SoundEffect"
#define SOUND_EFFECT_PATH       "/com/deepin/daemon/SoundEffect"
#define SOUND_EFFECT_INTERFACE  "com.deepin.daemon.SoundEffect"
#define SOUND_EFFECT_METHOD(method) QDBusMessage::createMethodCall(SOUND_EFFECT_SERVICE, SOUND_EFFECT_PATH, SOUND_EFFECT_INTERFACE, (method))

// gesture 触控板手势
#define GESTURE_SERVICE          "com.deepin.daemon.Gesture"
#define GESTURE_PATH             "/com/deepin/daemon/Gesture"
#define GESTURE_INTERFACE        "com.deepin.daemon.Gesture"
#define GESTURE_SIGNAL           "Event"

// deepin wm 窗管
#define WM_SERVICE                       "com.deepin.wm"
#define WM_PATH                         "/com/deepin/wm"
#define WM_INTERFACE                    "com.deepin.wm"
#define WM_WORKSPACESWITCHED            "WorkspaceSwitched"

#define dbusPlaySound(sound) QDBusConnection::sessionBus().call(SOUND_EFFECT_METHOD("PlaySound")<<(sound))
#define dbusIsSoundEnabled(sound) QDBusConnection::sessionBus().call(SOUND_EFFECT_METHOD("IsSoundEnabled")<<(sound))
#define dbusEnableSound(sound, enable) QDBusConnection::sessionBus().call(SOUND_EFFECT_METHOD("EnableSound")<<(sound)<<(enable))

/*******************************************************************************
 1. @类名:    DBusManager
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class DBusManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", TERMINALSERVER)
public:
    DBusManager();
    ~DBusManager();
    // 初始化terminal相关dbus
    bool initDBus();

    // kwin dbus
    static int callKDECurrentDesktop();
    static void callKDESetCurrentDesktop(int index);

    // Appearance
    static QStringList callAppearanceFont(QString fontType);

    /******** Add by ut001000 renfeixiang 2020-06-16:增加 调用DBUS的show获取的等宽字体，并转换成QStringList Begin***************/
    //将QJsonArray文件转换成QStringList
    static QStringList converToList(const QString &type, const QJsonArray &array);
    //调用DBUS的SHow方法
    static QStringList callAppearanceShowFont(QStringList fontList, QString fontType);
    /******** Add by ut001000 renfeixiang 2020-06-16:增加 调用DBUS的show获取的等宽字体，并转换成QStringList End***************/

    // deepin terminal
    // 创建窗口
    static void callTerminalEntry(QStringList args);


    /** add by ut001121 zhangmeng 20200720 for sp3 keyboard interaction*/
    static void callSystemSound(const QString &sound = "dialog-error");

    // 监听触控板事件
    void listenTouchPadSignal();
    // 监听桌面(工作区切换)
    void listenDesktopSwitched();

public slots:
    void entry(QStringList args);

signals:
    // To Service entry function
    void entryArgs(QStringList args);
};

#endif // DBUSMANAGER_H
