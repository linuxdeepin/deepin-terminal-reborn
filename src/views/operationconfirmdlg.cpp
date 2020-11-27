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
#include "operationconfirmdlg.h"
#include "utils.h"

#include <DApplicationHelper>
#include <DGuiApplicationHelper>
#include <DButtonBox>
#include <DIconButton>
#include <DFontSizeManager>
#include <DVerticalLine>
#include <DLog>


OperationConfirmDlg::OperationConfirmDlg(QWidget *parent)
    : DAbstractDialog(parent)
{
    Utils::set_Object_Name(this);
    initUI();
    initContentLayout();
    initConnections();
}

/*******************************************************************************
 1. @函数:    initUI
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    初始化UI
*******************************************************************************/
void OperationConfirmDlg::initUI()
{
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(422, 202);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(10, 0, 0, 0);

    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("titleBar");//Add by ut001000 renfeixiang 2020-08-13
    m_titleBar->setFixedHeight(50);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleBar->setLayout(titleLayout);

    m_logoIcon = new DLabel(this);
    m_logoIcon->setObjectName("logoIcon");//Add by ut001000 renfeixiang 2020-08-13
    m_logoIcon->setFixedSize(QSize(32, 32));
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_logoIcon->setPixmap(QIcon::fromTheme("deepin-terminal").pixmap(QSize(32, 32)));

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setObjectName("closeButton");//Add by ut001000 renfeixiang 2020-08-13
    m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setIconSize(QSize(50, 50));

    m_titleText = new DLabel(this);
    m_titleText->setObjectName("titleText");//Add by ut001000 renfeixiang 2020-08-13
    m_titleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleText->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titleText, DFontSizeManager::T6);

    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_titleText);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setObjectName("contentLayout");//Add by ut001000 renfeixiang 2020-08-13
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setObjectName("content");//Add by ut001000 renfeixiang 2020-08-13
    m_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_content);
    setLayout(mainLayout);
}

/*******************************************************************************
 1. @函数:    initContentLayout
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    初始化内容布局
*******************************************************************************/
void OperationConfirmDlg::initContentLayout()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setObjectName("mainLayout");//Add by ut001000 renfeixiang 2020-08-13
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(10, 0, 10, 10);

    QWidget *mainFrame = new QWidget(this);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_operateTypeName = new DLabel(this);
    m_operateTypeName->setObjectName("operateTypeName");//Add by ut001000 renfeixiang 2020-08-13
    //m_operateTypeName->setFixedHeight(20);
    m_operateTypeName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 字色
    DPalette titlepalette = m_operateTypeName->palette();
    titlepalette.setBrush(QPalette::WindowText, titlepalette.color(DPalette::BrightText));
    m_operateTypeName->setPalette(titlepalette);
    // 字号
    DFontSizeManager::instance()->bind(m_operateTypeName, DFontSizeManager::T6, QFont::Medium);

    m_tipInfo = new DLabel(this);
    m_tipInfo->setObjectName("tipInfo");//Add by ut001000 renfeixiang 2020-08-13
    //m_tipInfo->setFixedHeight(20);
    m_tipInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 字号
    DFontSizeManager::instance()->bind(m_tipInfo, DFontSizeManager::T6, QFont::Normal);
    // 字色
    DPalette palette = m_tipInfo->palette();
    QColor color = DGuiApplicationHelper::adjustColor(palette.color(QPalette::WindowText), 0, 0, 0, 0, 0, 0, -30);
    palette.setColor(QPalette::WindowText, color);
    m_tipInfo->setPalette(palette);

    m_actionLayout = new QHBoxLayout();
    m_actionLayout->setObjectName("actionLayout");//Add by ut001000 renfeixiang 2020-08-13
    m_actionLayout->setSpacing(0);
    m_actionLayout->setContentsMargins(0, 0, 0, 0);

    QFont btnFont;
    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setObjectName("cancelBtn");//Add by ut001000 renfeixiang 2020-08-13
    m_cancelBtn->setFixedHeight(38);
    m_cancelBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_cancelBtn->setFont(btnFont);

    m_confirmBtn = new DWarningButton(this);
    m_confirmBtn->setObjectName("confirmBtn");//Add by ut001000 renfeixiang 2020-08-13
    m_confirmBtn->setFixedHeight(38);
    m_confirmBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_confirmBtn->setFont(btnFont);

    //设置回车键默认响应的按钮
    m_confirmBtn->setDefault(true);

    DVerticalLine *verticalLine = new DVerticalLine(this);
    verticalLine->setFixedSize(1, 28);

    m_actionLayout->addWidget(m_cancelBtn);
    m_actionLayout->addSpacing(8);
    m_actionLayout->addWidget(verticalLine);
    m_actionLayout->addSpacing(8);
    m_actionLayout->addWidget(m_confirmBtn);

    mainFrame->setLayout(m_mainLayout);

    addContent(mainFrame);
}

