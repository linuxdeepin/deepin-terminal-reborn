/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     zhangmeng <zhangmeng@uniontech.com>
 *
 * Maintainer: zhangmeng <zhangmeng@uniontech.com>
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
#include "settings.h"
#include "newdspinbox.h"
//#include "utils.h"
#include "shortcutmanager.h"
#include "service.h"
#include "dbusmanager.h"
#include "tabrenamewidget.h"

#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DLog>
#include <DSlider>
#include <DApplicationHelper>
#include <DKeySequenceEdit>

#include <QApplication>
#include <QStandardPaths>
#include <QFontDatabase>

DWIDGET_USE_NAMESPACE
#define PRIVATE_PROPERTY_translateContext "_d_DSettingsWidgetFactory_translateContext"
Settings *Settings::m_settings_instance = new Settings();
DComboBox *Settings::comboBox = nullptr;
DComboBox *Settings::g_shellConfigCombox = nullptr;

// 全局变量  变量定义的位置可以变，目前只有这边用，所以定义到这儿
const QString DEFAULT_SHELL = "$SHELL";

Settings::Settings() : QObject(qApp)
{
    Utils::set_Object_Name(this);
}

Settings *Settings::instance()
{
    return m_settings_instance;
}

Settings::~Settings()
{
    if (nullptr != m_Watcher) {
        m_Watcher->deleteLater();
    }
    if (nullptr != m_backend) {
        m_backend->deleteLater();
    }
    if (nullptr != settings) {
        settings->deleteLater();
    }
}

// 统一初始化以后方可使用。
void Settings::init()
{
    m_configPath = QString("%1/%2/%3/config.conf")
                   .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), qApp->organizationName(), qApp->applicationName());
    m_backend = new QSettingBackend(m_configPath);
    m_backend->setObjectName("SettingsQSettingBackend");//Add by ut001000 renfeixiang 2020-08-13

    // 默认配置
    settings = DSettings::fromJsonFile(":/other/default-config.json");

    // 加载自定义配置
    settings->setBackend(m_backend);

    /************************ Add by sunchengxi 2020-09-15:Bug#47880 终端默认主题色应改为深色,当配置文件不存在或者配置项不是Light Begin************************/
    QFile file(m_configPath);
    if (!file.exists() || "Light" != m_backend->getOption("basic.interface.theme").toString()) {
        /************************ Mod by sunchengxi 2020-09-17:Bug#48349 主题色选择跟随系统异常 Begin************************/
        //DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
        //DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
        //setColorScheme("Dark");
        /************************ Mod by sunchengxi 2020-09-17:Bug#48349 主题色选择跟随系统异常 End ************************/
        if (m_backend->getOption("basic.interface.expand_theme").toString().isEmpty()) {
            DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
            setColorScheme("Dark");
        }

    }
    /************************ Add by sunchengxi 2020-09-15:Bug#47880 终端默认主题色应改为深色,当配置文件不存在或者配置项不是Light End ************************/

    /******** Modify by n014361 wangpeili 2020-01-10:   增加窗口状态选项  ************/
    auto windowState = settings->option("advanced.window.use_on_starting");
    QMap<QString, QVariant> windowStateMap;

    windowStateMap.insert("keys",
                          QStringList() << "window_normal"
                          << "split_screen"
                          << "window_maximum"
                          << "fullscreen");
    windowStateMap.insert("values",
                          QStringList() << tr("Normal window") << tr("Split screen") << tr("Maximum") << tr("Fullscreen"));
    windowState->setData("items", windowStateMap);

    for (QString &key : settings->keys()) {
        qDebug() << key << settings->value(key);
    }
    /********************* Modify by n014361 wangpeili End ************************/

    initConnection();
    loadDefaultsWhenReinstall();
    /******** Modify by ut000439 wangpeili 2020-06-12: 多进程模式，暂时取消使用*********/
