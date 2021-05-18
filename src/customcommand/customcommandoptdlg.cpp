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
#include "customcommandoptdlg.h"
#include "termcommandlinkbutton.h"
#include "utils.h"
#include "service.h"

#include <DButtonBox>
#include <DPushButton>
#include <DGroupBox>
#include <DLabel>
#include <DKeySequenceEdit>
#include <DCommandLinkButton>
#include <DDialogCloseButton>
#include <DApplicationHelper>
#include <DGuiApplicationHelper>
#include <DWidgetUtil>
#include <DLog>
#include <DFontSizeManager>
#include <DVerticalLine>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>

CustomCommandOptDlg::CustomCommandOptDlg(CustomCmdOptType type, CustomCommandData *currItemData, QWidget *parent)
    : DAbstractDialog(parent)
    , m_type(type)
    , m_nameLineEdit(new DLineEdit)
    , m_commandLineEdit(new DLineEdit)
    , m_shortCutLineEdit(new DKeySequenceEdit)
    , m_bDelOpt(false)
{
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 Begin***************/
    Utils::set_Object_Name(this);
    m_nameLineEdit->setObjectName("CustomNameLineEdit");
    m_commandLineEdit->setObjectName("CustomCommandLineEdit");
    m_shortCutLineEdit->setObjectName("CustomShortCutLineEdit");
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 End***************/
    setWindowModality(Qt::WindowModal);
    if (currItemData) {
        m_currItemData = new CustomCommandData;
        *m_currItemData = *currItemData;
    }

    initUITitle();
    initTitleConnections();
    initUI();

    connect(Service::instance(), &Service::refreshCommandPanel, this, &CustomCommandOptDlg::slotRefreshData);
}

CustomCommandOptDlg::~CustomCommandOptDlg()
{
    if (m_currItemData) {
        delete m_currItemData;
        m_currItemData = nullptr;
    }
    if (m_newAction) {
        delete  m_newAction;
    }
}

