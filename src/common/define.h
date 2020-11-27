/*
* Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
*
* Author:     wangpeili <wangpeili@uniontech.com>
* Maintainer: wangpeili <wangpeili@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DTNG_DEFINE_H
#define DTNG_DEFINE_H

#include <QtGlobal>
#include <QString>
/*
   TERMINALWIDGET_VERSION is (major << 16) + (minor << 8) + patch.
   can be used like #if (TERMINALWIDGET_VERSION >= QT_VERSION_CHECK(0, 7, 1))
*/
#define TERMINALWIDGET_VERSION      QT_VERSION_CHECK(TERMINALWIDGET_VERSION_MAJOR, TERMINALWIDGET_VERSION_MINOR, TERMINALWIDGET_VERSION_PATCH)
#define KEY_WIDGET_RUN   (Qt::Key_Enter)

#define SEARCHBAR_RIGHT_MARGIN 382

// qterminal
const QString TERM_WIDGET_NAME = "Konsole::TerminalScreen";

// log
// 性能测试相关日志
const QString GRAB_POINT = "[GRABPOINT] ";
// 日志类型
const QString LOGO_TYPE = "APPTESTPINT-";
// 打开应用程序时间(ms)
const QString INIT_APP_TIME = "0001 ";
// 新建窗口时间(ms)
const QString CREATE_NEW_MAINWINDOE = "0004 ";
// 新建工作区时间(ms)
const QString CREATE_NEW_TAB_TIME = "0005 ";
// 查找时间(ms)
const QString SEARCH_TIME = "0006 ";
// 显示设置时间(ms)
const QString SHOW_SETTINGS_TIME = "0007 ";

// 搜索框焦点状态
enum SearchBar_State {
    // 显示搜索框，焦点落入搜索框
    SearchBar_Show = 0,
    // 隐藏搜索框，显示其他控件
    SearchBar_Hide = 1,
    // 隐藏搜索框，焦点落回终端
    SearchBar_FocusOut = 2
};

#endif // DTNG_DEFINE_H
