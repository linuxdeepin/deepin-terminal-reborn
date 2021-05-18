/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  daizhengwen<daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen<daizhengwen@uniontech.com>
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
#include "serverconfigoptdlg.h"
#include "serverconfigmanager.h"
#include "termcommandlinkbutton.h"
#include "utils.h"

#include <DFontSizeManager>
#include <DPushButton>
#include <DSuggestButton>
#include <DVerticalLine>
#include <DApplicationHelper>
#include <DGuiApplicationHelper>
#include <DFileDialog>
#include <DPalette>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextCodec>
#include <QSpacerItem>
#include <iterator>//added byq qinyaning
#include <QTimer>
#include <QDebug>

DGUI_USE_NAMESPACE

ServerConfigOptDlg::ServerConfigOptDlg(ServerConfigOptType type, ServerConfig *curServer, QWidget *parent)
    : DAbstractDialog(parent),
      m_type(type),
      m_curServer(curServer),
      m_titleLabel(new DLabel),
      m_iconLabel(new DLabel),
      m_closeButton(new DWindowCloseButton(this)),
      m_serverName(new DLineEdit),
      m_address(new DLineEdit),
      m_port(new DSpinBox),
      m_port_tip(new DAlertControl(m_port)),
      m_userName(new DLineEdit),
      m_password(new DPasswordEdit),
      m_privateKey(new DLineEdit),
      m_fileDialog(new DSuggestButton),
      m_group(new DLineEdit),
      m_path(new DLineEdit),
      m_command(new DLineEdit),
      m_coding(new DComboBox),
      m_backSapceKey(new DComboBox),
      m_deleteKey(new DComboBox),
      m_advancedOptions(new DPushButton(tr("Advanced options"))),
      m_delServer(new TermCommandLinkButton())

{
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 Begin***************/
    Utils::set_Object_Name(this);
    m_titleLabel->setObjectName("RemoteTitleLabel");
    m_iconLabel->setObjectName("RemoteIconLabel");
    m_closeButton->setObjectName("RemoteCloseButton");
    m_serverName->setObjectName("RemoteServerNameLineEdit");
    m_address->setObjectName("RemoteAddressLineEdit");
    m_port->setObjectName("RemotePortDSpinBox");
    m_userName->setObjectName("RemoteUserNameLineEdit");
    m_password->setObjectName("RemoteDPasswordEdit");
    m_privateKey->setObjectName("RemotePrivateKeyLineEdit");
    m_fileDialog->setObjectName("RemoteFileDialogDSuggestButton");
    m_group->setObjectName("RemoteGroupLineEdit");
    m_path->setObjectName("RemotePathLineEdit");
    m_command->setObjectName("RemoteCommandLineEdit");
    m_coding->setObjectName("RemoteEncodeComboBox");
    m_backSapceKey->setObjectName("RemoteBackSapceKeyComboBox");
    m_deleteKey->setObjectName("RemoteDeleteKeyComboBox");
    m_advancedOptions->setObjectName("RemoteAdvancedOptionsQPushButton");
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 End***************/
    setWindowModality(Qt::WindowModal);
    setFixedWidth(459);
    setAutoFillBackground(true);
    initUI();
    initData();
}

