#include "settings.h"
#include <QDebug>

Settings::Settings(QObject *parent) : QObject(parent)
{

}

QVariant Settings::get(const QString &group, const QString &setting, QVariant def)
{
    QVariant v = m_settings.value(group + QStringLiteral("/") + setting, def);
    if (v.toString().toLower() == QStringLiteral("false") || v.toString().toLower() == QStringLiteral("true")) {
        return v.toBool();
    }
    return v;
}

void Settings::set(const QString &group, const QString &setting, QVariant value)
{
    m_settings.setValue(group + QStringLiteral("/") + setting, value);
    m_settings.sync();
}
