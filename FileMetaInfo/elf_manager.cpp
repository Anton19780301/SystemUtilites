#include "elf_manager.h"
#include "elfdisassembler.h"
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <vector>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include "jsonhighlighter.h"

ElfManager::ElfManager(QObject *parent) : QObject(parent) {}

bool ElfManager::loadFile(const QString &filePath, bool silent) {
    QString path = QUrl(filePath).toLocalFile();
    if (path.isEmpty()) path = filePath;

    int fd = open(path.toStdString().c_str(), O_RDONLY);
    if (fd < 0) {
        emit messageReady("Не удалось открыть файл", "error");
        return false;
    }

    struct stat st;
    fstat(fd, &st);
    m_fileSize = st.st_size;

    void* map = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        return false;
    }

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)map;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        munmap(map, st.st_size);
        close(fd);
        emit messageReady("Это не ELF файл", "error");
        return false;
    }

    m_entryPoint = "0x" + QString::number(ehdr->e_entry, 16).toUpper();
    switch(ehdr->e_machine)
    {
    case EM_X86_64: m_machineType = "x86-64"; break;
    case EM_386:    m_machineType = "Intel 80386"; break;
    case EM_ARM:    m_machineType = "ARM"; break;
    case EM_AARCH64:m_machineType = "AArch64"; break;
    default:        m_machineType = "Unknown (" + QString::number(ehdr->e_machine) + ")";
    }

    Elf64_Shdr* shdr = (Elf64_Shdr*)((uint8_t*)map + ehdr->e_shoff);
    char* strtab = (char*)map + shdr[ehdr->e_shstrndx].sh_offset;

    // Очищаем старые данные
    m_sections.clear();
    m_sectionDetails.clear();

    // ОДИН цикл для всего
    for (int i = 0; i < ehdr->e_shnum; i++) {
        QString sName = QString::fromLocal8Bit(&strtab[shdr[i].sh_name]);
        if (sName.isEmpty()) sName = QString("[%1]").arg(i);

        m_sections << sName;

        SectionInfo info;
        info.name = sName;
        info.offset = static_cast<qint64>(shdr[i].sh_offset);
        info.size = static_cast<qint64>(shdr[i].sh_size);
        info.index = i;

        m_sectionDetails.append(QVariant::fromValue(info));
    }

    m_currentFilePath = path;

    //исследуем гапы
    QList<SectionInfo> sortedDetails;
    for(const auto& v : m_sectionDetails) sortedDetails.append(v.value<SectionInfo>());

    std::sort(sortedDetails.begin(), sortedDetails.end(), [](const SectionInfo& a, const SectionInfo& b) {
        return a.offset < b.offset;
    });

    qint64 currentPos = 0;
    qint64 totalGaps = 0;

    for (const auto& sec : sortedDetails) {
        if (sec.offset > currentPos) {
            qint64 gapSize = sec.offset - currentPos;
            totalGaps += gapSize;
            // Можно добавить информацию о дырках в отдельный список, если захочешь их подсветить
        }
        currentPos = sec.offset + sec.size;
    }

    // Проверяем хвост файла
    if (currentPos < m_fileSize) {
        totalGaps += (m_fileSize - currentPos);
    }

    // Отправляем сигнал в баннер для статистики
    if (totalGaps > 0) {
        emit messageReady(QString("Обнаружено %1 байт неиспользуемого пространства").arg(totalGaps), "info");
    }

    // Освобождаем память ТОЛЬКО ПОСЛЕ завершения всех чтений
    munmap(map, st.st_size);
    close(fd);

    if (!silent) {
        emit messageReady("Файл успешно загружен", "success");
    }
    emit sectionsChanged();
    return true;
}

QString ElfManager::getSectionContent(const QString &name) {
    QByteArray data = getRawSectionData(name);
    if (data.isEmpty()) return "[Пусто или данных нет]";
    return QString::fromUtf8(data);
}