void ServerConfigOptDlg::initUI()
{
    //all layout
    QVBoxLayout *m_VBoxLayout = new QVBoxLayout();
    m_VBoxLayout->setSpacing(SPACEHEIGHT);
    m_VBoxLayout->setContentsMargins(0, 0, 0, SPACEHEIGHT);
    this->setFixedHeight(392);
    //head layout
    m_iconLabel->setFixedSize(ICONSIZE_50, ICONSIZE_50);
    m_titleLabel->setText(tr("Add Server"));
    m_titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_closeButton->setFocusPolicy(Qt::TabFocus);
    m_closeButton->setFixedWidth(ICONSIZE_50);
    m_closeButton->setIconSize(QSize(ICONSIZE_50, ICONSIZE_50));

    // 字体
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T5, QFont::DemiBold);
    // 字色
    DPalette palette = m_titleLabel->palette();
    QColor color;
    if (DApplicationHelper::instance()->themeType() == DApplicationHelper::DarkType) {
        color = QColor::fromRgb(192, 198, 212, 255);
    } else {
        color = QColor::fromRgb(0, 26, 46, 255);
    }
    palette.setBrush(QPalette::WindowText, color);
    m_titleLabel->setPalette(palette);

    QHBoxLayout *headLayout = new QHBoxLayout();
    headLayout->setSpacing(0);
    headLayout->setContentsMargins(0, 0, 0, 0);
    // 为了和closebutton对应是标题居中
    headLayout->addWidget(m_iconLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    headLayout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    headLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignTop);
    connect(m_closeButton, &DDialogCloseButton::clicked, this, [ = ]() {
        reject();
    });
    m_VBoxLayout->addLayout(headLayout, Qt::AlignTop);
    m_VBoxLayout->addStretch(SPACEHEIGHT);
    //main layout
    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->setColumnStretch(1, 1);
    pGridLayout->setContentsMargins(28, 0, 30, 0);
    pGridLayout->setSpacing(SPACEHEIGHT);
    DLabel *pServerNameLabel = new DLabel(tr("Server name:"));
    setLabelStyle(pServerNameLabel);

    m_serverName->lineEdit()->setPlaceholderText(tr("Required"));
    QTimer::singleShot(30, this, [&]() {
        m_serverName->lineEdit()->selectAll();
    });
    pGridLayout->addWidget(pServerNameLabel);
    pGridLayout->addWidget(m_serverName);

    DLabel *pAddressLabel = new DLabel(tr("Address:"));
    setLabelStyle(pAddressLabel);
    m_address->lineEdit()->setPlaceholderText(tr("Required"));
    m_address->setFixedWidth(150);
    pGridLayout->addWidget(pAddressLabel);
    pGridLayout->addWidget(m_address);

    QHBoxLayout *portLayout = new QHBoxLayout();
    DLabel *pPortLabel = new DLabel(tr("Port:"));
    pPortLabel->setAlignment(Qt::AlignLeft);
    pPortLabel->setAlignment(Qt::AlignVCenter);
    pPortLabel->setFixedWidth(40);
    DFontSizeManager::instance()->bind(pPortLabel, DFontSizeManager::T6);
    m_port->setRange(0, 65535);
    m_port->setValue(DEFAULTPORT);
    m_port->setSingleStep(1);
    m_port->setFixedWidth(70);
    //comment code for old version dtk build
#if (DTK_VERSION_MAJOR > 5 || (DTK_VERSION_MAJOR >= 5 && (DTK_VERSION_MINOR > 1 || (DTK_VERSION_MINOR >= 1 && DTK_VERSION_BUILD >= 2))))
    m_port->lineEdit()->setClearButtonEnabled(false);