void CustomCommandOptDlg::initUI()
{
    QWidget *contentFrame = new QWidget;

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setSpacing(m_iSpaceSizeTen);
    contentLayout->setContentsMargins(m_iLayoutSizeZero, m_iLayoutSizeZero, m_iLayoutSizeZero, m_iLayoutSizeZero);

    QWidget *nameFrame = new QWidget;
    nameFrame->setFixedWidth(m_iFixedWidth);
    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->setSpacing(m_iSpaceSizeZero);
    nameLayout->setContentsMargins(m_iLayoutLeftSize, m_iLayoutSizeZero, m_iLayoutRightSize, m_iLayoutSizeZero);
    nameFrame->setLayout(nameLayout);

    QWidget *cmdFrame = new QWidget;
    QHBoxLayout *cmdLayout = new QHBoxLayout;
    cmdLayout->setSpacing(m_iSpaceSizeZero);
    cmdLayout->setContentsMargins(m_iLayoutLeftSize, m_iLayoutSizeZero, m_iLayoutRightSize, m_iLayoutSizeZero);
    cmdFrame->setLayout(cmdLayout);

    QWidget *scFrame = new QWidget;
    QHBoxLayout *scLayout = new QHBoxLayout;
    scLayout->setSpacing(m_iSpaceSizeZero);
    scLayout->setContentsMargins(m_iLayoutLeftSize, m_iLayoutSizeZero, m_iLayoutRightSize, m_iLayoutSizeZero);
    scFrame->setLayout(scLayout);

    DLabel *nameLabel = new DLabel(tr("Name:"));

    DLabel *cmdLabel = new DLabel(tr("Command:"));

    DLabel *shortCutLabel = new DLabel(tr("Shortcuts:"));

    m_nameLineEdit->setFixedWidth(m_iLineEditWidth);
    m_commandLineEdit->setFixedWidth(m_iLineEditWidth);
    m_shortCutLineEdit->setFixedWidth(m_iLineEditWidth);
    m_shortCutLineEdit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    QTimer::singleShot(m_iSingleShotTime, this, [&]() {
        m_nameLineEdit->lineEdit()->selectAll();
    });

    m_nameLineEdit->lineEdit()->setPlaceholderText(tr("Required"));
    m_commandLineEdit->lineEdit()->setPlaceholderText(tr("Required"));
    m_shortCutLineEdit->ShortcutDirection(Qt::AlignLeft);

    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_nameLineEdit);

    cmdLayout->addWidget(cmdLabel);
    cmdLayout->addWidget(m_commandLineEdit);

    scLayout->addWidget(shortCutLabel);
    scLayout->addWidget(m_shortCutLineEdit);

    contentLayout->addWidget(nameFrame);
    contentLayout->addWidget(cmdFrame);
    contentLayout->addWidget(scFrame);

    contentFrame->setLayout(contentLayout);

    connect(m_nameLineEdit, &DLineEdit::editingFinished, this, &CustomCommandOptDlg::slotNameLineEditingFinished);

    connect(m_commandLineEdit, &DLineEdit::editingFinished, this, &CustomCommandOptDlg::slotCommandLineEditingFinished);

    addContent(contentFrame);
    //判断是添加操作窗口还是修改操作窗口
    if (CCT_ADD == m_type) {
        setFixedSize(m_iFixedWidth, m_iFixedHeightAddSize);
        setTitle(tr("Add Command"));
        initCommandFromClipBoardText();

        getMainLayout()->addSpacing(m_iSpaceSizeEighteen);
    } else {
        setFixedSize(m_iFixedWidth, m_iFixedHeightEditSize);
        setTitle(tr("Edit Command"));

        QWidget *deleteCmdWidget = new QWidget;
        deleteCmdWidget->setFixedHeight(m_iFixedHeight);

        QHBoxLayout *deleteCmdLayout = new QHBoxLayout();
        deleteCmdLayout->setSpacing(m_iSpaceSizeZero);
        deleteCmdLayout->setContentsMargins(m_iLayoutSizeZero, m_iLayoutSizeZero, m_iLayoutSizeZero, m_iLayoutSizeZero);
        TermCommandLinkButton *deleteCmdBtn = new TermCommandLinkButton();
        deleteCmdBtn->setFocusPolicy(Qt::TabFocus);
        deleteCmdBtn->setText(tr("Delete Command"));

        deleteCmdLayout->addStretch();
        deleteCmdLayout->addWidget(deleteCmdBtn);
        deleteCmdLayout->addStretch();
        deleteCmdWidget->setLayout(deleteCmdLayout);

        getMainLayout()->addWidget(deleteCmdWidget);

        connect(deleteCmdBtn, &DCommandLinkButton::clicked, this, &CustomCommandOptDlg::slotDelCurCustomCommand);

        QString strName = m_currItemData->m_cmdName;
        QString strCommad = m_currItemData->m_cmdText;
        QKeySequence keyseq = QKeySequence(m_currItemData->m_cmdShortcut);
        m_lastCmdShortcut = m_currItemData->m_cmdShortcut;
        m_nameLineEdit->setText(strName);
        m_commandLineEdit->setText(strCommad);
        m_shortCutLineEdit->setKeySequence(keyseq);

#ifdef UI_DEBUG
        deleteCmdWidget->setStyleSheet("background:green");
#endif
    }

    addCancelConfirmButtons();
    setCancelBtnText(tr("Cancel"));
    if (CCT_ADD == m_type) {
        setConfirmBtnText(tr("Add"));
    } else {
        setConfirmBtnText(tr("Save"));
    }
    m_lastCmdShortcut = m_shortCutLineEdit->keySequence().toString();
    connect(this, &CustomCommandOptDlg::confirmBtnClicked, this, &CustomCommandOptDlg::slotAddSaveButtonClicked);
    connect(m_shortCutLineEdit, &KeySequenceEdit::editingFinished, this, &CustomCommandOptDlg::slotShortCutLineEditingFinished);
}

