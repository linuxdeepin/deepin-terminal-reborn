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
#include "itemwidget.h"
#include "serverconfigmanager.h"
#include "utils.h"
#include "iconbutton.h"

// dtk

// qt
#include <QDebug>
#include <QApplication>

// 需要选择Item类型
ItemWidget::ItemWidget(ItemFuncType itemType, QWidget *parent)
    : FocusFrame(parent),
      m_mainLayout(new QHBoxLayout(this)),
      m_iconLayout(new QVBoxLayout),
      m_textLayout(new QVBoxLayout),
      m_funcLayout(new QVBoxLayout),
      m_iconButton(new DIconButton(this)),
      m_firstline(new DLabel(this)),
      m_secondline(new DLabel(this)),
      m_funcButton(new IconButton(this)),
      m_functType(itemType)
{
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 Begin***************/
    Utils::set_Object_Name(this);
    m_mainLayout->setObjectName("ItemWidgetMainLayout");
    m_iconLayout->setObjectName("ItemWidgetIconLayout");
    m_textLayout->setObjectName("ItemWidgetTextLayout");
    m_funcLayout->setObjectName("ItemWidgetFuncLayout");
    m_iconButton->setObjectName("ItemWidgetIconButton");
    m_firstline->setObjectName("ItemWidgetFirstLineLabel");
    m_secondline->setObjectName("ItemWidgetSecondLineLabel");
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 End***************/
    // 界面初始化
    initUI();
    // 信号槽初始化
    initConnections();

    /***add begin by ut001121 zhangmeng 20200924 安装过滤器 修复BUG48618***/
    m_iconButton->installEventFilter(this);
    m_funcButton->installEventFilter(this);
    installEventFilter(this);
    /***add end by ut001121***/
}

ItemWidget::~ItemWidget()
{
}

/*******************************************************************************
 1. @函数:    setIcon
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-10
 4. @说明:    设置图表(远程组图标 dt_server_group 远程服务器图标 dt_server 自定义图标 dt_command)
*******************************************************************************/
void ItemWidget::setIcon(const QString &icon)
{
    m_iconButton->setIcon(QIcon::fromTheme(icon));
    m_iconButton->setIconSize(QSize(44, 44));
}

/*******************************************************************************
 1. @函数:    setFuncIcon
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-10
 4. @说明:    根据类型设置功能图标 (编辑 dt_edit 显示分组 dt_arrow_right)
*******************************************************************************/
void ItemWidget::setFuncIcon(ItemFuncType iconType)
{
    // 统一设置大小
    m_funcButton->setIconSize(QSize(20, 20));
    switch (iconType) {
    case ItemFuncType_Item:
        m_funcButton->setIcon(QIcon::fromTheme("dt_edit"));
        m_funcButton->hide();
        break;
    case ItemFuncType_Group:
        m_funcButton->setIcon(QIcon::fromTheme("dt_arrow_right"));
        m_funcButton->show();
        break;
    default:
        break;
    }

}

/*******************************************************************************
 1. @函数:    setText
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-10
 4. @说明:    设置文字内容
*******************************************************************************/
void ItemWidget::setText(const QString &firstline, const QString &secondline)
{
    // 第一行信息 唯一值
    m_firstText = firstline;
    m_firstline->setText(firstline);

    // 第二行信息
    switch (m_functType) {
    case ItemFuncType_Item:
        // 输入的第二行信息
        m_secondText = secondline;
        break;
    case ItemFuncType_Group:
        // 第二行 组内服务器个数
        int serverCount = ServerConfigManager::instance()->getServerCount(firstline);
        if (serverCount <= 0) {
            qDebug() << "get error count " << serverCount;
            serverCount = 0;
        }
        m_secondText = QString("%1 server").arg(serverCount);
        break;
    }
    // 设置第二行信息
    m_secondline->setText(m_secondText);
}

/*******************************************************************************
 1. @函数:    isEqual
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    判断是否是此widget
*******************************************************************************/
bool ItemWidget::isEqual(ItemFuncType type, const QString &key)
{
    // fistText是唯一值
    return (m_functType == type && key == m_firstText);
}

/*******************************************************************************
 1. @函数:    getFocus
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    获取焦点，显示编辑按钮, 这是键盘获取焦点
*******************************************************************************/
void ItemWidget::getFocus()
{
    m_isFocus = true;
    // 项显示功能键
    if (m_functType == ItemFuncType_Item) {
        m_funcButton->show();
        qDebug() << "edit button show";
    }
}

