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
#include "titlebar.h"
#include "utils.h"

#include <DApplication>
#include <DIconButton>

#include <QIcon>
#include <QLabel>

#include <QDebug>

DWIDGET_USE_NAMESPACE

TitleBar::TitleBar(QWidget *parent, bool isQuakeWindowStyle) : QWidget(parent), m_layout(new QHBoxLayout(this))
{
    Utils::set_Object_Name(this);
    m_layout->setObjectName("TitleBarLayout");//Add by ut001000 renfeixiang 2020-08-13
    /******** Modify by m000714 daizhengwen 2020-04-15: 标签栏和Dtk标签色保持一致****************/
//    DPalette palette = this->palette();
//    palette.setBrush(DPalette::Background, palette.color(DPalette::Base));
//    this->setPalette(palette);
    this->setBackgroundRole(DPalette::Base);
    this->setAutoFillBackground(true);
    // daizhengwen fix bug#22927 动画出的矩形框会 -50 设置标题栏为50
    this->setFixedHeight(WIN_TITLE_BAR_HEIGHT);
    /********************* Modify by m000714 daizhengwen End ************************/
    m_layout->setContentsMargins(0, 0, 0, 0);

    if (isQuakeWindowStyle) {
        return;
    }

    if (DApplication::isDXcbPlatform()) {

        DIconButton *iconLabel = new DIconButton(this);
        iconLabel->setIcon(QIcon::fromTheme("deepin-terminal"));
        iconLabel->setIconSize(QSize(ICONSIZE_36, ICONSIZE_36));
        iconLabel->setFocusPolicy(Qt::NoFocus);
        iconLabel->setFlat(true);

        m_layout->addSpacing(10);

        /******** Modify by n014361 wangpeili 2020-02-12: 修改居中样式***********×****/
        m_layout->addWidget(iconLabel, 0, Qt::AlignVCenter);
        /***************** Modify by n014361 End ********************×****/
        m_layout->addSpacing(10);
    }
}

TitleBar::~TitleBar()
{
}

/*******************************************************************************
 1. @函数:    setTabBar
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    设置标签栏
*******************************************************************************/
void TitleBar::setTabBar(QWidget *widget)
{
    /******** Modify by n014361 wangpeili 2020-02-12: 修改居中样式***********×****/
    m_layout->addWidget(widget, 0, Qt::AlignVCenter);
    /***************** Modify by n014361 End ********************×****/
}

/*******************************************************************************
 1. @函数:    rightSpace
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    右边的空间
*******************************************************************************/
int TitleBar::rightSpace()
{
    return m_rightSpace;
}
