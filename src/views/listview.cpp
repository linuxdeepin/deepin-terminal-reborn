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
#include "listview.h"
#include "service.h"
#include "serverconfigmanager.h"
#include "dbusmanager.h"
#include "customcommandoptdlg.h"
#include "serverconfigoptdlg.h"

// qt
#include <QDebug>
#include <QScroller>

ListView::ListView(ListType type, QWidget *parent)
    : QScrollArea(parent),
      m_type(type),
      m_mainWidget(new DWidget(this)),
      m_mainLayout(new QVBoxLayout(m_mainWidget))
{
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 Begin***************/
    Utils::set_Object_Name(this);
    m_mainWidget->setObjectName("ListViewMainWidget");
    m_mainLayout->setObjectName("ListViewMainLayout");
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 End***************/
    // 初始化界面
    initUI();

    /***add by ut001121 zhangmeng 20200924 修复BUG48618***/
    QScroller::grabGesture(this, QScroller::TouchGesture);
}

/*******************************************************************************
 1. @函数:    ~ListView
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    析构，用于释放列表中的数据
*******************************************************************************/
ListView::~ListView()
{
    clearData();
}

/*******************************************************************************
 1. @函数:    addItem
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    根据所给信息添加项
 1）参数 type ItemFuncType_Item 可执行的项 ItemFuncType_Group 分组
*******************************************************************************/
void ListView::addItem(ItemFuncType type, const QString &key, const QString &strDescription)
{
    // 初始化项
    ItemWidget *itemWidget = new ItemWidget(type, this);
    // 设置文字
    itemWidget->setText(key, strDescription);
    // 设置图标
    setItemIcon(type, itemWidget);
    // qDebug() << "add item to list type:" << type << "key:" << key;
    // 添加到布局
    // 远程管理的分组和项需要排序
    // 首先获得位置
    int index = getWidgetIndex(itemWidget);
    // 添加进去
    m_mainLayout->insertWidget(index, itemWidget);
    // 添加到列表
    m_itemList.append(itemWidget);
    // 列表数量变化
    emit listItemCountChange();

    // 分组项被点击
    connect(itemWidget, &ItemWidget::groupClicked, this, [ = ](const QString & strName, bool isFocus = false) {
        emit groupClicked(strName, isFocus);
        // 若焦点不在项上
        if (!isFocus) {
            // 清空index记录
            clearIndex();
            // 焦点状态为false
            m_focusState = false;
        }
    });
    // 列表项被点击
    connect(itemWidget, &ItemWidget::itemClicked, this, &ListView::itemClicked);
    // 修改列表
    // connect(itemWidget, &ItemWidget::itemModify, this, &ListView::itemModify);
    connect(itemWidget, &ItemWidget::itemModify, this, &ListView::onItemModify);
    // 焦点切出 Tab切出不返回
    connect(itemWidget, &ItemWidget::focusOut, this, [ = ](Qt::FocusReason type) {
        emit focusOut(type);
        lostFocus(m_currentIndex);
        m_focusState = false;
    });
    // 列表被点击，编辑按钮隐藏
    connect(itemWidget, &ItemWidget::itemClicked, this, [ = ]() {
        lostFocus(m_currentIndex);
    });

}

/*******************************************************************************
 1. @函数:    removeItem
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    根据类型和key值移除项
*******************************************************************************/
bool ListView::removeItem(ItemFuncType type, const QString &key)
{
    bool isFind = false;
    for (ItemWidget *item : m_itemList) {
        // 匹配选项
        if (item->isEqual(type, key)) {
            // 删除此项
            m_itemList.removeOne(item);
            m_mainLayout->removeWidget(item);
            delete item;
            // 找到设为true
            isFind = true;
            // 列表数量变化
            emit listItemCountChange();
        }
    }
    return isFind;
}

