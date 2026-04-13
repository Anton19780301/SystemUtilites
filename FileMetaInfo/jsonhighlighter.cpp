#include "jsonhighlighter.h"

JsonHighlighter::JsonHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    // Ключи (Cyan)
    rule.pattern = QRegularExpression(R"("\w*"\s*:)");
    rule.format.setForeground(QColor("#00f2ff"));
    rules.append(rule);

    // Значения-строки (Зеленый неон)
    rule.pattern = QRegularExpression(R"(:\s*".*?")");
    rule.format.setForeground(QColor("#00ff9f"));
    rules.append(rule);

    // Числа (Маджента)
    rule.pattern = QRegularExpression(R"(\b\d+\b)");
    rule.format.setForeground(QColor("#ff00ff"));
    rules.append(rule);

    // Булевы и null (розовый)
    rule.pattern = QRegularExpression(R"(\b(true|false|null)\b)");
    rule.format.setForeground(QColor("#f92672"));
    rules.append(rule);
}

void JsonHighlighter::highlightBlock(const QString &text) {
    for (const auto &rule : rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}