# if 0
    //增加文件昨监视，以便多进程共享配置文件
    m_Watcher = new QFileSystemWatcher();
    m_Watcher->addPath(m_configPath);
    connect(m_Watcher, &QFileSystemWatcher::fileChanged, this, [this](QString file) {
        qDebug() << "fileChanged" << file;
        reload();
        //监控完一次就不再监控了，所以要再添加
        m_Watcher->addPath(m_configPath);
    });
#endif

    /***add begin by ut001121 zhangmeng 20200912 设置字号限制 修复42250***/
    auto option = settings->option("basic.interface.font_size");
    Konsole::__minFontSize = option->data("min").isValid() ? option->data("min").toInt() : DEFAULT_MIN_FONT_SZIE;
    Konsole::__maxFontSize = option->data("max").isValid() ? option->data("max").toInt() : DEFAULT_MAX_FONT_SZIE;

    // 校验正确
    if (Konsole::__minFontSize > Konsole::__maxFontSize) {
        qSwap(Konsole::__minFontSize, Konsole::__maxFontSize);
    }
    /***add end by ut001121***/

    //自定义主题配置初始化处理
    m_configCustomThemePath = QString("%1/%2/%3/customTheme.colorscheme")
                              .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), qApp->organizationName(), qApp->applicationName());
    themeSetting = new QSettings(m_configCustomThemePath, QSettings::IniFormat, this);

    QFile customThemeFile(m_configCustomThemePath);
    if (!customThemeFile.exists()) {
        //标题栏风格   // custom_theme_title_style
        themeSetting->setValue("CustomTheme/TitleStyle", "Dark");

        //背景颜色 “黑色” （0x252525)
        QColor backgroundColor(37, 37, 37);
        themeSetting->setValue("Background/Color", color2str(backgroundColor));
        //前景色 “绿色” （0x00FF00）
        QColor foregroundColor(0, 255, 0);
        themeSetting->setValue("Foreground/Color", color2str(foregroundColor));

        //自定义主题中实际修改的PS1参数只是一个"Color2Intense/Color" ，而配置"Color2/Color" 是为了保证python脚本取色RGB正常显示使用的固定不变，跟终端的所有主题都保持一致 //Green
        QColor color2Color(24, 178, 24);
        //提示符   PS1  “土黄色” （0x859900）
        QColor pS1Color(133, 153, 0);

        themeSetting->setValue("Color2/Color", color2str(color2Color));
        themeSetting->setValue("Color2Intense/Color", color2str(pS1Color));

        //自定义主题中实际修改的参数PS2只是一个"Color4Intense/Color" ，而配置"Color4/Color" 是为了保证python脚本取色RGB正常显示使用的固定不变，跟终端的所有主题都保持一致 //Blue
        QColor color4Color(24, 24, 178);
        //提示符   PS2  “蓝色” （0x3465A4）
        QColor pS2Color(52, 101, 164);
        themeSetting->setValue("Color4/Color", color2str(color4Color));
        themeSetting->setValue("Color4Intense/Color", color2str(pS2Color));

        //配置"Color1/Color"与"Color1Intense/Color"  是为了保证python脚本取色RGB正常显示使用的固定不变，跟终端的所有主题都保持一致 //Red
        QColor color1Color(178, 24, 24);
        QColor color1IntenseColor(255, 84, 84);
        themeSetting->setValue("Color1/Color", color2str(color1Color));
        themeSetting->setValue("Color1Intense/Color", color2str(color1IntenseColor));


        //参考深色主题与浅色主题的配置文件，增加默认参数，保证python脚本取色在自定义主题下可以取到相应参数的配色，begin
        //Color3
        themeSetting->setValue("Color3/Color", color2str(QColor(255, 192, 5)));
        //Color3Intense
        themeSetting->setValue("Color3Intense/Color", color2str(QColor(255, 192, 5)));

        //Color5
        themeSetting->setValue("Color5/Color", color2str(QColor(236, 0, 72)));
        //Color5Intense
        themeSetting->setValue("Color5Intense/Color", color2str(QColor(236, 0, 72)));

        //Color6
        themeSetting->setValue("Color6/Color", color2str(QColor(42, 167, 231)));
        //Color6Intense
        themeSetting->setValue("Color6Intense/Color", color2str(QColor(42, 167, 231)));

        //Color7
        themeSetting->setValue("Color7/Color", color2str(QColor(242, 242, 242)));
        //Color7Intense
        themeSetting->setValue("Color7Intense/Color", color2str(QColor(242, 242, 242)));

        //参考深色主题与浅色主题的配置文件，增加默认参数，保证python脚本取色在自定义主题下可以取到相应参数的配色，end
    }

}

