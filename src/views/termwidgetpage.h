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
#ifndef TERMWIDGETPAGE_H
#define TERMWIDGETPAGE_H

#include "define.h"
#include "termproperties.h"
#include "pagesearchbar.h"
#include "mainwindow.h"
#include "utils.h"
#include "tabrenamedlg.h"

#include <DSplitter>

#include <QWidget>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    TermWidgetPage
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:
*******************************************************************************/

class TermWidget;
class MainWindow;
class TermWidgetPage : public QWidget
{
    Q_OBJECT
public:
    TermWidgetPage(const TermProperties &properties, QWidget *parent = nullptr);
    // mainwindow指针，parent()会变化？？？所以要在构造的时候保存。
    /**
     * @brief 获取父主窗口
     * @author ut000439 wangpeili
     * @return
     */
    MainWindow *parentMainWindow();
    /**
     * @brief 当标签页拖拽到新的MainWindow时，重新设置TermWidgetPage的父窗口
     * @author ut000438 王亮
     * @param mainWin
     */
    void setParentMainWindow(MainWindow *mainWin);
    /**
     * @brief 获取当前终端
     * @author ut000439 wangpeili
     * @return
     */
    TermWidget *currentTerminal();

    /**
     * @brief 获取当前终端的title
     */
    QString getCurrentTerminalTitle();

    /**
     * @brief 分屏
     * @author ut000439 王培利
     * @param orientation 方向
     */
    void split(Qt::Orientation orientation);
    /**
     * @brief 创建分屏
     * @author ut000439 王培利
     * @param term
     * @param orientation 方向
     * @return
     */
    DSplitter *createSubSplit(TermWidget *term, Qt::Orientation orientation);
    /**
     * @brief 关闭分屏
     * @author ut000439 王培利
     * @param term
     * @param hasConfirmed
     */
    void closeSplit(TermWidget *term, bool hasConfirmed = false);
    /**
     * @brief 显示退出确认对话框
     * @author ut000439 wangpeili
     * @param type
     * @param count
     * @param parent
     */
    void showExitConfirmDialog(Utils::CloseType type, int count = 1, QWidget *parent = nullptr);
    // 标识page的唯一ID， 和tab匹配，存在tab中，tabid 用的index，是变化的。
    /**
     * @brief 获取识别码
     * @author ut000439 wangpeili
     * @return
     */
    const QString identifier();
    /**
     * @brief 设置焦点到当前终端
     * @author ut000439 wangpeili
     */
    void focusCurrentTerm();
    /**
     * @brief 关闭其它终端
     * @author ut000439 wangpeili
     * @param hasConfirmed
     */
    void closeOtherTerminal(bool hasConfirmed = false);
    /**
     * @brief 焦点导航
     * @author ut000439 wangpeili
     * @param dir
     */
    void focusNavigation(Qt::Edge dir);
    /**
     * @brief 获取当前页的终端个数
     * @author ut000439 王培利
     * @return
     */
    int getTerminalCount();
    /******** Add by ut001000 renfeixiang 2020-08-07:用于查找当前所有term中是否含有水平分屏线，有返回true，反之false，#bug#41436***************/
    /**
     * @brief 用于查找当前所有term中是否含有水平分屏线，有返回true，反之false，#bug#41436
     * @author ut001000 任飞翔
     * @return
     */
    bool hasHasHorizontalSplit();
    /******** Modify by n014361 wangpeili 2020-01-08: 计算上下左右判断方法 ******×****/
    /**
     * @brief 获取标签页内所有子窗体的区域
     * @author ut000439 王培利
     * @param term
     * @return
     */
    QRect GetRect(TermWidget *term);
    /**
     * @brief 获取窗口上下左右键需要判断的点位信息，只需要获取偏离1个像素信息，即可
     * @author ut000439 王培利
     * @param term
     * @param dir
     * @return
     */
    QPoint GetComparePoint(TermWidget *term, Qt::Edge dir);
    /********************* Modify by n014361 wangpeili End ************************/
    /**
     * @brief 获取正在执行的终端个数
     * @author ut000439 wangpeili
     * @return
     */
    int runningTerminalCount();
    /**
     * @brief 创建当前终端属性
     * @author ut000439 wangpeili
     * @return
     */
    TermProperties createCurrentTerminalProperties();