inline void CustomCommandOptDlg::slotNameLineEditingFinished()
{
    if (m_nameLineEdit->text().isEmpty()) {
        m_nameLineEdit->lineEdit()->setPlaceholderText(tr("Required"));
    }
}

inline void CustomCommandOptDlg::slotCommandLineEditingFinished()
{
    if (m_commandLineEdit->text().isEmpty()) {
        m_commandLineEdit->lineEdit()->setPlaceholderText(tr("Required"));
    }
}

inline void CustomCommandOptDlg::slotShortCutLineEditingFinished(const QKeySequence &sequence)
{
    //删除
    if ("Backspace" == sequence.toString()) {
        qDebug() << "The KeySequenceE is Backspace";
        m_shortCutLineEdit->clear();
        m_lastCmdShortcut = "";
        return;
    }
    // 取消
    if ("Esc" == sequence.toString()) {
        qDebug() << "The KeySequenceE is Esc";
        m_shortCutLineEdit->clear();
        m_shortCutLineEdit->setKeySequence(QKeySequence(m_lastCmdShortcut));
        /***add by ut001121 zhangmeng 20200521 在快捷键编辑框中按下ESC键时退出窗口 修复BUG27554***/
        close();
        return;
    }
    QString checkName;
    //QString seq = m_shortCutLineEdit->text();
    if (m_type != CCT_ADD) {
        checkName = m_nameLineEdit->text();
    }
    QString reason;
    // 判断快捷键是否冲突
    if (!ShortcutManager::instance()->checkShortcutValid(checkName, sequence.toString(), reason)) {
        qDebug() << "checkShortcutValid";
        // 冲突
        if (sequence.toString() != "Esc") {
            qDebug() << "sequence != Esc";
            showShortcutConflictMsgbox(reason);
        }
        m_shortCutLineEdit->clear();
        m_shortCutLineEdit->setKeySequence(QKeySequence(m_lastCmdShortcut));
        return;
    }
    m_lastCmdShortcut = sequence.toString();

#ifdef UI_DEBUG
    contentFrame->setStyleSheet("background:cyan");
    nameLabel->setStyleSheet("background:#0000ff");
    cmdLabel->setStyleSheet("background:#00ff00");
    shortCutLabel->setStyleSheet("background:yellow");

    m_nameLineEdit->setStyleSheet("background:#ff00ff");
    m_commandLineEdit->setStyleSheet("background:#ff00ff");
    m_shortCutLineEdit->setStyleSheet("background:#ff00ff");
#endif
}

void CustomCommandOptDlg::initUITitle()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 10);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("CustomTitleBar");//Add by ut001000 renfeixiang 2020-08-13
    m_titleBar->setFixedHeight(50);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleBar->setLayout(titleLayout);

    m_logoIcon = new DLabel(this);
    m_logoIcon->setObjectName("CustomLogoIcon");//Add by ut001000 renfeixiang 2020-08-13
    m_logoIcon->setFixedSize(QSize(50, 50));
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setObjectName("CustomCloseButton");//Add by ut001000 renfeixiang 2020-08-13
    m_closeButton->setFocusPolicy(Qt::TabFocus);//m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setIconSize(QSize(50, 50));

    m_titleText = new DLabel(this);
    m_titleText->setObjectName("CustomTitleTextLabel");//Add by ut001000 renfeixiang 2020-08-13
    m_titleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleText->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titleText, DFontSizeManager::T5, QFont::DemiBold);
    // 字色
    DPalette palette = m_titleText->palette();
    QColor color;
    if (DApplicationHelper::DarkType == DApplicationHelper::instance()->themeType()) {
        color = QColor::fromRgb(192, 198, 212, 255);
    } else {
        color = QColor::fromRgb(0, 26, 46, 255);
    }
    palette.setBrush(QPalette::WindowText, color);
    m_titleText->setPalette(palette);

    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_titleText, 0, Qt::AlignHCenter);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignTop);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setObjectName("CustomContentLayout");//Add by ut001000 renfeixiang 2020-08-13
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setObjectName("CustomContentWidget");//Add by ut001000 renfeixiang 2020-08-13
    m_content->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar, 0, Qt::AlignTop);
    mainLayout->addWidget(m_content);
    setLayout(mainLayout);

    m_mainLayout = mainLayout;
}

