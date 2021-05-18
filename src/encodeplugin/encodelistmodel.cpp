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

#include "encodelistmodel.h"
#include "utils.h"

#include <DStandardItem>
#include <DLog>

#include <QTextCodec>
#include <QRegExp>

DWIDGET_USE_NAMESPACE

EncodeListModel::EncodeListModel(QObject *parent) : QStandardItemModel(parent)
{
    Utils::set_Object_Name(this);
    initEncodeData();
}

int EncodeListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    //
    return m_encodeData.count();
}

QVariant EncodeListModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);
    const int row = index.row();
    return m_encodeData[row];
}

QList<QByteArray> EncodeListModel::listData()
{
    return  m_encodeData;
}

void EncodeListModel::initEncodeData()
{
    // m_encodeData = QTextCodec::availableCodecs().toSet().toList();
    QList<QByteArray> showEncodeList;
    showEncodeList << "UTF-8" << "GB18030" << "GB2312" << "GBK" /*简体中文*/
                   << "BIG5" << "BIG5-HKSCS" //<< "EUC-TW"      /*繁体中文*/
                   << "EUC-JP"  << "SHIFT_JIS"  //<< "ISO-2022-JP"/*日语*/
                   << "EUC-KR" //<< "ISO-2022-KR" //<< "UHC"      /*韩语*/
                   << "IBM864" << "ISO-8859-6" << "ARABIC" << "WINDOWS-1256"   /*阿拉伯语*/
                   //<< "ARMSCII-8"    /*美国语*/
                   << "ISO-8859-13" << "ISO-8859-4" << "WINDOWS-1257"  /*波罗的海各国语*/
                   << "ISO-8859-14"    /*凯尔特语*/
                   << "IBM-852" << "ISO-8859-2" << "x-mac-CE" << "WINDOWS-1250" /*中欧*/
                   //<< "x-mac-CROATIAN"  /*克罗地亚*/
                   << "IBM855" << "ISO-8859-5"  << "KOI8-R" << "MAC-CYRILLIC" << "WINDOWS-1251" //<< "ISO-IR-111" /*西里尔语*/
                   << "CP866" /*西里尔语或俄语*/
                   << "KOI8-U" << "x-MacUkraine" /*西里尔语或乌克兰语*/
                   //<< "GEORGIAN-PS"
                   << "ISO-8859-7" << "x-mac-GREEK" << "WINDOWS-1253"  /*希腊语*/
                   //<< "x-mac-GUJARATI"
                   //<< "x-mac-GURMUKHI"
                   << "IBM862" << "ISO-8859-8-I" << "WINDOWS-1255"//<< "x-mac-HEBREW"  /*希伯来语*/
                   << "ISO-8859-8" /*希伯来语*/
                   //<< "x-mac-DEVANAGARI"
                   //<< "x-mac-ICELANDIC" /*冰岛语*/
                   << "ISO-8859-10"     /*北欧语*/
                   //<< "x-mac-FARSI"     /*波斯语*/
                   //<< "x-mac-ROMANIAN" //<< "ISO-8859-16" /*罗马尼亚语*/
                   << "ISO-8859-3"      /*西欧语*/
                   << "TIS-620"         /*泰语*/
                   << "IBM857" << "ISO-8859-9" << "x-mac-TURKISH" << "WINDOWS-1254" /*土耳其语*/
                   << "WINDOWS-1258" //<< "TCVN" << "VISCII"  /*越南语*/
                   << "IBM850" << "ISO-8859-1" << "ISO-8859-15" << "x-ROMAN8" << "WINDOWS-1252"; /*西方国家*/

    // 自定义的名称，系统里不一定大小写完全一样，再同步一下。
    QList<QByteArray> all = QTextCodec::availableCodecs();
    for (const QByteArray &name : showEncodeList) {
        QString strname1 = name;
        bool bFind = false;
        QByteArray encodename;
        for (const QByteArray &name2 : all) {
            QString strname2 = name2;
            if (strname1.compare(strname2, Qt::CaseInsensitive) == 0) {
                bFind = true;
                encodename = name2;
                break;
            }
        }
        if (!bFind) {
            qDebug() << "encode name :" << name << "not find!";
        } else {
            m_encodeData << encodename;
        }
    }

    qDebug() << "QTextCodec::availableCodecs" << m_encodeData.count();
}
