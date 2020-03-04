#include "customcommanddelegate.h"
#include "customcommanditemmodel.h"
#include "utils.h"

#include <DGuiApplicationHelper>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyleHelper>

#include <QPainter>
#include <QPixmap>
#include <DLog>

CustomCommandDelegate::CustomCommandDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(parent)
{
}

void CustomCommandDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QVariant varDisplay = index.data(Qt::DisplayRole);

        CustomCommandItemData itemData = varDisplay.value<CustomCommandItemData>();

        QStyleOptionViewItem viewOption(option);  //用来在视图中画一个item
        DPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? DPalette::Normal : DPalette::Disabled;
        if (cg == DPalette::Normal && !(option.state & QStyle::State_Active)) {
            cg = DPalette::Inactive;
        }

        QRect bgRect;
        bgRect.setX(option.rect.x() + 10);
        bgRect.setY(option.rect.y() + 10);
        bgRect.setWidth(option.rect.width() - 20);
        bgRect.setHeight(option.rect.height() - 10);

        QPainterPath path;
        int cornerSize = 16;
        int arcRadius = 8;

        path.moveTo(bgRect.left() + arcRadius, bgRect.top());
        path.arcTo(bgRect.left(), bgRect.top(), cornerSize, cornerSize, 90.0, 90.0);
        path.lineTo(bgRect.left(), bgRect.bottom() - arcRadius);
        path.arcTo(bgRect.left(), bgRect.bottom() - cornerSize, cornerSize, cornerSize, 180.0, 90.0);
        path.lineTo(bgRect.right() - arcRadius, bgRect.bottom());
        path.arcTo(bgRect.right() - cornerSize, bgRect.bottom() - cornerSize, cornerSize, cornerSize, 270.0, 90.0);
        path.lineTo(bgRect.right(), bgRect.top() + arcRadius);
        path.arcTo(bgRect.right() - cornerSize, bgRect.top(), cornerSize, cornerSize, 0.0, 90.0);

        if (option.state & QStyle::State_MouseOver) {
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);
            fillColor.setAlphaF(0.3);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        }
        else {
            DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        }

        int cmdIconSize = 44;
        int editIconSize = 20;

        QString themeType = "light";
        DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
        if (DGuiApplicationHelper::DarkType == appHelper->themeType()) {
            themeType = "dark";
        }
        QString strCmdIconSrc = QString(":/images/buildin/%1/command.svg").arg(themeType);
        QPixmap cmdIconPixmap = Utils::renderSVG(strCmdIconSrc, QSize(cmdIconSize, cmdIconSize));

        QRect cmdIconRect = QRect(bgRect.left(), bgRect.top() + (bgRect.height() - cmdIconSize) / 2,
                                  cmdIconSize, cmdIconSize);
        painter->drawPixmap(cmdIconRect, cmdIconPixmap);

        if (option.state & QStyle::State_MouseOver) {
            QString strEditIconSrc = QString(":/images/buildin/%1/edit.svg").arg(themeType);
            QPixmap editIconPixmap = Utils::renderSVG(strEditIconSrc, QSize(editIconSize, editIconSize));
            QRect editIconRect = QRect(bgRect.right() - editIconSize - 6, bgRect.top() + (bgRect.height() - editIconSize) / 2,
                                       editIconSize, editIconSize);
            painter->drawPixmap(editIconRect, editIconPixmap);
        }

        QString strCmdName = itemData.m_cmdName;
        QString strCmdShortcut = itemData.m_cmdShortcut;

        int labelHeight = 35;
        QRect cmdNameRect = QRect(bgRect.left() + cmdIconSize + 5, bgRect.top() + 5, bgRect.width() - cmdIconSize - editIconSize, labelHeight);
        painter->drawText(cmdNameRect, Qt::AlignLeft | Qt::AlignVCenter, strCmdName);

        QRect cmdShortcutRect = QRect(bgRect.left() + cmdIconSize + 5, bgRect.bottom() - labelHeight - 5, bgRect.width() - cmdIconSize - editIconSize, labelHeight);
        painter->drawText(cmdShortcutRect, Qt::AlignLeft | Qt::AlignVCenter, strCmdShortcut);

        painter->restore();
    } else {
        DStyledItemDelegate::paint(painter, option, index);
    }
}

QSize CustomCommandDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    Q_UNUSED(index)

    return QSize(option.rect.width(), 70);
}