void CustomCommandOptDlg::initTitleConnections()
{
    connect(m_closeButton, &DWindowCloseButton::clicked, this, &CustomCommandOptDlg::slotCloseButtonClicked);
    // 字体颜色随主题变化变化
    connect(DApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &CustomCommandOptDlg::slotThemeTypeChanged);
}

inline void CustomCommandOptDlg::slotCloseButtonClicked()
{
    this->close();
}

inline void CustomCommandOptDlg::slotThemeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    DPalette palette = m_titleText->palette();
    QColor color;
    if (DApplicationHelper::DarkType == themeType) {
        color = QColor::fromRgb(192, 198, 212, 255);
    } else {
        color = QColor::fromRgb(0, 26, 46, 255);
    }
    palette.setBrush(QPalette::WindowText, color);
    m_titleText->setPalette(palette);
}

void CustomCommandOptDlg::initCommandFromClipBoardText()
{
    if (m_commandLineEdit) {
        MainWindow *main = Utils::getMainWindow(this);//;getMainWindow();
        if (main != nullptr) {
            QString clipText = main->selectedText();
            m_commandLineEdit->setText(clipText.trimmed());
        }
    }
}

QAction *CustomCommandOptDlg::getCurCustomCmd()
{
    return m_newAction;
}

bool CustomCommandOptDlg::isDelCurCommand()
{
    return m_bDelOpt;
}

void CustomCommandOptDlg::slotAddSaveButtonClicked()
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    QString strName = m_nameLineEdit->text();
    QString strCommand = m_commandLineEdit->text();
    QKeySequence keytmp = m_shortCutLineEdit->keySequence();

    strName = strName.trimmed();//空格的名称是无效的，剔除名称前后的空格
    if (strName.isEmpty()) {
        qDebug() << "Name is empty";
        m_nameLineEdit->showAlertMessage(tr("Please enter a name"), m_nameLineEdit->parentWidget());
        return;
    }

    /***add begin by ut001121 zhangmeng 20200615 限制名称字符长度 修复BUG31286***/
    if (strName.length() > MAX_NAME_LEN) {
        qDebug() << "The name should be no more than 32 characters";
        m_nameLineEdit->showAlertMessage(QObject::tr("The name should be no more than 32 characters"), m_nameLineEdit->parentWidget());
        return;
    }
    /***add end by ut001121***/
    QString strCommandtemp = strCommand.trimmed();//空格的命令是无效的
    if (strCommandtemp.isEmpty()) {
        qDebug() << "Command is empty";
        m_commandLineEdit->showAlertMessage(tr("Please enter a command"), m_commandLineEdit->parentWidget());
        return;
    }

    m_newAction = new QAction(ShortcutManager::instance());
    m_newAction->setObjectName("CustomQAction");//Add by ut001000 renfeixiang 2020-08-13
    m_newAction->setText(strName);
    m_newAction->setData(strCommand);
    m_newAction->setShortcut(m_shortCutLineEdit->keySequence());

    /************************ Mod by m000743 sunchengxi 2020-04-21:自定义命令修改的异常问题 Begin************************/
    QAction *existAction = nullptr;
    int icount = 0;
    if (CCT_MODIFY == m_type) {
        qDebug() << "It is the modify type of custom command operation";
        if (m_bRefreshCheck) {
            qDebug() << "CustomCommand was refreshed";
            QAction *refreshExitAction = nullptr;
            refreshExitAction = ShortcutManager::instance()->checkActionIsExist(*m_newAction);
            if (refreshExitAction) {
                qDebug() << "CustomCommand already exists";
                accept();
                return;
            }
        }

        if (strName == m_currItemData->m_cmdName && strCommand == m_currItemData->m_cmdText && keytmp == QKeySequence(m_currItemData->m_cmdShortcut)) {
            qDebug() << "CustomCommand don't need to save again";
            accept();
            return;
        }

        existAction = ShortcutManager::instance()->checkActionIsExistForModify(*m_newAction);

        if (strName != m_currItemData->m_cmdName) {
            qDebug() << "CustomCommand is changed";
            QList<QAction *> &customCommandActionList = ShortcutManager::instance()->getCustomCommandActionList();
            for (int i = 0; i < customCommandActionList.size(); i++) {
                QAction *curAction = customCommandActionList[i];
                QString strCmdName = curAction->text();
                if (strCmdName == strName) {
                    icount++;
                }
            }
        }
    } else {
        existAction = ShortcutManager::instance()->checkActionIsExist(*m_newAction);
    }

    if (nullptr != existAction || icount) {
        qDebug() << "The name already exists";
        QString strFistLine = tr("The name already exists,");
        QString strSecondeLine = tr("please input another one.");
        Utils::showSameNameDialog(this, strFistLine, strSecondeLine);
    } else {
        accept();
    }
    /************************ Mod by m000743 sunchengxi 2020-04-21:自定义命令修改的异常问题 End  ************************/
}