QStringList Settings::color2str(QColor color)
{
    QStringList ret;
    ret << QString::number(color.red());
    ret << QString::number(color.green());
    ret << QString::number(color.blue());
    return ret;
}

//重新安装终端后在这里重置状态
void Settings::loadDefaultsWhenReinstall()
{
    QDir installFlagPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!installFlagPath.exists()) {
        installFlagPath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }
}

void Settings::addShellOption()
{
    g_shellConfigCombox->clear();
    // 获取shells
    QMap<QString, QString> shellsMap = Service::instance()->getShells();
    // item列表
    QStringList keysList;
    // 初始值
    keysList << DEFAULT_SHELL;
    // 数据转换
    for (const QString key : shellsMap.keys()) {
        keysList << key;
    }
    g_shellConfigCombox->addItems(keysList);
}

QPair<QWidget *, QWidget *>  Settings::createTabTitleFormatWidget(QObject *opt, bool isRemote)
{
    DTK_CORE_NAMESPACE::DSettingsOption *option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(opt);
    // tabrenaemewidget 第一个false表示标签重命名 第二个true表示不加标签提示
    TabRenameWidget *tabTitleFormat = new TabRenameWidget(isRemote, true);

    tabTitleFormat->getInputedit()->setText(option->value().toString());

    QPair<QWidget *, QWidget *> optionWidget =
        DSettingsWidgetFactory::createStandardItem(QByteArray(), option, tabTitleFormat);
    // 别的窗口修改了设置，这里需要同步设置显示
    connect(option, &DSettingsOption::valueChanged, tabTitleFormat, [ = ](QVariant var) {
        tabTitleFormat->getInputedit()->setText(var.toString());
    });

    // 点击按钮，改变输入框内容后给输入框设置焦点
    option->connect(tabTitleFormat->getInputedit(), &DLineEdit::textChanged, option, [ = ]() {
        tabTitleFormat->getInputedit()->lineEdit()->setFocus();
    });
    // 编辑结束才修改标签
    option->connect(tabTitleFormat->getInputedit(), &DLineEdit::editingFinished, option, [ = ]() {
        option->setValue(tabTitleFormat->getInputedit()->text());
    });



    return optionWidget;
}

void Settings::initConnection()
{
    connect(settings, &Dtk::Core::DSettings::valueChanged, this, [ = ](const QString & key, const QVariant & value) {
        Q_UNUSED(value)
        if (key.contains("basic.interface.") || key.contains("advanced.cursor.") || key.contains("advanced.scroll.")) {
            emit terminalSettingChanged(key);
        } else if (key.contains("shortcuts.")) {
            emit shortcutSettingChanged(key);
        } else {
            emit windowSettingChanged(key);
        }
    });

    QPointer<DSettingsOption> opacity = settings->option("basic.interface.opacity");
    connect(opacity, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit opacityChanged(value.toInt() / 100.0);
    });

    QPointer<DSettingsOption> cursorShape = settings->option("advanced.cursor.cursor_shape");
    connect(cursorShape, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit cursorShapeChanged(value.toInt());
    });

    QPointer<DSettingsOption> cursorBlink = settings->option("advanced.cursor.cursor_blink");
    connect(cursorBlink, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit cursorBlinkChanged(value.toBool());
    });

    QPointer<DSettingsOption> backgroundBlur = settings->option("advanced.window.blurred_background");
    connect(backgroundBlur, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit backgroundBlurChanged(value.toBool());
    });

    /******** Modify by n014361 wangpeili 2020-01-06:  字体，字体大小实时生效 ****************/
    QPointer<DSettingsOption> fontSize = settings->option("basic.interface.font_size");
    connect(fontSize, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit fontSizeChanged(value.toInt());
    });

    QPointer<DSettingsOption> family = settings->option("basic.interface.font");
    connect(family, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit fontChanged(value.toString());
    });

    QPointer<DSettingsOption> PressingScroll = settings->option("advanced.scroll.scroll_on_key");
    connect(PressingScroll, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit pressingScrollChanged(value.toBool());
    });
    /********************* Modify by n014361 wangpeili End ************************/

    // 标签标题格式变化
    QPointer<DSettingsOption> tabFormat = settings->option("basic.tab_title.tab_title_format");
    connect(tabFormat, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit tabFormatChanged(value.toString());
    });

    // 远程标签标题格式变化
    QPointer<DSettingsOption> remoteTabFormat = settings->option("basic.tab_title.remote_tab_title_format");
    connect(remoteTabFormat, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit remoteTabFormatChanged(value.toString());
    });
}