/*******************************************************************************
 1. @函数:    updateItem
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    针对列表中的一项进行更新
*******************************************************************************/
bool ListView::updateItem(ItemFuncType type, const QString &key, const QString &newKey, const QString &strDescription)
{
    bool isFind = false;
    // 遍历，找到需要修改的项，修改
    // 组的修改是，更新数量
    // 项的修改是修改内容
    for (ItemWidget *item : m_itemList) {
        // 匹配选项
        if (item->isEqual(type, key)) {
            //更新
            item->setText(newKey, strDescription);
            // 找到设为true
            isFind = true;
        }
    }
    // 返回更新结果
    return isFind;
}

/*******************************************************************************
 1. @函数:    count
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    获取列表中项的数量
*******************************************************************************/
int ListView::count()
{
    return m_itemList.count();
}

/*******************************************************************************
 1. @函数:    clearData
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    刷新列表需要清空数据
*******************************************************************************/
void ListView::clearData()
{
    for (ItemWidget *item : m_itemList) {
        // 从列表中移除
        m_itemList.removeOne(item);
        // 删除项！！
        delete item;
    }
}

/*******************************************************************************
 1. @函数:    indexFromString
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-21
 4. @说明:    根据名称获取现有列表的index
*******************************************************************************/
int ListView::indexFromString(const QString &key, ItemFuncType type)
{
    // 遍历布局
    for (int i = 0; i < m_mainLayout->count(); ++i) {
        // 获取每个窗口
        QLayoutItem *item = m_mainLayout->itemAt(i);
        ItemWidget *widget = qobject_cast<ItemWidget *>(item->widget());
        if (nullptr != widget) {
            // 比较是否相等
            if (widget->isEqual(type, key)) {
                // 相等 返回index
                return i;
            }
        }
    }
    // 没找到，返回-1
    return -1;
}

/*******************************************************************************
 1. @函数:    currentIndex
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-28
 4. @说明:    返回焦点选中的index
*******************************************************************************/
int ListView::currentIndex()
{
    return m_currentIndex;
}

/*******************************************************************************
 1. @函数:    getNextIndex
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-24
 4. @说明:    获取当前下一个的index 实质判断现在此index是否可用
*******************************************************************************/
int ListView::getNextIndex(int index)
{
    if (index < 0) {
        // 输入错误的index
        qDebug() << __FUNCTION__ << "input wrong index" << index;
        return -1;
    }
    int count = m_itemList.count();
    // index小于列表最大数量
    if (index < count) {
        return index;
    }
    // index大于最大数量,且列表数量不为0
    if (index >= count && m_itemList.count() != 0) {
        index = count - 1;
        return index;
    } else {
        // 列表没有项
        return -2;
    }
}

/*******************************************************************************
 1. @函数:    setCurrentIndex
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-23
 4. @说明:    设置当前焦点
*******************************************************************************/
void ListView::setCurrentIndex(int currentIndex)
{
    // 首先判断currentIndex的有效性
    if (!indexIsValid(currentIndex)) {
        // 无效，返回
        qDebug() << "index : " << currentIndex << " is wrong";
        emit focusOut(Qt::NoFocusReason);
        m_focusState = false;
        return;
    }
    qDebug() << __FUNCTION__ << currentIndex;
    // 之前的焦点丢失
    if (currentIndex != m_currentIndex) {
        lostFocus(m_currentIndex);
    }
    // 设置焦点
    QLayoutItem *item = m_mainLayout->itemAt(currentIndex);
    ItemWidget *widget = qobject_cast<ItemWidget *>(item->widget());
    if (item != nullptr) {
        // 设置焦点
        qDebug() << widget << "get focus" << "current index" << currentIndex;
        // Todo 让焦点不要进入主窗口
        widget->setFocus();
        m_focusState = true;
    }
    // 设置滚动条
    setScroll(currentIndex);
    m_currentIndex = currentIndex;
}

/*******************************************************************************
 1. @函数:    clearIndex
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-28
 4. @说明:    清空index
*******************************************************************************/
void ListView::clearIndex()
{
    m_currentIndex = -1;
    qDebug() << __FUNCTION__ << "clear index to  -1";
}