bool ElfManager::saveSection(const QString &name, const QString &content) {
    if (m_currentFilePath.isEmpty()) return false;

    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::ReadWrite)) return false;

    //сделаем копию
    this->makeBackup(m_currentFilePath);
    emit messageReady("Файл успешно сохранен (создан .bak)");

    // 1. Читаем заголовок ELF
    Elf64_Ehdr ehdr;
    file.read((char*)&ehdr, sizeof(ehdr));

    // 2. Читаем таблицу заголовков
    std::vector<Elf64_Shdr> shdrs(ehdr.e_shnum);
    file.seek(ehdr.e_shoff);
    file.read((char*)shdrs.data(), ehdr.e_shnum * sizeof(Elf64_Shdr));

    // 3. Читаем таблицу строк для поиска секции по имени
    Elf64_Shdr &strtabShdr = shdrs[ehdr.e_shstrndx];
    file.seek(strtabShdr.sh_offset);
    QByteArray strtabData = file.read(strtabShdr.sh_size);

    bool found = false;
    for (int i = 0; i < ehdr.e_shnum; i++) {
        QString sName = QString::fromLocal8Bit(strtabData.data() + shdrs[i].sh_name);

        if (sName == name) {
            // 4. Пишем новый контент в самый конец файла
            QByteArray newData = content.toUtf8();
            int64_t newOffset = file.size();

            // Выравнивание для порядка
            while (newOffset % 8 != 0) { file.putChar(0); newOffset++; }

            file.seek(newOffset);
            file.write(newData);

            // 5. Обновляем данные в заголовке секции
            shdrs[i].sh_offset = newOffset;
            shdrs[i].sh_size = newData.size();
            found = true;
            break;
        }
    }

    if (found) {
        // 6. Перезаписываем таблицу заголовков (тоже в конец, чтобы не гадать с местом)
        int64_t newShoff = file.size();
        while (newShoff % 8 != 0) { file.putChar(0); newShoff++; }

        file.seek(newShoff);
        file.write((char*)shdrs.data(), shdrs.size() * sizeof(Elf64_Shdr));

        // 7. Обновляем смещение таблицы в главном заголовке
        ehdr.e_shoff = newShoff;
        file.seek(0);
        file.write((char*)&ehdr, sizeof(ehdr));
    }

    file.close();
    emit messageReady("Файл успешно изменен");
    return found;
}


bool ElfManager::addSection(const QString &sectionName, const QString &content) {
    if (m_currentFilePath.isEmpty()) return false;

    //сделаем копию
    this->makeBackup(m_currentFilePath);
    // Используем QFile для удобства работы со смещениями
    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::ReadWrite)) return false;

    // 1. Читаем заголовок ELF
    Elf64_Ehdr ehdr;
    file.read((char*)&ehdr, sizeof(ehdr));

    // 2. Читаем все текущие заголовки секций
    std::vector<Elf64_Shdr> shdrs(ehdr.e_shnum);
    file.seek(ehdr.e_shoff);
    file.read((char*)shdrs.data(), ehdr.e_shnum * sizeof(Elf64_Shdr));

    // 3. Работаем с таблицей строк (.shstrtab)
    // Она находится по индексу e_shstrndx
    Elf64_Shdr &strtabShdr = shdrs[ehdr.e_shstrndx];
    QByteArray newName = sectionName.toUtf8() + '\0';
    uint32_t newNameOffset = strtabShdr.sh_size; // Имя будет в конце таблицы

    // Читаем старую таблицу строк, добавляем новое имя
    file.seek(strtabShdr.sh_offset);
    QByteArray strtabData = file.read(strtabShdr.sh_size);
    strtabData.append(newName);

    // Перезаписываем таблицу строк в конец файла (чтобы не двигать всё остальное)
    int64_t newStrtabOffset = file.size();
    file.seek(newStrtabOffset);
    file.write(strtabData);

    // Обновляем заголовок таблицы строк в нашем векторе
    strtabShdr.sh_offset = newStrtabOffset;
    strtabShdr.sh_size = strtabData.size();

    // 4. Готовим данные новой секции (контент)
    QByteArray sectionData = content.toUtf8();
    int64_t newContentOffset = file.size();
    // Выравнивание по 8 байт
    while (newContentOffset % 8 != 0) { file.putChar(0); newContentOffset++; }
    file.seek(newContentOffset);
    file.write(sectionData);

    // 5. Создаем заголовок для новой секции
    Elf64_Shdr newShdr;
    memset(&newShdr, 0, sizeof(newShdr));
    newShdr.sh_name = newNameOffset; // Указываем смещение в таблице строк
    newShdr.sh_type = SHT_PROGBITS;
    newShdr.sh_flags = SHF_ALLOC;
    newShdr.sh_offset = newContentOffset;
    newShdr.sh_size = sectionData.size();
    newShdr.sh_addralign = 8;

    shdrs.push_back(newShdr);

    // 6. Записываем новую таблицу заголовков секций в конец файла
    int64_t newShoff = file.size();
    while (newShoff % 8 != 0) { file.putChar(0); newShoff++; }
    file.seek(newShoff);
    file.write((char*)shdrs.data(), shdrs.size() * sizeof(Elf64_Shdr));

    // 7. Финальное обновление главного заголовка
    ehdr.e_shoff = newShoff;
    ehdr.e_shnum += 1;
    file.seek(0);
    file.write((char*)&ehdr, sizeof(ehdr));

    file.close();
    emit messageReady("Секция успешно добавлена");


    // Обновляем список в UI
    return loadFile(m_currentFilePath);
}


