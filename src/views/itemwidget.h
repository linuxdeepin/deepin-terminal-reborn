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
 1. @类名:    ItemWidget
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    项控件
             可以通过类型选择选择项控件的展示内容
             1）分组项
             2）普通项
*******************************************************************************/
#ifndef ITEMWIDGET_H
#define ITEMWIDGET_H
// custom
#include "focusframe.h"

// dtk
//#include <DFrame>
#include <DIconButton>
#include <DLabel>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <DPalette>
#include <DApplicationHelper>
// qt
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE
#define COORDINATE_ERROR_Y  5

// 功能键类型
enum ItemFuncType {
    // 项按钮（编辑按钮）
    ItemFuncType_Item = 0,
    // 分组按钮（用于远程管理显示分组）
    ItemFuncType_Group = 1
};

// 文字颜色
enum ItemTextColor {
    // Text      // 第一行颜色
    ItemTextColor_Text = 0,
    // TextTips  // 第二行颜色
    ItemTextColor_TextTips
};

class IconButton;
// 远程管理项的通用view类
class ItemWidget : public FocusFrame
{
    Q_OBJECT
public:
    ItemWidget(ItemFuncType itemType, QWidget *parent = nullptr);
    ~ItemWidget();

    // 设置图标
    // 根据名称设置图标
    void setIcon(const QString &icon);
    // 设置功能图标
    void setFuncIcon(ItemFuncType iconType);
    // 设置文字
    void setText(const QString &firstline, const QString &secondline = "");
    // 是否匹配
    bool isEqual(ItemFuncType type, const QString &key);
    // 获取焦点
    void getFocus();
    // 丢失焦点
    void lostFocus();
    // 比较大小
    friend bool operator >(const ItemWidget &item1, const ItemWidget &item2);
    friend bool operator <(const ItemWidget &item1, const ItemWidget &item2);

public slots:
    // 处理功能键被点击的点击事件
    void onFuncButtonClicked();
    // 处理图标被点击的事件
    void onIconButtonClicked();
    // 处理焦点出事件
    void onFocusReback();
    void onFocusOut(Qt::FocusReason type);

signals:
    // 功能按钮被点击
    // 参数: item的名称 (数据的唯一值)
    // 执行项
    void itemClicked(const QString &strName);
    // 显示分组
    void groupClicked(const QString &strName, bool isFocus = false);
    // 修改项
    void itemModify(const QString &strName, bool isFocus = false);
    // 焦点从控件中出
    void focusOut(Qt::FocusReason type);

protected:
    // 初始化UI界面
    void initUI();
    // 初始化信号槽
    void initConnections();
    // 处理重绘事件
    void paintEvent(QPaintEvent *event) override;
    // 进入事件和出事件
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    // 鼠标点击事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    // 事件过滤
    bool eventFilter(QObject *watched, QEvent *event) override;

    // 键盘事件
    void keyPressEvent(QKeyEvent *event) override;
    // 焦点出入
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    // 主窗口布局
    QHBoxLayout *m_mainLayout = nullptr;
    // 图表布局
    QVBoxLayout *m_iconLayout = nullptr;
    // 文字布局
    QVBoxLayout *m_textLayout = nullptr;
    // 功能键布局
    QVBoxLayout *m_funcLayout = nullptr;

    // 图表
    DIconButton *m_iconButton = nullptr;
    // 文字
    DLabel *m_firstline = nullptr;
    DLabel *m_secondline = nullptr;
    // 功能键
    IconButton *m_funcButton = nullptr;

    // 文字内容
    QString m_firstText = "";
    QString m_secondText = "";
    // 功能按键默认是编辑按钮
    int m_functType;
    // 焦点是否在窗口上 用于编辑按钮是否显示
    bool m_isFocus = false;
  
    /***add begin by ut001121 zhangmeng 20200924 修复BUG48618***/
    //记录触摸屏下点击时Y轴坐标
    int m_touchPressPosY = 0;
    //记录触摸屏下滑动时Y轴移动最大距离
    int m_touchSlideMaxY = 0;
    // 移动事件来源
    QObject* m_moveSource = nullptr;
    /***add end by ut001121***/

    // 设置文字大小和颜色
    void setFont(DLabel *label, DFontSizeManager::SizeType fontSize, ItemTextColor colorType);
    // 设置字体大小
    void setFontSize(DLabel *label, DFontSizeManager::SizeType fontSize);
    // 设置字体颜色
    void setFontColor(DLabel *label, ItemTextColor colorType);
    // 返回指定颜色值
    QColor getColor(ItemTextColor colorType);
    // 处理键盘事件
    void rightKeyPress();
    // 项被点击
    void onItemClicked();
private slots:
    // 设置颜色随主题变化变化（部分组件不支持，需要手动变色）
    void slotThemeChange(DGuiApplicationHelper::ColorType themeType);
};

#endif // ITEMWIDGET_H
