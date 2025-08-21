// markdownhighlighter.h

#ifndef MARKDOWNHIGHLIGHTER_H
#define MARKDOWNHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression> // 引入規則運算式類別
#include <QTextCharFormat>    // 引入文字格式類別

class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:

    MarkdownHighlighter(QTextDocument *parent,const  QFont &baseFont);

protected:

    void highlightBlock(const QString &text) override;

private:

    struct HighlightingRule
    {
        QRegularExpression pattern; // 規則運算式
        QTextCharFormat format;     // 對應的格式
    };
    QVector<HighlightingRule> m_highlightingRules; // 儲存所有規則的向量
};

#endif // MARKDOWNHIGHLIGHTER_H