bool ElfManager::removeSection(const QString &name) {
    // 1. Используем наш новый метод для проверки безопасности
    if (!isDeletable(name)) {
        emit messageReady("Ошибка: нельзя удалять системный раздел " + name);
        return false;
    }

    if (m_currentFilePath.isEmpty()) return false;

    // 2. Создаем резервную копию перед операцией
    makeBackup(m_currentFilePath);

    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::ReadWrite)) {
        emit messageReady("Ошибка: файл занят другим процессом");
        return false;
    }

    Elf64_Ehdr ehdr;
    file.read((char*)&ehdr, sizeof(ehdr));

    std::vector<Elf64_Shdr> shdrs(ehdr.e_shnum);
    file.seek(ehdr.e_shoff);
    file.read((char*)shdrs.data(), ehdr.e_shnum * sizeof(Elf64_Shdr));

    // Читаем таблицу строк для сопоставления имен
    Elf64_Shdr &strtabShdr = shdrs[ehdr.e_shstrndx];
    file.seek(strtabShdr.sh_offset);
    QByteArray strtabData = file.read(strtabShdr.sh_size);

    // 3. Ищем и удаляем заголовок секции из вектора
    auto it = std::remove_if(shdrs.begin(), shdrs.end(), [&](const Elf64_Shdr& s) {
        // Проверка границ, чтобы не выйти за пределы таблицы строк
        if (s.sh_name >= (uint32_t)strtabData.size()) return false;
        const char* sName = strtabData.data() + s.sh_name;
        return name == QString::fromLocal8Bit(sName);
    });

    if (it != shdrs.end()) {
        shdrs.erase(it, shdrs.end());
        ehdr.e_shnum--;

        // 4. Записываем новую таблицу заголовков в конец файла
        int64_t newShoff = file.size();
        // Выравнивание (традиционно 8 байт)
        while (newShoff % 8 != 0) { file.putChar(0); newShoff++; }

        file.seek(newShoff);
        file.write((char*)shdrs.data(), shdrs.size() * sizeof(Elf64_Shdr));

        // 5. Обновляем смещение и количество секций в главном заголовке
        ehdr.e_shoff = newShoff;
        file.seek(0);
        file.write((char*)&ehdr, sizeof(ehdr));

        file.close();
        emit messageReady("Раздел " + name + " удален. Создан .bak");
        return loadFile(m_currentFilePath);
    }

    file.close();
    emit messageReady("Ошибка: раздел не найден");
    return false;
}

bool ElfManager::isDeletable(const QString &name) const {
    static const QStringList protectedSections = {
        ".text", ".data", ".bss", ".rodata", ".shstrtab",
        ".strtab", ".symtab", ".dynamic", ".interp"
    };
    return !protectedSections.contains(name);
}

void ElfManager::setupHighlighter(QQuickItem *textAreaItem) {
    QVariant doc = textAreaItem->property("textDocument");
    if (doc.canConvert<QQuickTextDocument*>()) {
        QQuickTextDocument *qdoc = doc.value<QQuickTextDocument*>();
        new JsonHighlighter(qdoc->textDocument());
    }
}

bool ElfManager::stripAll() {
    if (m_currentFilePath.isEmpty()) return false;

    makeBackup(m_currentFilePath);

    // Собираем список на удаление
    QStringList toRemove;
    for (const QString &name : m_sections) {
        // Удаляем отладку, комментарии и ноты
        if (name.startsWith(".debug") ||
            name.startsWith(".zdebug") ||
            name == ".comment" ||
            name.startsWith(".note")) {

            // Но проверяем нашу защиту
            if (isDeletable(name)) {
                toRemove << name;
            }
        }
    }

    if (toRemove.isEmpty()) {
        emit messageReady("Удалять нечего, файл уже чист", "info");
        return true;
    }

    int count = 0;
    for (const QString &name : toRemove) {
        if (removeSection(name)) count++;
    }

    emit messageReady(QString("Успешно удалено %1 секций").arg(count), "success");
    return loadFile(m_currentFilePath);
}


QString ElfManager::getSectionHex(const QString &name) {
    QByteArray data = getRawSectionData(name);
    if (data.isEmpty()) return "Нет данных для отображения";

    QString result;
    const int bytesPerLine = 16;

    for (int i = 0; i < data.size(); i += bytesPerLine) {
        // Выводим смещение (offset)
        result += QString("%1: ").arg(i, 8, 16, QChar('0')).toUpper();

        QString hexPart;
        QString asciiPart;

        for (int j = 0; j < bytesPerLine; j++) {
            if (i + j < data.size()) {
                unsigned char c = static_cast<unsigned char>(data[i + j]);
                hexPart += QString("%1 ").arg(c, 2, 16, QChar('0')).toUpper();
                // ASCII колонка: только печатные символы
                asciiPart += (c >= 32 && c <= 126) ? QChar(c) : QChar('.');
            } else {
                hexPart += "   "; // Пробелы для выравнивания последней строки
            }
        }

        result += hexPart + " | " + asciiPart + "\n";
    }
    return result;
}