bool CustomCommandOptDlg::checkSequence(const QKeySequence &sequence)
{
    QString checkName = m_nameLineEdit->text();

    if (sequence.toString().isEmpty()) {
        return true;
    }

    QString reason;
    // 判断快捷键是否冲突
    if (!ShortcutManager::instance()->checkShortcutValid(checkName, sequence.toString(), reason)) {
        // 冲突
        if (sequence.toString() != "Esc") {
            showShortcutConflictMsgbox(reason);
        }
        m_shortCutLineEdit->clear();
        m_shortCutLineEdit->setKeySequence(QKeySequence(m_lastCmdShortcut));

        QTimer::singleShot(30, this, [&]() {
            m_shortCutLineEdit->setFocus();
        });
        return false;
    }

    return true;
}

void CustomCommandOptDlg::setModelIndex(QModelIndex mi)
{
    modelIndex = mi;
}

void CustomCommandOptDlg::slotDelCurCustomCommand()
{
    m_bDelOpt = true;
    reject();
}

void CustomCommandOptDlg::addCancelConfirmButtons()
{
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(9);
    buttonsLayout->setContentsMargins(10, 0, 10, 0);

    QFont btnFont;
    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setObjectName("CustomCancelButton");//Add by ut001000 renfeixiang 2020-08-13
    m_cancelBtn->setFixedWidth(209);
    m_cancelBtn->setFixedHeight(36);
    m_cancelBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_cancelBtn->setFont(btnFont);

    m_confirmBtn = new DSuggestButton(this);
    m_confirmBtn->setObjectName("CustomConfirmButton");//Add by ut001000 renfeixiang 2020-08-13
    m_confirmBtn->setFixedWidth(209);
    m_confirmBtn->setFixedHeight(36);
    m_confirmBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_confirmBtn->setFont(btnFont);

    //设置回车键默认响应的按钮
    m_confirmBtn->setDefault(true);

    setTabOrder(m_confirmBtn, m_closeButton);//设置右上角关闭按钮的tab键控制顺序

    DVerticalLine *verticalLine = new DVerticalLine(this);
    DPalette pa = DApplicationHelper::instance()->palette(verticalLine);
    QColor splitColor = pa.color(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitColor);
    verticalLine->setPalette(pa);
    verticalLine->setBackgroundRole(QPalette::Background);
    verticalLine->setAutoFillBackground(true);
    verticalLine->setFixedSize(3, 28);

    buttonsLayout->addWidget(m_cancelBtn);
    buttonsLayout->addWidget(verticalLine);
    buttonsLayout->addWidget(m_confirmBtn);
    /************************ Add by m000743 sunchengxi 2020-04-15:默认enter回车按下，走确认校验流程 Begin************************/
    m_confirmBtn->setDefault(true);
    /************************ Add by m000743 sunchengxi 2020-04-15:默认enter回车按下，走确认校验流程 End ************************/

    connect(m_cancelBtn, &DPushButton::clicked, this, &CustomCommandOptDlg::slotCancelBtnClicked);

    connect(m_confirmBtn, &DPushButton::clicked, this, &CustomCommandOptDlg::slotConfirmBtnClicked);

    /************************ Add by m000743 sunchengxi 2020-04-21:快捷键编辑结束后，enter按下可以响应校验 Begin************************/
    connect(m_shortCutLineEdit, &DKeySequenceEdit::editingFinished, this, &CustomCommandOptDlg::slotShortCutLineEditFinished);
    /************************ Add by m000743 sunchengxi 2020-04-21:快捷键编辑结束后，enter按下可以响应校验 End  ************************/

    m_mainLayout->addLayout(buttonsLayout);
}

