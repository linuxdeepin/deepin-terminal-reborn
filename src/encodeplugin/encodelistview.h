/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:      zhangmeng <zhangmeng@uniontech.com>
 *
 * Maintainer:  zhangmeng <zhangmeng@uniontech.com>
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

#ifndef ENCODELISTVIEW_H
#define ENCODELISTVIEW_H

#include "mainwindow.h"
#include <DListView>
#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

// add by ut001121 zhangmeng 20200731 定义无效焦点原因 修复BUG40390
#define INVALID_FOCUS_REASON Qt::MouseFocusReason

#define ENCODE_COORDINATE_ERROR_Y  10

/*******************************************************************************
 1. @类名:    EncodeListView
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class EncodeListModel;
class EncodeListView : public DListView
{
    Q_OBJECT

public:
    explicit EncodeListView(QWidget *parent = nullptr);

protected:
    /**
     * @brief 处理焦点事件
     * @author ut001121 张猛
     * @param event 焦点事件
     */
    void focusInEvent(QFocusEvent *event) override;
    /**
     * @brief 处理焦点离开事件
     * @author ut001121 zhangmeng
     * @param event 焦点离开事件
     */
    void focusOutEvent(QFocusEvent *event) override;
    /**
     * @brief 处理尺寸变化事件
     * @author ut001121 张猛
     * @param event 尺寸变化事件
     */
    void resizeEvent(QResizeEvent *event) override;
    /**
      * @brief 处理键盘事件
      * @author ut001121 张猛
      * @param 键盘事件
      */
    void keyPressEvent(QKeyEvent *event) override;
    /**
     * @brief 处理鼠标按下事件
     * @author ut001121 张猛
     */
    void mousePressEvent(QMouseEvent *) override;
    /**
     * @brief 处理鼠标抬起事件
     * @author ut001121 张猛
     */
    void mouseReleaseEvent(QMouseEvent *) override;
    /**
     * @brief 处理鼠标移动事件
     * @author ut001121 张猛
     */
    void mouseMoveEvent(QMouseEvent *) override;

signals:
    void focusOut();

public slots:
    /**
     * @brief 列表视图上单击响应函数
     * @author ut001121 zhangmeng
     * @param index
     */
    void onListViewClicked(const QModelIndex &index);
    /**
     * @brief 查询编码
     * @author ut001121 zhangmeng
     * @param encode 编码
     */
    void checkEncode(QString encode);
    /** add by ut001121 zhangmeng 20200718 for sp3 keyboard interaction*/
    inline Qt::FocusReason getFocusReason() {return m_foucusReason;}

private:
    /**
     * @brief 初始化编码项
     * @author ut001121 zhangmeng
     */
    void initEncodeItems();

    MainWindow *m_Mainwindow  = nullptr;

    EncodeListModel *m_encodeModel = nullptr;

    QStandardItemModel *m_standardModel = nullptr;
    const int m_Space = 10;
    const int m_ContentWidth = 240;
    const int m_ItemWidth = 220;
    const int m_ItemHeight = 60;
    const int m_ListLenth = 1500;

    Qt::FocusReason m_foucusReason = INVALID_FOCUS_REASON;
    QModelIndex m_modelIndexChecked;

    //记录触摸屏下点击时Y轴坐标
    int m_touchPressPosY = 0;
    //记录触摸屏下滑动时Y轴移动最大距离
    int m_touchSlideMaxY = 0;
};

/*******************************************************************************
 1. @类名:    EncodeDelegate
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class EncodeDelegate : public DStyledItemDelegate
{
public:
    explicit EncodeDelegate(QAbstractItemView *parent = nullptr)
        : DStyledItemDelegate(parent), m_parentView(parent) {}

    /**
     * @brief 自绘事件
     * @author ut001121 张猛
     * @param painter
     * @param option
     * @param index
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    /**
     * @brief 尺寸提示
     * @author ut001121 zhangmeng
     * @param option
     * @param index
     * @return
     */
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    QAbstractItemView *m_parentView = nullptr;
};

#endif  // THEMELISTVIEW_H
