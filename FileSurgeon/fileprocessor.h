#ifndef FILE_PROCESSOR_H
#define FILE_PROCESSOR_H

#include <QObject>
#include <QString>
#include <QFile>
#include <windows.h>
#include "pemodel.h"

class FileProcessor : public QObject {
    Q_OBJECT
public:
    explicit FileProcessor(PEModel *model, QObject *parent = nullptr);

    // Эти методы вызываем из кнопок в QML
    Q_INVOKABLE bool openFile(const QString &filePath);
    Q_INVOKABLE void saveChanges(const QString &content);
    Q_INVOKABLE void selectNode(const QModelIndex &index);
    Q_INVOKABLE void addNewSection(const QString &name);
    Q_INVOKABLE void removeSection(const QModelIndex &index);
    Q_INVOKABLE void removeCurrentSection();

private:
    PEModel *m_model;
    QString m_currentFilePath;
    QModelIndex m_currentIndex; // <--- Добавь это
    // Вспомогательный метод для парсинга PE
    void parsePE(const QByteArray &data);
    void physicallyAddSection(const QString &name, const QByteArray &content);
};

#endif