#endif
    // 去除上下按钮
    m_port->setButtonSymbols(DSpinBox::NoButtons);
    // 禁用输入法
    m_port->setAttribute(Qt::WA_InputMethodEnabled, false);
    portLayout->addWidget(pPortLabel);
    portLayout->addWidget(m_port);
    pGridLayout->addLayout(portLayout, 1, 1, Qt::AlignRight);

    DLabel *pUsernameLabel = new DLabel(tr("Username:"));
    setLabelStyle(pUsernameLabel);
    m_userName->lineEdit()->setPlaceholderText(tr("Required"));
    pGridLayout->addWidget(pUsernameLabel);
    pGridLayout->addWidget(m_userName);

    DLabel *pPasswordLabel = new DLabel(tr("Password:"));
    setLabelStyle(pPasswordLabel);

    m_password->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    pGridLayout->addWidget(pPasswordLabel);
    pGridLayout->addWidget(m_password);

    DLabel *pPrivateKeyLabel = new DLabel(tr("Certificate:"));
    setLabelStyle(pPrivateKeyLabel);


    /******** Modify by m000714 daizhengwen 2020-04-20: 添加DFileChooseEditDialog****************/
    pGridLayout->addWidget(pPrivateKeyLabel);
    QHBoxLayout *privateKeyLayout = new QHBoxLayout();
    privateKeyLayout->setSpacing(5);
    privateKeyLayout->setContentsMargins(0, 0, 0, 0);
    privateKeyLayout->addWidget(m_privateKey);
    privateKeyLayout->addWidget(m_fileDialog);
    m_fileDialog->setIcon(DStyleHelper(m_fileDialog->style()).standardIcon(DStyle::SP_SelectElement, nullptr));
    m_fileDialog->setIconSize(QSize(24, 24));
    connect(m_fileDialog, &DSuggestButton::clicked, this, &ServerConfigOptDlg::slotFileChooseDialog);
    pGridLayout->addLayout(privateKeyLayout, 4, 1);
    /********************* Modify by m000714 daizhengwen End ************************/

    //senior layout
    DWidget *seniorWidget = new DWidget;
    seniorWidget->setFixedWidth(459);
    seniorWidget->setContentsMargins(0, 0, 0, 0);
    QGridLayout *seniorLayout = new QGridLayout();
    seniorLayout->setAlignment(Qt::AlignTop);
    seniorLayout->setSpacing(10);
    seniorLayout->setContentsMargins(28, 0, 30, 0);
    seniorLayout->setColumnStretch(1, 1);
    DLabel *pGroupLabel = new DLabel(tr("Group:"));
    setLabelStyle(pGroupLabel);
    seniorLayout->addWidget(pGroupLabel);
    seniorLayout->addWidget(m_group);
    m_group->setFixedWidth(DEFAULTHEIGHT);

    DLabel *pPathLabel = new DLabel(tr("Path:"));
    setLabelStyle(pPathLabel);
    seniorLayout->addWidget(pPathLabel);
    seniorLayout->addWidget(m_path);
    m_path->setFixedWidth(DEFAULTHEIGHT);

    DLabel *pCommandLabel = new DLabel(tr("Command:"));
    setLabelStyle(pCommandLabel);
    seniorLayout->addWidget(pCommandLabel);
    seniorLayout->addWidget(m_command);
    m_command->setFixedWidth(DEFAULTHEIGHT);

    DLabel *pCodingLabel = new DLabel(tr("Encoding:"));
    setLabelStyle(pCodingLabel);
    seniorLayout->addWidget(pCodingLabel);
    seniorLayout->addWidget(m_coding);
    m_coding->setFixedWidth(DEFAULTHEIGHT);

    DLabel *pBackspaceKeyLabel = new DLabel(tr("Backspace key:"));
    setLabelStyle(pBackspaceKeyLabel);
    seniorLayout->addWidget(pBackspaceKeyLabel);
    seniorLayout->addWidget(m_backSapceKey);
    m_backSapceKey->setFixedWidth(DEFAULTHEIGHT);

    DLabel *pDeleteKeyLabel = new DLabel(tr("Delete key:"));
    setLabelStyle(pDeleteKeyLabel);
    seniorLayout->addWidget(pDeleteKeyLabel);
    seniorLayout->addWidget(m_deleteKey);
    m_deleteKey->setFixedWidth(DEFAULTHEIGHT);
    seniorWidget->setLayout(seniorLayout);

    m_VBoxLayout->addLayout(pGridLayout, Qt::AlignHCenter);
    QSpacerItem *upItem = new QSpacerItem(this->width(), 10);
    m_VBoxLayout->addSpacerItem(upItem);

    palette = DApplicationHelper::instance()->palette(m_advancedOptions);
    palette.setColor(DPalette::ButtonText, palette.color(DPalette::Highlight));
    m_advancedOptions->setPalette(palette);

    m_advancedOptions->setFlat(true);
    m_advancedOptions->setFocusPolicy(Qt::TabFocus);

    m_VBoxLayout->addWidget(m_advancedOptions, 0, Qt::AlignHCenter);
    QSpacerItem *downItem = new QSpacerItem(this->width(), 10);
    m_VBoxLayout->addSpacerItem(downItem);
    m_VBoxLayout->addWidget(seniorWidget, 0, Qt::AlignHCenter);
    m_delServer->setText(tr("Delete server"));
    m_VBoxLayout->addWidget(m_delServer, 0, Qt::AlignHCenter);
    m_delServer->hide();
    seniorWidget->hide();

    DFontSizeManager::instance()->bind(m_advancedOptions, DFontSizeManager::T8, QFont::Normal);
    connect(m_advancedOptions, &DCommandLinkButton::clicked, this, [ = ]() {
        seniorWidget->show();
        setTabOrder(m_advancedOptions, m_group);
        m_advancedOptions->hide();
        upItem->changeSize(this->width(), 0);
        downItem->changeSize(this->width(), 0);
        if (m_type == SCT_MODIFY) {
            m_delServer->show();
            this->setFixedHeight(670);
            qDebug() << "remote dialog show advance options";
        } else {
            this->setFixedHeight(630);
        }
    });

    DPushButton *pCancelButton = new DPushButton(tr("Cancel"));
    DSuggestButton *pAddSaveButton = new DSuggestButton(tr("Add"));
    //Add a line by m000750 zhangmeng 2020-04-22设置回车触发默认按钮
    pAddSaveButton->setDefault(true);
    if (m_type == SCT_MODIFY) {
        m_titleLabel->setText(tr("Edit Server"));
        pAddSaveButton->setText(tr("Save"));
    }
    Utils::setSpaceInWord(pCancelButton);
    Utils::setSpaceInWord(pAddSaveButton);
    DPalette pa = DApplicationHelper::instance()->palette(pAddSaveButton);
    QBrush brush = pa.textLively().color();
    pa.setBrush(DPalette::ButtonText, brush);
    pAddSaveButton->setPalette(pa);
    QHBoxLayout *pBtHbLayout = new QHBoxLayout();
    DVerticalLine *line = new DVerticalLine();
    pBtHbLayout->setContentsMargins(10, 0, 10, 0);
    pBtHbLayout->setSpacing(9);
    pBtHbLayout->addWidget(pCancelButton);
    pBtHbLayout->addWidget(line);
    pBtHbLayout->addWidget(pAddSaveButton);
    m_VBoxLayout->addStretch(10);
    m_VBoxLayout->addLayout(pBtHbLayout, Qt::AlignBottom);
    setLayout(m_VBoxLayout);

    connect(pCancelButton, &DPushButton::clicked, this, [ = ]() {
        reject();
    });
    connect(pAddSaveButton, &DPushButton::clicked, this, &ServerConfigOptDlg::slotAddSaveButtonClicked);
    connect(m_delServer, &TermCommandLinkButton::clicked, this, [ = ]() {
        setDelServer(true);
        accept();
    });
    // 字体颜色随主题变化变化
    connect(DApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ServerConfigOptDlg::handleThemeTypeChanged);

    // 设置焦点顺序
    setTabOrder(pAddSaveButton, m_closeButton);
}

