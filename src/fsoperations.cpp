#include "fsoperations.h"
#include <QAbstractListModel>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

FSOperations::FSOperations(QObject *parent) : QObject(parent)
{
}

bool FSOperations::deleteFile(const QString &path)
{
    QFile f(path);
    return f.remove();
}

QString FSOperations::writableLocation(const QString &type, const QString &baseDir)
{
    QString dir;
    if (type == QLatin1String("image")) {
        dir = baseDir + QStringLiteral("/Pictures/Shutter");
    } else if (type == QStringLiteral("video")) {
        dir = baseDir + QStringLiteral("/Videos/Shutter");
    } else {
        return QString();
    }

    if (!createFolder(dir)) {
        qWarning() << "Unable to create" << dir << ", fallback to home dir!";
        Q_EMIT rescan(QLatin1String("/media/sdcard"));
        return writableLocation(type, QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    }
    return dir;
}

bool FSOperations::createFolder(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return dir.mkpath(QLatin1String("."));
    }
    return true;
}

qint64 FSOperations::getFileSize(const QString &path)
{
    QFileInfo info(path);
    if (info.exists())
        return info.size();
    return 0;
}

QString FSOperations::getFileSizeHuman(const QString &path)
{
    float num = (float)getFileSize(path);
    QStringList list;
    list << QLatin1String("KiB") << QLatin1String("MiB") << QLatin1String("GiB") << QLatin1String("TiB");

    QStringListIterator i(list);
    QLatin1String unit("B");

    if (num < 1024.0)
        return QString().setNum(num, 'f', 0) + QStringLiteral(" ") + unit;

    while(num >= 1024.0 && i.hasNext())
     {
        unit = QLatin1String(i.next().toLatin1());
        num /= 1024.0;
    }
    return QString().setNum(num, 'f', 2) + QStringLiteral(" ") + unit;
}
