#include "controlmodel.h"

ControlModel::ControlModel(QObject *parent)
    : QAbstractListModel{parent}
{

}

QHash<int, QByteArray> ControlModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ControlName] = "name";
    roles[ControlCode] = "code";
    roles[ControlType] = "type";
    roles[ControlMinimumValue] = "min";
    roles[ControlMaximumValue] = "max";
    roles[ControlDefaultValue] = "def";

    return roles;
}

int ControlModel::rowCount(const QModelIndex &parent) const
{
    return m_controls.size();
}

QVariant ControlModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if (!index.isValid() || index.row() > rowCount(index) || index.row() < 0) {
        return v;
    }

    auto it = m_controls.begin();
    std::advance(it, index.row());

    if (role == ControlName) {
        v = QString::fromStdString(it->first->name());
    } else if (role == ControlCode) {
        v = it->first->id();
    } else if (role == ControlType) {
        v = (CameraProxy::ControlType)(it->first->type());
    } else if (role == ControlMinimumValue) {
        v = controlValue(it->second.min());
    } else if (role == ControlMaximumValue) {
        v = controlValue(it->second.max());
    } else if (role == ControlDefaultValue) {
        v = controlValue(it->second.def());
    }

    return v;
}

void ControlModel::setCameraProxy(std::shared_ptr<CameraProxy> cameraProxy)
{
    qDebug() << Q_FUNC_INFO;

    m_cameraProxy = cameraProxy;

    connect(m_cameraProxy.get(), &CameraProxy::cameraChanged, this, &ControlModel::cameraChanged);
}

void ControlModel::cameraChanged()
{
    qDebug() << Q_FUNC_INFO;
    beginResetModel();
    m_controls = m_cameraProxy->supportedControls();
    endResetModel();
}

QVariant ControlModel::controlValue(libcamera::ControlValue val) const
{
    switch(val.type()) {
    case libcamera::ControlTypeNone:
        return QVariant();
    case libcamera::ControlTypeBool:
        return QVariant::fromValue(val.get<bool>());
    case libcamera::ControlTypeByte:
        return QVariant::fromValue(val.get<uint8_t>());
    case libcamera::ControlTypeInteger32:
        return QVariant::fromValue(val.get<int32_t>());
    case libcamera::ControlTypeInteger64:
        return QVariant::fromValue(val.get<int64_t>());
    case libcamera::ControlTypeFloat:
        return QVariant::fromValue(val.get<float>());
    case libcamera::ControlTypeString:
        return QVariant::fromValue(QString::fromStdString(val.get<std::string>()));
    case libcamera::ControlTypeRectangle:
        return QVariant();
    case libcamera::ControlTypeSize:
        return QVariant();
    default:
        return QString("unknown");
    }
}