inline void ServerConfigOptDlg::handleThemeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    DPalette palette = m_titleLabel->palette();
    //palette.setBrush(QPalette::WindowText, palette.color(DPalette::TextTitle));
    QColor color;
    if (themeType == DApplicationHelper::DarkType) {
        color = QColor::fromRgb(192, 198, 212, 255);
    } else {
        color = QColor::fromRgb(0, 26, 46, 255);
    }
    palette.setBrush(QPalette::WindowText, color);
    m_titleLabel->setPalette(palette);

    palette = DApplicationHelper::instance()->palette(m_delServer);
    palette.setColor(DPalette::ButtonText, palette.color(DPalette::TextWarning));
    m_delServer->setPalette(palette);
}

void ServerConfigOptDlg::initData()
{
    QList<QString> textCodeList = getTextCodec();
    m_coding->addItems(textCodeList);
    QList<QString> backSpaceKeyList = getBackSpaceKey();
    m_backSapceKey->addItems(backSpaceKeyList);
    QList<QString> deleteKeyList = getDeleteKey();
    m_deleteKey->addItems(deleteKeyList);
    if (m_type == SCT_MODIFY && m_curServer != nullptr) {
        m_serverName->setText(m_curServer->m_serverName);
        m_currentServerName = m_serverName->text();
        m_address->setText(m_curServer->m_address);
        m_port->setValue(m_curServer->m_port.toInt());
        m_userName->setText(m_curServer->m_userName);
        m_password->setText(m_curServer->m_password);
        m_privateKey->setText(m_curServer->m_privateKey);
        m_group->setText(m_curServer->m_group);
        m_path->setText(m_curServer->m_path);
        m_command->setText(m_curServer->m_command);
        if (!m_curServer->m_encoding.isEmpty()) {
            int textCodeIndex = textCodeList.indexOf(m_curServer->m_encoding);
            m_coding->setCurrentIndex(textCodeIndex);
        }
        if (!m_curServer->m_backspaceKey.isEmpty()) {
            int backSpaceKeyIndex = backSpaceKeyList.indexOf(m_curServer->m_backspaceKey);
            m_backSapceKey->setCurrentIndex(backSpaceKeyIndex);
        }
        if (!m_curServer->m_deleteKey.isEmpty()) {
            int deleteKeyIndex = deleteKeyList.indexOf(m_curServer->m_deleteKey);
            m_deleteKey->setCurrentIndex(deleteKeyIndex);
        }
    }
}