/*******************************************************************************
 1. @函数:    initConnections
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    初始化连接
*******************************************************************************/
void OperationConfirmDlg::initConnections()
{
    connect(m_cancelBtn, &DPushButton::clicked, this, [ = ]() {
        qDebug() << "cancelBtnClicked";
        m_confirmResultCode = QDialog::Rejected;
        reject();
        close();
    });
    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
        qDebug() << "confirmBtnClicked";
        m_confirmResultCode = QDialog::Accepted;
        close();
    });

    connect(m_closeButton, &DIconButton::clicked, this, [ = ]() {
        qDebug() << "dialog close Btn Clicked";
        m_confirmResultCode = QDialog::Rejected;
        close();
    });
}

/*******************************************************************************
 1. @函数:    closeEvent
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    关闭事件
*******************************************************************************/
void OperationConfirmDlg::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    done(-1);
    Q_EMIT closed();
}

/*******************************************************************************
 1. @函数:    setTitle
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置标题名
*******************************************************************************/
void OperationConfirmDlg::setTitle(const QString &title)
{
    if (nullptr != m_titleText) {
        m_titleText->setText(title);
    }
}

/*******************************************************************************
 1. @函数:    getContentLayout
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    获取内容布局
*******************************************************************************/
QLayout *OperationConfirmDlg::getContentLayout()
{
    return m_contentLayout;
}

/*******************************************************************************
 1. @函数:    addContent
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    增加内容
*******************************************************************************/
void OperationConfirmDlg::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != getContentLayout());

    getContentLayout()->addWidget(content);
}

/*******************************************************************************
 1. @函数:    setIconPixmap
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置图标像素图
*******************************************************************************/
void OperationConfirmDlg::setIconPixmap(const QPixmap &iconPixmap)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setPixmap(iconPixmap);
    }
}

/*******************************************************************************
 1. @函数:    setOperatTypeName
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置操作类型名称
*******************************************************************************/
void OperationConfirmDlg::setOperatTypeName(const QString &strName)
{
    m_mainLayout->addWidget(m_operateTypeName, 0,  Qt::AlignVCenter | Qt::AlignHCenter);
    m_mainLayout->addSpacing(8);
    m_operateTypeName->setText(strName);
}

/*******************************************************************************
 1. @函数:    setTipInfo
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置提示信息
*******************************************************************************/
void OperationConfirmDlg::setTipInfo(const QString &strInfo)
{

    m_mainLayout->addWidget(m_tipInfo, 0, Qt::AlignVCenter | Qt::AlignHCenter);
    m_mainLayout->addStretch();
    m_tipInfo->setText(strInfo);
}

/*******************************************************************************
 1. @函数:    setOKCancelBtnText
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置确定取消按钮文本
*******************************************************************************/
void OperationConfirmDlg::setOKCancelBtnText(const QString &strConfirm, const QString &strCancel)
{
    m_mainLayout->addLayout(m_actionLayout);
    m_confirmBtn->setText(strConfirm);
    m_cancelBtn->setText(strCancel);
    Utils::setSpaceInWord(m_confirmBtn);
    Utils::setSpaceInWord(m_cancelBtn);
}

/*******************************************************************************
 1. @函数:    getConfirmResult
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    得到确认结果
*******************************************************************************/
QDialog::DialogCode OperationConfirmDlg::getConfirmResult()
{
    return m_confirmResultCode;
}
