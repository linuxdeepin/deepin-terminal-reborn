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
#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include "qtermwidget.h"
#include "termwidgetpage.h"

/*******************************************************************************
 1. @类名:    TermWidget
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

// 程序名 %n
const QString PROGRAM_NAME = "%n";
// 当前目录长 %D
const QString DIR_L = "%D";
// 当前目录短 %d
const QString DIR_S = "%d";
// 会话编号 %#
const QString TAB_NUM = "%#";
// 用户名
const QString USER_NAME = "%u";
// 用户名@
const QString USER_NAME_L = "%U";
// 本地主机
const QString LOCAL_HOST_NAME = "%h";
// 远程主机
const QString REMOTE_HOST_NAME = "%h";
// shell设置的窗口标题
const QString SHELL_TITLE = "%w";

// 标签标题格式参数
const QString TAB_ARGS = "%n %d %D %# %u %h %w";
// 远程标签标题格式参数
const QString REMOTE_ARGS = "%u %U %h %# %w";

// 删除和退格键可选模式
enum EraseMode {
    EraseMode_Auto,
    EraseMode_Ascii_Delete,
    EraseMode_Control_H,
    EraseMode_TTY,
    EraseMode_Escape_Sequeue
};

// 标签标题格式
struct TabFormat {
    // 当前标签标题格式
    QString currentTabFormat;
    // 远程标签标题格式
    QString remoteTabFormat;
    // 是否是全局设置
    bool isGlobal = true;
};

class TermProperties;
struct ServerConfig;
class TermWidget : public QTermWidget
{
    Q_OBJECT
public:
    TermWidget(const TermProperties &properties, QWidget *parent = nullptr);
    ~TermWidget();
    /**
     * @brief 获取父页面
     * @author ut000610 daizhengwen
     * @return
     */
    TermWidgetPage *parentPage();
    /**
     * @brief 是否在远程服务器中
     * @author ut000610 daizhengwen
     * @return
     */
    bool isInRemoteServer();
