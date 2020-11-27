/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 * Maintainer: rekols <rekols@foxmail.com>
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

#include "utils.h"
#include "operationconfirmdlg.h"
#include "termwidget.h"

#include <DLog>
#include <DMessageBox>
#include <DLineEdit>
#include <DFileDialog>

#include <QDBusMessage>
#include <QDBusConnection>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontInfo>
#include <QMimeType>
#include <QApplication>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QImageReader>
#include <QPixmap>
#include <QFile>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QTextLayout>
#include <QTime>
#include <QFontMetrics>
#include "terminputdialog.h"

#include "dbusmanager.h"

QHash<QString, QPixmap> Utils::m_imgCacheHash;
QHash<QString, QString> Utils::m_fontNameCache;

Utils::Utils(QObject *parent) : QObject(parent)
{
}

Utils::~Utils()
{
}
/*******************************************************************************
 1. @函数:    getQssContent
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    获取QSS中内容
*******************************************************************************/
QString Utils::getQssContent(const QString &filePath)
{
    QFile file(filePath);
    QString qss;

    if (file.open(QIODevice::ReadOnly)) {
        qss = file.readAll();
    }

    return qss;
}
/*******************************************************************************
 1. @函数:    getConfigPath
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    获取config的路径
*******************************************************************************/
QString Utils::getConfigPath()
{
    static QStringList list = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    const QString path = list.first();
    QDir dir(
        QDir(path).filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}
/*******************************************************************************
 1. @函数:    suffixList
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    设置后缀列表只包含*.ttf *.ttc *.otf格式
*******************************************************************************/
QString Utils::suffixList()
{
    return QString("Font Files (*.ttf *.ttc *.otf)");
}
/*******************************************************************************
 1. @函数:    renderSVG
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/
QPixmap Utils::renderSVG(const QString &filePath, const QSize &size)
{
    if (m_imgCacheHash.contains(filePath)) {
        return m_imgCacheHash.value(filePath);
    }

    QImageReader reader;
    QPixmap pixmap;

    reader.setFileName(filePath);

    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        pixmap.load(filePath);
    }

    m_imgCacheHash.insert(filePath, pixmap);

    return pixmap;
}
/*******************************************************************************
 1. @函数:    loadFontFamilyFromFiles
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    从文件中加载字体系列
*******************************************************************************/
QString Utils::loadFontFamilyFromFiles(const QString &fontFileName)
{
    if (m_fontNameCache.contains(fontFileName)) {
        return m_fontNameCache.value(fontFileName);
    }

    QString fontFamilyName = "";

    QFile fontFile(fontFileName);
    if (!fontFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Open font file error";
        return fontFamilyName;
    }

    int loadedFontID = QFontDatabase::addApplicationFontFromData(fontFile.readAll());
    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
    if (!loadedFontFamilies.empty()) {
        fontFamilyName = loadedFontFamilies.at(0);
    }
    fontFile.close();

    m_fontNameCache.insert(fontFileName, fontFamilyName);
    return fontFamilyName;
}
/*******************************************************************************
 1. @函数:    getElidedText
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    获取删除的文字
*******************************************************************************/
QString Utils::getElidedText(QFont font, QString text, int MaxWith, Qt::TextElideMode elideMode)
{
    if (text.isEmpty()) {
        return "";
    }

    QFontMetrics fontWidth(font);

    // 计算字符串宽度
    int width = fontWidth.width(text);

    // 当字符串宽度大于最大宽度时进行转换
    if (width >= MaxWith) {
        // 右部显示省略号
        text = fontWidth.elidedText(text, elideMode, MaxWith);
    }

    return text;
}
/*******************************************************************************
 1. @函数:    holdTextInRect
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    在矩形中保留文本
*******************************************************************************/
const QString Utils::holdTextInRect(const QFont &font, QString text, const QSize &size)
{
    QFontMetrics fm(font);
    QTextLayout layout(text);

    layout.setFont(font);

    QStringList lines;
    QTextOption &text_option = *const_cast<QTextOption *>(&layout.textOption());

    text_option.setWrapMode(QTextOption::WordWrap);
    text_option.setAlignment(Qt::AlignTop | Qt::AlignLeft);

    layout.beginLayout();

    QTextLine line = layout.createLine();
    int height = 0;
    int lineHeight = fm.height();

    while (line.isValid()) {
        height += lineHeight;

        if (height + lineHeight > size.height()) {
            const QString &end_str = fm.elidedText(text.mid(line.textStart()), Qt::ElideRight, size.width());

            layout.endLayout();
            layout.setText(end_str);

            text_option.setWrapMode(QTextOption::NoWrap);
            layout.beginLayout();
            line = layout.createLine();
            line.setLineWidth(size.width() - 1);
            text = end_str;
        } else {
            line.setLineWidth(size.width());
        }

        lines.append(text.mid(line.textStart(), line.textLength()));

        if (height + lineHeight > size.height())
            break;

        line = layout.createLine();
    }

    layout.endLayout();

    return lines.join("");
}
/*******************************************************************************
 1. @函数:    convertToPreviewString
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    转换为预览字符串
*******************************************************************************/
QString Utils::convertToPreviewString(QString fontFilePath, QString srcString)
{
    if (fontFilePath.isEmpty()) {
        return srcString;
    }

    QString strFontPreview = srcString;

    QRawFont rawFont(fontFilePath, 0, QFont::PreferNoHinting);
    bool isSupport = rawFont.supportsCharacter(QChar('a'));
    bool isSupportF = rawFont.supportsCharacter(QChar('a' | 0xf000));
    if ((!isSupport && isSupportF)) {
        QChar *chArr = new QChar[srcString.length() + 1];
        for (int i = 0; i < srcString.length(); i++) {
            int ch = srcString.at(i).toLatin1();
            //判断字符ascii在32～126范围内(共95个)
            if (ch >= 32 && ch <= 126) {
                ch |= 0xf000;
                chArr[i] = QChar(ch);
            } else {
                chArr[i] = srcString.at(i);
            }
        }
        chArr[srcString.length()] = '\0';
        QString strResult(chArr);
        strFontPreview = strResult;
        delete[] chArr;
    }

    return strFontPreview;
}
/*******************************************************************************
 1. @函数:    getRandString
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    获取随机字符串
*******************************************************************************/
QString Utils::getRandString()
{
    int max = 6;  //字符串长度
    QString tmp = QString("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    QString str;
    QTime t;
    t = QTime::currentTime();
    qsrand(t.msec() + t.second() * 1000);
    for (int i = 0; i < max; i++) {
        int len = qrand() % tmp.length();
        str[i] = tmp.at(len);
    }
    return str;
}
/*******************************************************************************
 1. @函数:    showDirDialog
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    显示目录对话框
*******************************************************************************/
QString Utils::showDirDialog(QWidget *widget)
{
    QString curPath = QDir::currentPath();
    QString dlgTitle = QObject::tr("Select a directory to save the file");

    DFileDialog dialog(widget, dlgTitle, curPath);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(DFileDialog::DontConfirmOverwrite);
    dialog.setLabelText(QFileDialog::Accept, QObject::tr("Select"));
    int code = dialog.exec();

    if (code == QDialog::Accepted && !dialog.selectedFiles().isEmpty()) {
        QStringList list = dialog.selectedFiles();
        const QString dirName = list.first();
        return dirName;
    } else {
        return "";
    }
}
/*******************************************************************************
 1. @函数:    showFilesSelectDialog
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    显示文件选择对话框
*******************************************************************************/
QStringList Utils::showFilesSelectDialog(QWidget *widget)
{
    QString curPath = QDir::currentPath();
    QString dlgTitle = QObject::tr("Select file to upload");

    DFileDialog dialog(widget, dlgTitle, curPath);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setLabelText(QFileDialog::Accept, QObject::tr("Upload"));

    // 选择文件，却点击了叉号
    int code = dialog.exec();
    if (code == QDialog::Accepted) {
        return dialog.selectedFiles();
    } else {
        return QStringList();
    }
}
/*******************************************************************************
 1. @函数:    showExitConfirmDialog
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    显示退出确认对话框
*******************************************************************************/
bool Utils::showExitConfirmDialog(CloseType type, int count)
{
    /******** Modify by m000714 daizhengwen 2020-04-17: 统一使用dtk Dialog****************/
#ifndef USE_DTK
    OperationConfirmDlg optDlg;
    optDlg.setFixedSize(380, 160);
    optDlg.setOperatTypeName(QObject::tr("Programs are still running in terminal"));
    optDlg.setTipInfo(QObject::tr("Are you sure you want to exit?"));
    optDlg.setOKCancelBtnText(QObject::tr("Exit"), QObject::tr("Cancel"));
    optDlg.exec();

    return (optDlg.getConfirmResult() == QDialog::Accepted);
#else
    // count < 1 不提示
    if (count < 1) {
        return true;
    }
    QString title;
    QString txt;
    if (type != CloseType_Window) {
        // 默认的count = 1的提示
        title = QObject::tr("Close this terminal?");
        txt = QObject::tr("There is still a process running in this terminal. "
                          "Closing the terminal will kill it.");
        // count > 1 提示
        if (count > 1) {
            txt = QObject::tr("There are still %1 processes running in this terminal. "
                              "Closing the terminal will kill all of them.")
                  .arg(count);
        }
    } else {
        title = QObject::tr("Close this window?");
        txt = QObject::tr("There are still processes running in this window. Closing the window will kill all of them.");
    }

    DDialog dlg(title, txt);
    dlg.setIcon(QIcon::fromTheme("deepin-terminal"));
    dlg.addButton(QString(tr("Cancel")), false, DDialog::ButtonNormal);
    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close Begin***************/
    dlg.addButton(QString(tr("Close")), true, DDialog::ButtonWarning);
    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close End***************/
    return (dlg.exec() == DDialog::Accepted);
#endif
    /********************* Modify by m000714 daizhengwen End ************************/
}
/*******************************************************************************
 1. @函数:    getExitDialogText
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    获取退出对话框文本
*******************************************************************************/
void Utils::getExitDialogText(CloseType type, QString &title, QString &txt, int count)
{
    // count < 1 不提示
    if (count < 1) {
        return ;
    }
    //QString title;
    //QString txt;
    if (type == CloseType_Window) {
        title = QObject::tr("Close this window?");
        txt = QObject::tr("There are still processes running in this window. Closing the window will kill all of them.");
    } else {
        // 默认的count = 1的提示
        title = QObject::tr("Close this terminal?");
        txt = QObject::tr("There is still a process running in this terminal. "
                          "Closing the terminal will kill it.");
        // count > 1 提示
        if (count > 1) {
            txt = QObject::tr("There are still %1 processes running in this terminal. "
                              "Closing the terminal will kill all of them.")
                  .arg(count);
        }
    }
}
/*******************************************************************************
 1. @函数:    showExitUninstallConfirmDialog
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    显示退出卸载确认对话框
*******************************************************************************/
bool Utils::showExitUninstallConfirmDialog()
{
    DDialog dlg(QObject::tr("Programs are still running in terminal"), QObject::tr("Are you sure you want to uninstall it?"));
    dlg.setIcon(QIcon::fromTheme("deepin-terminal"));
    dlg.addButton(QString(tr("Cancel")), false, DDialog::ButtonNormal);
    dlg.addButton(QString(tr("OK")), true, DDialog::ButtonWarning);
    return (dlg.exec() == DDialog::Accepted);
}
/*******************************************************************************
 1. @函数:    showUnistallConfirmDialog
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    显示卸载确认对话框
*******************************************************************************/
//单词可能拼错了showUninstallConfirmDialog
bool Utils::showUnistallConfirmDialog(QString commandname)
{
#ifndef USE_DTK
    OperationConfirmDlg dlg;
    dlg.setFixedSize(380, 160);
    dlg.setOperatTypeName(QObject::tr("Are you sure you want to uninstall this application?"));
    dlg.setTipInfo(QObject::tr("You will not be able to use Terminal any longer."));
    dlg.setOKCancelBtnText(QObject::tr("OK"), QObject::tr("Cancel"));
    dlg.exec();

    return (dlg.getConfirmResult() == QDialog::Accepted);
#else
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 根据remove和purge卸载命令，显示不同的弹框信息 Begin***************/
    QString title = "", text = "";
    if (commandname == "remove") {
        title = QObject::tr("Are you sure you want to uninstall this application?");
        text = QObject::tr("You will not be able to use Terminal any longer.");
    } else if (commandname == "purge") {
        //后面根据产品提供的信息，修改此处purge命令卸载时的弹框信息
        title = QObject::tr("Are you sure you want to uninstall this application?");
        text = QObject::tr("You will not be able to use Terminal any longer.");
    }
    DDialog dlg(title, text);
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 根据remove和purge卸载命令，显示不同的弹框信息 Begin***************/
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.addButton(QObject::tr("Cancel"), false, DDialog::ButtonNormal);
    dlg.addButton(QObject::tr("OK"), true, DDialog::ButtonWarning);
    return (dlg.exec() == DDialog::Accepted);
#endif
}

/*******************************************************************************
 1. @函数:    showShortcutConflictDialog
 2. @作者:    n014361 王培利
 3. @日期:    2020-03-31
 4. @说明:    快捷键冲突框显示
*******************************************************************************/
bool Utils::showShortcutConflictDialog(QString conflictkey)
{
    QString str = qApp->translate("DSettingsDialog", "This shortcut conflicts with %1")
                  .arg(QString("<span style=\"color: rgba(255, 90, 90, 1);\">%1</span>").arg(conflictkey));

    OperationConfirmDlg optDlg;
    QPixmap warnning = QIcon::fromTheme("dialog-warning").pixmap(QSize(32, 32));
    optDlg.setIconPixmap(warnning);
    optDlg.setOperatTypeName(str);
    optDlg.setTipInfo(QObject::tr("Click on Add to make this shortcut effective immediately"));
    optDlg.setOKCancelBtnText(QObject::tr("Replace"), QObject::tr("Cancel"));
    optDlg.setFixedSize(380, 160);
    optDlg.exec();
    return optDlg.getConfirmResult() == QDialog::Accepted;
}
/*******************************************************************************
 1. @函数:    showShortcutConflictMsgbox
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    显示快捷方式冲突消息框
*******************************************************************************/
bool Utils::showShortcutConflictMsgbox(QString txt)
{

    DDialog dlg;
//    dlg.setTitle(QString(txt ));
//    dlg.setMessage(QObject::tr(" please set another one."));
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.setTitle(QString(txt + QObject::tr("please set another one.")));
    /***mod by ut001121 zhangmeng 20200521 将确认按钮设置为默认按钮 修复BUG26960***/
    dlg.addButton(QString(tr("OK")), true, DDialog::ButtonNormal);
    dlg.exec();
    return  true;
}

/*******************************************************************************
 1. @函数:     setSpaceInWord
 2. @作者:     m000714 戴正文
 3. @日期:     2020-04-10
 4. @说明:     为按钮两个中文之间添加空格
*******************************************************************************/
void Utils::setSpaceInWord(DPushButton *button)
{
    const QString &text = button->text();

    if (text.count() == 2) {
        for (const QChar &ch : text) {
            switch (ch.script()) {
            case QChar::Script_Han:
            case QChar::Script_Katakana:
            case QChar::Script_Hiragana:
            case QChar::Script_Hangul:
                break;
            default:
                return;
            }
        }

        button->setText(QString().append(text.at(0)).append(QChar::Nbsp).append(text.at(1)));
    }
}
/*******************************************************************************
 1. @函数:    showRenameTitleDialog
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    显示重命名标题对话框
*******************************************************************************/
void Utils::showRenameTitleDialog(QString oldTitle, QWidget *parentWidget)
{
    TermInputDialog *pDialog = new TermInputDialog(parentWidget);
    pDialog->setWindowModality(Qt::ApplicationModal);
    pDialog->setFixedSize(380, 180);
    pDialog->setIcon(QIcon::fromTheme("deepin-terminal"));
    pDialog->setFocusPolicy(Qt::NoFocus);
    pDialog->showDialog(oldTitle, parentWidget);
}

/*******************************************************************************
 1. @函数:    showSameNameDialog
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-04-16
 4. @说明:    当有相同名称时，弹出弹窗给用户确认
*******************************************************************************/
void Utils::showSameNameDialog(QWidget *parent, const QString &firstLine, const QString &secondLine)
{
    DDialog *dlg = new DDialog(parent);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setTitle(QString(firstLine + secondLine));
    dlg->setIcon(QIcon::fromTheme("dialog-warning"));
    dlg->addButton(QString(QObject::tr("OK")), true, DDialog::ButtonNormal);
    dlg->show();
    moveToCenter(dlg);
}
/*******************************************************************************
 1. @函数:    clearChildrenFocus
 2. @作者:    n014361 王培利
 3. @日期:    2020-05-08
 4. @说明:    清空控件内部所有子控件的焦点获取
 　　　　　　　安全考虑，不要全局使用．仅在个别控件中使用
*******************************************************************************/
void Utils::clearChildrenFocus(QObject *objParent)
{
    // 可以获取焦点的控件名称列表
    QStringList foucswidgetlist;
    foucswidgetlist << "QLineEdit" << TERM_WIDGET_NAME;

    //qDebug() << "checkChildrenFocus start" << objParent->children().size();
    for (QObject *obj : objParent->children()) {
        if (!obj->isWidgetType()) {
            continue;
        }
        QWidget *widget = static_cast<QWidget *>(obj);
        if (Qt::NoFocus != widget->focusPolicy()) {
            //qDebug() << widget << widget->focusPolicy() << widget->metaObject()->className();
            if (!foucswidgetlist.contains(widget->metaObject()->className())) {
                widget->setFocusPolicy(Qt::NoFocus);
            }
        }
        clearChildrenFocus(obj);
    }

    //qDebug() << "checkChildrenFocus over" << objParent->children().size();
}
/*******************************************************************************
 1. @函数:    parseCommandLine
 2. @作者:    ut000439 王培利
 3. @日期:    2020-06-01
 4. @说明:    函数参数解析
             appControl = true, 为main函数使用，遇到-h -v 时，整个进程会退出
             为false时，为唯一进程使用，主要是解析变量出来。
*******************************************************************************/
void Utils::parseCommandLine(QStringList arguments, TermProperties &Properties, bool appControl)
{
    QCommandLineParser parser;
    parser.setApplicationDescription(qApp->applicationDescription());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions);
    QCommandLineOption optWorkDirectory({ "w", "work-directory" },
                                        QObject::tr("Set the work directory"),
                                        "path");
    QCommandLineOption optWindowState({ "m", "window-mode" },
                                      QString(QObject::tr("Set the window mode on starting") + " (normal, maximum, fullscreen, splitscreen)"),
                                      "state-mode");
    QCommandLineOption optExecute({ "e", "execute" },
                                  QObject::tr("Execute a command in the terminal"),
                                  "command");
    QCommandLineOption optScript({ "C", "run-script" },
                                 QObject::tr("Run script string in the terminal"),
                                 "script");
    // QCommandLineOption optionExecute2({"x", "Execute" }, "Execute command in the terminal", "command");
    QCommandLineOption optQuakeMode({ "q", "quake-mode" },
                                    QObject::tr("Run in quake mode"),
                                    "");
    QCommandLineOption optKeepOpen("keep-open",
                                   QObject::tr("Keep terminal open when command finishes"),
                                   "");
    // parser.addPositionalArgument("e",  "Execute command in the terminal", "command");

    parser.addOptions({ optWorkDirectory,
                        optExecute, /*optionExecute2,*/
                        optQuakeMode,
                        optWindowState,
                        optKeepOpen,
                        optScript });
    // parser.addPositionalArgument("-e", QObject::tr("Execute command in the terminal"), "command");

    // 解析参数
    if (!parser.parse(arguments)) {
        qDebug() << "parser error:" << parser.errorText();
    }

    if (parser.isSet(optExecute)) {
        /************************ Add by sunchengxi 2020-09-15:Bug#42864 无法同时打开多个终端 Begin************************/
        Properties[KeepOpen] = false;
        Properties[Execute] = parseExecutePara(arguments);
        /************************ Add by sunchengxi 2020-09-15:Bug#42864 无法同时打开多个终端 End ************************/
    }
    if (parser.isSet(optWorkDirectory)) {
        Properties[WorkingDir] = parser.value(optWorkDirectory);
    }
    if (parser.isSet(optKeepOpen)) {
        Properties[KeepOpen] = "";
    }
    if (parser.isSet(optScript)) {
        Properties[Script] = parser.value(optScript);
    }

    if (parser.isSet(optQuakeMode)) {
        Properties[QuakeMode] = true;
    } else {
        Properties[QuakeMode] = false;
    }
    // 默认均为非首个
    Properties[SingleFlag] = false;
    if (parser.isSet(optWindowState)) {
        Properties[StartWindowState] = parser.value(optWindowState);
        if (appControl) {
            QStringList validString = { "maximum", "fullscreen", "splitscreen", "normal" };
            // 参数不合法时，会显示help以后，直接退出。
            if (!validString.contains(parser.value(optWindowState))) {
                parser.showHelp();
                qApp->quit();
            }
        }
    }

    if (appControl) {
        // 处理相应参数，当遇到-v -h参数的时候，这里进程会退出。
        //qDebug() << "parse commandLine";
        parser.process(arguments);
    } else {
        qDebug() << "input args:" << qPrintable(arguments.join(" "));
        qDebug() << "arg: optionWorkDirectory" << parser.value(optWorkDirectory);
        qDebug() << "arg: optionExecute" << Properties[Execute].toStringList().join(" ");
        //    qDebug() << "optionExecute2"<<parser.value(optionExecute2);
        qDebug() << "arg: optionQuakeMode" << parser.isSet(optQuakeMode);
        qDebug() << "arg: optionWindowState" << parser.isSet(optWindowState);
        // 这个位置参数解析出来是无法匹配的，可是不带前面标识符，无法准确使用。
        // qDebug() << "arg: positionalArguments" << parser.positionalArguments();
    }

    //qDebug() << "parse commandLine is ok";

    return;
}
/*******************************************************************************
 1. @函数:    parseExecutePara
 2. @作者:    ut000439 王培利
 3. @日期:    2020-06-03
 4. @说明:    解析execute参数,解析出来后，会从参数表中删除相关内容，防止process异常．
             任意长，任意位置,
             支持解析例子：
            deepin-terminal -e "bash -c 'ping 127.0.0.1 -c 5'" -w /
            deepin-terminal -e 'bash -c "ping 127.0.0.1 -c 5"' -w /
            deepin-terminal -e  bash -c 'ping 127.0.0.1 -c 5'  -w /
            deepin-terminal -e "ping  127.0.0.1  -c 5"
            deepin-terminal -e "ping  127.0.0.1  -c 5"         -w /
            deepin-terminal -e  ping  127.0.0.1  -c 5          -w /
            deepin-terminal -e  ping "127.0.0.1" -c 5          -w /
*******************************************************************************/
QStringList Utils::parseExecutePara(QStringList &arguments)
{
    QVector<QString> keys;
    keys << "-e"
         << "--execute";
    keys << "-h"
         << "--help";
    keys << "-v"
         << "--version";
    keys << "-w"
         << "--work-directory";
    keys << "-q"
         << "--quake-mode";
    keys << "-m"
         << "--window-mode";
    keys << "--keep-open";
    keys << "-C"
         << "--run-script";
    QString opt = "-e";
    int index = arguments.indexOf(opt);
    if (index == -1) {
        opt = "--execute";
        index = arguments.indexOf(opt);
    }

    index++;
    int startIndex = index;
    QStringList paraList;
    while (index < arguments.size()) {
        QString str = arguments.at(index);
        // qDebug()<<"check arg"<<str;
        // 如果找到下一个指令符就停
        if (keys.contains(str)) {
            break;
        }
        if (index == startIndex) {
            // 第一个参数，支持嵌入二次解析，其它的参数不支持
            paraList += parseNestedQString(str);
        } else {
            paraList.append(str);
        }

        index++;
    }
    // 将-e 以及后面参数全部删除，防止出现参数被终端捕捉异常情况
    if (paraList.size() != 0) {
        for (int i = 0; i < index - startIndex; i++) {
            arguments.removeAt(startIndex);
            qDebug() << arguments.size();
        }
        arguments.removeOne("-e");
        arguments.removeOne("--execute");
        qDebug() <<  opt << paraList << "arguments" << arguments;
    }

    return paraList;
}
/*******************************************************************************
 1. @函数:    parseNestedQString
 2. @作者:    ut000439 王培利
 3. @日期:    2020-06-04
 4. @说明:    解析嵌套的qstring为qstringlist,只支持一级
             如："bash -c 'ping 127.0.0.1'" -> "bash", "-c", "ping 127.0.0.1"
             如：'bash -c "ping 127.0.0.1"' -> "bash, "-c", "ping 127.0.0.1"
             如："bash -c  ping 127.0.0.1"-> "bash, "-c", "ping 127.0.0.1"
*******************************************************************************/
QStringList Utils::parseNestedQString(QString str)
{
    QStringList paraList;
    int iLeft = NOT_FOUND;
    int iRight = NOT_FOUND;

    // 如果只有一个引号
    if (str.count("\"") >= 1) {
        iLeft = str.indexOf("\"");
        iRight = str.lastIndexOf("\"");
    } else if (str.count("\'") >= 1) {
        iLeft = str.indexOf("\'");
        iRight = str.lastIndexOf("\'");
    } else {

        //对路径带空格的脚本，右键执行时不进行拆分处理， //./deepin-terminal "-e"  "/home/lx777/Desktop/a b/PerfTools_1.9.sh"
        QFileInfo fi(str);
        if (fi.isFile()) {
            qDebug() << "this is file,not split.";
            paraList.append(str);
            return paraList;
        }

        paraList.append(str.split(QRegExp(QStringLiteral("\\s+")), QString::SkipEmptyParts));
        return  paraList;
    }

    paraList.append(str.left(iLeft).split(QRegExp(QStringLiteral("\\s+")), QString::SkipEmptyParts));
    paraList.append(str.mid(iLeft + 1, iRight - iLeft - 1));
    if (str.size() != iRight + 1) {
        paraList.append(str.right(str.size() - iRight - 1).split(QRegExp(QStringLiteral("\\s+")), QString::SkipEmptyParts));
    }
    return paraList;
}

/*******************************************************************************
 1. @函数:    encodeList
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-05-27
 4. @说明:    获取编码列表
*******************************************************************************/
QList<QByteArray> Utils::encodeList()
{
    QList<QByteArray> all = QTextCodec::availableCodecs();
    QList<QByteArray> showEncodeList;
    // 这是ubuntu18.04支持的编码格式，按国家排列的
//    showEncodeList << "GB18030" << "GB2312" << "GBK"
//                   << "BIG5" << "BIG5-HKSCS" << "EUC-TW"
//                   << "EUC-JP" << "ISO-2022-JP" << "SHIFT_JIS"
//                   << "EUC-KR" << "ISO-2022-KR" << "UHC"
//                   << "IBM864" << "ISO-8859-6" << "MAC_ARABIC" << "WINDOWS-1256"
//                   << "ARMSCII-8"
//                   << "ISO-8859-13" << "ISO-8859-4" << "WINDOWS-1257"
//                   << "ISO-8859-14"
//                   << "IBM-852" << "ISO-8859-2" << "MAC_CE" << "WINDOWS-1250"
//                   << "MAC_CROATIAN"
//                   << "IBM855" << "ISO-8859-5" << "ISO-IR-111" << "ISO-IR-111" << "KOI8-R" << "MAC-CYRILLIC" << "WINDOWS-1251"
//                   << "CP866"
//                   << "KOI8-U" << "MAC_UKRAINIAN"
//                   << "GEORGIAN-PS"
//                   << "ISO-8859-7" << "MAC_GREEK" << "WINDOWS-1253"
//                   << "MAC_GUJARATI"
//                   << "MAC_GURMUKHI"
//                   << "IBM862" << "ISO-8859-8-I" << "MAC_HEBREW" << "WINDOWS-1255"
//                   << "ISO-8859-8"
//                   << "MAC_DEVANAGARI"
//                   << "MAC_ICELANDIC"
//                   << "ISO-8859-10"
//                   << "MAC_FARSI"
//                   << "ISO-8859-16" << "MAC_ROMANIAN"
//                   << "ISO-8859-3"
//                   << "TIS-620"
//                   << "IBM857" << "ISO-8859-9" << "MAC_TURKISH" << "WINDOWS-1254"
//                   << "TCVN" << "VISCII" << "WINDOWS-1258"
//                   << "IBM850" << "ISO-8859-1" << "ISO-8859-15" << "MAC_ROMAN" << "WINDOWS-1252";
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


    // meld提供的编码格式,按名称排列的
//    showEncodeList<<"UTF-8"
//                 <<"ISO-8859-1"
//                 <<"ISO-8859-2"
//                 <<"ISO-8859-3"
//                 <<"ISO-8859-4"
//                 <<"ISO-8859-5"
//                 <<"ISO-8859-6"
//                 <<"ISO-8859-7"
//                 <<"ISO-8859-8"
//                 <<"ISO-8859-9"
//                 <<"ISO-8859-10"
//                 <<"ISO-8859-13"
//                 <<"ISO-8859-14"
//                 <<"ISO-8859-15"
//                 //<<"ISO-8859-16"
//                 <<"UTF-7"
//                 <<"UTF-16"
//                 <<"UTF-16BE"
//                 <<"UTF-16LE"
//                 <<"UTF-32"
//                 <<"UCS-2"
//                 <<"UCS-4"
//                 //<<"ARMSCII-8"
//                 <<"BIG5"
//                 <<"BIG5-HKSCS"
//                 <<"CP866"
//                 <<"EUC-JP"
//                 //<<"EUC-JP-MS"
//                 <<"CP932"
//                 <<"EUC-KR"
//                 //<<"EUC-TW"
//                 <<"GB18030"
//                 <<"GB2312"
//                 <<"GBK"
//                 //<<"GEORGIAN-ACADEMY"
//                 <<"IBM850"
//                 <<"IBM852"
//                 <<"IBM855"
//                 <<"IBM857"
//                 <<"IBM862"
//                 <<"IBM864"
//                 <<"ISO-2022-JP"
//                 <<"ISO-2022-KR"
//                 //<<"ISO-IR-111"
//                 //<<"JOHAB"
//                 <<"KOI8-R"
//                 <<"KOI8-U"
//                 <<"SHIFT_JIS"
//                 //<<"TCVN"
//                 <<"TIS-620"
//                 //<<"UHC"
//                 //<<"VISCII"
//                 <<"WINDOWS-1250"
//                 <<"WINDOWS-1251"
//                 <<"WINDOWS-1252"
//                 <<"WINDOWS-1253"
//                 <<"WINDOWS-1254"
//                 <<"WINDOWS-1255"
//                 <<"WINDOWS-1256"
//                 <<"WINDOWS-1257"
//                 <<"WINDOWS-1258";


    QList<QByteArray> encodeList;
    // 自定义的名称，系统里不一定大小写完全一样，再同步一下。
    for (QByteArray &name : showEncodeList) {
        QString strname1 = name;
        bool bFind = false;
        QByteArray encodename;
        for (QByteArray &name2 : all) {
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
            encodeList << encodename;
        }
    }
    // 返回需要的值
    return encodeList;
}

/*******************************************************************************
 1. @函数:    set_Object_Name
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-08-13
 4. @说明:    设置类的objectname,统一成一个函数
*******************************************************************************/
void Utils::set_Object_Name(QObject *object)
{
    if (object != nullptr) {
        object->setObjectName(object->metaObject()->className());
    }
}

/*******************************************************************************
 1. @函数:    getMainWindow
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-06-02
 4. @说明:    根据当前窗口获取最外层的主窗口，当前窗口：currWidget，返回值非空就找到最外层主窗口：MainWindow
*******************************************************************************/
MainWindow *Utils::getMainWindow(QWidget *currWidget)
{
    MainWindow *main = nullptr;
    QWidget *pWidget = currWidget->parentWidget();
    while (pWidget != nullptr) {
        qDebug() << pWidget->metaObject()->className();
        if ((pWidget->objectName() == "NormalWindow") || (pWidget->objectName() == "QuakeWindow")) {
            qDebug() << "has find MainWindow";
            main = static_cast<MainWindow *>(pWidget);
            break;
        }
        pWidget = pWidget->parentWidget();
    }
    return  main;
}

/******** Add by ut001000 renfeixiang 2020-06-15:增加 处理等宽字体的类 Begin***************/
Q_GLOBAL_STATIC(FontFilter, FontFilters)
FontFilter *FontFilter::instance()
{
    return FontFilters;
}

FontFilter::FontFilter()
{
    m_thread = new QThread();
    this->moveToThread(m_thread);
    QObject::connect(m_thread, &QThread::started, this, [ = ]() {
        CompareWhiteList();
        m_thread->quit();
    });
}

FontFilter::~FontFilter()
{
    if (m_thread != nullptr) {
        setStop(true);
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }
}

/*******************************************************************************
 1. @函数:    HandleWidthFont
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-08-11
 4. @说明:    启动thread，打印等宽字体函数
*******************************************************************************/
void FontFilter::HandleWidthFont()
{
    if (!m_thread->isRunning()) {
        m_thread->start();
        return;
    }
    qDebug() << "m_thread is Running";
}

/*******************************************************************************
 1. @函数:    setStop
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-08-11
 4. @说明:    设置线程结束标志 true = 结束 false = 正常
*******************************************************************************/
void FontFilter::setStop(bool stop)
{
    m_bstop = stop;
}

/*******************************************************************************
 1. @函数:    CompareWhiteList
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-08-11
 4. @说明:    打印DBUS获取等宽字体和比较字体字符方法获取等宽字体，用来定位DBUS获取字体失败后的问题
*******************************************************************************/
void FontFilter::CompareWhiteList()
{
    QStringList DBUSWhitelist = DBusManager::callAppearanceFont("monospacefont");
    std::sort(DBUSWhitelist.begin(), DBUSWhitelist.end(), [ = ](const QString & str1, const QString & str2) {
        QCollator qc;
        return qc.compare(str1, str2) < 0;
    });

    //在REPCHAR中增加了一个空格，空格在非等宽字体中长度和字符长度不同
    char REPCHAR[]  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "abcdefgjijklmnopqrstuvwxyz"
                      "0123456789 ./+@";
    QFontDatabase fontDatabase;
    QStringList fontLst = fontDatabase.families();
    QStringList Whitelist;
    Whitelist << "Courier 10 Pitch" << "DejaVu Sans Mono" << "Liberation Mono" << "Monospace"
              << "Noto Mono" << "Noto Sans Mono" << "Noto Sans Mono CJK JP" << "Noto Sans Mono CJK JP Bold"
              << "Noto Sans Mono CJK KR" << "Noto Sans Mono CJK KR Bold" << "Noto Sans Mono CJK SC"
              << "Noto Sans Mono CJK SC Bold" << "Noto Sans Mono CJK TC" << "Noto Sans Mono CJK TC Bold" << "Unifont";
    QStringList Blacklist;
    Blacklist << "webdings" << "Symbol" << "MT Extra [unknown]" << "Bitstream Charter" << "CESI仿宋-GB13000" << "CESI仿宋-GB18030"
              << "CESI仿宋-GB2312" << "CESI宋体-GB13000" << "CESI宋体-GB18030" << "CESI宋体-GB2312" << "CESI小标宋-GB13000"
              << "CESI小标宋-GB18030" << "CESI小标宋-GB2312" << "CESI楷体-GB13000" << "CESI楷体-GB18030" << "CESI楷体-GB2312" << "CESI黑体-GB13000"
              << "CESI黑体-GB18030" << "CESI黑体-GB2312" << "DejaVu Math TeX Gyre" << "DejaVu Sans" << "DejaVu Serif" << "Liberation Sans"
              << "Liberation Sans Narrow" << "Liberation Serif" << "Lohit Devanagari" << "MT Extra [PfEd]" << "Noto Kufi Arabic" << "Noto Music"
              << "Noto Naskh Arabic" << "Noto Nastaliq Urdu" << "Noto Sans" << "Noto Sans Adlam" << "Noto Sans Adlam Unjoined"
              << "Noto Sans Anatolian Hieroglyphs" << "Noto Sans Arabic" << "Noto Sans Armenian" << "Noto Sans Avestan" << "Noto Sans Bamum"
              << "Noto Sans Bassa Vah" << "Noto Sans Batak" << "Noto Sans Bengali" << "Noto Sans Bhaiksuki" << "Noto Sans Brahmi"
              << "Noto Sans Buginese" << "Noto Sans Buhid" << "Noto Sans Canadian Aboriginal" << "Noto Sans Carian" << "Noto Sans Caucasian Albanian"
              << "Noto Sans Chakma" << "Noto Sans Cham" << "Noto Sans Cherokee" << "Noto Sans CJK JP" << "Noto Sans CJK JP Bold" << "Noto Sans CJK KR"
              << "Noto Sans CJK KR Bold" << "Noto Sans CJK SC" << "Noto Sans CJK SC Bold" << "Noto Sans CJK TC" << "Noto Sans CJK TC Bold"
              << "Noto Sans Coptic" << "Noto Sans Cuneiform" << "Noto Sans Cypriot" << "Noto Sans Deseret" << "Noto Sans Devanagari" << "Noto Sans Display"
              << "Noto Sans Duployan" << "Noto Sans Egyptian Hieroglyphs" << "Noto Sans Elbasan" << "Noto Sans Ethiopic" << "Noto Sans Georgian"
              << "Noto Sans Glagolitic" << "Noto Sans Gothic" << "Noto Sans Grantha" << "Noto Sans Gujarati" << "Noto Sans Gurmukhi" << "Noto Sans Hanunoo"
              << "Noto Sans Hatran" << "Noto Sans Hebrew" << "Noto Sans Imperial Aramaic" << "Noto Sans Inscriptional Pahlavi" << "Noto Sans Inscriptional Parthian"
              << "Noto Sans Javanese" << "Noto Sans Kaithi" << "Noto Sans Kannada" << "Noto Sans Kayah Li" << "Noto Sans Kharoshthi" << "Noto Sans Khmer"
              << "Noto Sans Khojki" << "Noto Sans Khudawadi" << "Noto Sans Lao" << "Noto Sans Lepcha" << "Noto Sans Limbu" << "Noto Sans Linear A"
              << "Noto Sans Linear B" << "Noto Sans Lisu" << "Noto Sans Lycian" << "Noto Sans Lydian" << "Noto Sans Mahajani" << "Noto Sans Malayalam"
              << "Noto Sans Mandaic" << "Noto Sans Manichaean" << "Noto Sans Marchen" << "Noto Sans Math" << "Noto Sans Meetei Mayek" << "Noto Sans Mende Kikakui"
              << "Noto Sans Meroitic" << "Noto Sans Miao" << "Noto Sans Modi" << "Noto Sans Mongolian" << "Noto Sans Mro" << "Noto Sans Multani" << "Noto Sans Myanmar"
              << "Noto Sans Nabataean" << "Noto Sans New Tai Lue" << "Noto Sans Newa" << "Noto Sans NKo" << "Noto Sans Ogham" << "Noto Sans Ol Chiki"
              << "Noto Sans Old Hungarian" << "Noto Sans Old Italic" << "Noto Sans Old North Arabian" << "Noto Sans Old Permic" << "Noto Sans Old Persian"
              << "Noto Sans Old South Arabian" << "Noto Sans Old Turkic" << "Noto Sans Oriya" << "Noto Sans Osage" << "Noto Sans Osmanya" << "Noto Sans Pahawh Hmong"
              << "Noto Sans Palmyrene" << "Noto Sans Pau Cin Hau" << "Noto Sans PhagsPa" << "Noto Sans Phoenician" << "Noto Sans Psalter Pahlavi" << "Noto Sans Rejang"
              << "Noto Sans Runic" << "Noto Sans Samaritan" << "Noto Sans Saurashtra" << "Noto Sans Sharada" << "Noto Sans Shavian" << "Noto Sans Sinhala"
              << "Noto Sans Sora Sompeng" << "Noto Sans Sundanese" << "Noto Sans Syloti Nagri" << "Noto Sans Symbols" << "Noto Sans Symbols2" << "Noto Sans Syriac"
              << "Noto Sans Syriac Eastern" << "Noto Sans Syriac Estrangela" << "Noto Sans Syriac Western" << "Noto Sans Tagalog" << "Noto Sans Tagbanwa"
              << "Noto Sans Tai Le" << "Noto Sans Tai Tham" << "Noto Sans Tai Viet" << "Noto Sans Takri" << "Noto Sans Tamil" << "Noto Sans Telugu" << "Noto Sans Thaana"
              << "Noto Sans Thai" << "Noto Sans Tibetan" << "Noto Sans Tifinagh" << "Noto Sans Tirhuta" << "Noto Sans Ugaritic" << "Noto Sans Vai"
              << "Noto Sans Warang Citi" << "Noto Sans Yi" << "Noto Serif" << "Noto Serif Ahom" << "Noto Serif Armenian" << "Noto Serif Balinese"
              << "Noto Serif Bengali" << "Noto Serif CJK JP" << "Noto Serif CJK KR" << "Noto Serif CJK SC" << "Noto Serif CJK TC" << "Noto Serif Devanagari"
              << "Noto Serif Display" << "Noto Serif Ethiopic" << "Noto Serif Georgian" << "Noto Serif Gujarati" << "Noto Serif Gurmukhi" << "Noto Serif Hebrew"
              << "Noto Serif Kannada" << "Noto Serif Khmer" << "Noto Serif Lao" << "Noto Serif Malayalam" << "Noto Serif Myanmar" << "Noto Serif Sinhala"
              << "Noto Serif Tamil" << "Noto Serif Tamil Slanted" << "Noto Serif Telugu" << "Noto Serif Thai" << "Noto Serif Tibetan" << "Sans Serif" << "Serif"
              << "Symbola" << "Unifont CSUR" << "Unifont Upper" << "Wingdings" << "Wingdings 2" << "Wingdings 3";

    for (QString &sfont : fontLst) {
        if (m_bstop) {
            break;
        }
        if (Whitelist.contains(sfont) | Blacklist.contains(sfont)) {
            continue;
        }
        bool fixedFont = true;
        QFont font(sfont);
        QFontMetrics fm(font);
        int fw = fm.width(REPCHAR[0]);
        //qDebug() << "sfont" << sfont;

        for (unsigned int i = 1; i < qstrlen(REPCHAR); i++) {
            if (fw != fm.width(QLatin1Char(REPCHAR[i]))) {
                fixedFont = false;
                break;
            }
        }
        if (fixedFont) {
            Whitelist.append(sfont);
        } else {
            Blacklist.append(sfont);
        }
    }
    qDebug() << "DBUS get font:" << DBUSWhitelist;
    qDebug() << "Compare font get font:" << Whitelist;
}
/******** Add by ut001000 renfeixiang 2020-06-15:增加 处理等宽字体的类 End***************/