/*******************************************************************************
 1. @函数:    onItemModify
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-21
 4. @说明:    列表项被修改，弹出弹窗给用户修改
*******************************************************************************/
void ListView::onItemModify(const QString &key, bool isFocusOn)
{
    switch (m_type) {
    case ListType_Remote:
        onRemoteItemModify(key, isFocusOn);
        break;
    case ListType_Custom:
        onCustomItemModify(key, isFocusOn);
        break;
    }

}

/*******************************************************************************
 1. @函数:    onRemoteItemModify
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-03
 4. @说明:    处理远程管理修改项的弹窗操作
*******************************************************************************/
void ListView::onRemoteItemModify(const QString &key, bool isFocusOn)
{
    int curIndex = m_currentIndex;
    // 弹窗出，丢失焦点
    lostFocus(m_currentIndex);
    m_currentIndex = curIndex;
    // 弹窗显示
    ServerConfig *curItemServer = ServerConfigManager::instance()->getServerConfig(key);
    qDebug() << __FUNCTION__ << "modify remote " << curItemServer->m_serverName;
    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);
    // 根据点击事件还是键盘事件设置焦点状态
    if (isFocusOn) {
        // 键盘
        m_focusState = true;
        qDebug() << "keyboard clicked";
    } else {
        // 鼠标
        qDebug() << "mouse press";
        m_focusState = false;
        m_currentIndex = -1;
    }

    // 1.显示弹窗
    m_configDialog = new ServerConfigOptDlg(ServerConfigOptDlg::SCT_MODIFY, curItemServer, this);
    connect(m_configDialog, &ServerConfigOptDlg::finished, this, [ = ](int result) {
        // 弹窗隐藏或消失
        qDebug() << "focus state " << m_focusState;
        // 3. 对弹窗操作进行分析
        // 判断是否删除
        if (result == ServerConfigOptDlg::Accepted) {
            // 判断是否需要删除
            if (m_configDialog->isDelServer()) {
                // 弹出删除弹窗
                qDebug() << "delete " << m_configDialog->getCurServer()->m_serverName << m_configDialog;
                DDialog *deleteDialog = new DDialog(tr("Delete Server"), tr("Are you sure you want to delete %1?").arg(m_configDialog->getServerName()), m_configDialog);
                deleteDialog->setObjectName("RemoteDeleteDialog");
                deleteDialog->setAttribute(Qt::WA_DeleteOnClose);
                connect(deleteDialog, &DDialog::finished, this, [ = ](int result2) {
                    // 获取index
                    int index = indexFromString(m_configDialog->getCurServer()->m_serverName);
                    // 删除
                    if (result2 == DDialog::Accepted) {
                        // 关闭所有相关弹窗
                        ServerConfigManager::instance()->closeAllDialog(m_configDialog->getCurServer()->m_serverName);
                        ServerConfigManager::instance()->delServerConfig(m_configDialog->getCurServer());
                        emit ServerConfigManager::instance()->refreshList();
                        emit listItemCountChange();
                    } else {
                        // 关闭后及时将弹窗删除
                        ServerConfigManager::instance()->removeDialog(m_configDialog);
                    }
                    // Todo : 焦点返回下一个
                    index = getNextIndex(index);
                    if (m_focusState) {
                        // 根据返回值判断焦点位置
                        if (index >= 0) {
                            // 回到列表,仅回到大的列表，没有回到具体的哪个点
                            setFocus();
                            // 列表不为空
                            setCurrentIndex(index);
                        } else {
                            // 列表为空焦点自动往下
                            m_focusState = false;
                            emit focusOut(Qt::NoFocusReason);
                        }
                    }
                });
                deleteDialog->setWindowModality(Qt::WindowModal);
                deleteDialog->setIcon(QIcon::fromTheme("deepin-terminal"));
                deleteDialog->addButton(QObject::tr("Cancel"), false, DDialog::ButtonNormal);
                deleteDialog->addButton(QObject::tr("Delete"), true, DDialog::ButtonWarning);
                deleteDialog->show();
                // 释放窗口
                Service::instance()->setIsDialogShow(window(), false);
            } else {
                // 不删除，修改
                // 释放窗口
                Service::instance()->setIsDialogShow(window(), false);
                // 修改后会有信号刷新列表
                // 不需要删除，修改了转到这条修改的记录
                // 设置滚轮
                // 关闭后及时将弹窗删除
                // 记住修改前的位置 m_currentIndex
                qDebug() << "index before modify " << m_currentIndex;
                ServerConfigManager::instance()->removeDialog(m_configDialog);
                // 刷新列表
                emit ServerConfigManager::instance()->refreshList();
                // 获取index
                int index = indexFromString(m_configDialog->getCurServer()->m_serverName);
                // 若找不到
                if (index  < 0) {
                    // 旧位置的下一个
                    index = getNextIndex(m_currentIndex);
                }
                // 依旧没有找到啦
                if (index < 0) {
                    qDebug() << "no next item";
                    if (m_focusState) {
                        // 有焦点，焦点出
                        emit focusOut(Qt::NoFocusReason);
                    }
                }
                // 找的到
                else {
                    if (m_focusState) {
                        // 设置焦点
                        setFocus();
                        // 有焦点，设置焦点
                        setCurrentIndex(index);
                    } else {
                        // 没焦点,设置滚轮
                        setScroll(index);
                    }
                }
            }
        }
        // 不接受，reject 修改弹窗
        else {
            // 释放窗口
            Service::instance()->setIsDialogShow(window(), false);
            if (m_focusState) {
                int index = indexFromString(m_configDialog->getCurServer()->m_serverName);
                // 回到列表,仅回到大的列表，没有回到具体的哪个点
                setFocus();
                qDebug() << "current Index ListView" << m_currentIndex;
                setCurrentIndex(index);
            }
            // 取消后及时将弹窗删除
            ServerConfigManager::instance()->removeDialog(m_configDialog);
        }

    });
    // 2. 记录弹窗
    ServerConfigManager::instance()->setModifyDialog(curItemServer->m_serverName, m_configDialog);
    m_configDialog->show();
}

