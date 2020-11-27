/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2018 Deepin, Inc.
 *
 * Author:     Wang Peili <wangpeili@deepin.com>
 * Maintainer: Rekols     <rekols@foxmail.com>
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
#include <private/qtextengine_p.h>
#include <private/qfontengine_p.h>
#include <private/qpainter_p.h>

#include <DLog>
#include <QAction>
#include "utils.h"

NewDspinBox::NewDspinBox(QWidget *parent) : DWidget(parent)
{
    Utils::set_Object_Name(this);
    m_DIconBtnAdd = new DIconButton(DStyle::SP_IncreaseElement);
    m_DIconBtnAdd->setObjectName("NewDspinBoxAddDIconButton");//Add by ut001000 renfeixiang 2020-08-13
    m_DIconBtnAdd->setFixedSize(36, 36);
    m_DIconBtnSubtract = new DIconButton(DStyle::SP_DecreaseElement);
    m_DIconBtnSubtract->setObjectName("NewDspinBoxSubDIconButton");//Add by ut001000 renfeixiang 2020-08-13
    m_DIconBtnSubtract->setFixedSize(36, 36);
    m_DLineEdit = new DLineEdit();
    m_DLineEdit->setObjectName("NewDspinBoxFontSizeDLineEdit");//Add by ut001000 renfeixiang 2020-08-13
    m_DLineEdit->setClearButtonEnabled(false);
    m_DLineEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    QHBoxLayout *pHBoxLayout = new QHBoxLayout();

    pHBoxLayout->setSpacing(10);
    pHBoxLayout->setContentsMargins(0, 0, 0, 0);
    pHBoxLayout->addWidget(m_DLineEdit);
    pHBoxLayout->addWidget(m_DIconBtnAdd);
    pHBoxLayout->addWidget(m_DIconBtnSubtract);
    setLayout(pHBoxLayout);

    /******** Add by nt001000 renfeixiang 2020-05-26:增加正则表达式限制00000现象 Begin***************/
    QRegExp regExp("(^[1-4][0-9]$)|(^[5][0]$)|(^[1-9]$)");
    m_DLineEdit->lineEdit()->setValidator(new QRegExpValidator(regExp, this));
    /******** Add by nt001000 renfeixiang 2020-05-26:增加正则表达式限制00000现象 End***************/
    m_DLineEdit->setFocusPolicy(Qt::NoFocus);

    /** add begin by ut001121 zhangmeng 20200723 for sp3 keyboard interaction */
    // "+"和"-"按钮不接受Tab焦点
    m_DIconBtnAdd->setFocusPolicy(Qt::NoFocus);
    m_DIconBtnSubtract->setFocusPolicy(Qt::NoFocus);

    //安装事件过滤器
    m_DLineEdit->installEventFilter(this);

    // 完成输入后矫正数据并发送信号
    connect(m_DLineEdit, &DLineEdit::editingFinished, this, [ = ] {
        correctValue();
        emit valueChanged(m_DLineEdit->lineEdit()->text().toInt());
    });
    /** add end by ut001121 zhangmeng 20200723 */

    connect(m_DIconBtnAdd, &QAbstractButton::clicked, this, [ = ] {
        /***add by ut001121 zhangmeng 20200509 修复BUG#24848 设置中点击“+”“-”按钮修改字体大小，前方输入框未高亮***/
        m_DLineEdit->lineEdit()->setFocus();

        int value = m_DLineEdit->lineEdit()->text().toInt();
        if (value < MAX_FONT_SZIE)
        {
            m_DLineEdit->lineEdit()->setText(QString::number(value + 1));
            emit valueChanged(m_DLineEdit->lineEdit()->text().toInt());
        }
    });
    connect(m_DIconBtnSubtract, &QAbstractButton::clicked, this, [ = ] {
        /***add by ut001121 zhangmeng 20200509 修复BUG#24848 设置中点击“+”“-”按钮修改字体大小，前方输入框未高亮***/
        m_DLineEdit->lineEdit()->setFocus();

        int value = m_DLineEdit->lineEdit()->text().toInt();
        if (value > MIN_FONT_SZIE)
        {
            m_DLineEdit->lineEdit()->setText(QString::number(value - 1));
            emit valueChanged(m_DLineEdit->lineEdit()->text().toInt());
        }
    });

    // 脱离焦点后校正生效．
    connect(m_DLineEdit, &DLineEdit::focusChanged, this, [ = ](bool var) {
        // 退出编辑的时候，数据做个校正
        if (!var) {
            /******** Add by nt001000 renfeixiang 2020-05-25:增加正常数据的过滤不需要处理，textChanged信号已经处理 Begin***************/
            int value = m_DLineEdit->lineEdit()->text().toInt();
            if (value >= MIN_FONT_SZIE)
                return ;
            /******** Add by nt001000 renfeixiang 2020-05-25:增加正常数据的过滤不需要处理，textChanged信号已经处理 End***************/
            correctValue();
            emit valueChanged(m_DLineEdit->lineEdit()->text().toInt());
        }
    });

    /******** Add by nt001000 renfeixiang 2020-05-25:增加m_DLineEdit的textChanged信号的响应 Begin***************/
    connect(m_DLineEdit, &DLineEdit::textChanged, this, [ = ](const QString & value) {
        //过滤范围之外的数据
        if (value.toInt() < MIN_FONT_SZIE)
            return ;
        // 对输入的数据做个校正
        //correctValue();
        emit valueChanged(m_DLineEdit->lineEdit()->text().toInt());
    });
    /******** Add by nt001000 renfeixiang 2020-05-25:增加m_DLineEdit的textChanged信号的响应 End***************/

    // 选择即进入
    connect(m_DLineEdit, &DLineEdit::selectionChanged, this, [ = ] {
        if (!m_DLineEdit->lineEdit()->hasFocus()
                && !m_DLineEdit->lineEdit()->selectedText().isEmpty())
        {
            m_DLineEdit->lineEdit()->setFocus();
        }
    });
}

