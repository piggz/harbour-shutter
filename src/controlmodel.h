#ifndef CONTROLMODEL_H
#define CONTROLMODEL_H

#include "libcamera/camera_manager.h"
#include "libcamera/controls.h"
#include "src/cameraproxy.h"
#include <QAbstractListModel>

class ControlModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ControlModel(QObject *parent = nullptr);

    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

    enum ControlRoles {
        ControlName = Qt::UserRole + 1,
        ControlCode,
        ControlType,
        ControlMinimumValue,
        ControlMaximumValue,
        ControlDefaultValue
    };

    virtual QHash<int, QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    void setCameraProxy(std::shared_ptr<CameraProxy> cameraProxy);

private:
    std::shared_ptr<libcamera::CameraManager> m_cameraManager;
    libcamera::ControlInfoMap m_controls;
    std::shared_ptr<CameraProxy> m_cameraProxy;

    void cameraChanged();
    QVariant controlValue(libcamera::ControlValue val) const;

Q_SIGNALS:
    void rowCountChanged();
};

#endif // CONTROLMODEL_H