qreal Settings::opacity() const
{
    return settings->option("basic.interface.opacity")->value().toInt() / 100.0;
}

QString Settings::colorScheme() const
{
    return settings->option("basic.interface.theme")->value().toString();
}

QString Settings::encoding() const
{
    return m_EncodeName;
}

QString Settings::fontName()
{
    return settings->option("basic.interface.font")->value().toString();
}

int Settings::fontSize()
{
    return settings->option("basic.interface.font_size")->value().toInt();
}

bool Settings::PressingScroll()
{
    return settings->option("advanced.scroll.scroll_on_key")->value().toBool();
}

bool Settings::OutputtingScroll()
{
    return settings->option("advanced.scroll.scroll_on_output")->value().toBool();
}

/*******************************************************************************
 1. @函数:    reload
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    设置界面重新加载设置的键值
*******************************************************************************/
//void Settings::reload()
//{
//    QSettings  newSettings(m_configPath, QSettings::IniFormat);
//    for (QString &key : newSettings.childGroups()) {
//        //　当系统变更键值的时候，配置文件中会有一些＂垃圾＂配置，删除他
//        if (!settings->keys().contains(key)) {
//            qDebug() << "reload failed: system not found " << key << "now remove it";
//            newSettings.remove(key);
//            continue;
//        }
//        if (settings->value(key) != newSettings.value(key + "/value")) {
//            qDebug() << "reload update:" << key << settings->value(key);
//            settings->option(key)->setValue(newSettings.value(key + "/value"));
//        }
//    }
//}

QString Settings::tabTitleFormat() const
{
    return settings->option("basic.tab_title.tab_title_format")->value().toString();
}

QString Settings::remoteTabTitleFormat() const
{
    return settings->option("basic.tab_title.remote_tab_title_format")->value().toString();
}

QString Settings::shellPath() const
{
    QString strShellProgram = settings->option("advanced.shell.default_shell")->value().toString();
    // $SHELL无法写入配置文件中
    if (DEFAULT_SHELL == strShellProgram || strShellProgram.isEmpty()) {
        QString shell{ getenv("SHELL") };
        return shell;
    }

    return strShellProgram;
}

void Settings::reloadShellOptions()
{
    // 记录设置当前值
    QString strShellProgram = settings->option("advanced.shell.default_shell")->value().toString();
    // 更新shell选项
    addShellOption();
    QMap<QString, QString> shellMap = Service::instance()->shellsMap();
    // 设置之前的项 若strShellProgram不存在会自动选取第一项
    g_shellConfigCombox->setCurrentText(shellMap.key(strShellProgram));
}

int Settings::cursorShape() const
{
    return settings->option("advanced.cursor.cursor_shape")->value().toInt();
}

bool Settings::cursorBlink() const
{
    return settings->option("advanced.cursor.cursor_blink")->value().toBool();
}

bool Settings::backgroundBlur() const
{
    return settings->option("advanced.window.blurred_background")->value().toBool();
}

void Settings::setColorScheme(const QString &name)
{
    return settings->option("basic.interface.theme")->setValue(name);
}

