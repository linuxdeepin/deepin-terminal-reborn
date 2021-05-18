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


#ifndef NEWDSPINBOX_H
#define NEWDSPINBOX_H

#include <DSpinBox>

#include <QRegExpValidator>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    NewDspinBox
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    设置里面重新写的spinBox 2020-11-06 renfeixiang 修改成继承DSpinBox 修改bug#53551
*******************************************************************************/
class NewDspinBox : public DSpinBox
{
    Q_OBJECT
public:
    explicit NewDspinBox(QWidget *parent = nullptr);

protected:
    /**
     * @brief 重写wheelEvent函数，只有有焦点的情况，才进行字体大小变化
     * @author ut001000 任飞翔
     * @param event
     */
    void wheelEvent(QWheelEvent *event) override;
};



#endif