QList<QString> ServerConfigOptDlg::getTextCodec()
{
    /******** Modify by ut000610 daizhengwen 2020-05-28: 编码列表和编码插件一致****************/
    //    QList<QByteArray> list = QTextCodec::availableCodecs();
    QList<QByteArray> list = Utils::encodeList();
    /********************* Modify by ut000610 daizhengwen End ************************/
    QList<QString> textCodecList;
    for (QByteArray &byteArr : list) {
        QString str = QString(byteArr);
        if (!textCodecList.contains(str)) {
            textCodecList.append(str);
        }
    }
    return textCodecList;
}

QList<QString> ServerConfigOptDlg::getBackSpaceKey()
{
    QList<QString> eraseKeyList;
    eraseKeyList.append(tr("ascii-del"));
    eraseKeyList.append(tr("auto"));
    eraseKeyList.append(tr("control-h"));
    eraseKeyList.append(tr("escape-sequence"));
    eraseKeyList.append(tr("tty"));
    return eraseKeyList;
}

QList<QString> ServerConfigOptDlg::getDeleteKey()
{
    QList<QString> eraseKeyList;
    eraseKeyList.append(tr("escape-sequence"));
    eraseKeyList.append(tr("ascii-del"));
    eraseKeyList.append(tr("auto"));
    eraseKeyList.append(tr("control-h"));
    eraseKeyList.append(tr("tty"));
    return eraseKeyList;
}

void ServerConfigOptDlg::setLabelStyle(DLabel *label)
{
    label->setAlignment(Qt::AlignLeft);
    label->setAlignment(Qt::AlignVCenter);
    label->setFixedWidth(100);
    label->setMinimumHeight(36);

    DFontSizeManager::instance()->bind(label, DFontSizeManager::T6);
}

ServerConfigOptDlg::~ServerConfigOptDlg()
{
}

void ServerConfigOptDlg::updataData(ServerConfig *curServer)
{
    qDebug() << "ServerConfigOptDlg server configuration options updata data.";
    // 读取配置
    QList<QString> textCodeList = getTextCodec();
    QList<QString> backSpaceKeyList = getBackSpaceKey();
    QList<QString> deleteKeyList = getDeleteKey();
    // 重置m_curServer
//    resetCurServer(curServer);
    m_curServer = curServer;
    // 设置数据
    m_serverName->setText(curServer->m_serverName);
    m_address->setText(curServer->m_address);
    m_port->setValue(curServer->m_port.toInt());
    m_userName->setText(curServer->m_userName);
    m_password->setText(curServer->m_password);
    m_privateKey->setText(curServer->m_privateKey);
    m_group->setText(curServer->m_group);
    m_path->setText(curServer->m_path);
    m_command->setText(curServer->m_command);
    if (!curServer->m_encoding.isEmpty()) {
        int textCodeIndex = textCodeList.indexOf(curServer->m_encoding);
        m_coding->setCurrentIndex(textCodeIndex);
    }
    if (!curServer->m_backspaceKey.isEmpty()) {
        int backSpaceKeyIndex = backSpaceKeyList.indexOf(curServer->m_backspaceKey);
        m_backSapceKey->setCurrentIndex(backSpaceKeyIndex);
    }
    if (!curServer->m_deleteKey.isEmpty()) {
        int deleteKeyIndex = deleteKeyList.indexOf(curServer->m_deleteKey);
        m_deleteKey->setCurrentIndex(deleteKeyIndex);
    }

    m_currentServerName = m_serverName->text();
}

ServerConfig ServerConfigOptDlg::getData()
{
    ServerConfig config;
    config.m_serverName = m_serverName->text();
    config.m_address = m_address->text();
    config.m_userName = m_userName->text();
    config.m_password = m_password->text();
    config.m_privateKey = m_privateKey->text();
    config.m_port = m_port->text();
    config.m_group = m_group->text();
    config.m_path = m_path->text();
    config.m_command = m_command->text();
    config.m_encoding = m_coding->currentText();
    config.m_backspaceKey = m_backSapceKey->currentText();
    config.m_deleteKey = m_deleteKey->currentText();
    return config;
}

