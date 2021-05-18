/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangpeili <wangpeili@uniontech.com>
 *
 * Maintainer: wangpeili <wangpeili@uniontech.com>
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
#include <QKeySequence>

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
#define KWINDBUSSERVICE "org.kde.KWin"
#define KWINDBUSPATH "/KWin"

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
    explicit Utils(QObject *parent = nullptr);
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
    /**
     * @brief 获取QSS中内容
     * @author ut000439 wangpeili
     * @param filePath 文件路径
     * @return
     */
    static QString getQssContent(const QString &filePath);
    /**
     * @brief 获取config的路径
     * @author ut000439 wangpeili
     * @return
     */
    static QString getConfigPath();
    /**
     * @brief 设置后缀列表只包含*.ttf *.ttc *.otf格式
     * @author ut000439 wangpeili
     * @return
     */
    static QString suffixList();
    /**
     * @brief 获取删除的文字
     * @author ut000439 wangpeili
     * @param font 字体
     * @param str 文字
     * @param MaxWith
     * @param elideMode
     * @return
     */
    static QString getElidedText(QFont font, QString text, int MaxWith, Qt::TextElideMode elideMode = Qt::ElideRight);
    /**
     * @brief 获取随机字符串
     * @author ut000439 wangpeili
     * @return
     */
    static QString getRandString();

    /**
     * @brief 显示目录对话框
     * @author ut000439 wangpeili
     * @param widget
     * @return
     */
    static QString showDirDialog(QWidget *widget);
    /**
     * @brief 显示文件选择对话框
     * @author ut000439 wangpeili
     * @param widget
     * @return
     */
    static QStringList showFilesSelectDialog(QWidget *widget);

    /**
     * @brief 显示退出确认对话框
     * @author ut000439 wangpeili
     * @param type 类型
     * @param count 数量
     * @return
     */
    static bool showExitConfirmDialog(CloseType type, int count = 1);
    /**
     * @brief 获取退出对话框文本
     * @author ut000439 wangpeili
     * @param type 类型
     * @param title 标题
     * @param txt 文本
     * @param count
     */
    static void getExitDialogText(CloseType type, QString &title, QString &txt, int count = 1);
    /**
     * @brief 显示退出卸载确认对话框
     * @author ut000439 wangpeili
     * @return
     */
    static bool showExitUninstallConfirmDialog();
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/
    /**
     * @brief 显示卸载确认对话框
     * @author ut000439 wangpeili
     * @param commandname
     * @return
     */
    static bool showUnistallConfirmDialog(QString commandname);
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/
    /**
     * @brief 显示快捷方式冲突消息框
     * @author ut000439 wangpeili
     * @param txt
     * @return
     */
    static bool showShortcutConflictMsgbox(QString txt);
    /**
     * @brief 为按钮两个中文之间添加空格
     * @author m000714 戴正文
     * @param button 按钮
     */
    static void setSpaceInWord(DPushButton *button);

    /**
     * @brief 当有相同名称时，弹出弹窗给用户确认
     * @author ut000610 戴正文
     * @param parent
     * @param firstLine
     * @param secondLine
     */
    static void showSameNameDialog(QWidget *parent, const QString &firstLine, const QString &secondLine);
    /**
     * @brief 清空控件内部所有子控件的焦点获取(安全考虑，不要全局使用．仅在个别控件中使用)
     * @author n014361 王培利
     * @param objParent
     */
    static void clearChildrenFocus(QObject *objParent);
    /**
     * @brief 函数参数解析
     * @author ut000439 王培利
     * @param arguments 参数
     * @param Properties 属性
     * @param appControl appControl = true, 为main函数使用，遇到-h -v 时，整个进程会退出为false时，为唯一进程使用，主要是解析变量出来。
     */
    static void parseCommandLine(QStringList arguments,  TermProperties &Properties, bool appControl = false);
    /**
     * @brief 解析execute参数,解析出来后，会从参数表中删除相关内容，防止process异常．支持解析例子：
              deepin-terminal -e "bash -c 'ping 127.0.0.1 -c 5'" -w /
              deepin-terminal -e 'bash -c "ping 127.0.0.1 -c 5"' -w /
              deepin-terminal -e  bash -c 'ping 127.0.0.1 -c 5'  -w /
              deepin-terminal -e "ping  127.0.0.1  -c 5"
              deepin-terminal -e "ping  127.0.0.1  -c 5"         -w /
              deepin-terminal -e  ping  127.0.0.1  -c 5          -w /
              deepin-terminal -e  ping "127.0.0.1" -c 5          -w /
     * @author ut000439 王培利
     * @param arguments 参数
     * @return
     */
    static QStringList parseExecutePara(QStringList &arguments);
    /**
     * @brief 解析嵌套的qstring为qstringlist,只支持一级
              如："bash -c 'ping 127.0.0.1'" -> "bash", "-c", "ping 127.0.0.1"
              如：'bash -c "ping 127.0.0.1"' -> "bash, "-c", "ping 127.0.0.1"
              如："bash -c  ping 127.0.0.1"-> "bash, "-c", "ping 127.0.0.1"
     * @author ut000439 王培利
     * @param str
     * @return
     */
    static QStringList parseNestedQString(QString str);
    /**
     * @brief 根据当前窗口获取最外层的主窗口，当前窗口：currWidget，返回值非空就找到最外层主窗口：MainWindow
     * @author ut000125 sunchengxi
     * @param currWidget 当前窗口
     * @return
     */
    static MainWindow *getMainWindow(QWidget *currWidget);

    /**
     * @brief 获取编码列表
     * @author ut000610 戴正文
     * @return
     */
    static QList<QByteArray> encodeList();
    static const int NOT_FOUND = -1;

    /**
     * @brief 设置类的objectname,统一成一个函数
     * @author ut001000 任飞翔
     * @param object
     */
    static void set_Object_Name(QObject *object);

    /**
     * @brief 转换up2down
     * @author ut000610 戴正文
     * @param keysequence
     * @return
     */
    static QString converUpToDown(QKeySequence keysequence);
    //此接口暂时注释保留不删除，2020.12.23
    /**
     * @brief 转换down2up
     * @author ut000610 戴正文
     * @param keysequence
     * @return
     */
    static QString converDownToUp(QKeySequence keysequence);

    /**
     * @brief 从环境变量里面获取当前语言
     * @author ut000438 王亮
     * @return
     */
    static QString getCurrentEnvLanguage();
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
    /**
     * @brief 启动thread，打印等宽字体函数
     * @author ut001000 任飞翔
     */
    void HandleWidthFont();
    /**
     * @brief 设置线程结束标志
     * @param stop true = 结束 false = 正常
     */
    void setStop(bool stop);

private:
    /**
     * @brief 打印DBUS获取等宽字体和比较字体字符方法获取等宽字体，用来定位DBUS获取字体失败后的问题
     * @author ut001000 任飞翔
     */
    void CompareWhiteList();
    //线程成员变量
    QThread *m_thread = nullptr;
    //线程结束标志位
    bool m_bstop = false;
};
/******** Add by ut001000 renfeixiang 2020-06-15:增加 处理等宽字体的类 End***************/

#endif