QString Settings::extendColorScheme() const
{
    return settings->option("basic.interface.expand_theme")->value().toString();
}

void Settings::setExtendColorScheme(const QString &name)
{
    return settings->option("basic.interface.expand_theme")->setValue(name);
}

/*******************************************************************************
 1. @函数:    setEncoding
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    设置界面设置编码
*******************************************************************************/
//void Settings::setEncoding(const QString &name)
//{
//    if (name != m_EncodeName) {
//        m_EncodeName = name;
//        emit encodeSettingChanged(name);
//        qDebug() << "encode changed to" << name;
//    }
//}

/*******************************************************************************
 1. @函数:    setKeyValue
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    设置界面设置键值
*******************************************************************************/
//void Settings::setKeyValue(const QString &name, const QString &value)
//{
//    settings->option(name)->setValue(value);
//}

bool Settings::IsPasteSelection()
{
    return settings->option("advanced.cursor.auto_copy_selection")->value().toBool();
}

bool Settings::isShortcutConflict(const QString &Name, const QString &Key)
{
    for (QString &tmpKey : settings->keys()) {
        // 获取设置里面的快捷键键值
        QString strKey = settings->value(tmpKey).toString();
        // 比较前将快捷键都设置为同一级别的键值，然后比较
        // 例ctlr+shift+? => ctrl+shift+/
        if (Utils::converUpToDown(strKey) == Utils::converUpToDown(Key)) {
            if (Name != tmpKey) {
                qDebug() << Name << Key << "is conflict with Settings!" << tmpKey << settings->value(tmpKey);
                return  true;
            }
        }
    }
    return  false;
}

/******** Add by ut001000 renfeixiang 2020-06-15:增加 每次显示设置界面时，更新设置的等宽字体 Begin***************/
void Settings::HandleWidthFont()
{
    QStringList Whitelist;
    Whitelist = DBusManager::callAppearanceFont("monospacefont");

    //将新安装的字体，加载到字体库中
    QFontDatabase base;
    for (int i = 0; i < Whitelist.count(); ++i) {
        QString name = Whitelist.at(i);
        if (-1 == comboBox->findText(name)) {
            QString fontpath =  QDir::homePath() + "/.local/share/fonts/" + name + "/";// + name + ".ttf";
            QDir dir(fontpath);
            if (dir.count() > 2) {
                fontpath = fontpath + dir[2];
            }
            int ret = base.addApplicationFont(fontpath);
            if (-1 == ret) {
                qDebug() << "load " << name << " font faild";
            }
        }
    }
    std::sort(Whitelist.begin(), Whitelist.end(), [ = ](const QString & str1, const QString & str2) {
        QCollator qc;
        return qc.compare(str1, str2) < 0;
    });

    QString fontname = comboBox->currentText();
    comboBox->clear();
    comboBox->addItems(Whitelist);
    qDebug() << "HandleWidthFont has update";
    if (Whitelist.contains(fontname)) {
        comboBox->setCurrentText(fontname);
    }
}


bool Settings::enableControlFlow(void)
{
    return !settings->option("advanced.shell.enable_ctrl_flow")->value().toBool();
}

/******** Add by ut001000 renfeixiang 2020-06-15:增加 每次显示设置界面时，更新设置的等宽字体 End***************/

/*******************************************************************************
 1. @函数:    getKeyshortcutFromKeymap
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    设置界面从键盘映射获取密钥快捷方式
*******************************************************************************/
//QString Settings::getKeyshortcutFromKeymap(const QString &keyCategory, const QString &keyName)
//{
//    return settings->option(QString("shortcuts.%1.%2").arg(keyCategory, keyName))->value().toString();
//}

/******** Modify by n014361 wangpeili 2020-01-04: 创建Combox控件        ***********×****/

