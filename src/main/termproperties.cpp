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
#include "termproperties.h"

TermProperties::TermProperties(QString workingDir)
{
    m_properties.insert(WorkingDir, workingDir);
}

TermProperties::TermProperties(QString workingDir, QString colorScheme)
{
    m_properties.insert(WorkingDir, workingDir);
    m_properties.insert(ColorScheme, colorScheme);
}

TermProperties::TermProperties(QMap<TermProperty, QVariant> list) : m_properties(list)
{
}

void TermProperties::setWorkingDir(QString workingDir)
{
    m_properties.insert(WorkingDir, workingDir);
}

void TermProperties::setTermPropertyMap(QMap<TermProperty, QVariant> list)
{
    m_properties = list;
}

bool TermProperties::contains(TermProperty propertyType) const
{
    return m_properties.contains(propertyType);
}

QVariant &TermProperties::operator[](const TermProperty &key)
{
    return m_properties[key];
}

const QVariant TermProperties::operator[](const TermProperty &key) const
{
    return m_properties[key];
}
