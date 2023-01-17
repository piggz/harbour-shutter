#include "cameramodel.h"
#include <QDebug>
#include <libcamera/camera.h>

CameraModel::CameraModel(QObject *parent, std::shared_ptr<libcamera::CameraManager> cameraManager)
    : QAbstractListModel{parent}, m_cameraManager{cameraManager}
{
    beginResetModel();
    for (const auto &cam : m_cameraManager->cameras()) {
            m_cameras << QString::fromStdString(cam->id());
            qDebug() << "Camera: " << QString::fromStdString(cam->id());
    }
    endResetModel();
}

QHash<int, QByteArray> CameraModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[CameraName] = "name";
    return roles;
}

int CameraModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_cameras.size();
}

QVariant CameraModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if (!index.isValid() || index.row() > rowCount(index) || index.row() < 0) {
        return v;
    }

    if (role == CameraName) {
        v = m_cameras.at(index.row());
    }

    return v;
}

QVariant CameraModel::get(int idx)
{
    return m_cameras.at(idx);
}