QPair<QWidget *, QWidget *> Settings::createFontComBoBoxHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    /******** Modify by ut001000 renfeixiang 2020-06-15:修改 comboBox修改成成员变量，修改DBUS获取失败场景，设置成系统默认等宽字体 Begin***************/
    comboBox = new DComboBox;
    comboBox->setObjectName("SettingsFontFamilyComboBox");//Add by ut001000 renfeixiang 2020-08-14

    QPair<QWidget *, QWidget *> optionWidget =
        DSettingsWidgetFactory::createStandardItem(QByteArray(), option, comboBox);

    QStringList Whitelist;
    Whitelist = DBusManager::callAppearanceFont("monospacefont");

    std::sort(Whitelist.begin(), Whitelist.end(), [ = ](const QString & str1, const QString & str2) {
        QCollator qc;
        return qc.compare(str1, str2) < 0;
    });

    qDebug() << "createFontComBoBoxHandle get system monospacefont";
    if (Whitelist.size() <= 0) {
        //一般不会走这个分支，除非DBUS出现问题
        qDebug() << "DBusManager::callAppearanceFont failed, get control font failed.";
        //DBUS获取字体失败后，设置系统默认的等宽字体
        Whitelist << "Courier 10 Pitch" << "DejaVu Sans Mono" << "Liberation Mono"
                  << "Noto Mono" << "Noto Sans Mono" << "Noto Sans Mono CJK JP"
                  << "Noto Sans Mono CJK KR" << "Noto Sans Mono CJK SC"
                  << "Noto Sans Mono CJK TC";
    }
    comboBox->addItems(Whitelist);
    /******** Modify by ut001000 renfeixiang 2020-06-15:修改 comboBox修改成成员变量，修改DBUS获取失败场景，设置成系统默认等宽字体 End***************/

    if (option->value().toString().isEmpty()) {
        option->setValue(QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
    }

    // init.
    comboBox->setCurrentText(option->value().toString());

    connect(option, &DSettingsOption::valueChanged, comboBox, [ = ](QVariant var) {
        comboBox->setCurrentText(var.toString());
    });

    option->connect(
    comboBox, &QComboBox::currentTextChanged, option, [ = ](const QString & text) {
        option->setValue(text);
    });

    return optionWidget;
}

QPair<QWidget *, QWidget *> Settings::createCustomSliderHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    DSlider *slider = new DSlider;
    slider->setIconSize(QSize(20, 20));
    slider->setLeftIcon(QIcon::fromTheme("dt_opacity_left"));
    slider->setRightIcon(QIcon::fromTheme("dt_opacity_right"));
    slider->setMaximum(option->data("max").toInt());
    slider->setMinimum(option->data("min").toInt());
    slider->setValue(static_cast<int>(instance()->opacity() * 100));
    //fix bug 65140 焦点在透明度，点击Page up/ down，点击8次即可调节到最前/后，和控制中心不一致
    slider->setPageStep(1);
    slider->slider()->setTickInterval(1);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, slider);

    connect(option, &DSettingsOption::valueChanged, slider, [ = ](QVariant var) {
        slider->setValue(var.toInt());
    });

    option->connect(slider, &DSlider::valueChanged, option, [ = ](QVariant var) {
        option->setValue(var.toInt());
    });

    return optionWidget;
}

QPair<QWidget *, QWidget *> Settings::createSpinButtonHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    auto rightWidget = new NewDspinBox();

    rightWidget->setValue(option->value().toInt());

    QPair<QWidget *, QWidget *> optionWidget =
        DSettingsWidgetFactory::createStandardItem(QByteArray(), option, rightWidget);
    connect(
    option, &DSettingsOption::valueChanged, rightWidget, [ = ](QVariant var) {
        rightWidget->setValue(var.toInt());
    });
    //Add by ut001000 renfeixiang 2020-11-06 使用QSpinBox自带的valueChanged信号
    option->connect(rightWidget, static_cast<void (QSpinBox::*)(int value)>(&QSpinBox::valueChanged),
    option, [ = ](int value) {
        option->setValue(value);
    });

    return optionWidget;
} /********************* Modify by n014361 wangpeili End ************************/