/*******************************************************************************
 1. @函数:    eventFilter
 2. @作者:    ut001121 张猛
 3. @日期:    2020-07-23
 4. @说明:    事件过滤器
*******************************************************************************/
bool NewDspinBox::eventFilter(QObject *watched, QEvent *event)
{
    /** add begin ut001121 zhangmeng 20200723 for sp3 keyboard interaction */
    // 处理编辑框上键事件,调整字体大小
    if (watched == m_DLineEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        switch (key_event->key()) {
        case Qt::Key_Up:
            m_DIconBtnAdd->animateClick(80);
            return true;

        case Qt::Key_Down:
            m_DIconBtnSubtract->animateClick(80);
            return true;

        default:
            break;
        }
    }
    /** add end ut001121 zhangmeng 20200723 */

    return DWidget::eventFilter(watched, event);
}

/*******************************************************************************
 1. @函数:    setValue
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    设置界面设置值
*******************************************************************************/
void NewDspinBox::setValue(int val)
{
    m_DLineEdit->setText(QString::number(val));
}

/*******************************************************************************
 1. @函数:    correctValue
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    设置界面检测当前值是否在最小值和最大值范围内
*******************************************************************************/
void NewDspinBox::correctValue()
{
    int val = m_DLineEdit->text().toInt();
    if (val > MAX_FONT_SZIE) {
        m_DLineEdit->lineEdit()->setText(QString::number(MAX_FONT_SZIE));
    }
    if (val < MIN_FONT_SZIE) {
        m_DLineEdit->lineEdit()->setText(QString::number(MIN_FONT_SZIE));
    }
}

/***del begin by ut001121 zhangmeng 20200515 修复BUG44606***/
#if 0
//拷贝了Qt QTextLayout类的源码
void QTextLayout::drawCursor(QPainter *p, const QPointF &pos, int cursorPosition, int width) const
{
    Q_UNUSED(width)

    if (d->lines.isEmpty())
        return;

    if (!d->layoutData)
        d->itemize();

    QPointF position = pos + d->position;

    cursorPosition = qBound(0, cursorPosition, d->layoutData->string.length());
    int line = d->lineNumberForTextPosition(cursorPosition);
    if (line < 0)
        line = 0;
    if (line >= d->lines.size())
        return;

    QTextLine l(line, d);
    const QScriptLine &sl = d->lines.at(line);

    qreal x = position.x() + l.cursorToX(cursorPosition);

    int itm;

    if (d->visualCursorMovement()) {
        if (cursorPosition == sl.from + sl.length)
            cursorPosition--;
        itm = d->findItem(cursorPosition);
    } else
        itm = d->findItem(cursorPosition - 1);

    QFixed base = sl.base();
    QFixed descent = sl.descent;
    bool rightToLeft = d->isRightToLeft();
    if (itm >= 0) {
        const QScriptItem &si = d->layoutData->items.at(itm);
        if (si.ascent > 0)
            base = si.ascent;
        if (si.descent > 0)
            descent = si.descent;
        rightToLeft = si.analysis.bidiLevel % 2;
    }
    qreal y = position.y() + (sl.y + sl.base() - base).toReal();
    bool toggleAntialiasing = !(p->renderHints() & QPainter::Antialiasing)
                              && (p->transform().type() > QTransform::TxTranslate);
    if (toggleAntialiasing)
        p->setRenderHint(QPainter::Antialiasing);
    QPainter::CompositionMode origCompositionMode = p->compositionMode();
    if (p->paintEngine()->hasFeature(QPaintEngine::RasterOpModes))
        p->setCompositionMode(QPainter::RasterOp_NotDestination);
    //fix bug 25203 字体输入框的光标显示较粗
    //直接将原有的qreal(width) 替换成了0.1
    p->fillRect(QRectF(x, y, 0.1, (base + descent).toReal()), p->pen().brush());
    p->setCompositionMode(origCompositionMode);
    if (toggleAntialiasing)
        p->setRenderHint(QPainter::Antialiasing, false);
    if (d->layoutData->hasBidi) {
        const int arrow_extent = 4;
        int sign = rightToLeft ? -1 : 1;
        p->drawLine(QLineF(x, y, x + (sign * arrow_extent / 2), y + arrow_extent / 2));
        p->drawLine(QLineF(x, y + arrow_extent, x + (sign * arrow_extent / 2), y + arrow_extent / 2));
    }
    return;
}
#endif
/***del end by ut001121***/
