/**
harbour-advanced-camera C++ Camera Models
Copyright (C) 2019 Adam Pigg (adam@piggz.co.uk)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**/
#ifndef FLASHMODEL_H
#define FLASHMODEL_H

#include <QAbstractListModel>
#include <QCamera>

#if QT_VERSION > 0x060000
    typedef QCamera QCameraExposure;
    #define FlashManual FlashAuto
#endif

class FlashModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:

    enum FlashRoles {
        FlashName = Qt::UserRole + 1,
        FlashValue
    };

    FlashModel();

    virtual QHash<int, QByteArray> roleNames() const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void setCamera(QObject *camera);

private:
    std::vector<std::pair<int, QString>> m_flashModes;

    QCamera *m_camera = nullptr;

    QString flashName(QCameraExposure::FlashMode flash) const;

Q_SIGNALS:
    void rowCountChanged();
};

#endif // FOCUSMODEL_H
