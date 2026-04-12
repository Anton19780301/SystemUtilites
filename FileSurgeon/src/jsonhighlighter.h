#ifndef JSONHIGHLIGHTER_H
#define JSONHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QQuickTextDocument>
#include <QRegularExpression>
#include <QtQml/qqmlregistration.h> // Проверьте наличие этой строки!

class JsonHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
    QML_ELEMENT

public:
    // Оставляем только этот конструктор для QML
    explicit JsonHighlighter(QObject *parent = nullptr);

    Q_PROPERTY(QQuickTextDocument* textDocument READ textDocument WRITE setTextDocument NOTIFY textDocumentChanged)

    QQuickTextDocument* textDocument() const { return m_textDocument; }
    void setTextDocument(QQuickTextDocument* doc);

signals:
    void textDocumentChanged();

protected:
    void highlightBlock(const QString &text) override;

private:
    QQuickTextDocument* m_textDocument = nullptr;

    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
};

#endif
