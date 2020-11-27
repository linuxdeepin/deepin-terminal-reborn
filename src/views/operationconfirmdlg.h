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
#ifndef OPERATIONCONFIRMDLG_H
#define OPERATIONCONFIRMDLG_H

#include <DSuggestButton>
#include <DLabel>
#include <DButtonBox>
#include <DDialog>
#include <DWindowCloseButton>
#include <DPushButton>
#include <DWarningButton>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    OperationConfirmDlg
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class OperationConfirmDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit OperationConfirmDlg(QWidget *parent = nullptr);

    void addContent(QWidget *content);
    void setIconPixmap(const QPixmap &iconPixmap);
    void setTitle(const QString &title);

    void setOperatTypeName(const QString &strName);
    void setTipInfo(const QString &strInfo);
    void setOKCancelBtnText(const QString &strConfirm, const QString &strCancel);
    QDialog::DialogCode getConfirmResult();

protected:
    QLayout *getContentLayout();
    void closeEvent(QCloseEvent *event) override;

private:
    void initUI();
    void initContentLayout();
    void initConnections();

    DLabel  *m_logoIcon = nullptr;
    QWidget *m_titleBar = nullptr;
    DLabel  *m_titleText = nullptr;

    QWidget *m_content = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_actionLayout = nullptr;

    DLabel *m_operateTypeName = nullptr;
    DLabel *m_tipInfo = nullptr;
    DPushButton *m_cancelBtn = nullptr;
    DWarningButton *m_confirmBtn = nullptr;
    DWindowCloseButton *m_closeButton = nullptr;
    QDialog::DialogCode m_confirmResultCode;

signals:
    void closed();
};

#endif  // OPERATIONCONFIRMDLG_H