/*******************************************************************************
 1. @函数:    lostFocus
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    丢失焦点，显示编辑按钮
*******************************************************************************/
void ItemWidget::lostFocus()
{
    m_isFocus = false;
    // 项影藏功能键
    if (m_functType == ItemFuncType_Item && !m_isHover) {
        m_funcButton->hide();
    }
}

/*******************************************************************************
 1. @函数:    重载比较 >
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    根据类型，文字比较大小
*******************************************************************************/
bool operator >(const ItemWidget &item1, const ItemWidget &item2)
{
    // item1 item2都是组 或 item1 item2 都不是组
    if (item1.m_functType == item2.m_functType) {
        // 比较文字
        if (QString::compare(item1.m_firstText, item2.m_firstText) < 0) {
            // 小于 0 例:ab bc
            return true;
        } else {
            // 其他情况
            return false;
        }
    } else if (item1.m_functType > item2.m_functType) {
        // item1 是组 1 item2不是 0
        return true;
    } else {
        // item1 不是组 0 item2是 1
        return false;
    }
}

/*******************************************************************************
 1. @函数:    重载比较 <
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    根据类型，文字比较大小
*******************************************************************************/
bool operator <(const ItemWidget &item1, const ItemWidget &item2)
{
    return !(item1 > item2);
}

/*******************************************************************************
 1. @函数:    onFuncButtonClicked
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-13
 4. @说明:    处理功能键点击的点击事件
              1) 分组=>显示分组所有的项
              2) 项=>修改项
*******************************************************************************/
void ItemWidget::onFuncButtonClicked()
{
    /***add begin by ut001121 zhangmeng 20200924 判断移动事件来源 修复BUG48618***/
    if(m_moveSource){
        m_moveSource = nullptr;
        return ;
    }
    /***add end by ut001121***/

    // 判断类型执行操作
    switch (m_functType) {
    case ItemFuncType_Group:
        // 显示分组
        qDebug() << "group show" << m_firstText;
        // 第一个参数是分组名，第二个参数是当前是否有焦点
        emit groupClicked(m_firstText, m_isFocus);
        break;
    case ItemFuncType_Item: {
        // 修改项
        qDebug() << "modify item" << m_firstText;
        bool isFocusOn = false;
        if (m_funcButton->hasFocus() || m_isFocus) {
            // 焦点在大框或者编辑按钮上
            isFocusOn = true;
        }
        itemModify(m_firstText, isFocusOn);
    }
    break;
    default:
        break;
    }
}

/*******************************************************************************
 1. @函数:    onFuncButtonClicked
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-16
 4. @说明:    处理图标点击的点击事件
              1) 分组=>显示分组所有的项
              2) 项=>执行操作
*******************************************************************************/
void ItemWidget::onIconButtonClicked()
{
    /***add begin by ut001121 zhangmeng 20200924 判断移动事件来源 修复BUG48618***/
    if(m_moveSource){
        m_moveSource = nullptr;
        return ;
    }
    /***add end by ut001121***/

    // 判断类型执行操作
    switch (m_functType) {
    case ItemFuncType_Group:
        // 显示分组
        qDebug() << "group show" << m_firstText;
        // 第一个参数是分组名，第二个参数是当前是否有焦点
        emit groupClicked(m_firstText, m_isFocus);
        break;
    case ItemFuncType_Item:
        // 项被点击
        qDebug() << "item clicked" << m_firstText;
        emit itemClicked(m_firstText);
        break;
    default:
        qDebug() << "default" ;
        break;
    }
}

/*******************************************************************************
 1. @函数:    onFuncButtonFocusOut
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-17
 4. @说明:    功能键丢失焦点，当前窗口获得焦点
*******************************************************************************/
void ItemWidget::onFocusReback()
{
    qDebug() << __FUNCTION__;
    setFocus();
}

/*******************************************************************************
 1. @函数:    onFocusOut
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-21
 4. @说明:    焦点从功能键切出，切不在当前控件上
*******************************************************************************/
void ItemWidget::onFocusOut(Qt::FocusReason type)
{
    // Tab切出
    if (type == Qt::TabFocusReason || type == Qt::BacktabFocusReason) {
        emit focusOut(type);
    }
    if (type == Qt::ActiveWindowFocusReason) {
        // 例如:super后返回都需要将焦点返回项
        setFocus();
        qDebug() << "set focus back itemwidget";
    }
    // 项
    if (m_functType == ItemFuncType_Item) {
        if (type != Qt::OtherFocusReason && !m_isHover) {
            m_funcButton->hide();
        }
    }

}

