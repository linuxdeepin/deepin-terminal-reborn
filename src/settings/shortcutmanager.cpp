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
#include "shortcutmanager.h"
#include "termwidgetpage.h"
#include "mainwindow.h"
#include "settings.h"
#include "listview.h"
#include "settingio.h"

#include <QStandardPaths>
#include <QTextCodec>
#include <QSettings>
#include <QDebug>
#include <QDir>

/* del by ut001121 zhangmeng 20201221 修复BUG58747
#define INI_FILE_CODEC QTextCodec::codecForName("UTF-8")*/

// this class only provided for convenience, do not do anything in the construct function,
// let the caller decided when to create the shortcuts.
ShortcutManager *ShortcutManager::m_instance = nullptr;

ShortcutManager::ShortcutManager(QObject *parent) : QObject(parent)
{
    Utils::set_Object_Name(this);
}

ShortcutManager *ShortcutManager::instance()
{
    if (nullptr == m_instance) {
        m_instance = new ShortcutManager();
    }
    return m_instance;
}

void ShortcutManager::initShortcuts()
{
    m_builtinShortcuts << "F1";
    m_builtinShortcuts << "Ctrl+C";
    m_builtinShortcuts << "Ctrl+D";

    /******** Modify by ut000439 wangpeili 2020-07-27: bug 39494   ****************/
    m_builtinShortcuts << QString(MainWindow::QKEYSEQUENCE_PASTE_BUILTIN);

    /******** Modify by ut001000 renfeixiang 2020-08-28:修改 bug 44477***************/
    m_builtinShortcuts << QString(MainWindow::QKEYSEQUENCE_COPY_BUILTIN);
    /********************* Modify by n014361 wangpeili End ************************/

    /******** Modify by ut000439 wangpeili 2020-07-20:  SP3 右键快捷键被内置   ****************/
    m_builtinShortcuts << "Alt+M";
    /********************* Modify by n014361 wangpeili End ************************/
    // 切换标签页，防止和ctrl+shift+1等冲突（无法识别）

    createCustomCommandsFromConfig();
    m_mapReplaceText.insert("Ctrl+Shift+!", "Ctrl+Shift+1");
    m_mapReplaceText.insert("Ctrl+Shift+@", "Ctrl+Shift+2");
    m_mapReplaceText.insert("Ctrl+Shift+#", "Ctrl+Shift+3");
    m_mapReplaceText.insert("Ctrl+Shift+$", "Ctrl+Shift+4");
    m_mapReplaceText.insert("Ctrl+Shift+%", "Ctrl+Shift+5");
    m_mapReplaceText.insert("Ctrl+Shift+^", "Ctrl+Shift+6");
    m_mapReplaceText.insert("Ctrl+Shift+&", "Ctrl+Shift+7");
    m_mapReplaceText.insert("Ctrl+Shift+*", "Ctrl+Shift+8");
    m_mapReplaceText.insert("Ctrl+Shift+(", "Ctrl+Shift+9");
    m_mapReplaceText.insert("Return", "Enter");
}
ShortcutManager::~ShortcutManager()
{
    qDeleteAll(m_customCommandActionList.begin(), m_customCommandActionList.end());
    m_customCommandActionList.clear();
}


void ShortcutManager::initConnect(MainWindow *mainWindow)
{
    for (auto &commandAction : m_customCommandActionList) {
        connect(commandAction, &QAction::triggered, mainWindow, &MainWindow::onCommandActionTriggered);
    }
    mainWindow->addActions(m_customCommandActionList);
}

void ShortcutManager::createCustomCommandsFromConfig()
{
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        return ;
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("command-config.conf"));
    qDebug() << "load Custom Commands Config: " << customCommandConfigFilePath;
    if (!QFile::exists(customCommandConfigFilePath)) {
        return ;
    }

    /***modify begin by ut001121 zhangmeng 20201221 修复BUG58747 远程管理和自定义命令名称中带/重启后会出现命令丢失***/
    /* del
    QSettings::Format fmt = QSettings::registerFormat("conf", SettingIO::readIniFunc, SettingIO::writeIniFunc);
    QSettings commandsSettings(customCommandConfigFilePath, QSettings::CustomFormat1);

    commandsSettings.setIniCodec(INI_FILE_CODEC);*/
    /* add */
    USettings commandsSettings(customCommandConfigFilePath);
    /***modify end by ut001121***/
    QStringList commandGroups = commandsSettings.childGroups();
    for (const QString &commandName : commandGroups) {
        commandsSettings.beginGroup(commandName);
        if (!commandsSettings.contains("Command")) {
            continue;
        }
        QAction *action = new QAction(commandName, this);
        action->setData(commandsSettings.value("Command").toString());  // make sure it is a QString
        if (commandsSettings.contains("Shortcut")) {
            QVariant shortcutVariant = commandsSettings.value("Shortcut");
            if (shortcutVariant.type() == QVariant::KeySequence) {
                action->setShortcut(shortcutVariant.convert(QMetaType::QKeySequence));
            } else if (shortcutVariant.type() == QVariant::String) {
                //兼容deepin-terminal 旧的command-config.conf配置文件
                QString shortcutStr = shortcutVariant.toString().remove(QChar(' '));
                action->setShortcut(QKeySequence(shortcutStr));
            }
        }
        commandsSettings.endGroup();
        m_customCommandActionList.append(action);
    }

    if (SettingIO::rewrite) {
        //删除自定义命令文件
        QFile fileTemp(customCommandConfigFilePath);
        fileTemp.remove();
        qDebug() << "remove file:" << customCommandConfigFilePath;

        //将内存数据写入配置文件
        for (QAction *action : m_customCommandActionList) {
            saveCustomCommandToConfig(action, -1);
        }
    }
}

