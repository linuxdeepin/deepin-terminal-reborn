/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangpeili<wangpeili@uniontech.com>
 *
 * Maintainer:wangpeili<wangpeili@uniontech.com>
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

#ifndef TERMPROPERTIES_H
#define TERMPROPERTIES_H

#include <QVariant>

enum TermProperty {
    SingleFlag,        // mainwindow使用, 默认为false
    QuakeMode,         // mainwindow使用, 右键菜单要用到．
    WorkingDir,        // 每个terminal单独使用
    ColorScheme,       // 未使用
    ShellProgram,      // 仅供第一个terminal使用
    Execute,           // 仅供第一个terminal使用，任意长，任意位置，QStringList
    StartWindowState,  // mainwindow使用
    KeepOpen,          // 仅供第一个terminal使用
    Script,            // 仅供第一个terminal使用
    DragDropTerminal   // 窗口标签拖拽时使用
};

/*******************************************************************************
 1. @类名:    TermProperties
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-31
 4. @说明:    终端属性集
             支持任意格式属性
*******************************************************************************/
class TermProperties
{
    Q_GADGET
public:
    explicit TermProperties() = default;
    explicit TermProperties(QString workingDir);
    explicit TermProperties(QString workingDir, QString colorScheme);
    explicit TermProperties(QMap<TermProperty, QVariant> list);

    /**
     * @brief 判断终端属性中是否包含参数属性
     * @author ut000439 wangpeili
     * @param propertyType 参数属性
     * @return
     */
    bool contains(TermProperty propertyType) const;

    /**
     * @brief 设置终端属性图
     * @author ut000439 wangpeili
     * @param list
     */
    void setTermPropertyMap(QMap<TermProperty, QVariant> list);
    /**
     * @brief 设置工作目录
     * @author ut000439 wangpeili
     * @param workingDir 工作目录
     */
    void setWorkingDir(QString workingDir);

    /**
     * @brief 重载[]操作符
     * @author ut000439 wangpeili
     * @param key
     * @return
     */
    QVariant &operator[](const TermProperty &key);
    /**
     * @brief 重载[]操作符
     * @author ut000439 wangpeili
     * @param key
     * @return
     */
    const QVariant operator[](const TermProperty &key) const;

private:
    QMap<TermProperty, QVariant> m_properties;
};

#endif  // TERMPROPERTIES_H
