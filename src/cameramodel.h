#ifndef CAMERAMODEL_H
#define CAMERAMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <libcamera/camera_manager.h>

class CameraModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    enum CameraRoles {
        CameraName = Qt::UserRole + 1
    };

    explicit CameraModel(QObject *parent, std::shared_ptr<libcamera::CameraManager> cameraManager);
    virtual QHash<int, QByteArray> roleNames() const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    Q_INVOKABLE virtual QVariant get(int idx);

private:
    std::shared_ptr<libcamera::CameraManager> m_cameraManager;
    QStringList m_cameras;

Q_SIGNALS:
    void rowCountChanged();
};

#endif // CAMERAMODEL_H