QPair<QWidget *, QWidget *> Settings::createShortcutEditOptionHandle(/*DSettingsWidgetFactoryPrivate *p,*/ QObject *opt)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(opt);
    auto rightWidget = new KeySequenceEdit(option);

    rightWidget->setObjectName("OptionShortcutEdit");
    rightWidget->ShortcutDirection(Qt::AlignLeft);

    auto optionValue = option->value();
    auto translateContext = opt->property(PRIVATE_PROPERTY_translateContext).toByteArray();

    // 控件初始加载配置文件的值
    auto updateWidgetValue = [ = ](const QVariant & optionValue, DTK_CORE_NAMESPACE::DSettingsOption * opt) {
        Q_UNUSED(opt)
        QKeySequence sequence(optionValue.toString());
        QString keyseq = sequence.toString();
        if (keyseq == SHORTCUT_VALUE) {
            return;
        }
        rightWidget->setKeySequence(sequence);
    };
    updateWidgetValue(optionValue, option);

    // 控件输入
    option->connect(rightWidget, &KeySequenceEdit::editingFinished, rightWidget, [ = ](const QKeySequence & sequence) {
        // 删除
        if (sequence.toString() == "Backspace") {
            rightWidget->clear();
            option->setValue(SHORTCUT_VALUE);
            return ;
        }
        // 取消
        if (sequence.toString() == "Esc") {
            rightWidget->clear();
            rightWidget->setKeySequence(QKeySequence(rightWidget->option()->value().toString()));
            return ;
        }

        QString reason;
        // 有效查询
        if (!ShortcutManager::instance()->checkShortcutValid(rightWidget->option()->key(), sequence.toString(), reason)) {
            if (sequence.toString() != "Esc") {
                Service::instance()->showShortcutConflictMsgbox(reason);
            }
            // 界面数据还原
            rightWidget->clear();
            rightWidget->setKeySequence(QKeySequence(rightWidget->option()->value().toString()));
            return ;
        }
        option->setValue(sequence.toString());
    });

    // 配置修改
    option->connect(option, &DTK_CORE_NAMESPACE::DSettingsOption::valueChanged, rightWidget, [ = ](const QVariant & value) {
        QString keyseq = value.toString();
        qDebug() << "valueChanged" << rightWidget->option()->key() << keyseq;
        if (keyseq == SHORTCUT_VALUE || keyseq.isEmpty()) {
            rightWidget->clear();
            return;
        }

        rightWidget->setKeySequence(QKeySequence(keyseq));
    });

    return DSettingsWidgetFactory::createStandardItem(translateContext, option, rightWidget);
}

QPair<QWidget *, QWidget *> Settings::createTabTitleFormatOptionHandle(QObject *opt)
{
    return createTabTitleFormatWidget(opt, false);
}

QPair<QWidget *, QWidget *> Settings::createRemoteTabTitleFormatOptionHandle(QObject *opt)
{
    return createTabTitleFormatWidget(opt, true);
}

QPair<QWidget *, QWidget *> Settings::createShellConfigComboxOptionHandle(QObject *obj)
{
    DSettingsOption *option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    // shell配置框
    g_shellConfigCombox = new DComboBox;
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, g_shellConfigCombox);
    // 添加shell配置选项
    addShellOption();

    connect(option, &DSettingsOption::valueChanged, g_shellConfigCombox, [ = ](QVariant var) {
        // 恢复默认值
        if (DEFAULT_SHELL == var.toString()) {
            g_shellConfigCombox->setCurrentText(DEFAULT_SHELL);
            return;
        }
        // 不是默认值的其他情况
        QMap<QString, QString> shellMap = Service::instance()->shellsMap();
        g_shellConfigCombox->setCurrentText(shellMap.key(var.toString()));
    });

    option->connect(g_shellConfigCombox, &DComboBox::currentTextChanged, option, [ = ](const QString & strShell) {
        QMap<QString, QString> shellMap = Service::instance()->shellsMap();
        option->setValue(shellMap[strShell]);
    });

    QMap<QString, QString> shellMap = Service::instance()->shellsMap();
    // 设置默认值
    g_shellConfigCombox->setCurrentText(shellMap.key(option->value().toString()));

    return optionWidget;
}


