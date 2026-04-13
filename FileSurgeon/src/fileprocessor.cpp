#include "fileprocessor.h"
#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <windows.h>

// Помощник для выравнивания
uint32_t alignVal(uint32_t size, uint32_t alignment) {
    if (alignment == 0) return size;
    return ((size + alignment - 1) / alignment) * alignment;
}

// 1. ЧТЕНИЕ ФАЙЛА И ПАРСИНГ
void FileProcessor::parsePE(const QByteArray &data) {
    m_model->clear();
    if (data.size() < sizeof(IMAGE_DOS_HEADER)) return;

    const char* base = data.data();
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)base;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return;

    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(base + dosHeader->e_lfanew);
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);

    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
        // Имя: берем ровно 8 байт и отсекаем всё после первого \0
        QByteArray nameBytes((const char*)section[i].Name, 8);
        QString sName = QString::fromLatin1(nameBytes.data()).split(QChar('\0'))[0];

        // Данные: берем из файла и СРАЗУ чистим от "квадратиков" (\0)
        QByteArray sData = data.mid(section[i].PointerToRawData, section[i].SizeOfRawData);
        int firstZero = sData.indexOf('\0');
        if (firstZero != -1) sData = sData.left(firstZero);

        m_model->addRootNode(sName, "section", sData);
    }
}

// 2. ВЫБОР СЕКЦИИ В ДЕРЕВЕ
void FileProcessor::selectNode(const QModelIndex &index) {
    if (!index.isValid()) return;
    m_currentIndex = index;

    PENode *node = static_cast<PENode*>(index.internalPointer());
    if (node) {
        // Передаем в текстовое поле чистые данные
        m_model->setCurrentContent(QString::fromUtf8(node->data));
    }
}

// 3. УДАЛЕНИЕ ВЫБРАННОЙ СЕКЦИИ (Безопасное)
void FileProcessor::removeCurrentSection() {
    if (!m_currentIndex.isValid() || m_currentFilePath.isEmpty()) return;

    PENode *node = static_cast<PENode*>(m_currentIndex.internalPointer());
    QString targetName = node->name;

    // Защита системных секций
    if (targetName.startsWith(".") && targetName.length() <= 5) {
        if (QStringList({".text", ".data", ".rsrc", ".reloc", ".rdata"}).contains(targetName.toLower())) {
            qDebug() << "Отмена: системная секция!"; return;
        }
    }

    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::ReadWrite)) return;
    QByteArray data = file.readAll();

    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)data.data();
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(data.data() + dos->e_lfanew);
    PIMAGE_SECTION_HEADER sects = IMAGE_FIRST_SECTION(nt);

    int foundIdx = -1;
    for (int i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
        if (QString::fromLatin1((char*)sects[i].Name).startsWith(targetName)) {
            foundIdx = i; break;
        }
    }

    if (foundIdx != -1) {
        // Сдвигаем таблицу заголовков
        int toMove = nt->FileHeader.NumberOfSections - foundIdx - 1;
        if (toMove > 0) {
            memcpy(&sects[foundIdx], &sects[foundIdx + 1], toMove * sizeof(IMAGE_SECTION_HEADER));
        }
        nt->FileHeader.NumberOfSections--;

        // Пишем обновленный заголовок обратно в файл
        file.seek(dos->e_lfanew);
        file.write((char*)nt, sizeof(IMAGE_NT_HEADERS));
        file.seek(dos->e_lfanew + sizeof(IMAGE_NT_HEADERS));
        file.write((char*)sects, nt->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));
    }
    file.close();

    // Обновляем интерфейс
    m_currentIndex = QModelIndex();
    m_model->setCurrentContent("");
    openFile(m_currentFilePath); // Перечитываем файл
}