qint64 ElfManager::ElfManager::calculateGaps() {
    qint64 busySize = 0;
    for(const auto& v : m_sectionDetails) {
        SectionInfo info = v.value<SectionInfo>();
        busySize += info.size;
    }
    //TODO: Очень грубый расчет: размер файла минус сумма размеров всех секций, переделать
    return m_fileSize - busySize;
}


void ElfManager::makeBackup(const QString &filePath) {
    QString bkp = filePath + ".bak";
    if (QFile::exists(bkp)) QFile::remove(bkp);
    QFile::copy(filePath, bkp);
}


QByteArray ElfManager::getRawSectionData(const QString &name) {
    if (m_currentFilePath.isEmpty()) return QByteArray();

    int fd = open(m_currentFilePath.toStdString().c_str(), O_RDONLY);
    if (fd < 0) return QByteArray();

    struct stat st;
    fstat(fd, &st);

    void* map = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        return QByteArray();
    }

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)map;
    Elf64_Shdr* shdr = (Elf64_Shdr*)((uint8_t*)map + ehdr->e_shoff);
    char* strtab = (char*)map + shdr[ehdr->e_shstrndx].sh_offset;

    QByteArray data;

    for (int i = 0; i < ehdr->e_shnum; i++) {
        QString sName = QString::fromLocal8Bit(&strtab[shdr[i].sh_name]);
        if (sName == name) {
            if (shdr[i].sh_type != SHT_NOBITS) {
                data = QByteArray((const char*)map + shdr[i].sh_offset, shdr[i].sh_size);
            }
            break;
        }
    }

    munmap(map, st.st_size);
    close(fd);
    return data;
}


bool ElfManager::repackElf() {
    if (m_currentFilePath.isEmpty()) return false;
    makeBackup(m_currentFilePath);

    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::ReadWrite)) return false;

    Elf64_Ehdr ehdr;
    file.read((char*)&ehdr, sizeof(ehdr));

    std::vector<Elf64_Shdr> shdrs(ehdr.e_shnum);
    file.seek(ehdr.e_shoff);
    file.read((char*)shdrs.data(), ehdr.e_shnum * sizeof(Elf64_Shdr));

    // Сортируем копию для анализа
    std::vector<int> sortedIdx(ehdr.e_shnum);
    std::iota(sortedIdx.begin(), sortedIdx.end(), 0);
    std::sort(sortedIdx.begin(), sortedIdx.end(), [&](int a, int b) {
        return shdrs[a].sh_offset < shdrs[b].sh_offset;
    });

    QByteArray newFileData;
    // Оставляем заголовок и всё до первой секции как есть
    qint64 firstOffset = shdrs[sortedIdx[1]].sh_offset; // 0-я обычно NULL
    file.seek(0);
    newFileData.append(file.read(firstOffset));

    qint64 currentOffset = firstOffset;

    for (int idx : sortedIdx) {
        if (shdrs[idx].sh_type == SHT_NULL) continue;

        // Читаем контент
        file.seek(shdrs[idx].sh_offset);
        QByteArray content = file.read(shdrs[idx].sh_size);

        // Выравнивание
        qint64 align = shdrs[idx].sh_addralign;
        if (align > 1) {
            while (currentOffset % align != 0) {
                newFileData.append('\0');
                currentOffset++;
            }
        }

        // Обновляем смещение и пишем данные
        shdrs[idx].sh_offset = currentOffset;
        newFileData.append(content);
        currentOffset += content.size();
    }

    // Таблицу секций пишем в самый конец
    while (currentOffset % 8 != 0) { newFileData.append('\0'); currentOffset++; }
    ehdr.e_shoff = currentOffset;

    // Обновляем Ehdr в буфере
    memcpy(newFileData.data(), &ehdr, sizeof(ehdr));

    // Дописываем таблицу секций
    newFileData.append((const char*)shdrs.data(), shdrs.size() * sizeof(Elf64_Shdr));

    file.seek(0);
    file.resize(newFileData.size());
    file.write(newFileData);
    file.close();

    return loadFile(m_currentFilePath, true);
}

QString ElfManager::getSectionDisasm(const QString &name) {
    QByteArray data = getRawSectionData(name);
    if (data.isEmpty()) return "No data";

    // Находим смещение секции, чтобы адреса в дампе были реальными
    qint64 offset = 0;
    for(const auto& v : m_sectionDetails) {
        SectionInfo info = v.value<SectionInfo>();
        if(info.name == name) { offset = info.offset; break; }
    }

    return ElfDisassembler::disassemble(data, offset);
}

