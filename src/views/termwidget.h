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

// 删除和退格键可选模式
enum EraseMode {
    EraseMode_Auto,
    EraseMode_Ascii_Delete,
    EraseMode_Control_H,
    EraseMode_TTY,
    EraseMode_Escape_Sequeue
};

class TermProperties;
class TermWidget : public QTermWidget
{
    Q_OBJECT
public:
    TermWidget(TermProperties properties, QWidget *parent = nullptr);
    ~TermWidget();
    TermWidgetPage *parentPage();
    bool isInRemoteServer();
public:
    // 跳转到下一个命令
    void skipToNextCommand();
    // 跳转到前一个命令
    void skipToPreCommand();
    void setTermOpacity(qreal opacity);
    // 修改字体
    void setTermFont(const QString &fontName);
    // 修改字体大小
    void setTermFontSize(const int fontSize);
    // 修改光标形状
    void setCursorShape(int shape);
    void setPressingScroll(bool enable);
    // 设置编码
    void selectEncode(QString encode);

    bool enterSzCommand() const;
    void setEnterSzCommand(bool enterSzCommand);

    bool isConnectRemote() const;
    void setIsConnectRemote(bool isConnectRemote);

    QString encode() const;
    void setEncode(const QString &encode);

    QString RemoteEncode() const;
    void setRemoteEncode(const QString &RemoteEncode);

    // 设置退格键模式（用户选择接口）
    void setBackspaceMode(const EraseMode &backspaceMode);

    // 设置删除键模式（用户选择接口）
    void setDeleteMode(const EraseMode &deleteMode);

    // 获取该终端距离page的层次
    int getTermLayer();

public slots:
    void wpasteSelection();
    void onSettingValueChanged(const QString &keyName);
    // 处理触控板事件
    void onTouchPadSignal(QString name, QString direction, int fingers);

signals:
    void termRequestRenameTab(QString newTabName);
    void termIsIdle(int currSessionId, bool bIdle);
    void termTitleChanged(QString titleText);

protected:
    // 鼠标滚轮事件
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void customContextMenuCall(const QPoint &pos);
    void handleTermIdle(bool bIdle);
private:
    /*** 修复 bug 28162 鼠标左右键一起按终端会退出 ***/
    void addMenuActions(const QPoint &pos);

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
};

#endif  // TERMWIDGET_H
