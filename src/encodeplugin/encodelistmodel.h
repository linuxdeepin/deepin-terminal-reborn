/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:      zhangmeng <zhangmeng@uniontech.com>
 *
 * Maintainer:  zhangmeng <zhangmeng@uniontech.com>
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

#ifndef ENCODELISTMODEL_H
#define ENCODELISTMODEL_H

#include <QStandardItemModel>

/*******************************************************************************
 1. @类名:    EncodeListModel
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class EncodeListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit EncodeListModel(QObject *parent = nullptr);

    /**
     * @brief 获取行数
     * @author ut001121 zhangmeng
     * @param parent
     * @return
     */
    int rowCount(const QModelIndex &parent) const override;
    /**
     * @brief 根据行信息获取当前编码内容
     * @author ut001121 zhangmeng
     * @param index 行
     * @param role
     * @return
     */
    QVariant data(const QModelIndex &index, int role) const override;
    /**
     * @brief 返回编码数据列表
     * @author ut001121 zhangmeng
     * @return
     */
    QList<QByteArray> listData();

private:
    /**
     * @brief 初始化编码数据
     * @author ut001121 zhangmeng
     */
    void initEncodeData();

    QList<QByteArray> m_encodeData;
};

#endif  // THEMELISTMODEL_H
