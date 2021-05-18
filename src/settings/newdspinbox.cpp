/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     zhukewei <zhukewei@uniontech.com>
*
* Maintainer: zhukewei <zhukewei@uniontech.com>
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

#include "newdspinbox.h"
#include "utils.h"

NewDspinBox::NewDspinBox(QWidget *parent) : DSpinBox(parent)
{
    Utils::set_Object_Name(this);

    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setButtonSymbols(QAbstractSpinBox::PlusMinus);
    /******** Add by nt001000 renfeixiang 2020-05-26:增加正则表达式限制00000现象 Begin***************/
    QRegExp regExp("(^[1-4][0-9]$)|(^[5][0]$)|(^[1-9]$)");
    lineEdit()->setValidator(new QRegExpValidator(regExp, this));
    this->setMinimum(5);
    this->setMaximum(50);
    /******** Add by nt001000 renfeixiang 2020-05-26:增加正则表达式限制00000现象 End***************/
}

void NewDspinBox::wheelEvent(QWheelEvent *event)
{
    if (lineEdit()->hasFocus()) {
        DSpinBox::wheelEvent(event);
    }
}

