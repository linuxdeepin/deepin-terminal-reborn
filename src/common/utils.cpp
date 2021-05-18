/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangpeili <wangpeili@uniontech.com>
 *
 * Maintainer: wangpeili <wangpeili@uniontech.com>
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
#include "termwidget.h"
#include "dbusmanager.h"

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

QHash<QString, QPixmap> Utils::m_imgCacheHash;
QHash<QString, QString> Utils::m_fontNameCache;

Utils::Utils(QObject *parent) : QObject(parent)
{
}

Utils::~Utils()
{
}

QString Utils::getQssContent(const QString &filePath)
{
    QFile file(filePath);
    QString qss;

    if (file.open(QIODevice::ReadOnly)) {
        qss = file.readAll();
    }

    return qss;
}

QString Utils::getConfigPath()
{
    static QStringList list = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    const QString path = list.value(0);
    QDir dir(
        QDir(path).filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}

QString Utils::suffixList()
{
    return QString("Font Files (*.ttf *.ttc *.otf)");
}

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
    }
    return QStringList();
}

bool Utils::showExitConfirmDialog(CloseType type, int count)
{
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
}

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

bool Utils::showExitUninstallConfirmDialog()
{
    DDialog dlg(QObject::tr("Programs are still running in terminal"), QObject::tr("Are you sure you want to uninstall it?"));
    dlg.setIcon(QIcon::fromTheme("deepin-terminal"));
    dlg.addButton(QString(tr("Cancel")), false, DDialog::ButtonNormal);
    dlg.addButton(QString(tr("OK")), true, DDialog::ButtonWarning);
    return (dlg.exec() == DDialog::Accepted);
}

//单词可能拼错了showUninstallConfirmDialog
bool Utils::showUnistallConfirmDialog(QString commandname)
{
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
}

bool Utils::showShortcutConflictMsgbox(QString txt)
{

    DDialog dlg;
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.setTitle(QString(txt + QObject::tr("please set another one.")));
    /***mod by ut001121 zhangmeng 20200521 将确认按钮设置为默认按钮 修复BUG26960***/
    dlg.addButton(QString(tr("OK")), true, DDialog::ButtonNormal);
    dlg.exec();
    return  true;
}

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

QList<QByteArray> Utils::encodeList()
{
    QList<QByteArray> all = QTextCodec::availableCodecs();
    QList<QByteArray> showEncodeList;
    // 这是ubuntu18.04支持的编码格式，按国家排列的
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

void Utils::set_Object_Name(QObject *object)
{
    if (object != nullptr) {
        object->setObjectName(object->metaObject()->className());
    }
}

QString Utils::converUpToDown(QKeySequence keysequence)
{
    // 获取现在的快捷键字符串
    QString strKey = keysequence.toString();

    // 是否有shift修饰
    if (!(strKey.contains("Shift") || strKey.contains("shift"))) {
        // 没有直接返回字符串
        return strKey;
    }

    // 遍历是否存在有shift修饰的字符串
    for (int i = 0; i < SHORTCUT_CONVERSION_UP.count(); ++i) {
        QString key = SHORTCUT_CONVERSION_UP[i];
        if (strKey.endsWith(key)) {
            // 若存在则替换字符
            strKey.replace(strKey.length() - 1, 1, SHORTCUT_CONVERSION_DOWN[i]);
        }
    }

    return strKey;
}

QString Utils::converDownToUp(QKeySequence keysequence)
{
    // 获取现在的快捷键字符串
    QString strKey = keysequence.toString();
    // 是否有shift修饰
    if (!(strKey.contains("Shift") || strKey.contains("shift"))) {
        // 没有直接返回字符串
        return strKey;
    }

    // 遍历是否存在有shift修饰的字符串
    for (int i = 0; i < SHORTCUT_CONVERSION_DOWN.count(); ++i) {
        QString key = SHORTCUT_CONVERSION_DOWN[i];
        if (strKey.contains(key)) {
            // 若存在则替换字符
            strKey.replace(key, SHORTCUT_CONVERSION_UP[i]);
        }
    }

    return strKey;
}

QString Utils::getCurrentEnvLanguage()
{
    return QString::fromLocal8Bit(qgetenv("LANGUAGE"));
}

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

void FontFilter::HandleWidthFont()
{
    if (!m_thread->isRunning()) {
        m_thread->start();
        return;
    }
    qDebug() << "m_thread is Running";
}

void FontFilter::setStop(bool stop)
{
    m_bstop = stop;
}

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

    for (const QString &sfont : fontLst) {
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
