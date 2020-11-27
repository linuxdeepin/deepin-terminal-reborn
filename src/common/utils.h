/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#ifndef UTILS_H
#define UTILS_H
#include "settings.h"
#include "termproperties.h"
#include "define.h"

#include <DPushButton>
#include <DApplication>

#include <QObject>
#include <QHash>
#include <QFont>
#include <QList>
#include <QByteArray>
#include <QVariant>
#include <QCommandLineParser>

namespace Konsole {
extern __attribute__((visibility("default"))) int __minFontSize;
extern __attribute__((visibility("default"))) int __maxFontSize;
}

/******** Modify by m000714 daizhengwen 2020-04-03: 自绘的显示控件限制item的宽度****************/
#define ITEMMAXWIDTH 140
/********************* Modify by m000714 daizhengwen End ************************/

/******** Modify by m000714 daizhengwen 2020-04-17: 使用DTK的Dialog****************/
#define USE_DTK
/********************* Modify by m000714 daizhengwen End ************************/

// kwin dbus
#define KWinDBusService "org.kde.KWin"
#define KWinDBusPath "/KWin"

#define MAX_NAME_LEN 32

// 列表最小高度
#define LISTMINHEIGHT 222

// 行间空格间距
#define SPACEHEIGHT 10
#define SPACEWIDTH 10
// 控件通用高度
#define COMMONHEIGHT 36
// 图标大小
#define ICONSIZE_50 50
#define ICONSIZE_36 36
#define ICONSIZE_20 20

// limit font size
#define DEFAULT_MIN_FONT_SZIE 5
#define DEFAULT_MAX_FONT_SZIE 50
#define MIN_FONT_SZIE ({Konsole::__minFontSize;})
#define MAX_FONT_SZIE ({Konsole::__maxFontSize;})

//Viewport Margins
#define MARGINS_TOP 10
#define MARGINS_BOTTOM 10
#define MARGINS_LEFT 10
#define MARGINS_RIGHT 10

// MainWindow
#define WINDOW_DEFAULT_WIDTH 1000   //终端窗口默认宽度
#define WINDOW_DEFAULT_HEIGHT 600   //终端窗口默认高度
#define WINDOW_DEFAULT_SIZE QSize(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT)      //终端窗口默认尺寸
#define ICON_EXIT_FULL_SIZE QSize(ICONSIZE_36, ICONSIZE_36)                         //退出全屏按钮尺寸
#define WIN_TITLE_BAR_HEIGHT 50 //标题栏高度

//Encode Plugin
#define ENCODE_ITEM_WIDTH 220
#define ENCODE_ITEM_HEIGHT 60

/*******************************************************************************
 1. @类名:    Utils
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class MainWindow;
class Utils : public QObject
{
    Q_OBJECT

public:
    Utils(QObject *parent = nullptr);
    ~Utils();
    enum CloseType {
        CloseType_Window,
        CloseType_Tab,
        CloseType_OtherTab,
        CloseType_Terminal,
        CloseType_OtherTerminals,
    };

    static QHash<QString, QPixmap> m_imgCacheHash;
    static QHash<QString, QString> m_fontNameCache;

    static QString getQssContent(const QString &filePath);
    static QString getConfigPath();
    static QString suffixList();
    static QPixmap renderSVG(const QString &filePath, const QSize &size);
    static QString loadFontFamilyFromFiles(const QString &fontFileName);
    static QString getElidedText(QFont font, QString str, int MaxWith, Qt::TextElideMode elideMode = Qt::ElideRight);

    static const QString holdTextInRect(const QFont &font, QString text, const QSize &size);

    static QString convertToPreviewString(QString fontFilePath, QString srcString);

    static QString getRandString();

    static QString showDirDialog(QWidget *widget);
    static QStringList showFilesSelectDialog(QWidget *widget);

    static bool showExitConfirmDialog(CloseType type, int count = 1);
    static void getExitDialogText(CloseType type, QString &title, QString &txt, int count = 1);
    static bool showExitUninstallConfirmDialog();
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/
    static bool showUnistallConfirmDialog(QString commandname);
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/
    static bool showShortcutConflictDialog(QString conflictkey);
    static bool showShortcutConflictMsgbox(QString txt);
    static void setSpaceInWord(DPushButton *button);

    static void showRenameTitleDialog(QString oldTitle, QWidget *parentWidget);

    static void showSameNameDialog(QWidget *parent, const QString &firstLine, const QString &secondLine);
    static void clearChildrenFocus(QObject *objParent);

    static void parseCommandLine(QStringList arguments,  TermProperties &Properties, bool appControl = false);
    static QStringList parseExecutePara(QStringList &arguments);
    static QStringList parseNestedQString(QString str);
    static MainWindow *getMainWindow(QWidget *currWidget);

    static QList<QByteArray> encodeList();
    static const int NOT_FOUND = -1;

    //设置类的objectname
    static void set_Object_Name(QObject *object);

};

/*******************************************************************************
 1. @类名:    FontFilter
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    打印DBUS获取等宽字体和比较字体字符方法获取等宽字体，用来定位DBUS获取字体失败后的问题
*******************************************************************************/
/******** Add by ut001000 renfeixiang 2020-06-15:增加 处理等宽字体的类 Begin***************/
class FontFilter : public QObject
{
    Q_OBJECT
public:
    static FontFilter *instance();
    FontFilter();
    ~FontFilter();
    //启动thread，打印等宽字体函数
    void HandleWidthFont();
    //设置线程结束标志 true = 结束 false = 正常
    void setStop(bool stop);

private:
    //打印DBUS获取等宽字体和比较字体字符方法获取等宽字体，用来定位DBUS获取字体失败后的问题
    void CompareWhiteList();
    //线程成员变量
    QThread *m_thread = nullptr;
    //线程结束标志位
    bool m_bstop = false;
};
/******** Add by ut001000 renfeixiang 2020-06-15:增加 处理等宽字体的类 End***************/

#endif