    /**
     * @brief 设置当前终端的透明度
     * @author ut000439 wangpeili
     * @param opacity
     */
    void setTerminalOpacity(qreal opacity);
    /**
     * @brief 设置主题
     * @author ut000439 wangpeili
     * @param name
     */
    void setColorScheme(const QString &name);

    /**
     * @brief 发送文本到当前终端
     * @author ut000439 wangpeili
     * @param text 文本
     * @param isRemoteConnect 是否远程连接
     */
    void sendTextToCurrentTerm(const QString &text, bool isRemoteConnect = false);

    /**
     * @brief 复制到剪贴板
     * @author ut000439 wangpeili
     */
    void copyClipboard();
    /**
     * @brief 用户粘贴时候，直接粘贴剪切板内容
     * @author ut000439 王培利
     */
    void pasteClipboard();
    /**
     * @brief 切换显示搜索栏
     * @author ut000439 wangpeili
     */
    void toggleShowSearchBar();

    /**
     * @brief 放大当前端子
     * @author ut000439 wangpeili
     */
    void zoomInCurrentTierminal();
    /**
     * @brief 缩小当前端子
     * @author ut000439 wangpeili
     */
    void zoomOutCurrentTerminal();

    /******** Modify by n014361 wangpeili 2020-01-06:增加相关设置功能 ***********×****/
    /**
     * @brief 设置字体大小
     * @author ut000439 王培利
     * @param 字体大小
     */
    void setFontSize(int fontSize);
    // 字体
    /**
     * @brief 设置字体
     * @author ut000439 王培利
     * @param 字体名称
     */
    void setFont(QString fontName);
    // 全选

    /*******************************************************************************
     1. @函数:   void TermWidgetPage::selectAll()
     2. @作者:     ut000439 王培利
     3. @日期:     2020-01-10
     4. @说明:    全选
    *******************************************************************************/
    /**
     * @brief 全选
     * @author ut000439 王培利
     */
    void selectAll();
    /**
     * @brief 跳转到下一命令
     * @author ut000439 王培利
     */
    void skipToNextCommand();
    /**
     * @brief 跳转到前一命令
     * @author ut000439 王培利
     */
    void skipToPreCommand();
    /**
     * @brief 设置光标形状
     * @author ut000439 王培利
     * @param shape 光标形状
     */
    void setcursorShape(int shape);
    /**
     * @brief 设置光标闪烁
     * @author 王培利ut000439
     * @param enable 是否闪烁
     */
    void setBlinkingCursor(bool enable);
    /**
     * @brief 设置按键时是否滚动
     * @author ut000439 wangpeili
     * @param enable 是否滚动
     */
    void setPressingScroll(bool enable);
    /**
     * @brief 是否显示搜索框
     * @author ut000439 王培利
     * @param state 1) 显示搜索框 0 SearchBar_Show 2) 隐藏搜索框 => 显示其他列表框，焦点顺移 1 SearchBar_Hide 3) 隐藏搜索框 => 焦点返回终端 2 SearchBar_FocusOut
     */
    void showSearchBar(int state);
    /********************* Modify by n014361 wangpeili End ************************/
    /**
     * @brief 设置文本编解码器
     * @author ut000439 wangpeili
     * @param codec
     */
    void setTextCodec(QTextCodec *codec);
    /**
     * @brief 是否将输入框标为警告
     * @author ut000439 王培利
     * @param alert 警告
     */
    void setMismatchAlert(bool alert);

    /**
     * @brief 显示重命名弹窗，判断是否有重命名弹窗，有则显示，没有则创建
     * @author ut000610 戴正文
     */
    void showRenameTitleDialog();

