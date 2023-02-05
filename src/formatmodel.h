#ifndef FORMATMODEL_H
#define FORMATMODEL_H

#include <QAbstractListModel>
#include "cameraproxy.h"

class FormatModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit FormatModel(QObject *parent = nullptr);
    ~FormatModel();

    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

    enum FormatRoles {
        FormatName = Qt::UserRole + 1,
        FormatValue
    };

    virtual QHash<int, QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    void setCameraProxy(std::shared_ptr<CameraProxy> cameraProxy);
    Q_INVOKABLE QString defaultFormat() const;

private:
    QStringList m_formats;
    std::shared_ptr<CameraProxy> m_cameraProxy;
    void populateFormats();

Q_SIGNALS:
    void rowCountChanged();
};

#endif // FORMATMODEL_H
