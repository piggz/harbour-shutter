#ifndef STORAGEMODEL_H
#define STORAGEMODEL_H

#include <QAbstractListModel>
#include <QObject>

class Storage
{
public:
    explicit Storage(const QString &name, const QString &path);
    QString name() const { return m_name; }
    QString path() const { return m_path; }
private:
    QString m_name, m_path;
};

class StorageModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    enum StorageRoles
    {
        StorageName = Qt::UserRole + 1,
        StoragePath
    };

    explicit StorageModel(QObject *parent = nullptr);
    virtual QHash<int, QByteArray> roleNames() const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    Q_INVOKABLE QVariant getName(int index) const { return m_storage.at(index).name(); }
    Q_INVOKABLE QVariant getPath(int index) const { return m_storage.at(index).path(); }
public Q_SLOTS:
    void scan();
private:
    QList<Storage> m_storage;

Q_SIGNALS:
    void rowCountChanged();

};

#endif // STORAGEMODEL_H
