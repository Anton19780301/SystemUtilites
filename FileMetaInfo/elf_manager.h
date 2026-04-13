#ifndef ELF_MANAGER_H
#define ELF_MANAGER_H

#include <QObject>
#include <QVariantList>
#include <QStringList>
#include <QQuickTextDocument>
#include "elf_types.h"
#include <QtQml>

struct ElfSection {
    QString name;
    int index;
    bool isCustom; // Для защиты от удаления системных секций
};

class ElfManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QStringList sectionNames READ sectionNames NOTIFY sectionsChanged)
    Q_PROPERTY(QVariantList sectionDetails READ sectionDetails NOTIFY sectionsChanged)
    Q_PROPERTY(qint64 fileSize READ fileSize NOTIFY sectionsChanged)
    Q_PROPERTY(QString entryPoint READ entryPoint NOTIFY sectionsChanged)
    Q_PROPERTY(QString machineType READ machineType NOTIFY sectionsChanged)

public:
    explicit ElfManager(QObject *parent = nullptr);

    // Вызывается из QML при нажатии "Открыть"
    Q_INVOKABLE bool loadFile(const QString &filePath, bool silent = false);

    // Вызывается при клике на элемент дерева
    Q_INVOKABLE QString getSectionContent(const QString &name);

    // Сохранение отредактированного текста
    Q_INVOKABLE bool saveSection(const QString &name, const QString &content);

    //добавление секции
    Q_INVOKABLE bool addSection(const QString &sectionName, const QString &content);

    Q_INVOKABLE bool removeSection(const QString &name);

    //проверка можно ли удалять секцию
    Q_INVOKABLE bool isDeletable(const QString &name) const;

    //добавление подсветки JSON
    Q_INVOKABLE void setupHighlighter(QQuickItem *textAreaItem);

    //удалить информацию о сборке
    Q_INVOKABLE bool stripAll();

    //Hex-просмотрщик
    Q_INVOKABLE QString getSectionHex(const QString &name);

    //вычисление общего обьема пустот
    Q_INVOKABLE qint64 calculateGaps();

    //репак файла (удалим гепы) TODO: не работает , приводит к ошибке сегментации файла
    Q_INVOKABLE bool repackElf();


    QString entryPoint() const { return m_entryPoint; }
    QString machineType() const { return m_machineType; }

    QStringList sectionNames() const { return m_sections; }
    QVariantList sectionDetails() const { return m_sectionDetails; }
    qint64 fileSize() const { return m_fileSize; }

signals:
    void sectionsChanged();
    void errorOccurred(QString message);
    void messageReady(QString text, QString type = "info");

private:
    QString m_currentFilePath;
    QStringList m_sections;
    QByteArray getRawSectionData(const QString &name);
    QVariantList m_sectionDetails;
    qint64 m_fileSize = 0;
    QString m_entryPoint;
    QString m_machineType;

    //создание резервной копии
    void makeBackup(const QString &filePath);
    QString getSectionDisasm(const QString &name);

};

#endif // ELF_MANAGER_H
