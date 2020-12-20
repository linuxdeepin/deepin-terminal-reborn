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
#include "terminputdialog.h"

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
    TermWidgetPage(TermProperties properties, QWidget *parent = nullptr);
    // mainwindow指针，parent()会变化？？？所以要在构造的时候保存。
    MainWindow *parentMainWindow();
    TermWidget *currentTerminal();

    // 分屏功能
    void split(Qt::Orientation orientation);
    DSplitter *createSubSplit(TermWidget *term, Qt::Orientation orientation);
    void closeSplit(TermWidget *term, bool hasConfirmed = false);
    void showExitConfirmDialog(Utils::CloseType type, int count = 1, QWidget *parent = nullptr);
    // 标识page的唯一ID， 和tab匹配，存在tab中，tabid 用的index，是变化的。
    const QString identifier();
    void focusCurrentTerm();
    void closeOtherTerminal(bool hasConfirmed = false);
    void focusNavigation(Qt::Edge dir);
    int getTerminalCount();
    /******** Add by ut001000 renfeixiang 2020-08-07:用于查找当前所有term中是否含有水平分屏线，有返回true，反之false，#bug#41436***************/
    bool hasHasHorizontalSplit();
    /******** Modify by n014361 wangpeili 2020-01-08: 计算上下左右判断方法 ******×****/
    QRect GetRect(TermWidget *term);
    QPoint GetComparePoint(TermWidget *term, Qt::Edge dir);
    /********************* Modify by n014361 wangpeili End ************************/
    int runningTerminalCount();
    TermProperties createCurrentTerminalProperties();

    void setTerminalOpacity(qreal opacity);
    void setColorScheme(const QString &name);

    void sendTextToCurrentTerm(const QString &text);

    void copyClipboard();
    void pasteClipboard();
    void pasteSelection();
    void toggleShowSearchBar();

    void zoomInCurrentTierminal();
    void zoomOutCurrentTerminal();

    /******** Modify by n014361 wangpeili 2020-01-06:增加相关设置功能 ***********×****/
    // 字体大小
    void setFontSize(int fontSize);
    // 字体
    void setFont(QString fontName);
    // 全选
    void selectAll();
    // 跳转到下一命令
    void skipToNextCommand();
    // 跳转到前一命令
    void skipToPreCommand();
    // 设置光标形状
    void setcursorShape(int shape);
    // 设置光标闪烁
    void setBlinkingCursor(bool enable);
    void setPressingScroll(bool enable);
    void showSearchBar(int state);
    /********************* Modify by n014361 wangpeili End ************************/
    void setTextCodec(QTextCodec *codec);
    void setMismatchAlert(bool alert);

    // 显示重命名弹窗
    void showRenameTitleDialog(QString oldTitle);

protected:
    //将窗口设置为随着窗口变化而变化
    virtual void resizeEvent(QResizeEvent *event) override;

public slots:
    void onTermRequestRenameTab(QString newTabName);
    void onTermTitleChanged(QString title) const;
    void onTermGetFocus();
    void onTermClosed();
    // 查找下一个接口
    void handleFindNext();
    // 查找上一个接口
    void handleFindPrev();
    // 预留
    void slotFindbarClose();
    // 预留
    void handleRemoveSearchKeyword();
    // 更新搜索关键词接口
    void handleUpdateSearchKeyword(const QString &keyword);
    /******** Modify by n014361 wangpeili 2020-03-11: 非DTK控件手动匹配系统主题的修改 **********/
    void applyTheme();
    /********************* Modify by n014361 wangpeili End ************************/
    void updateSplitStyle();

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

private slots:
    void setCurrentTerminal(TermWidget *term);

private:
    TermWidget *createTerm(TermProperties properties);
    void setSplitStyle(DSplitter *splitter);

    TermWidget *m_currentTerm = nullptr;
    PageSearchBar *m_findBar = nullptr;
    MainWindow *m_MainWindow = nullptr;
    QVBoxLayout *m_layout = nullptr;

    // 重命名弹框
    TermInputDialog *m_renameDialog = nullptr;
};
#endif  // TERMWIDGETPAGE_H