/*******************************************************************************
 1. @函数:    onCustomItemModify
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-03
 4. @说明:    修改自定义项处理弹窗的操作
*******************************************************************************/
void ListView::onCustomItemModify(const QString &key, bool isFocusOn)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }

    int curIndex = m_currentIndex;
    // 弹窗出，丢失焦点
    lostFocus(m_currentIndex);
    m_currentIndex = curIndex;

    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);

    qDebug() << "this->count()=" << this->count();
    // 根据点击事件还是键盘事件设置焦点状态
    if (isFocusOn) {
        // 键盘
        m_focusState = true;
    } else {
        // 鼠标
        m_focusState = false;
        m_currentIndex = -1;
    }

    // 获取数据
    QAction *itemAction = ShortcutManager::instance()->findActionByKey(key);
    CustomCommandData itemData;
    itemData.m_cmdName = itemAction->text();
    itemData.m_cmdText = itemAction->data().toString();
    itemData.m_cmdShortcut = itemAction->shortcut().toString();

    m_pdlg = new CustomCommandOptDlg(CustomCommandOptDlg::CCT_MODIFY, &itemData, this);
    connect(m_pdlg, &CustomCommandOptDlg::finished, this, [ &](int result) {

        int tempResult = result;

        // 弹窗隐藏或消失
        //Service::instance()->setIsDialogShow(window(), false);//暂时保留

        if (QDialog::Accepted == result) {
            //确认修改处理
            qDebug() <<  __FUNCTION__ << __LINE__ << ":mod Custom Command";
            QAction *newAction = m_pdlg->getCurCustomCmd();
            CustomCommandData itemData = *(m_pdlg->m_currItemData);
            CustomCommandData itemDel = itemData;

            itemData.m_cmdName = newAction->text();
            itemData.m_cmdText = newAction->data().toString();
            itemData.m_cmdShortcut = newAction->shortcut().toString();

            //newAction->setData(newAction->data());
            //newAction->setShortcut(newAction->shortcut());


            ShortcutManager::instance()->delCustomCommand(itemDel);
            ShortcutManager::instance()->addCustomCommand(*newAction);
            // 更新列表项
            //updateItem(ItemFuncType_Item, itemData.m_cmdName, itemData.m_cmdShortcut);
            updateItem(ItemFuncType_Item, itemDel.m_cmdName, itemData.m_cmdName);
            // 找到值
            m_pdlg->closeRefreshDataConnection();
            emit Service::instance()->refreshCommandPanel(itemDel.m_cmdName, itemData.m_cmdName);

            int index = indexFromString(itemData.m_cmdName);
            qDebug() << "-------------------------------------------index=" << index << ",itemData.m_cmdName=" << itemData.m_cmdName;
            if (m_focusState) {
                // 将焦点落回
                m_currentIndex = index;
                setCurrentIndex(index);

                // 滚轮滚动到最新的位置
                setScroll(index);
                setFocus();
            }

        } else if (QDialog::Rejected == result) {

            //Delete custom command 删除自定义命令处理
            if (m_pdlg->isDelCurCommand()) {
                qDebug() <<  __FUNCTION__ << __LINE__ << ":del Custom Command";
                DDialog *dlgDelete = new DDialog(this);
                dlgDelete->setObjectName("CustomDeleteDialog");
                dlgDelete->setAttribute(Qt::WA_DeleteOnClose);
                dlgDelete->setWindowModality(Qt::WindowModal);
                m_pdlg->m_dlgDelete = dlgDelete;

                dlgDelete->setIcon(QIcon::fromTheme("deepin-terminal"));
                dlgDelete->setTitle(tr("Are you sure you want to delete %1?").arg(m_pdlg->m_currItemData->m_cmdName));
                dlgDelete->addButton(QObject::tr("Cancel"), false, DDialog::ButtonNormal);
                dlgDelete->addButton(QObject::tr("Confirm"), true, DDialog::ButtonWarning);
                connect(dlgDelete, &DDialog::finished, this, [ & ](int result) {
                    //是否删除自定义命令再次确认操作
                    if (QDialog::Accepted == result) {
                        // 删除前获取位置
                        int index = indexFromString(m_pdlg->m_currItemData->m_cmdName);
                        ShortcutManager::instance()->delCustomCommand(*(m_pdlg->m_currItemData));
                        // 删除项
                        removeItem(ItemFuncType_Item, m_pdlg->m_currItemData->m_cmdName);
                        emit listItemCountChange();
                        // 删除后焦点位置
                        if (m_focusState) {
                            // 获取下一个的位置
                            index = getNextIndex(index);
                            if (index >= 0) {
                                // 找的到
                                setCurrentIndex(index);
                                setScroll(index);
                                setFocus();
                            } else {
                                // 找不到
                                qDebug() << "can't find index" << index;
                                if (m_itemList.count() == 0) {
                                    emit focusOut(Qt::NoFocusReason);
                                }
                            }
                        }
                        m_pdlg->closeRefreshDataConnection();
                        emit Service::instance()->refreshCommandPanel(m_pdlg->m_currItemData->m_cmdName, m_pdlg->m_currItemData->m_cmdName);//emit Service::instance()->refreshCommandPanel("", "");
                    }

                });
                dlgDelete->show();
            }
        }

        Service::instance()->setIsDialogShow(window(), false);

        if (m_focusState) {
            // 将焦点落回
            setCurrentIndex(m_currentIndex);

            // 滚轮滚动到最新的位置
            setScroll(m_currentIndex);
            setFocus();
        }

        qDebug() << "================================tempResult=" << tempResult;

    });
    m_pdlg->show();
}

