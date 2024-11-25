#include "settings.h"
#include <QDebug>

Settings::Settings(QObject *parent) : QObject(parent)
{

}

QVariant Settings::get(const QString &group, const QString &setting, QVariant def)
{
    return m_settings.value(group + QStringLiteral("/") + setting, def);
}

void Settings::set(const QString &group, const QString &setting, QVariant value)
{
    m_settings.setValue(group + QStringLiteral("/") + setting, value);
    m_settings.sync();
}