inline void CustomCommandOptDlg::slotCancelBtnClicked()
{
    qDebug() << "cancelBtnClicked";
    m_confirmResultCode = QDialog::Rejected;
    reject();
    close();
}

inline void CustomCommandOptDlg::slotConfirmBtnClicked()
{
    qDebug() << "confirmBtnClicked";
    m_confirmResultCode = QDialog::Accepted;
    emit confirmBtnClicked();
}

inline void CustomCommandOptDlg::slotShortCutLineEditFinished()
{
    qDebug() << "shourtCut editingFinished";
    /******** Add by nt001000 renfeixiang 2020-05-14:修改快捷框输入后，添加（m_confirmBtn）按钮高亮问题,将光标从添加按钮取消，设置到快捷框上 Begin***************/
    //m_confirmBtn->setFocus();
    m_shortCutLineEdit->setFocus();
    /******** Add by nt001000 renfeixiang 2020-05-14:修改快捷框输入后，添加（m_confirmBtn）按钮高亮问题,将光标从添加按钮取消，设置到快捷框上 End***************/
}

QVBoxLayout *CustomCommandOptDlg::getMainLayout()
{
    return m_mainLayout;
}

QDialog::DialogCode CustomCommandOptDlg::getConfirmResult()
{
    return m_confirmResultCode;
}

/*******************************************************************************
 1. @函数:    setLogoVisable
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    设置自定义命令操作窗口的图标是否可见
*******************************************************************************/
//void CustomCommandOptDlg::setLogoVisable(bool visible)
//{
//    if (nullptr != m_logoIcon) {
//        m_logoIcon->setVisible(visible);
//    }
//}

void CustomCommandOptDlg::setTitle(const QString &title)
{
    if (nullptr != m_titleText) {
        m_titleText->setText(title);
    }
}

QLayout *CustomCommandOptDlg::getContentLayout()
{
    return m_contentLayout;
}

void CustomCommandOptDlg::setCancelBtnText(const QString &strCancel)
{
    m_cancelBtn->setText(strCancel);
    Utils::setSpaceInWord(m_cancelBtn);
}

void CustomCommandOptDlg::setConfirmBtnText(const QString &strConfirm)
{
    m_confirmBtn->setText(strConfirm);
    Utils::setSpaceInWord(m_confirmBtn);
}

void CustomCommandOptDlg::showShortcutConflictMsgbox(QString txt)
{
    /******** Modify by ut000610 daizhengwen 2020-05-27: 出现提示和快捷键显示不一致的问题 bug#28507****************/
    // fix#bug 37399
    for (QString key : ShortcutManager::instance()->m_mapReplaceText.keys()) {
        if (txt.contains(key)) {
            txt.replace(key, ShortcutManager::instance()->m_mapReplaceText[key]);
        }
    }
    /********************* Modify by ut000610 daizhengwen End ************************/
    // 若没有弹窗，初始化
    if (nullptr == m_shortcutConflictDialog) {
        m_shortcutConflictDialog = new DDialog(this);
        m_shortcutConflictDialog->setObjectName("CustomShortcutConflictDialog");//Add by ut001000 renfeixiang 2020-08-13
        /******** Modify by nt001000 renfeixiang 2020-05-29:修改 因为弹框改为非模态之后，快捷框冲突选中快捷框功能移动这 Begin***************/
        connect(m_shortcutConflictDialog, &DDialog::finished, this, &CustomCommandOptDlg::slotShortcutConflictDialogFinished);
        /******** Modify by nt001000 renfeixiang 2020-05-29:修改 修改因为弹框改为非模态之后，快捷框冲突选中快捷框功能移动这 End***************/
        m_shortcutConflictDialog->setIcon(QIcon::fromTheme("dialog-warning"));
        /***mod by ut001121 zhangmeng 20200521 将确认按钮设置为默认按钮 修复BUG26960***/
        m_shortcutConflictDialog->addButton(QString(tr("OK")), true, DDialog::ButtonNormal);
    }
    // 存在后显示
    m_shortcutConflictDialog->setTitle(QString(txt + QObject::tr("please set another one.")));
    m_shortcutConflictDialog->show();
}

