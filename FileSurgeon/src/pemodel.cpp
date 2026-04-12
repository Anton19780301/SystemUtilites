#include "pemodel.h"

PEModel::PEModel(QObject *parent) : QAbstractItemModel(parent) {
    m_root = new PENode{"Root", "Root"};
}

PEModel::~PEModel() { delete m_root; }

QModelIndex PEModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) return QModelIndex();
    PENode *parentItem = parent.isValid() ? static_cast<PENode*>(parent.internalPointer()) : m_root;
    return createIndex(row, column, parentItem->children[row]);
}

QModelIndex PEModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) return QModelIndex();
    PENode *childItem = static_cast<PENode*>(child.internalPointer());
    PENode *parentItem = childItem->parent;
    if (parentItem == m_root) return QModelIndex();

    // Находим строку родителя относительно его собственного родителя
    int row = 0;
    for(size_t i=0; i < parentItem->parent->children.size(); ++i) {
        if(parentItem->parent->children[i] == parentItem) row = i;
    }
    return createIndex(row, 0, parentItem);
}

int PEModel::rowCount(const QModelIndex &parent) const {
    PENode *parentItem = parent.isValid() ? static_cast<PENode*>(parent.internalPointer()) : m_root;
    return parentItem->children.size();
}

QVariant PEModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    PENode *item = static_cast<PENode*>(index.internalPointer());
    if (role == Qt::DisplayRole) return item->name;
    return QVariant();
}

QHash<int, QByteArray> PEModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    // Можно добавить свои роли, например для типа узла (секция/заголовок)
    // roles[Qt::UserRole + 1] = "nodeType";
    return roles;
}

void PEModel::addRootNode(const QString &name, const QString &type, const QByteArray &content) {
    beginInsertRows(QModelIndex(), m_root->children.size(), m_root->children.size());
    PENode* node = new PENode{name, type, content, {}, m_root};
    m_root->children.push_back(node);
    endInsertRows();
}

void PEModel::clear() {
    beginResetModel();
    // Удаляем детей m_root (но не сам m_root)
    for(auto child : m_root->children) delete child;
    m_root->children.clear();
    endResetModel();
}

bool PEModel::removeRows(int row, int count, const QModelIndex &parent) {
    PENode *parentItem = parent.isValid() ? static_cast<PENode*>(parent.internalPointer()) : m_root;
    beginRemoveRows(parent, row, row + count - 1);
    delete parentItem->children[row];
    parentItem->children.erase(parentItem->children.begin() + row);
    endRemoveRows();
    return true;
}
