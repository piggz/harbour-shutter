#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = nullptr);
    Q_INVOKABLE QVariant get(const QString &group, const QString &setting, QVariant def);
    Q_INVOKABLE void     set(const QString &group, const QString &setting, QVariant value);

private:
    QSettings m_settings;
};

#endif // SETTINGS_H