// 4. СОХРАНЕНИЕ ИЗМЕНЕНИЙ / ИНЪЕКЦИЯ
void FileProcessor::saveChanges(const QString &content) {

    if (m_currentFilePath.isEmpty() || !m_currentIndex.isValid()) return;

    // --- БЛОК СОЗДАНИЯ БЭКАПА ---
    QString backupPath = m_currentFilePath + ".bak";

    // Если бэкап уже есть, удаляем старый, чтобы создать свежий
    if (QFile::exists(backupPath)) {
        QFile::remove(backupPath);
    }

    // Копируем оригинальный файл
    if (QFile::copy(m_currentFilePath, backupPath)) {
        qDebug() << "Бэкап создан:" << backupPath;
    } else {
        qDebug() << "Предупреждение: Не удалось создать бэкап!";
    }

    if (m_currentFilePath.isEmpty() || !m_currentIndex.isValid()) return;

    PENode *node = static_cast<PENode*>(m_currentIndex.internalPointer());
    QByteArray newContent = content.toUtf8();

    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::ReadWrite)) return;

    QByteArray data = file.readAll();
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)data.data();
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(data.data() + dos->e_lfanew);
    PIMAGE_SECTION_HEADER sects = IMAGE_FIRST_SECTION(nt);

    bool found = false;
    for (int i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
        if (node->name == QString::fromLatin1((char*)sects[i].Name, 8).split(QChar('\0'))[0]) {
            uint32_t sz = qMin((uint32_t)newContent.size(), sects[i].SizeOfRawData);
            file.seek(sects[i].PointerToRawData);
            file.write(newContent.data(), sz);
            found = true; break;
        }
    }

    if (!found) {
        // Добавление новой
        uint32_t headerOff = dos->e_lfanew + sizeof(IMAGE_NT_HEADERS) + (nt->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));
        PIMAGE_SECTION_HEADER last = &sects[nt->FileHeader.NumberOfSections - 1];

        IMAGE_SECTION_HEADER ns;
        ZeroMemory(&ns, sizeof(ns));
        strncpy((char*)ns.Name, node->name.toLatin1().data(), 8);

        uint32_t fA = nt->OptionalHeader.FileAlignment;
        uint32_t sA = nt->OptionalHeader.SectionAlignment;

        ns.SizeOfRawData = alignVal(newContent.size(), fA);
        ns.PointerToRawData = alignVal(last->PointerToRawData + last->SizeOfRawData, fA);
        ns.Misc.VirtualSize = newContent.size();
        ns.VirtualAddress = alignVal(last->VirtualAddress + alignVal(last->Misc.VirtualSize, sA), sA);
        ns.Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;

        nt->FileHeader.NumberOfSections++;
        nt->OptionalHeader.SizeOfImage = ns.VirtualAddress + alignVal(ns.Misc.VirtualSize, sA);

        file.seek(dos->e_lfanew); file.write((char*)nt, sizeof(IMAGE_NT_HEADERS));
        file.seek(headerOff); file.write((char*)&ns, sizeof(ns));
        file.seek(ns.PointerToRawData); file.write(newContent);

        int pad = ns.SizeOfRawData - newContent.size();
        if (pad > 0) file.write(QByteArray(pad, 0));
    }
    file.close();
    node->data = newContent;
    emit statusMessage("Файл сохранен. Бэкап создан.");
}

FileProcessor::FileProcessor(PEModel *model, QObject *parent)
    : QObject(parent), m_model(model) {}

bool FileProcessor::openFile(const QString &filePath) {
    QString cleanPath = filePath;
    if (cleanPath.startsWith("file:///")) cleanPath.remove(0, 8);

    QFile file(cleanPath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    m_currentFilePath = cleanPath;
    QByteArray data = file.readAll();
    file.close();

    parsePE(data);
    emit statusMessage("Файл успешно открыт: " + file.fileName());
    return true;
}

void FileProcessor::addNewSection(const QString &name) {
    if (name.isEmpty()) return;
    // Добавляем в дерево пустую заготовку под JSON
    m_model->addRootNode(name, "section", QByteArray("{\n  \"info\": \"new\"\n}"));
    qDebug() << "Секция добавлена в дерево:" << name;
}

void FileProcessor::removeSection(const QModelIndex &index) {
    if (!index.isValid()) return;

    PENode *node = static_cast<PENode*>(index.internalPointer());
    QString name = node->name.toLower();

    // Защита от дурака: список системных секций
    QStringList systemSections = {".text", ".data", ".rdata", ".idata", ".pdata", ".rsrc", ".reloc", ".bss"};
    if (systemSections.contains(name)) {
        qDebug() << "Нельзя удалять системную секцию:" << name;
        return;
    }

    // 1. Удаляем из визуального дерева
    m_model->removeRow(index.row(), index.parent());

    // 2. Физическое удаление (через перезапись заголовка)
    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::ReadWrite)) return;

    QByteArray fileData = file.readAll();
    char* base = fileData.data();

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)base;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(base + dosHeader->e_lfanew);
    PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(ntHeaders);

    int targetIdx = -1;
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
        if (QString::fromLatin1((char*)sections[i].Name).startsWith(name)) {
            targetIdx = i;
            break;
        }
    }

    if (targetIdx != -1) {
        // Сдвигаем таблицу секций назад, затирая удаляемую
        int sectionsToMove = ntHeaders->FileHeader.NumberOfSections - targetIdx - 1;
        if (sectionsToMove > 0) {
            memcpy(&sections[targetIdx], &sections[targetIdx + 1], sectionsToMove * sizeof(IMAGE_SECTION_HEADER));
        }

        // Обнуляем место последней секции
        ZeroMemory(&sections[ntHeaders->FileHeader.NumberOfSections - 1], sizeof(IMAGE_SECTION_HEADER));

        // Уменьшаем счетчик
        ntHeaders->FileHeader.NumberOfSections--;

        // Записываем обновленные заголовки
        file.seek(dosHeader->e_lfanew);
        file.write((char*)ntHeaders, sizeof(IMAGE_NT_HEADERS));

        // Перезаписываем таблицу секций
        file.seek(dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS));
        file.write((char*)sections, ntHeaders->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));

        qDebug() << "Секция" << name << "удалена. Перезапустите файл для обновления дерева.";
    }
    qDebug() << "Index valid:" << index.isValid();

    file.close();
}


