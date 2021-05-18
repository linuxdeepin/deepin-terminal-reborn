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

#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include "define.h"
#include "utils.h"

#include <QList>
#include <QAction>

struct CustomCommandData {
    QString m_cmdName;
    QString m_cmdText;
    QString m_cmdShortcut;
};

class MainWindow;
class ListView;
/*******************************************************************************
 1. @类名:    ShortcutManager
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    全局快捷键管理

*******************************************************************************/
class ShortcutManager : public QObject
{
    Q_OBJECT
public:
    explicit ShortcutManager(QObject *parent = nullptr);
    static ShortcutManager *instance();
    ~ShortcutManager();
    /**
     * @brief 初始化快捷键
     * @author ut001121 zhangmeng
     */
    void initShortcuts();
    /**
     * @brief 快捷键初始化链接
     * @author ut001121 zhangmeng
     * @param mainWindow
     */
    void initConnect(MainWindow *mainWindow);

    /**
     * @brief 从配置创建自定义命令
     * @author ut001121 zhangmeng
     */
    void createCustomCommandsFromConfig();
    /**
     * @brief 从配置创建内置快捷方式
     * @author ut001121 zhangmeng
     * @return
     */
    QList<QAction *> createBuiltinShortcutsFromConfig();
    /**
     * @brief 获取自定义命令操作列表
     * @author ut001121 zhangmeng
     * @return
     */
    QList<QAction *> &getCustomCommandActionList();

    /**
     * @brief 增加自定义命令
     * @author ut001121 zhangmeng
     * @param action 增加操作
     * @return
     */
    QAction *addCustomCommand(QAction &action);
    /**
     * @brief 删除自定义命令
     * @author ut000125 sunchengxi
     * @param itemData 删除项数据
     */
    void delCustomCommand(CustomCommandData itemData);
    /**
     * @brief 保存自定义命令到配置
     * @author ut000125 sunchengxi
     * @param action 操作
     * @param saveIndex
     */
    void saveCustomCommandToConfig(QAction *action, int saveIndex);
    /**
     * @brief 从配置中删除自定义命令
     * @author ut000125 sunchengxi
     * @param itemData 删除项数据
     * @return
     */
    int delCustomCommandToConfig(CustomCommandData itemData);
    /**
     * @brief 检查操作是否存在
     * @author ut001121 zhangmeng
     * @param action 操作
     * @return
     */
    QAction *checkActionIsExist(QAction &action);
    /**
     * @brief 检查是否存在修改操作
     * @author ut000125 sunchengxi
     * @param action 修改操作
     * @return
     */
    QAction *checkActionIsExistForModify(QAction &action);
    /**
     * @brief f通过key值找到快捷键
     * @author ut000610 戴正文
     * @param strKey key值
     * @return
     */
    QAction *findActionByKey(const QString &strKey);
    /**
     * @brief 填充列表项
     * @author ut000610 戴正文
     * @param listview 列表
     * @param strFilter
     */
    void fillCommandListData(ListView *listview, const QString &strFilter = "");

    /**
     * @brief 判断快捷键是否合法可用，并进行界面处理
     * @author n014361 王培利
     * @param Name 快捷键名称
     * @param Key
     * @return
     */
    bool isValidShortcut(const QString &Name, const QString &Key);
    /**
     * @brief 检测快捷键是否合法可用，无界面;目前单键除了F1-F12, 其它单键均不可以设置,内置，自定义，设置分别检测冲突
     * @author n014361 王培利
     * @param Name 快捷键名称
     * @param Key 快捷键
     * @param Reason
     * @return
     */
    bool checkShortcutValid(const QString &Name, const QString &Key, QString &Reason);
    /**
     * @brief 快捷键是否已被自定义设置
     * @author n014361 王培利
     * @param Name 快捷键名称
     * @param Key 快捷键
     * @return
     */
    bool isShortcutConflictInCustom(const QString &Name, const QString &Key);

    // 快捷键显示映射 如：Return 显示成 Enter
    QMap<QString, QString> m_mapReplaceText;

signals:
    void addCustomCommandSignal(QAction *newAction);
    void removeCustomCommandSignal(QAction *newAction);

private:
    QList<QAction *> m_customCommandActionList;
    QStringList m_builtinShortcuts;
    static ShortcutManager *m_instance;
};

#endif  // SHORTCUTMANAGER_H