/*******************************************************************************
 1. @函数:    initUI
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-08
 4. @说明:    初始化单个窗口组建的布局和空间大小
*******************************************************************************/
void ItemWidget::initUI()
{
    // 初始化控件大小
    setGeometry(0, 0, 220, 60);
    setFixedSize(220, 60);
    setContentsMargins(0, 0, 0, 0);
    // 圆角
    setFrameRounded(true);
    // 设置焦点策略
    setFocusPolicy(Qt::NoFocus);

    // 图标布局
    // 图标起始大小
    m_iconButton->setGeometry(0, 0, 44, 44);
    m_iconButton->setFlat(true);
    m_iconLayout->setContentsMargins(8, 8, 8, 8);
    m_iconLayout->addWidget(m_iconButton, 0, Qt::AlignHCenter);
    m_iconButton->setFocusPolicy(Qt::NoFocus);

    // 文字布局
    // 设置文字格式（字体大小和颜色）
    setFont(m_firstline, DFontSizeManager::T7, ItemTextColor_Text);
    setFont(m_secondline, DFontSizeManager::T8, ItemTextColor_TextTips);
    m_firstline->setContentsMargins(0, 0, 0, 0);
    m_secondline->setContentsMargins(0, 0, 0, 0);
    m_firstline->setFixedWidth(138);
    m_secondline->setFixedWidth(138);
    m_textLayout->setContentsMargins(0, 0, 0, 0);
    m_textLayout->setSpacing(0);
    m_textLayout->addStretch(13);
    m_textLayout->addWidget(m_firstline, 13);
    m_textLayout->addStretch(9);
    m_textLayout->addWidget(m_secondline, 13);
    m_textLayout->addStretch(13);

    // 功能键布局
    setFuncIcon(ItemFuncType(m_functType));
    m_funcButton->setFlat(true);
    m_funcButton->setFocusPolicy(Qt::NoFocus);
    m_funcLayout->addStretch();
    m_funcLayout->setContentsMargins(5, 0, 5, 0);
    m_funcLayout->addWidget(m_funcButton);
    m_funcLayout->addStretch();

    // 整体布局
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addLayout(m_iconLayout);
    m_mainLayout->addLayout(m_textLayout);
    m_mainLayout->addLayout(m_funcLayout);
    m_mainLayout->addStretch();
    setLayout(m_mainLayout);
}

/*******************************************************************************
 1. @函数:    initConnections
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-08
 4. @说明:    关联信号槽
*******************************************************************************/
void ItemWidget::initConnections()
{
    // 颜色随主题变化
    connect(DApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ItemWidget::slotThemeChange);
    // 功能键被点击
    connect(m_funcButton, &DIconButton::clicked, this, &ItemWidget::onFuncButtonClicked);
    // 焦点从小图标切出，但在控件上
    connect(m_funcButton, &IconButton::preFocus, this, &ItemWidget::onFocusReback);
    // 焦点从小图标切出，不在控件上
    connect(m_funcButton, &IconButton::focusOut, this, &ItemWidget::onFocusOut);
    // 图标被点击
    connect(m_iconButton, &DIconButton::clicked, this, &ItemWidget::onIconButtonClicked);

}

// 重绘事件
/*******************************************************************************
 1. @函数:    paintEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-10
 4. @说明:    1.字体变长便短导致的问题 2. 悬浮的背景色
*******************************************************************************/
void ItemWidget::paintEvent(QPaintEvent *event)
{
    // 绘制文字长短
    QFont firstFont = m_firstline->font();
    QFont secondFont = m_secondline->font();
    // 限制文字长度,防止超出
    QString firstText = Utils::getElidedText(firstFont, m_firstText, ITEMMAXWIDTH);
    QString secondText = Utils::getElidedText(secondFont, m_secondText, ITEMMAXWIDTH);
    // 设置显示的文字
    m_firstline->setText(firstText);
    m_secondline->setText(secondText);

    FocusFrame::paintEvent(event);
}

/*******************************************************************************
 1. @函数:    enterEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-10
 4. @说明:    进入，编辑按钮显示
*******************************************************************************/
void ItemWidget::enterEvent(QEvent *event)
{
    qDebug() << __FUNCTION__;
    // 编辑按钮现
    if (m_functType == ItemFuncType_Item) {
        m_funcButton->show();
    }

    FocusFrame::enterEvent(event);
}