public:
    /**
     * @brief 跳转到下一个命令（这个功能没找到库的接口，现在是暂时是以虚拟键形式实现）
     * @author n014361 王培利
     */
    void skipToNextCommand();
    /**
     * @brief 跳转到前一个命令（这个功能没找到库的接口，现在是暂时是以虚拟键形式实现）
     * @author n014361 王培利
     */
    void skipToPreCommand();
    /**
     * @brief 设置不透明度
     * @author ut000610 daizhengwen
     * @param opacity 不透明度
     */
    void setTermOpacity(qreal opacity);
    /**
     * @brief 设置字体
     * @author n014361 王培利
     * @param fontName 字体名称
     */
    void setTermFont(const QString &fontName);
    /**
     * @brief 设置字体大小
     * @author n014361 王培利
     * @param fontSize 字体大小
     */
    void setTermFontSize(const int fontSize);
    /**
     * @brief 设置光标形状
     * @author n014361 王培利
     * @param shape 光标形状
     */
    void setCursorShape(int shape);
    /**
     * @brief 设置按键时是否滚动
     * @author ut001121 zhangmeng
     * @param enable 是否滚动
     */
    void setPressingScroll(bool enable);
    /**
     * @brief 选择编码
     * @author ut000610 daizhengwen
     * @param encode 编码
     */
    void selectEncode(QString encode);

    /**
     * @brief 是否进行下载
     * @author ut000610 daizhengwen
     * @return
     */
    bool enterSzCommand() const;
    /**
     * @brief 设置进行下载
     * @author ut000610 daizhengwen
     * @param enterSzCommand
     */
    void setEnterSzCommand(bool enterSzCommand);

    /**
     * @brief 是否连接远程管理
     * @author ut000610 daizhengwen
     * @return
     */
    bool isConnectRemote() const;
    /**
     * @brief 设置连接远程管理
     * @author ut000610 daizhengwen
     * @param isConnectRemote 是否连接远程管理
     */
    void setIsConnectRemote(bool isConnectRemote);
    /**
     * @brief 连接远程后修改当前标签标题
     * @author ut000610 戴正文
     * @param remoteConfig 远程配置
     */
    void modifyRemoteTabTitle(ServerConfig remoteConfig);

    /**
     * @brief 获取编码
     * @author ut000610 daizhengwen
     * @return
     */
    QString encode() const;
    /**
     * @brief 设置编码
     * @author ut000610 daizhengwen
     * @param encode 编码
     */
    void setEncode(const QString &encode);

    /**
     * @brief 获取远程管理的编码
     * @author ut000610 daizhengwen
     * @return
     */
    QString RemoteEncode() const;
    /**
     * @brief 设置远程管理的编码
     * @author ut000610 daizhengwen
     * @param RemoteEncode
     */
    void setRemoteEncode(const QString &RemoteEncode);

    /**
     * @brief 根据设置的模式改变退格模式（用户选择接口）
     * @author ut000610 戴正文
     * @param backspaceMode
     */
    void setBackspaceMode(const EraseMode &backspaceMode);

    /**
     * @brief 根据设置的模式改变删除模式（用户选择接口）
     * @author ut000610 戴正文
     * @param deleteMode
     */
    void setDeleteMode(const EraseMode &deleteMode);

    /**
     * @brief 获取当前terminal距离page的层次．用于限定分屏
     * @author ut000439 王培利
     * @return
     */
    int getTermLayer();

    /**
     * @brief 设置标签标题格式（全局设置）
     * @author ut000610 戴正文
     * @param tabFormat
     */
    void setTabFormat(const QString &tabFormat);
    /**
     * @brief 设置远程标签标题格式(全局设置)
     * @author ut000610 戴正文
     * @param remoteTabFormat
     */
    void setRemoteTabFormat(const QString &remoteTabFormat);
    // 修改所有的标签标题格式
    /**
     * @brief 重命名标签标题/远程标签标题格式
     * @author ut000610 戴正文
     * @param tabFormat 标签标题
     * @param remoteTabFormat 远程标签标题格式
     */
    void renameTabFormat(const QString &tabFormat, const QString &remoteTabFormat);
    /**
     * @brief 获取标签对应的标签标题
     * @author ut000610 戴正文
     * @return
     */
    QString getTabTitle();
    /**
     * @brief 初始化终端标签
     * @author ut000610 戴正文
     */
    void initTabTitle();
    /**
     * @brief 初始化标签标题参数列表
     * @author ut000610 戴正文
     */
    void initTabTitleArgs();
    /**
     * @brief 获取标签标题格式
     * @author ut000610 戴正文
     * @return
     */
    QString getTabTitleFormat();
    /**
     * @brief 获取远程标签标题格式
     * @author ut000610 戴正文
     * @return
     */
    QString getRemoteTabTitleFormat();
    /**
     * @brief 获取当前term显示的标签标题
     * @author ut000610 戴正文
     * @return
     */
    QString getCurrentTabTitleFormat();
    /**
     * @brief Ctrl+Q/Ctrl+S的悬浮提示框
     * @author 朱科伟
     * @param show 文案提示
     */
    void showFlowMessage(bool show);
    /**
     * @brief shell启动失败时的悬浮提示框
     * @author ut000610 戴正文
     * @param strWarnings 提示
     */
    void showShellMessage(QString strWarnings);
    /**
     * @brief 弹窗安装事件过滤器，更新DFloatingMessage的text
     * @author ut003135 changze
     * @param widget
     * @param text
     */
    void installEventMessageText(DFloatingMessage *widget, const QString &text);
public slots:
    /**
     * @brief Terminal的各项设置生效
     * @author n014361 王培利
     * @param keyName 设置项名称
     */
    void onSettingValueChanged(const QString &keyName);
    /**
     * @brief 处理拖拽进来的文件名，1) 正常模式下: 目前只显示,暂不处理
     * @author ut000610 戴正文
     * @param urls 拖拽进来的文件名
     */
    void onDropInUrls(const char *urls);
    /**
     * @brief 处理触控板事件
     * @author ut000610 戴正文
     * @param name 触控板事件类型(手势或者触摸类型) pinch 捏 tap 敲 swipe 右键单击 单键
     * @param direction 手势方向 触控板上 up 触控板下 down 左 left 右 right 无 none 向内 in 向外 out 触控屏上 top 触摸屏下 bot
     * @param fingers 手指数量 (1,2,3,4,5)
     */
    void onTouchPadSignal(QString name, QString direction, int fingers);
    /**
     * @brief 处理shell消息提示
     * @author ut000610 戴正文
     * @param currentShell 当前使用的shell
     * @param isSuccess 启用shell是否成功 true 替换了shell false 替换shell但启动失败
     */
    void onShellMessage(QString currentShell, bool isSuccess);

signals:
    void termRequestRenameTab(QString newTabName);
    void termIsIdle(QString tabIdentifier, bool bIdle);
    void termTitleChanged(QString titleText);

protected:
    /**
     * @brief 鼠标滚轮事件 支持Ctrl + 滚轮上下事件:Ctrl+滚轮上 放大;Ctrl+滚轮下 缩小
     * @author ut000610 戴正文
     * @param event 滚轮事件
     */
    void wheelEvent(QWheelEvent *event) override;
    /**
     * @brief 监控事件过滤，终端宽度缩小时，相关弹窗也同步变化
     * @author  ut003135 changze
     * @param o
     * @param e
     * @return
     */
    bool eventFilter(QObject *o, QEvent *e) override;