void ServerConfigOptDlg::resetCurServer(ServerConfig config)
{
    m_curServer = &config;
}

void ServerConfigOptDlg::slotAddSaveButtonClicked()
{
    qDebug() << "ServerConfigOptDlg add and save button clicled slot function.";
    // 服务器名为空
    if (m_serverName->text().trimmed().isEmpty()) {
        m_serverName->showAlertMessage(tr("Please enter a server name"), m_serverName);
        return;
    }

    /***add begin by ut001121 zhangmeng 20200615 限制名称字符长度 修复BUG31286***/
    if (m_serverName->text().length() > MAX_NAME_LEN) {
        m_serverName->showAlertMessage(QObject::tr("The name should be no more than 32 characters"), m_serverName);
        return;
    }
    /***add end by ut001121***/

    // 地址为空
    if (m_address->text().trimmed().isEmpty()) {
        m_address->showAlertMessage(tr("Please enter an IP address"), m_address);
        return;
    }
    // 端口为空
    if (m_port->text().trimmed().isEmpty()) {
        m_port_tip->showAlertMessage(tr("Please enter a port"));
        return;
    }

    //---added by qinyaning(nyq) to show the tip when username is empty---//
    if (m_userName->text().trimmed().isEmpty()) { //如果用户名为空， 提示用户名为空， 添加失败
        m_userName->showAlertMessage(tr("Please enter a username"), m_userName);
        return;
    }
    m_currentServerName = m_serverName->text();
    //------------------------------------------------------------------//
    //--added by qinyaning(nyq) to solve the bug 19116: You can create a new remote server with the same name--//
    if ((m_type == SCT_ADD)
            || ((m_type == SCT_MODIFY && m_curServer != nullptr)
                && (m_curServer->m_serverName.trimmed() != m_serverName->text().trimmed()))) { /*此时用户已经在修改模式下修改了服务器名称*/
        QMap<QString, QList<ServerConfig *>> severConfigs = ServerConfigManager::instance()->getServerConfigs();
        for (QMap<QString, QList<ServerConfig *>>::iterator iter = severConfigs.begin(); iter != severConfigs.end(); ++iter) {
            QList<ServerConfig *> value = iter.value();
            for (int i = 0; i < value.size(); i++) {
                if (value[i]->m_serverName.trimmed() == m_serverName->text().trimmed()) { //服务器名相同
                    QString strFirstLine = tr("The server name already exists,");
                    QString strSecondeLine = tr("please input another one. ");
                    Utils::showSameNameDialog(this, strFirstLine, strSecondeLine);
                    return;
                }
            }
        }
    }
    //-------------------------------------------------------------------//
    ServerConfig *config = new ServerConfig();
    config->m_serverName = m_serverName->text().trimmed();
    config->m_address = m_address->text().trimmed();
    config->m_userName = m_userName->text().trimmed();
    config->m_password = m_password->text();
    config->m_privateKey = m_privateKey->text();
    config->m_port = m_port->text();
    config->m_group = m_group->text().trimmed();
    config->m_path = m_path->text();
    config->m_command = m_command->text();
    config->m_encoding = m_coding->currentText();
    config->m_backspaceKey = m_backSapceKey->currentText();
    config->m_deleteKey = m_deleteKey->currentText();
    if (m_type == SCT_ADD) {
        ServerConfigManager::instance()->saveServerConfig(config);
        ServerConfigManager::instance()->refreshList();
    } else if (m_type == SCT_MODIFY && m_curServer != nullptr) {
        ServerConfigManager::instance()->modifyServerConfig(config, m_curServer);
    }
    qDebug() << __FUNCTION__ << "add or save remote config";
    accept();
    qDebug() << __FUNCTION__ << "add or save remote config finish";
}

void ServerConfigOptDlg::slotFileChooseDialog()
{
    DFileDialog dialog(this, QObject::tr("Select the private key file"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);
    dialog.setLabelText(QFileDialog::Accept, QObject::tr("Select"));

    int code = dialog.exec();

    if (code == QDialog::Accepted && !dialog.selectedFiles().isEmpty()) {
        QStringList list = dialog.selectedFiles();
        const QString fileName = list.first();

        m_privateKey->setText(fileName);
    }
}