/*******************************************************************************
 1. @函数:    leaveEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-10
 4. @说明:    移出，编辑按钮消失
*******************************************************************************/
void ItemWidget::leaveEvent(QEvent *event)
{
    // 判断焦点是否是选中状态，不是的话，清除选中效果
    if (!m_isFocus && !m_funcButton->hasFocus()) {
        // 编辑按钮出
        if (m_functType == ItemFuncType_Item) {
            m_funcButton->hide();
        }
    }

    FocusFrame::leaveEvent(event);
}

/*******************************************************************************
 1. @函数:    mousePressEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-13
 4. @说明:    处理item的点击事件
              1) 分组=>显示分组所有的项
              2) 项=>连接远程
*******************************************************************************/
void ItemWidget::mousePressEvent(QMouseEvent *event)
{
//    // 判断类型执行操作
//    onItemClicked();
//    // 捕获事件
//    event->accept();
#if 0
    //记录鼠标点击时的时间戳
    m_tapTimeSpace = event->timestamp();
#endif
    FocusFrame::mousePressEvent(event);
}

/*******************************************************************************
 1. @函数:    mouseReleaseEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-13
 4. @说明:    处理item的点击事件
*******************************************************************************/
void ItemWidget::mouseReleaseEvent(QMouseEvent *event)
{
#if 0
    qDebug() << __FUNCTION__ ;
    //根据鼠标按下弹起的差值判断是否触发点击事件
    if (event->timestamp() - m_tapTimeSpace > TAP_TIME_SPACE_T
            && (Qt::MouseEventNotSynthesized == event->source() || Qt::MouseEventSynthesizedByQt == event->source())) {
        event->accept();
        return;
    }
#endif
    // 判断类型执行操作
    onItemClicked();
    // 捕获事件
    event->accept();

    FocusFrame::mouseReleaseEvent(event);
}

/*******************************************************************************
 1. @函数:    eventFilter
 2. @作者:    ut001121 张猛
 3. @日期:    2020-09-24
 4. @说明:    事件过滤
*******************************************************************************/
bool ItemWidget::eventFilter(QObject *watched, QEvent *event)
{
    /***add begin by ut001121 zhangmeng 20200924 过滤并处理鼠标事件 修复BUG48618***/
    QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
    if(event->type() == QEvent::MouseButtonPress
            && mouse->source() == Qt::MouseEventSynthesizedByQt){
        //记录移动事件来源
        m_moveSource = watched;
        //记录移动事件起点
        m_touchPressPosY = mouse->globalPos().y();
        //重置滑动最大距离
        m_touchSlideMaxY = 0;
    }
    if(event->type() == QEvent::MouseButtonRelease
            && mouse->source() == Qt::MouseEventSynthesizedByQt){
        //判断最大移动距离
        if (m_touchSlideMaxY<=COORDINATE_ERROR_Y) {
            //移动事件源为空,表示未曾移动过
            m_moveSource = nullptr;
        }
    }
    if(event->type() == QEvent::MouseMove /*&& m_moveSource == watched*/
            && mouse->source() == Qt::MouseEventSynthesizedByQt){
        //记录滑动最大距离
        m_touchSlideMaxY = qMax(m_touchSlideMaxY, qAbs(m_touchPressPosY - mouse->globalPos().y()));
    }
    /***add end by ut001121***/
    return FocusFrame::eventFilter(watched, event);
}

/*******************************************************************************
 1. @函数:    keyPressEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-17
 4. @说明:    键盘点击事件，处理右键操作
*******************************************************************************/
void ItemWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Right:
        // 点击键盘右键
        qDebug() << "right key press";
        rightKeyPress();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Space:
        // 项被点击执行
        onItemClicked();
        break;
    default:
        FocusFrame::keyPressEvent(event);
        break;
    }

}

/*******************************************************************************
 1. @函数:    focusInEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-23
 4. @说明:    焦点进入事件
*******************************************************************************/
void ItemWidget::focusInEvent(QFocusEvent *event)
{
    m_isFocus = true;
    if (m_functType == ItemFuncType_Item) {
        m_funcButton->show();
    }
    FocusFrame::focusInEvent(event);
}

/*******************************************************************************
 1. @函数:    __FUNCTION__
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-23
 4. @说明:    焦点丢失事件
*******************************************************************************/
void ItemWidget::focusOutEvent(QFocusEvent *event)
{
    qDebug() << "ItemWidget" << __FUNCTION__ << event->reason();

    m_isFocus = false;
    // Tab切出
    Qt::FocusReason type = event->reason();
    if (type == Qt::TabFocusReason || type == Qt::BacktabFocusReason) {
        emit focusOut(type);
    }

    if (m_functType == ItemFuncType_Item) {
        // 编辑按钮也没焦点，则隐藏编辑按钮
        if (!m_funcButton->hasFocus() && !m_isHover) {
            m_funcButton->hide();
        }
    }
    FocusFrame::focusOutEvent(event);
}