/*******************************************************************************
 1. @函数:    keyPressEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    对Tab和上下键进行处理
*******************************************************************************/
void ListView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        setFocusFromeIndex(m_currentIndex, true);
        break;
    case Qt::Key_Down:
        setFocusFromeIndex(m_currentIndex, false);
        break;
    default:
        QScrollArea::keyPressEvent(event);
        break;
    }
}

/*******************************************************************************
 1. @函数:    focusInEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-21
 4. @说明:    焦点进
*******************************************************************************/
void ListView::focusInEvent(QFocusEvent *event)
{
    qDebug() << __FUNCTION__ << event->reason();
    if (m_currentIndex == -1) {
        m_currentIndex = 0;
    }
    if (event->reason() == Qt::TabFocusReason) {
        setCurrentIndex(0);
    } else {
        setCurrentIndex(m_currentIndex);
    }
    qDebug() << "ListView current index : " << m_currentIndex << event->reason();
    m_focusState = true;
    QScrollArea::focusInEvent(event);
}

/*******************************************************************************
 1. @函数:    setFocusState
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-24
 4. @说明:    设置焦点状态
*******************************************************************************/
void ListView::setFocusState(bool focusState)
{
    m_focusState = focusState;
}

/*******************************************************************************
 1. @函数:    getFocusState
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-24
 4. @说明:    获得焦点状态
*******************************************************************************/
bool ListView::getFocusState() const
{
    return m_focusState;
}

