/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  daizhengwen<daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen<daizhengwen@uniontech.com>
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
#include "remotemanagementpanel.h"
#include "service.h"
#include "utils.h"

#include <DLog>

RemoteManagementPanel::RemoteManagementPanel(QWidget *parent) : CommonPanel(parent)
{
    Utils::set_Object_Name(this);
    initUI();
}

/*******************************************************************************
 1. @函数:    refreshPanel
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    刷新远程管理面板
*******************************************************************************/
void RemoteManagementPanel::refreshPanel()
{
    // 清空搜索框
    clearSearchInfo();
    // 初始化列表
    ServerConfigManager::instance()->refreshServerList(ServerConfigManager::PanelType_Manage, m_listWidget);
    // 刷新搜索框状态
    refreshSearchState();
}

/*******************************************************************************
 1. @函数:    setFocusInPane
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-23
 4. @说明:    将焦点设置进入panel，有搜索框焦点进搜索框，没搜索框，焦点进入列表，没列表，焦点进添加按钮
*******************************************************************************/
void RemoteManagementPanel::setFocusInPanel()
{
    qDebug() << "RemoteManagementPanel focus in Panel.";
    if (m_searchEdit->isVisible()) {
        // 搜索框显示
        // 设置焦点
        m_searchEdit->lineEdit()->setFocus();
    } else if (m_listWidget->isVisible()) {
        // 列表显示
        /******** Modify by ut000610 daizhengwen 2020-07-27:bug#39775 Begin***************/
        // 将焦点设置在列表里的第一项
        m_listWidget->setCurrentIndex(0);
        /********************* Modify by ut000610 daizhengwen End ************************/
    } else {
        // 将焦点显示在添加按钮上
        m_pushButton->setFocus();
    }
}

/*******************************************************************************
 1. @函数:    setFocusBack
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-23
 4. @说明:    从分组中返回
*******************************************************************************/
void RemoteManagementPanel::setFocusBack(const QString &strGroup)
{
    qDebug() << "RemoteManagementPanel return from RemoteManageGroup";
    // 返回前判断之前是否要有焦点
    if (m_listWidget->getFocusState()) {
        // 要有焦点
        // 找到分组的新位置
        int index = m_listWidget->indexFromString(strGroup, ItemFuncType_Group);
        if (index < 0) {
            // 小于0代表没找到 获取下一个
            index = m_listWidget->getNextIndex(m_listWidget->currentIndex());
        }

        if (index >= 0) {
            // 找得到, 设置焦点
            m_listWidget->setCurrentIndex(index);
        } else {
            // 没找到焦点设置到添加按钮
            m_pushButton->setFocus();
        }
    }
    // 不要焦点
    else {
        Utils::getMainWindow(this)->focusCurrentPage();
    }
}

/*******************************************************************************
 1. @函数:    clearListFocus
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-28
 4. @说明:    清空列表的选中状态
*******************************************************************************/
void RemoteManagementPanel::clearListFocus()
{
    m_pushButton->clearFocus();
    m_listWidget->clearFocus();
    m_searchEdit->clearFocus();
    m_listWidget->clearIndex();
}

/*******************************************************************************
 1. @函数:    getListIndex
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-05
 4. @说明:    获取列表中当前焦点的位置
*******************************************************************************/
int RemoteManagementPanel::getListIndex()
{
    qDebug() << __FUNCTION__ << "current index : " << m_listWidget->currentIndex();
    return m_listWidget->currentIndex();
}

/*******************************************************************************
 1. @函数:    refreshSearchState
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    刷新远程管理搜索状态
*******************************************************************************/
void RemoteManagementPanel::refreshSearchState()
{
    if (m_listWidget->count() >= 2) {
        /************************ Add by m000743 sunchengxi 2020-04-22:搜索显示异常 Begin************************/
        m_searchEdit->clearEdit();
        /************************ Add by m000743 sunchengxi 2020-04-22:搜索显示异常  End ************************/
        m_searchEdit->show();
    } else {
        m_searchEdit->hide();
    }
}

/*******************************************************************************
 1. @函数:    onItemClicked
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-21
 4. @说明:    远程项被点击，连接远程管理
*******************************************************************************/
void RemoteManagementPanel::onItemClicked(const QString &key)
{
    // 获取远程信息
    ServerConfig *remote = ServerConfigManager::instance()->getServerConfig(key);
    if (nullptr != remote) {
        emit doConnectServer(remote);
    } else {
        qDebug() << "can't connect to remote" << key;
    }
}