    /**
     * @brief 打印查找花费的时间
     * @author ut000610 戴正文
     */
    void printSearchCostTime();

protected:
    /**
     * @brief 将窗口设置为随着窗口变化而变化, 重绘时候，调整查找框的位置
     * @author ut000439 王培利
     * @param event
     */
    virtual void resizeEvent(QResizeEvent *event) override;

public slots:
    /**
     * @brief 终端请求重命名便签响应
     * @param newTabName 新Tab标签名
     */
    void onTermRequestRenameTab(QString newTabName);
    /**
     * @brief 终端标题变化响应函数
     * @author ut000439 wangpeili
     * @param title 终端标题
     */
    void onTermTitleChanged(QString title);
    /**
     * @brief 终端获取焦点响应函数
     * @author ut000439 wangpeili
     */
    void onTermGetFocus();
    /**
     * @brief 终端关闭响应函数
     * @author ut000439 wangpeili
     */
    void onTermClosed();
    /**
     * @brief 查找下一个接口
     * @author ut000439 王培利
     */
    void handleFindNext();
    /**
     * @brief 查找上一个接口
     * @author ut000439 王培利
     */
    void handleFindPrev();
    // 预留
//    void slotFindbarClose();
    // 预留
//    void handleRemoveSearchKeyword();
    /**
     * @brief 更新搜索关键词接口，更新key(没有输入回车情况),自动查找接口
     * @author ut000439 王培利
     * @param keyword
     */
    void handleUpdateSearchKeyword(const QString &keyword);
    /******** Modify by n014361 wangpeili 2020-03-11: 非DTK控件手动匹配系统主题的修改 **********/
    /**
     * @brief 非DTK控件手动匹配系统主题的修改
     * @author ut000439 王培利
     */
    void applyTheme();
    /********************* Modify by n014361 wangpeili End ************************/
    /**
     * @brief 更新分屏样式
     * @author ut000439 wangpeili
     */
    void updateSplitStyle();

    void slotShowPluginChanged(const QString name);
    void slotQuakeHidePlugin();

signals:
    void tabTitleChanged(QString title) const;
    void termTitleChanged(QString title) const;
    void lastTermClosed(QString pageIdentifier) const;
    void termRequestOpenSettings() const;
    void termGetFocus() const;
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/
    bool uninstallTerminal(QString commandname);
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/
    // 下载失败时退出下载
    void quitDownload();
    // 对当前标签页的标题重命名，只对当前标签页有效
    void tabTitleFormatChanged(const QString &tabTitleFormat, const QString &remoteTabTitleFormat);

private slots:
    /**
     * @brief 设置当前终端标题
     * @author ut000439 wangpeili
     * @param term 当前终端
     */
    void setCurrentTerminal(TermWidget *term);

    /**
     * @brief 处理搜索关键词变化的槽
     * @author ut000438 王亮
     * @param keyword 搜索关键词
     */
    void handleKeywordChanged(QString keyword);
    /**
     * @brief 处理主题变化的槽
     * @author ut000438 王亮
     */
    void handleThemeTypeChanged();
    /**
     * @brief 理卸载终端时的确认对话框
     * @author ut000438 王亮
     * @param commandname 终端名称
     * @return
     */
    bool handleUninstallTerminal(QString commandname);
    /**
     * @brief 处理重命名标签标题对话框关闭
     * @author ut000438 王亮
     */
    void handleTabRenameDlgFinished();
    /**
     * @brief 处理在终端内点击鼠标左键
     * @author ut000438 王亮
     */
    void handleLeftMouseClick();

private:
    /**
     * @brief 创建终端
     * @author ut000439 wangpeili
     * @param properties 属性
     * @return
     */
    TermWidget *createTerm(TermProperties properties);
    /**
     * @brief 设置分割线样式
     * @author ut000439 wangpeili
     * @param splitter 分割线
     */
    void setSplitStyle(DSplitter *splitter);

    TermWidget *m_currentTerm = nullptr;
    PageSearchBar *m_findBar = nullptr;
    MainWindow *m_MainWindow = nullptr;
    QVBoxLayout *m_layout = nullptr;

    // 重命名弹框
    TabRenameDlg *m_renameDlg = nullptr;
    // 标签标题
    QString m_tabTitle;
};
#endif  // TERMWIDGETPAGE_H