/*******************************************************************************
 1. @函数:    initUI
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    初始化列表，设置列表内布局，垂直布局，控件间隔10px
*******************************************************************************/
void ListView::initUI()
{
    // 间隔
    setContentsMargins(0, 0, 0, 0);
    //布局
    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->setSpacing(10);
    m_mainLayout->addStretch();
    // 焦点策略
    setFocusPolicy(Qt::TabFocus);

    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizeAdjustPolicy(QScrollArea::AdjustIgnored);
    setWidgetResizable(true);
    verticalScrollBar()->setValue(0);

    m_mainWidget->setLayout(m_mainLayout);
    setWidget(m_mainWidget);

}

/*******************************************************************************
 1. @函数:    setItemIcon
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    根据列表类型设置图标
 (远程组图标 dt_server_group 远程服务器图标 dt_server 自定义图标 dt_command)
*******************************************************************************/
void ListView::setItemIcon(ItemFuncType type, ItemWidget *item)
{
    switch (type) {
    // 项图标
    case ItemFuncType_Item:
        if (m_type == ListType_Custom) {
            // 自定义项
            item->setIcon("dt_command");
        } else if (m_type == ListType_Remote) {
            // 远程项
            item->setIcon("dt_server");
        }
        break;
    // 远程图标
    case ItemFuncType_Group:
        item->setIcon("dt_server_group");
        break;
    default:
        break;
    }
}

/*******************************************************************************
 1. @函数:    getWidgetIndex
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    根据类型进行数据的插入位置的选择，按type和key值，以及strDescription排序
*******************************************************************************/
int ListView::getWidgetIndex(ItemWidget *itemWidget)
{
    // 从0开始
    int index = 0;
    // 遍历列表
    for (ItemWidget *item : m_itemList) {
        // 判断是否小于，是就将index++
        if (*itemWidget < *item) {
            ++index;
        }
    }
    return index;
}

