#ifndef SETTINGIO_H
#define SETTINGIO_H

#include <QObject>
#include <QSettings>
#include <QIODevice>

class SettingIO : public QObject
{
    Q_OBJECT
public:
    explicit SettingIO(QObject *parent = nullptr);
    static bool readIniFunc(QIODevice &device,  QSettings::SettingsMap &settingsMap);
    static bool writeIniFunc(QIODevice &device, const QSettings::SettingsMap &settingsMap);
private:
    static QString variantToString(const QVariant &v);
    static QVariant stringToVariant(const QString &s);
    static QByteArray escapedString(const QString &src);
    static QString unescapedString(const QString &src);
    static QString canTransfer(const QString &str);
    static bool iniUnescapedKey(const QByteArray &key, int from, int to, QString &result);
public:
    static bool rewrite;
};

#endif // SETTINGIO_H
