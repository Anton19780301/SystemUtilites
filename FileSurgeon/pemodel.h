#ifndef PEMODEL_H
#define PEMODEL_H

#include <QAbstractItemModel>
#include <QHash>
#include <QByteArray>
#include <vector>

struct PENode {
    QString name;
    QString type; // "Header", "Section", etc.
    QByteArray data;
    std::vector<PENode*> children;
    PENode* parent = nullptr;

    ~PENode() { qDeleteAll(children); }
};

class PEModel : public QAbstractItemModel {
    Q_OBJECT
    Q_PROPERTY(QString currentContent READ currentContent WRITE setCurrentContent NOTIFY contentChanged)

public:
    explicit PEModel(QObject *parent = nullptr);
    ~PEModel();

    // Обязательные методы для QML TreeView
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override { return 1; }
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;

    // Логика для работы с данными
    void clear();
    void addRootNode(const QString &name, const QString &type, const QByteArray &content = "");
    bool removeRows(int row, int count, const QModelIndex &parent) override;

    QString currentContent() const { return m_currentContent; }
    void setCurrentContent(const QString &content) {
        m_currentContent = content;
        emit contentChanged();
    }


signals:
    void contentChanged();

private:
    PENode *m_root;
    QString m_currentContent;
};

#endif