/*******************************************************************************
 1. @函数:    showCurSearchResult
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    显示远程管理当前搜索结果
*******************************************************************************/
void RemoteManagementPanel::showCurSearchResult()
{
    QString strTxt = m_searchEdit->text();
    if (strTxt.isEmpty()) {
        return;
    }
    emit showSearchPanel(strTxt);
}

/*******************************************************************************
 1. @函数:    showAddServerConfigDlg
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    显示远程管理添加服务器配置界面
*******************************************************************************/
void RemoteManagementPanel::showAddServerConfigDlg()
{
    qDebug() << "RemoteManagementPanel show add server config dialog.";
    // 判断控件是否有焦点
    bool focusState = m_pushButton->hasFocus();
    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);

    ServerConfigOptDlg *dlg = new ServerConfigOptDlg(ServerConfigOptDlg::SCT_ADD, nullptr, this);
    connect(dlg, &ServerConfigOptDlg::finished, this, [ = ](int result) {
        // 弹窗隐藏或消失
        Service::instance()->setIsDialogShow(window(), false);
        // 先判断是否要焦点
        if (focusState) {
            // 让焦点在平面上
            setFocus();
            // 添加完，将焦点设置在添加按钮上
            m_pushButton->setFocus();
        }
        if (result == QDialog::Accepted) {
            int index = m_listWidget->indexFromString(dlg->getServerName());
            m_listWidget->setScroll(index);
        }
    });
    dlg->show();
}

/*******************************************************************************
 1. @函数:    initUI
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    初始化远程管理UI界面
*******************************************************************************/
void RemoteManagementPanel::initUI()
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    m_searchEdit = new DSearchEdit(this);
    m_searchEdit->setObjectName("RemoteSearchEdit");
    m_listWidget = new ListView(ListType_Remote, this);
    m_listWidget->setObjectName("RemoteManageListWidget");
    m_pushButton = new DPushButton(this);
    m_pushButton->setObjectName("RemoteAddPushButton");

    m_searchEdit->setFixedHeight(COMMONHEIGHT);
    m_searchEdit->setClearButtonEnabled(true);

    m_pushButton->setFixedHeight(COMMONHEIGHT);
    m_pushButton->setText(tr("Add Server"));

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->addSpacing(SPACEWIDTH);
    hlayout->addWidget(m_searchEdit);
    hlayout->addSpacing(SPACEWIDTH);
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addSpacing(SPACEWIDTH);
    btnLayout->addWidget(m_pushButton);
    btnLayout->addSpacing(SPACEWIDTH);
    btnLayout->setSpacing(0);
    btnLayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->addSpacing(SPACEHEIGHT);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_listWidget);
    vlayout->addLayout(btnLayout);
    vlayout->addSpacing(SPACEHEIGHT);
    vlayout->setMargin(0);
    vlayout->setSpacing(SPACEHEIGHT);
    setLayout(vlayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &RemoteManagementPanel::showCurSearchResult);
    connect(m_pushButton, &DPushButton::clicked, this, &RemoteManagementPanel::showAddServerConfigDlg);
    connect(m_listWidget, &ListView::itemClicked, this, &RemoteManagementPanel::onItemClicked);
    connect(m_listWidget, &ListView::groupClicked, this, &RemoteManagementPanel::showGroupPanel);
    connect(m_listWidget, &ListView::listItemCountChange, this, &RemoteManagementPanel::refreshSearchState);
    connect(ServerConfigManager::instance(), &ServerConfigManager::refreshList, this, [ = ]() {
        if (m_isShow) {
            refreshPanel();
        }
    });
    connect(m_listWidget, &ListView::focusOut, this, [ = ](Qt::FocusReason type) {
        if (type == Qt::TabFocusReason || type == Qt::NoFocusReason) {
            // 下一个 或 列表为空， 焦点定位到添加按钮上
            m_pushButton->setFocus();
            m_listWidget->clearIndex();
            qDebug() << "set focus on add pushButton";
        } else if (type == Qt::BacktabFocusReason) {
            // 判断是否可见，可见设置焦点
            if (m_searchEdit->isVisible()) {
                m_searchEdit->lineEdit()->setFocus();
                m_listWidget->clearIndex();
                qDebug() << "set focus on add search edit";
            }
        }

    });
}
