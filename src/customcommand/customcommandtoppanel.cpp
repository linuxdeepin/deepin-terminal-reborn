/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     sunchengxi <sunchengxi@uniontech.com>
 *
 * Maintainer: sunchengxi <sunchengxi@uniontech.com>
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
#include "customcommandtoppanel.h"
#include "service.h"

#include <DPushButton>
#include <DLog>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

const int iAnimationDuration = 300;

CustomCommandTopPanel::CustomCommandTopPanel(QWidget *parent)
    : RightPanel(parent),
      m_customCommandPanel(new CustomCommandPanel(this)),
      m_customCommandSearchPanel(new CustomCommandSearchRstPanel(this))
{
    /******** Add by ut001000 renfeixiang 2020-08-14:增加 Begin***************/
    Utils::set_Object_Name(this);
    m_customCommandPanel->setObjectName("CustomCommandPanel");
    m_customCommandSearchPanel->setObjectName("CustomSearchPanel");
    /******** Add by ut001000 renfeixiang 2020-08-14:增加 End***************/
    setAttribute(Qt::WA_TranslucentBackground);
    connect(m_customCommandPanel,
            &CustomCommandPanel::showSearchResult,
            this,
            &CustomCommandTopPanel::showCustomCommandSearchPanel);
    connect(m_customCommandPanel,
            &CustomCommandPanel::handleCustomCurCommand,
            this,
            &CustomCommandTopPanel::handleCustomCurCommand);
    connect(m_customCommandSearchPanel,
            &CustomCommandSearchRstPanel::showCustomCommandPanel,
            this,
            &CustomCommandTopPanel::showCustomCommandPanel);
    connect(m_customCommandSearchPanel,
            &CustomCommandSearchRstPanel::handleCustomCurCommand,
            this,
            &CustomCommandTopPanel::handleCustomCurCommand);
    m_customCommandPanel->hide();
    m_customCommandSearchPanel->hide();
    connect(Service::instance(), &Service::refreshCommandPanel, this, &CustomCommandTopPanel::slotsRefreshCommandPanel);
}

void CustomCommandTopPanel::showCustomCommandPanel()
{
    qDebug() << "showCustomCommandPanel" << endl;
    m_customCommandPanel->resize(size());
    m_customCommandPanel->refreshCmdPanel();
    m_customCommandPanel->show();
    m_customCommandPanel->m_isShow = true;
    m_customCommandSearchPanel->m_isShow = false;

    //从自定义搜索面板返回到自定义列表面板时，如果搜索面板存在焦点，则切换后光标焦点依然在自定义列表面板上
    MainWindow *main = Utils::getMainWindow(this);
    if (main != nullptr) {
        if (main->isFocusOnList()) {
            m_customCommandPanel->setFocusInPanel();
            qDebug() << "custom command panel has focus";
        }
    }

    QPropertyAnimation *animationCommandSearchPanel = new QPropertyAnimation(m_customCommandSearchPanel, "geometry");
    animationCommandSearchPanel->setDuration(iAnimationDuration);
    animationCommandSearchPanel->setEasingCurve(QEasingCurve::OutQuad);

    QRect rect = geometry();
    animationCommandSearchPanel->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animationCommandSearchPanel->setEndValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animationCommandSearchPanel, &QPropertyAnimation::finished, m_customCommandSearchPanel, &QWidget::hide);
    connect(animationCommandSearchPanel, &QPropertyAnimation::finished, animationCommandSearchPanel, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animationCommandPanel = new QPropertyAnimation(m_customCommandPanel, "geometry");
    animationCommandPanel->setDuration(iAnimationDuration);
    animationCommandPanel->setEasingCurve(QEasingCurve::OutQuad);
    animationCommandPanel->setStartValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    animationCommandPanel->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animationCommandPanel, &QPropertyAnimation::finished, animationCommandPanel, &QPropertyAnimation::deleteLater);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animationCommandSearchPanel);
    group->addAnimation(animationCommandPanel);
    // 已验证：这个设定，会释放group以及所有组内动画。
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void CustomCommandTopPanel::showCustomCommandSearchPanel(const QString &strFilter)
{
    qDebug() << "showCustomCommandSearchPanel" << endl;
    m_customCommandSearchPanel->refreshData(strFilter);
    m_customCommandSearchPanel->show();
    m_customCommandSearchPanel->m_backButton->setFocus();//m_customCommandSearchPanel->setFocus();

    QPropertyAnimation *animationCommandSearchPanel = new QPropertyAnimation(m_customCommandSearchPanel, "geometry");
    animationCommandSearchPanel->setDuration(iAnimationDuration);
    animationCommandSearchPanel->setEasingCurve(QEasingCurve::OutQuad);

    QRect rect = geometry();
    animationCommandSearchPanel->setStartValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    animationCommandSearchPanel->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animationCommandSearchPanel, &QPropertyAnimation::finished, animationCommandSearchPanel, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animationCommandPanel = new QPropertyAnimation(m_customCommandPanel, "geometry");
    animationCommandPanel->setDuration(iAnimationDuration);
    animationCommandSearchPanel->setEasingCurve(QEasingCurve::OutQuad);
    animationCommandPanel->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animationCommandPanel->setEndValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animationCommandPanel, &QPropertyAnimation::finished, m_customCommandPanel, &QWidget::hide);
    connect(animationCommandPanel, &QPropertyAnimation::finished, animationCommandPanel, &QPropertyAnimation::deleteLater);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animationCommandPanel);
    group->addAnimation(animationCommandSearchPanel);
    // 已验证：这个设定，会释放group以及所有组内动画。
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void CustomCommandTopPanel::show(bool bSetFocus)
{
    this->showAnim();
    m_customCommandPanel->resize(size());
    m_customCommandPanel->move(0, 0);
    m_customCommandPanel->show();
    m_customCommandPanel->refreshCmdPanel();
    m_customCommandSearchPanel->resize(size());
    m_customCommandSearchPanel->hide();
    m_bSetFocus = bSetFocus;
    if (m_bSetFocus) {
        qDebug() << "Focus is in Custom panel";
        // 将焦点设置在平面上
        m_customCommandPanel->setFocusInPanel();
    } else {
        qDebug() << "Focus is in Current page";
        // 将焦点落回终端
        Utils::getMainWindow(this)->focusCurrentPage();
    }

}

void CustomCommandTopPanel::slotsRefreshCommandPanel()
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    m_customCommandPanel->resize(size());
    //m_customCommandPanel->show();
    m_customCommandPanel->refreshCmdPanel();
    m_customCommandSearchPanel->refreshData();

}


