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
#include "formatmodel.h"

FormatModel::FormatModel(QObject *parent)
    : QAbstractListModel{parent}
{
}

FormatModel::~FormatModel()
{
    m_cameraProxy.reset();
}

QHash<int, QByteArray> FormatModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FormatName] = "name";
    roles[FormatValue] = "value";
    return roles;
}

int FormatModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_formats.size();
}

QVariant FormatModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if (!index.isValid() || index.row() > rowCount(index) || index.row() < 0) {
        return v;
    }

    if (role == FormatName || role == FormatValue) {
        v = m_formats.at(index.row());
    }

    return v;
}

void FormatModel::setCameraProxy(std::shared_ptr<CameraProxy> cameraproxy)
{
    qDebug() << Q_FUNC_INFO;

    m_cameraProxy = cameraproxy;

    connect(m_cameraProxy.get(), &CameraProxy::cameraChanged, this, &FormatModel::populateFormats);
}

QString FormatModel::defaultFormat() const
{
    if (m_formats.contains("MJPEG")) {
        return "MJPEG";
    } else {
        if (m_formats.size() > 0) {
            return m_formats[0];
        }
    }
    return QString();
}

void FormatModel::populateFormats()
{
    qDebug() << Q_FUNC_INFO;
    beginResetModel();
    m_formats.clear();

    if (m_cameraProxy) {
        m_formats = m_cameraProxy->supportedFormats();
    }
    endResetModel();
    Q_EMIT rowCountChanged();

    if (m_formats.size() == 0) {
        qDebug() << "No formats found";
    }
}