QList<QAction *> ShortcutManager::createBuiltinShortcutsFromConfig()
{
    QList<QAction *> actionList;
    return actionList;
}

QList<QAction *> &ShortcutManager::getCustomCommandActionList()
{
    qDebug() << __FUNCTION__ << m_customCommandActionList;
    return m_customCommandActionList;
}

QAction *ShortcutManager::addCustomCommand(QAction &action)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    QAction *addAction = new QAction(action.text(), this);
    addAction->setData(action.data());
    addAction->setShortcut(action.shortcut());
    m_customCommandActionList.append(addAction);
    saveCustomCommandToConfig(addAction, -1);
    emit addCustomCommandSignal(addAction);

    return addAction;
}

QAction *ShortcutManager::checkActionIsExist(QAction &action)
{
    QString strNewActionName = action.text();
    for (int i = 0; i < m_customCommandActionList.size(); i++) {
        QAction *currAction = m_customCommandActionList[i];
        if (strNewActionName == currAction->text()) {
            return currAction;
        }
    }
    return nullptr;
}
/************************ Mod by m000743 sunchengxi 2020-04-21:自定义命令修改的异常问题 Begin************************/
QAction *ShortcutManager::checkActionIsExistForModify(QAction &action)
{
    QString strNewActionName = action.text();
    for (int i = 0; i < m_customCommandActionList.size(); i++) {
        QAction *currAction = m_customCommandActionList[i];
        if (strNewActionName == currAction->text() && action.data() == currAction->data() && action.shortcut() == currAction->shortcut()) {
            return currAction;
        }
    }
    return nullptr;
}

QAction *ShortcutManager::findActionByKey(const QString &strKey)
{
    for (QAction *action : m_customCommandActionList) {
        if (action->text() == strKey) {
            qDebug() << "find action " << action;
            return action;
        }
    }

    qDebug() << "not find action name " << strKey;
    return nullptr;
}

void ShortcutManager::fillCommandListData(ListView *listview, const QString &strFilter)
{
    listview->clearData();
    QList<QAction *> &customCommandActionList = ShortcutManager::instance()->getCustomCommandActionList();
    qDebug() << __FUNCTION__ << strFilter  << " : " << customCommandActionList.size();
    //根据条件进行刷新，strFilter 为空时 全部刷新，根据名称或者命令模糊匹配到strFilter的条件进行刷新
    if (strFilter.isEmpty()) {
        for (int i = 0; i < customCommandActionList.size(); i++) {
            QAction *curAction = customCommandActionList[i];
            QString strCmdName = curAction->text();
            QString strCmdShortcut = curAction->shortcut().toString();

            //listview->addItem(ItemFuncType_Item, strCmdName, strCmdShortcut);
            // 刷新列表,列表项中的值显示大写 down2up dzw 20201215
            listview->addItem(ItemFuncType_Item, strCmdName, Utils::converDownToUp(strCmdShortcut));
        }
    } else {
        for (int i = 0; i < customCommandActionList.size(); i++) {
            QAction *curAction = customCommandActionList[i];
            QString strCmdName = curAction->text();
            QString strCmdText = curAction->data().toString();
            QKeySequence keySeq = curAction->shortcut();
            QString strKeySeq = keySeq.toString();
            if (strCmdName.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)
                    || strCmdText.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)
                    || strKeySeq.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)) {
                //listview->addItem(ItemFuncType_Item, strCmdName, strKeySeq);
                // 刷新列表,列表项中的值显示大写 down2up dzw 20201215
                listview->addItem(ItemFuncType_Item, strCmdName, Utils::converDownToUp(strKeySeq));
            }
        }
    }
    qDebug() << "fill list data";
}

/************************ Mod by m000743 sunchengxi 2020-04-21:自定义命令修改的异常问题 End  ************************/
bool ShortcutManager::isShortcutConflictInCustom(const QString &Name, const QString &Key)
{
    for (auto &currAction : m_customCommandActionList) {
        if (Key == currAction->shortcut().toString()) {
            if (Name != currAction->text()) {
                qDebug() << Name << Key << "is conflict with custom shortcut!";
                return true;
            }
        }
    }
    return false;
}