/*******************************************************************************
1. @函数:    setFont
2. @作者:    ut000610 戴正文
3. @日期:    2020 - 07 - 08
4. @说明:    给指定label设置初始化字体和颜色
******************************************************************************* */
void ItemWidget::setFont(DLabel *label, DFontSizeManager::SizeType fontSize, ItemTextColor colorType)
{
    setFontSize(label, fontSize);
    setFontColor(label, colorType);
}

/*******************************************************************************
 1. @函数:    setFontSize
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-08
 4. @说明:    设置字体大小
*******************************************************************************/
void ItemWidget::setFontSize(DLabel *label, DFontSizeManager::SizeType fontSize)
{
    // 设置字体大小随系统变化
    DFontSizeManager::instance()->bind(label, fontSize);
}

/*******************************************************************************
 1. @函数:    setFontColor
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-08
 4. @说明:    设置字体颜色
*******************************************************************************/
void ItemWidget::setFontColor(DLabel *label, ItemTextColor colorType)
{
    DPalette fontPalette = DApplicationHelper::instance()->palette(label);
    QColor color = getColor(colorType);
    //    qDebug() << color;
    if (color.isValid()) {
        fontPalette.setBrush(DPalette::Text, color);
        label->setPalette(fontPalette);
        //        qDebug() << label->palette();
    } else {
        qDebug() << __FUNCTION__ << "can't get text color";
    }
}

/*******************************************************************************
 1. @函数:    getColor
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-08
 4. @说明:    根据指定值，返回颜色
             目的兼容DPalette::ColorType 和 QPalette::ColorRole
*******************************************************************************/
QColor ItemWidget::getColor(ItemTextColor colorType)
{
    // 获取标准颜色
    DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
    DPalette textPalette = appHelper->standardPalette(appHelper->themeType());
    // 获取对应主题下的颜色
    QColor color;
    switch (colorType) {
    case ItemTextColor_Text: {
        color =  textPalette.color(DPalette::Text);
    }
    break;
    case ItemTextColor_TextTips: {
        color =  textPalette.color(DPalette::TextTips);
    }
    break;
    default:
        qDebug() << "item widget text unknow color type!" << colorType;
        break;
    }
    return color;
}

/*******************************************************************************
 1. @函数:    RightKeyPress
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-17
 4. @说明:    右键操作，组 => 显示分组 项 => 设置焦点
*******************************************************************************/
void ItemWidget::rightKeyPress()
{
    switch (m_functType) {
    case ItemFuncType_Group: {
        // 显示分组
        qDebug() << "group show" << m_firstText;
        emit groupClicked(m_firstText, true);
    }
    break;
    case ItemFuncType_Item:
        // 编辑按钮获得焦点
        qDebug() << "item get focus" << m_firstText;
        m_funcButton->show();
        m_funcButton->setFocus();
        break;
    default:
        break;
    }
}

/*******************************************************************************
 1. @函数:    onItemClicked
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-22
 4. @说明:    根据类型发送不同的点击信号
*******************************************************************************/
void ItemWidget::onItemClicked()
{
    /***add begin by ut001121 zhangmeng 20200924 判断移动事件来源 修复BUG48618***/
    if(m_moveSource){
        m_moveSource = nullptr;
        return ;
    }
    /***add end by ut001121***/

    switch (m_functType) {
    case ItemFuncType_Group:
        // 显示分组
        qDebug() << "group show" << m_firstText;
        // 第一个参数是分组名，第二个参数是当前是否有焦点
        emit groupClicked(m_firstText, m_isFocus);
        break;
    case ItemFuncType_Item:
        // 项被点击
        qDebug() << "item clicked" << m_firstText;
        emit itemClicked(m_firstText);
        break;
    default:
        break;
    }
}

/*******************************************************************************
 1. @函数:    slotThemeChange
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-08
 4. @说明:    部分组件随主题颜色变化而变化
*******************************************************************************/
void ItemWidget::slotThemeChange(DGuiApplicationHelper::ColorType themeType)
{
    Q_UNUSED(themeType);
    setFontColor(m_firstline, ItemTextColor_Text);
    setFontColor(m_secondline, ItemTextColor_TextTips);
}