inline void CustomCommandOptDlg::slotShortcutConflictDialogFinished()
{
    m_shortcutConflictDialog->hide();
    /******** Add by nt001000 renfeixiang 2020-05-14:快捷框输入已经存在的快捷后，快捷框依然是选中状态 Begin***************/
    QTimer::singleShot(30, this, &CustomCommandOptDlg::slotSetShortCutLineEditFocus);
    /******** Add by nt001000 renfeixiang 2020-05-14:快捷框输入已经存在的快捷后，快捷框依然是选中状态 End***************/
}

inline void CustomCommandOptDlg::slotSetShortCutLineEditFocus()
{
    m_shortCutLineEdit->setFocus();
}

void CustomCommandOptDlg::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != getContentLayout());

    getContentLayout()->addWidget(content);
}

void CustomCommandOptDlg::setIconPixmap(const QPixmap &iconPixmap)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setVisible(true);
        m_logoIcon->setPixmap(iconPixmap);
    }
}

void CustomCommandOptDlg::closeEvent(QCloseEvent *event)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    Q_UNUSED(event)

    done(-1);

    Q_EMIT closed();
}

void CustomCommandOptDlg::slotRefreshData(QString oldCmdName, QString newCmdName)
{
    if (CCT_ADD == m_type) {
        qDebug() << "Currently is the add operation interface";
        return;
    }
    //不进行刷新操作
    if (oldCmdName.isEmpty() && newCmdName.isEmpty()) {
        return;
    }
    //当前的自定义命令的名称 不是被修改的自定义名称时，不进行刷新操作
    if (m_currItemData->m_cmdName != oldCmdName) {
        return;
    }
    m_bRefreshCheck = true;
    qDebug() << "slotRefreshData---" <<  m_nameLineEdit->text();

    QAction *currAction = new QAction(ShortcutManager::instance());
    if (nullptr == currAction) {
        qDebug() << "slotRefreshData---new QAction error!!!";
        close();
        return;
    }
    currAction->setText(newCmdName);
    QAction *existAction = nullptr;
    existAction = ShortcutManager::instance()->checkActionIsExist(*currAction);
    //根据 newCmdName 查找自定义命令
    if (nullptr == existAction) {
        //不存在，则认为当前自定义命令在其他编辑窗口中删除了，如果打开了删除确认界面，则关闭删除确认界面。关闭当前编辑窗口
        delete currAction;
        if (m_dlgDelete && m_dlgDelete->isVisible()) {
            m_dlgDelete->close();
            m_dlgDelete = nullptr;
        }

        close();
    } else {
        //当前自定义命令在其他窗口修改，则更新数据
        delete currAction;
        m_nameLineEdit->setText(existAction->text());
        m_commandLineEdit->setText(existAction->data().toString());
        m_shortCutLineEdit->setKeySequence(existAction->shortcut());
        m_currItemData->m_cmdName = newCmdName;
    }

}

void CustomCommandOptDlg::closeRefreshDataConnection()
{
    disconnect(Service::instance(), &Service::refreshCommandPanel, this, &CustomCommandOptDlg::slotRefreshData);
}