bool ShortcutManager::isValidShortcut(const QString &Name, const QString &Key)
{
    QString reason;
    if (!checkShortcutValid(Name, Key, reason)) {
        if (Key != "Esc") {
            Utils::showShortcutConflictMsgbox(reason);
        }
        return false;
    }
    return true;
}

bool ShortcutManager::checkShortcutValid(const QString &Name, const QString &Key, QString &Reason)
{
    /******** Modify by ut000610 daizhengwen 2020-07-10:给'<'做兼容 html中'<'的转译为 &lt; Begin***************/
    QString key = Key;
    if (key.contains("<")) {
        // 用<>断开 &lt; 与+号  +&lt; 会显示错误
        key.replace("<", "<>&lt;");
    }
    /********************* Modify by ut000610 daizhengwen End ************************/
    QString style = QString("<span style=\"color: rgba(255, 87, 54, 1);\">%1</span>").arg(key);
    qDebug() << style;

    // 单键
    if (Key.count("+") == 0) {
        //F1-F12是允许的，这个正则不够精确，但是没关系。
        QRegExp regexp("^F[0-9]{1,2}$");
        if (!Key.contains(regexp)) {
            qDebug() << Key << "is invalid!";
            Reason = tr("The shortcut %1 is invalid, ")
                     .arg(style);
            return  false;
        }
    }
    // 小键盘单键都不允许
    QRegExp regexpNum("^Num+.*");
    if (Key.contains(regexpNum)) {
        qDebug() << Key << "is invalid!";
        Reason = tr("The shortcut %1 is invalid, ")
                 .arg(style);
        return  false;
    }
    // 内置快捷键都不允许
    if (m_builtinShortcuts.contains(Key)) {
        qDebug() << Key << "is conflict with builtin shortcut!";
        Reason = tr("The shortcut %1 was already in use, ")
                 .arg(style);
        return  false;
    }

    // 与设置里的快捷键冲突检测
    if (Settings::instance()->isShortcutConflict(Name, Key)) {
        Reason = tr("The shortcut %1 was already in use, ")
                 .arg(style);
        return  false;
    }
    // 与自定义快捷键冲突检测
    if (isShortcutConflictInCustom(Name, Key)) {
        Reason = tr("The shortcut %1 was already in use, ")
                 .arg(style);
        return  false;
    }
    return true;
}

void ShortcutManager::delCustomCommand(CustomCommandData itemData)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    delCustomCommandToConfig(itemData);

    QString actionCmdName = itemData.m_cmdName;

    for (int i = 0; i < m_customCommandActionList.size(); i++) {
        QAction *currAction = m_customCommandActionList.at(i);
        QString currCmdName = currAction->text();
        if (actionCmdName == currCmdName) {
            emit removeCustomCommandSignal(m_customCommandActionList.at(i));
            m_customCommandActionList.at(i)->deleteLater();
            m_customCommandActionList.removeAt(i);
            break;
        }
    }

}

void ShortcutManager::saveCustomCommandToConfig(QAction *action, int saveIndex)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("command-config.conf"));
    /***modify begin by ut001121 zhangmeng 20201221 修复BUG58747 远程管理和自定义命令名称中带/重启后会出现命令丢失***/
    /* del
    //QSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    QSettings commandsSettings(customCommandConfigFilePath, QSettings::CustomFormat1);
    commandsSettings.setIniCodec(INI_FILE_CODEC);**/
    /* add */
    USettings commandsSettings(customCommandConfigFilePath);
    /***modify end by ut001121***/
    commandsSettings.beginGroup(action->text());
    commandsSettings.setValue("Command", action->data());
    commandsSettings.setValue("Shortcut", action->shortcut().toString());
    commandsSettings.endGroup();

    qDebug() << "saveIndex:" << saveIndex;
    if (saveIndex >= 0) {
        QAction *saveAction = new QAction;
        saveAction->setText(action->text());
        saveAction->setData(action->data());
        saveAction->setShortcut(action->shortcut());
        qDebug() << "old" << m_customCommandActionList[saveIndex]->shortcut();
        m_customCommandActionList[saveIndex] = saveAction;
        qDebug() << "new" << m_customCommandActionList[saveIndex]->shortcut();
    }
}

int ShortcutManager::delCustomCommandToConfig(CustomCommandData itemData)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("command-config.conf"));
    /***modify begin by ut001121 zhangmeng 20201221 修复BUG58747 远程管理和自定义命令名称中带/重启后会出现命令丢失***/
    /* del
    //QSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    QSettings commandsSettings(customCommandConfigFilePath, QSettings::CustomFormat1);
    commandsSettings.setIniCodec(INI_FILE_CODEC);*/
    /* add */
    USettings commandsSettings(customCommandConfigFilePath);
    /***modify end by ut001121***/
    commandsSettings.remove(itemData.m_cmdName);

    int removeIndex = -1;
    for (int i = 0; i < m_customCommandActionList.size(); i++) {
        QAction *currAction = m_customCommandActionList[i];
        if (itemData.m_cmdName == currAction->text()) {
            removeIndex = i;
            break;
        }
    }
    return removeIndex;
}

