/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen <daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen <daizhengwen@uniontech.com>
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

/*******************************************************************************
 1. @类名:    ListView
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    列表控件
             提供添加、删除和更新列表项和选择列表项焦点的功能
             根据自定义或远程设置不同的列表展示
*******************************************************************************/
#ifndef LISTVIEW_H
#define LISTVIEW_H
#include "itemwidget.h"

// dtk
#include <DWidget>

// qt
#include <QVBoxLayout>
#include <QList>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QScrollBar>
#include <QScrollArea>
#include <QWheelEvent>

DWIDGET_USE_NAMESPACE

enum ListType {
    // 自定义
    ListType_Custom,
    // 远程
    ListType_Remote
};

class ServerConfigOptDlg;
class CustomCommandOptDlg;
class ListView : public QScrollArea
{
    Q_OBJECT
public:
    ListView(ListType type, QWidget *parent = nullptr);
    ~ListView() override;

    // 添加项
    void addItem(ItemFuncType type, const QString &key, const QString &strDescription = "");
    // 删除项
    bool removeItem(ItemFuncType type, const QString &key);
    // 更新选项
    bool updateItem(ItemFuncType type, const QString &key, const QString &newKey, const QString &strDescription = "");
    // 获取数量
    int count();
    // 清空数据
    void clearData();
    // 根据名称获取index
    int indexFromString(const QString &key, ItemFuncType type = ItemFuncType_Item);
    // 获取当前焦点位置
    int currentIndex();
    // 获取下一个的index
    int getNextIndex(int index);
    // 设置当前焦点
    void setCurrentIndex(int currentIndex);
    // 清空焦点
    void clearIndex();
    // 切换焦点
    void setFocusFromeIndex(int currentIndex, bool UpOrDown);
    // 丢失焦点
    void lostFocus(int preIndex);
    // 设置滚轮
    void setScroll(int currentIndex);
    // 获取焦点状态
    bool getFocusState() const;
    // 设置焦点状态
    void setFocusState(bool focusState);

public slots:
    // 列表项被修改
    void onItemModify(const QString &key, bool isFocusOn);

signals:
    // 列表数量变化
    void listItemCountChange();
    // 列表项被点击
    void itemClicked(const QString &key);
    // 分组项被点击
    void groupClicked(const QString &key, bool isFocus);
    // 列表项要被修改
    // void itemModify(const QString &key);
    // 焦点Tab出
    void focusOut(Qt::FocusReason type);

protected:
    // 键盘事件
    void keyPressEvent(QKeyEvent *event) override;
    // 焦点事件
    void focusInEvent(QFocusEvent *event) override;
    // 鼠标移动
    void mouseMoveEvent(QMouseEvent *) override;
    // 鼠标按下
    void mousePressEvent(QMouseEvent *) override;

private:
    // 列表类型
    ListType m_type;
    // 焦点切出还是进入 true 焦点在  false 焦点不在
    bool m_focusState = false;
    // 当前焦点
    int m_currentIndex = -1;
    // 当前滚轮位置
    int m_scrollPostion = 0;
    // 幕布
    DWidget *m_mainWidget = nullptr;
    // 远程弹窗
    ServerConfigOptDlg *m_configDialog = nullptr;
    // 自定义弹窗
    CustomCommandOptDlg *m_pdlg = nullptr;
    // 主布局
    QVBoxLayout *m_mainLayout = nullptr;
    // 列表项数据
    QList<ItemWidget *> m_itemList;

    // 初始化界面
    void initUI();
    // 设置项图标
    void setItemIcon(ItemFuncType type, ItemWidget *item);
    // 添加项到布局
    int getWidgetIndex(ItemWidget *itemWidget);
    // 判断index的有效性
    bool indexIsValid(int index);
    // 计算range
    int calculateRange(int height);
    // 远程管理修改项
    void onRemoteItemModify(const QString &key, bool isFocusOn);
    // 自定义修改项
    void onCustomItemModify(const QString &key, bool isFocusOn);
};

#endif // LISTVIEW_H
