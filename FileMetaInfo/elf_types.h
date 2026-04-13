#ifndef ELF_TYPES_H
#define ELF_TYPES_H

#include <QString>
#include <QtQml>

struct SectionInfo {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(qint64 offset MEMBER offset)
    Q_PROPERTY(qint64 size MEMBER size)
    Q_PROPERTY(int index MEMBER index)

public:
    QString name;
    qint64 offset;
    qint64 size;
    int index;
};

Q_DECLARE_METATYPE(SectionInfo)

#endif // ELF_TYPES_H
