#include "elfdisassembler.h"
#include <QStringBuilder>
#include <qlist.h>

QString ElfDisassembler::disassemble(const QByteArray &data, qint64 baseAddress) {
    QString result;
    const unsigned char* code = reinterpret_cast<const unsigned char*>(data.data());
    int offset = 0;

    while (offset < data.size()) {
        result += QString("0x%1:  ").arg(baseAddress + offset, 8, 16, QChar('0')).toUpper();

        Instruction ins = decodeNext(code + offset, data.size() - offset);

        // Добавляем HEX-байты инструкции для красоты
        QString hexBytes;
        for(int i = 0; i < ins.length; ++i) {
            hexBytes += QString("%1 ").arg(code[offset + i], 2, 16, QChar('0')).toUpper();
        }
        result += hexBytes.leftJustified(15, ' ') + " " + ins.mnemonic + "\n";

        offset += ins.length;
        if (offset > 5000) { // Защита от зависания UI
            result += "... (truncated)";
            break;
        }
    }
    return result;
}

ElfDisassembler::Instruction ElfDisassembler::decodeNext(const unsigned char* code, int maxLength) {
    if (maxLength <= 0) return {0, ""};

    int p = 0;
    bool rexW = false;
    bool rexB = false; // Бит для расширения индекса регистра (r8-r15)
    unsigned char op = code[p];

    // 1. Декодируем REX-префикс
    if ((op & 0xF0) == 0x40) {
        rexW = (op & 0x08); // 64-bit operand size
        rexB = (op & 0x01); // Extension of the ModR/M r/m field or Base field
        p++;
        if (p >= maxLength) return {1, QString("db 0x%1").arg(code[0], 2, 16, QChar('0'))};
    }

    unsigned char realOp = code[p];

    // Списки регистров (добавим расширенные r8-r15)
    static const QStringList gpr64 = {"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};
    static const QStringList gpr32 = {"eax","ecx","edx","ebx","esp","ebp","esi","edi","r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d"};

    auto getRegName = [&](int index, bool is64) {
        int finalIdx = index + (rexB ? 8 : 0);
        return is64 ? gpr64[finalIdx] : gpr32[finalIdx];
    };

    switch (realOp) {
    case 0x90: return {p + 1, "nop"};

    // MOV immediate
    case 0xB8: case 0xB9: case 0xBA: case 0xBB:
    case 0xBC: case 0xBD: case 0xBE: case 0xBF: {
        int regIdx = realOp - 0xB8;
        if (rexW) { // MOVABS imm64
            if (p + 8 < maxLength) {
                uint64_t imm;
                memcpy(&imm, code + p + 1, 8); // Безопаснее чем reinterpret_cast
                return {p + 9, QString("movabs %1, 0x%2").arg(getRegName(regIdx, true)).arg(imm, 0, 16)};
            }
        } else { // MOV imm32
            if (p + 4 < maxLength) {
                uint32_t imm;
                memcpy(&imm, code + p + 1, 4);
                return {p + 5, QString("mov %1, 0x%2").arg(getRegName(regIdx, false)).arg(imm, 0, 16)};
            }
        }
        break;
    }

    // PUSH/POP (всегда 64-bit в режиме x64)
    case 0x50: case 0x51: case 0x52: case 0x53:
    case 0x54: case 0x55: case 0x56: case 0x57:
        return {p + 1, QString("push %1").arg(getRegName(realOp - 0x50, true))};

    case 0x58: case 0x59: case 0x5A: case 0x5B:
    case 0x5C: case 0x5D: case 0x5E: case 0x5F:
        return {p + 1, QString("pop %1").arg(getRegName(realOp - 0x58, true))};

    case 0xC3: return {p + 1, "ret"};
    }

    // Если не распознали — возвращаем только ОДИН байт (первый в последовательности)
    return {1, QString("db 0x%1").arg(code[0], 2, 16, QChar('0'))};
}



