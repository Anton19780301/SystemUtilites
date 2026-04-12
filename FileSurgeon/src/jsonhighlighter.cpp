#include "jsonhighlighter.h"

JsonHighlighter::JsonHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Ключи (например, "author") - неоновый голубой
    QTextCharFormat keyFormat;
    keyFormat.setForeground(QColor("#00f0ff"));
    keyFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(R"("\w*"\s*:)");
    rule.format = keyFormat;
    highlightingRules.append(rule);

    // Значения в кавычках - светло-зеленый
    QTextCharFormat valueFormat;
    valueFormat.setForeground(QColor("#39ff14"));
    rule.pattern = QRegularExpression(R"(:\s*".*")");
    rule.format = valueFormat;
    highlightingRules.append(rule);

    // Числа и булевы значения - оранжевый
    QTextCharFormat numberFormat;
    numberFormat.setForeground(QColor("#ffb86c"));
    rule.pattern = QRegularExpression(R"(\b(true|false|null|\d+)\b)");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // Скобки - серый
    QTextCharFormat braceFormat;
    braceFormat.setForeground(QColor("#aaaaaa"));
    rule.pattern = QRegularExpression(R"([\{\}\[\]])");
    rule.format = braceFormat;
    highlightingRules.append(rule);
}

void JsonHighlighter::highlightBlock(const QString &text) {
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

void JsonHighlighter::setTextDocument(QQuickTextDocument* doc) {
    if (m_textDocument == doc) return;

    m_textDocument = doc;

    if (m_textDocument) {
        // Связываем QSyntaxHighlighter с реальным QTextDocument внутри QQuickTextDocument
        setDocument(m_textDocument->textDocument());
    } else {
        setDocument(nullptr);
    }

    emit textDocumentChanged();
}
