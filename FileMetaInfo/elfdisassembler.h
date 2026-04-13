#ifndef ELF_DISASSEMBLER_H
#define ELF_DISASSEMBLER_H

#include <QString>
#include <QByteArray>
#include <vector>

class ElfDisassembler {
public:
    static QString disassemble(const QByteArray &data, qint64 baseAddress = 0);

private:
    // Вспомогательная структура для результата одной инструкции
    struct Instruction {
        int length;
        QString mnemonic;
    };

    static Instruction decodeNext(const unsigned char* code, int maxLength);
};

#endif