/*******************************************************************************
 1. @函数:    setFocusFromeIndex
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    根据index切换焦点
 1)参数1 currentIndex 当前项的值
 2)参数2 UpOrDown 向上或者向下 true 上  false 下
*******************************************************************************/
void ListView::setFocusFromeIndex(int currentIndex, bool UpOrDown)
{
    int count = m_itemList.count();

    int index = currentIndex;
    if (UpOrDown) {
        --index;
    } else {
        ++index;
    }

    // index >= 0 < 最大数量
    // 最上
    if (index < 0) {
        index = 0;
        m_scrollPostion = 0;
        // 提示音
        DBusManager::callSystemSound();
    }
    // 之前焦点所在位置丢失焦点
    if (index != currentIndex && index != count && index >= 0) {
        // 不是到头的情况下，前一个丢失焦点
        lostFocus(currentIndex);
    }
    // 最下
    if (index == count) {
        // 让index恢复正常
        --index;
        m_scrollPostion = verticalScrollBar()->value();
        // 提示音
        DBusManager::callSystemSound();
    } else {
        QLayoutItem *item = m_mainLayout->itemAt(index);
        ItemWidget *widget = qobject_cast<ItemWidget *>(item->widget());
        if (item != nullptr) {
            // 设置焦点
            widget->setFocus();
            int widget_y1 = widget->y();
            int widget_y2 = widget->y() + widget->height();
            qDebug() << "y1" << widget_y1;
            qDebug() << "y2" << widget_y2;
            qDebug() << "height" << height();
            // 设置滚动条 上下移动
            if (widget->y() + widget->height() < height()) {
                verticalScrollBar()->setValue(0);
            } else {
                if (UpOrDown) {
                    m_scrollPostion -= 70;
                } else if (currentIndex < count/* - 1*/) {
                    m_scrollPostion += 70;
                }
            }
        }
    }
    // 计算range
    int range = calculateRange(height());
    verticalScrollBar()->setRange(0, range);
    if (index == 0) {
        m_scrollPostion = 0;
    }
    verticalScrollBar()->setValue(m_scrollPostion);
    qDebug() << "up down scrollPostion : " << m_scrollPostion << verticalScrollBar()->value();

    // 需要让m_currentIndex于焦点所在位置同步
    m_currentIndex = index;
    qDebug() << "current index : " << m_currentIndex;
}

/*******************************************************************************
 1. @函数:    lostFocus
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    因为上下键或者Tab丢失焦点
*******************************************************************************/
void ListView::lostFocus(int preIndex)
{
    qDebug() << __FUNCTION__;
    // 没有前一个
    if (preIndex == -1) {
        return;
    }

    QLayoutItem *item = m_mainLayout->itemAt(preIndex);
    ItemWidget *widget = qobject_cast<ItemWidget *>(item->widget());
    if (widget != nullptr) {
        // 丢失焦点
        widget->lostFocus();
        qDebug() << widget << "lost focus";
    }
}

/*******************************************************************************
 1. @函数:    setScroll
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-21
 4. @说明:    将滚轮设置到当前项
*******************************************************************************/
void ListView::setScroll(int currentIndex)
{
    qDebug() << __FUNCTION__;
    // 计算range
    int range = calculateRange(height());
    verticalScrollBar()->setRange(0, range);
    QLayoutItem *item = m_mainLayout->itemAt(currentIndex);
    if (item != nullptr) {
        int postion = currentIndex  * 70;
        qDebug() << "postion : " << postion;
        if (postion + 60 < height()) {
            verticalScrollBar()->setValue(0);
            m_scrollPostion = 0;
        } else {
            m_scrollPostion = 0;
            // 差的距离
            int distance = postion - height();
            if (distance >= 0) {
                // 倍率
                int magn = distance / 10 + 1;
                m_scrollPostion = 0 + magn * 70;
            }
            verticalScrollBar()->setValue(m_scrollPostion);
        }
        qDebug() << "scrollPostion " << m_scrollPostion;
    }
}

/*******************************************************************************
 1. @函数:    indexIsValid
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-23
 4. @说明:    判断index的有效性
*******************************************************************************/
bool ListView::indexIsValid(int index)
{
    qDebug() << index << m_itemList.count();
    return (index >= 0 && index < m_itemList.count());
}

/*******************************************************************************
 1. @函数:    calculateRange
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-27
 4. @说明:    计算滚动条的range
*******************************************************************************/
int ListView::calculateRange(int height)
{
    int count = m_itemList.count();
    int range  = count * 70 - 10 - height;
    return range;
}

/*******************************************************************************
 1. @函数:    mousePressEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-27
 4. @说明:    鼠标按下
*******************************************************************************/
void ListView::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    //return QScrollArea::mousePressEvent(event);
}

/*******************************************************************************
 1. @函数:    mouseMoveEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-27
 4. @说明:    鼠标移动
*******************************************************************************/
void ListView::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    //return QScrollArea::mouseMoveEvent(event);
}