private slots:
    /**
     * @brief 自定义上下文菜单调用
     * @author ut000125 sunchengxi
     * @param pos
     */
    void customContextMenuCall(const QPoint &pos);
    /**
     * @brief 标签标题参数变化
     * @author ut000610 戴正文
     * @param key
     * @param value
     */
    void onTitleArgsChange(QString key, QString value);
    /**
     * @brief 主机名变化
     * @author ut000610 戴正文
     */
    void onHostnameChanged();

    void onQTermWidgetReceivedData(QString value);
    void onTermWidgetReceivedData(QString value);
    void onExitRemoteServer();
    void onUrlActivated(const QUrl &url, bool fromContextMenu);
    void onThemeTypeChanged(DGuiApplicationHelper::ColorType builtInTheme);
    void onTermIsIdle(bool bIdle);
    void onTitleChanged();
    void onWindowEffectEnabled(bool isWinEffectEnabled);
    void onCopyAvailable(bool enable);
    void onSetTerminalFont();
    void onSig_matchFound();
    void onSig_noMatchFound();

    void onCopy();
    void onPaste();
    /**
     * @brief 根据文件路径打开文件
     * @author ut000438 王亮
     */
    void onOpenFile();
    void onOpenFileInFileManager();
    void onUploadFile();
    void onDownloadFile();
    void onShowSettings();
    void onShowEncoding();
    void onShowCustomCommands();
    void onShowRemoteManagement();
    void onShowSearchBar();
    void onHorizontalSplit();
    void onVerticalSplit();
    void splitHorizontal();
    void splitVertical();
    void onCloseCurrWorkSpace();
    void onCloseOtherWorkSpaces();
    void onCreateNewTab();
    void onSwitchFullScreen();
    void openBing();
    void openBaidu();
    void openGithub();
    void openStackOverflow();
private:
    /**
     * @brief 初始化信号槽连接
     * @author ut000438 王亮
     */
    void initConnections();
    /*** 修复 bug 28162 鼠标左右键一起按终端会退出 ***/
    /**
     * @brief 添加菜单操作
     * @author ut000610 daizhengwen
     * @param pos
     */
    void addMenuActions(const QPoint &pos);
    /**
     * @brief 根据标签格式获取标签标题
     * @author ut000610 戴正文
     * @param format 格式
     * @param TabFormat 标签格式
     * @return
     */
    QString getTabTitle(QMap<QString, QString> format, QString TabFormat);
    /**
     * @brief 分屏时切换到当前选中主题方案
     * @author ut000125 sunchengxi
     */
    void switchThemeOnSplitScreen();
    void openUrl(QString strUrl);

    /**
     * @brief 根据选择的文件名字符串得到合法的文件名，去除文件名开头/结尾的''或""
     * @author ut000438 王亮
     * @param selectedText 选择的文件名字符串
     * @return
     */
    QString getFormatFileName(QString selectedText);
    /**
     * @brief 根据文件名拼接得到文件路径
     * @author ut000438 王亮
     * @param fileName 文件名
     * @return
     */
    QString getFilePath(QString fileName);

    TermWidgetPage *m_page = nullptr;
    TermProperties m_properties;

    DMenu *m_menu = nullptr;
    /******** Modify by ut000610 daizhengwen 2020-05-27: 当前窗口是否要进行下载操作****************/
    bool m_enterSzCommand = false;
    /********************* Modify by ut000610 daizhengwen End ************************/
    /******** Modify by ut000610 daizhengwen 2020-05-28: 当前窗口是否连接远程服务器****************/
    bool m_isConnectRemote = false;
    /********************* Modify by ut000610 daizhengwen End ************************/
    // 当前编码
    QString m_encode = "UTF-8";
    // 远程编码
    QString m_remoteEncode = "UTF-8";
    // 当前终端退格信号
    EraseMode m_backspaceMode = EraseMode_Ascii_Delete;
    // 当前终端删除信号
    EraseMode m_deleteMode = EraseMode_Escape_Sequeue;

    // 标签格式
    // 当前标签标题参数
    QMap<QString, QString> m_tabArgs;
    // 远程标签标题参数
    QMap<QString, QString> m_remoteTabArgs;
    // 标签标题格式
    TabFormat m_tabFormat;
    // 会话编号
    int m_sessionNumber;
    // Ctrl+Q/Ctrl+S的悬浮框
    DFloatingMessage *m_flowMessage = nullptr;
    //记录弹窗的信息
    QMap<QObject *, QString> m_messageTextMap;
};

#endif  // TERMWIDGET_H
