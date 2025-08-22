// markdownhighlighter.cpp

#include "markdownhighlighter.h"

// markdownhighlighter.cpp

#include "markdownhighlighter.h"
#include <QFont>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent, const QFont &baseFont)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // --- 基礎格式 ---
    // 預設文字使用基礎字體
    QTextCharFormat defaultFormat;
    defaultFormat.setFont(baseFont);

    // --- 在基礎字體上進行修改來定義所有規則 ---
    // 1.1 標題 (例如 # ## ###)
    QTextCharFormat headingFormat1;
    QFont headingFont1 = baseFont;
    headingFont1.setBold(true); // 標題加粗
    headingFont1.setPointSize(baseFont.pointSize() * 1.6); // 標題字號放大 40%
    headingFormat1.setFont(headingFont1);
    headingFormat1.setForeground(QColor(135, 206, 250)); // 淡藍色
    rule.pattern = QRegularExpression("^(#{1}\\s.*)");
    rule.format = headingFormat1;
    m_highlightingRules.append(rule);

    // 1.2 標題 (例如 # ## ###)
    QTextCharFormat headingFormat2;
    QFont headingFont2 = baseFont;
    headingFont2.setBold(true); // 標題加粗
    headingFont2.setPointSize(baseFont.pointSize() * 1.5); // 標題字號放大 40%
    headingFormat2.setFont(headingFont2);
    headingFormat2.setForeground(QColor(135, 206, 250)); // 淡藍色
    rule.pattern = QRegularExpression("^(#{2}\\s.*)");
    rule.format = headingFormat2;
    m_highlightingRules.append(rule);

    // 1.3 標題 (例如 # ## ###)
    QTextCharFormat headingFormat3;
    QFont headingFont3 = baseFont;
    headingFont3.setBold(true); // 標題加粗
    headingFont3.setPointSize(baseFont.pointSize() * 1.4); // 標題字號放大 40%
    headingFormat3.setFont(headingFont3);
    headingFormat3.setForeground(QColor(135, 206, 250)); // 淡藍色
    rule.pattern = QRegularExpression("^(#{3}\\s.*)");
    rule.format = headingFormat3;
    m_highlightingRules.append(rule);

    // 1.4 標題 (例如 # ## ###)
    QTextCharFormat headingFormat4;
    QFont headingFont4 = baseFont;
    headingFont4.setBold(true); // 標題加粗
    headingFont4.setPointSize(baseFont.pointSize() * 1.3); // 標題字號放大 40%
    headingFormat4.setFont(headingFont4);
    headingFormat4.setForeground(QColor(135, 206, 250)); // 淡藍色
    rule.pattern = QRegularExpression("^(#{4}\\s.*)");
    rule.format = headingFormat4;
    m_highlightingRules.append(rule);

    // 1.5 標題 (例如 # ## ###)
    QTextCharFormat headingFormat5;
    QFont headingFont5 = baseFont;
    headingFont5.setBold(true); // 標題加粗
    headingFont5.setPointSize(baseFont.pointSize() * 1.2); // 標題字號放大 40%
    headingFormat5.setFont(headingFont5);
    headingFormat5.setForeground(QColor(135, 206, 250)); // 淡藍色
    rule.pattern = QRegularExpression("^(#{5}\\s.*)");
    rule.format = headingFormat5;
    m_highlightingRules.append(rule);

    // 1.6 標題 (例如 # ## ###)
    QTextCharFormat headingFormat6;
    QFont headingFont6 = baseFont;
    headingFont6.setBold(true); // 標題加粗
    headingFont6.setPointSize(baseFont.pointSize() * 1.4); // 標題字號放大 40%
    headingFormat6.setFont(headingFont6);
    headingFormat6.setForeground(QColor(135, 206, 250)); // 淡藍色
    rule.pattern = QRegularExpression("^(#{6}\\s.*)");
    rule.format = headingFormat6;
    m_highlightingRules.append(rule);

    // 2. 粗體 (例如 **文字** 或 __文字__)
    QTextCharFormat boldFormat;
    QFont boldFont = baseFont;
    boldFont.setBold(true);
    boldFont.setPointSize(baseFont.pointSize());
    boldFormat.setFont(boldFont);
    rule.pattern = QRegularExpression("(\\*\\*|__)(.*?)\\1");
    rule.format = boldFormat;
    m_highlightingRules.append(rule);

    // 3. 斜體 (例如 *文字* 或 _文字_)
    QTextCharFormat italicFormat;
    QFont italicFont = baseFont;
    italicFont.setItalic(true);
    italicFont.setPointSize(baseFont.pointSize());
    italicFormat.setFont(italicFont);
    rule.pattern = QRegularExpression("(?<!\\*)\\*(?!\\*|_)(.*?)(?<!_)\\*(?!\\*)|(?<!_)_{(?!_)(.*?)(?<!_)_{(?!_)");
    rule.format = italicFormat;
    m_highlightingRules.append(rule);

    // 4. 程式碼區塊分隔符 (例如 ```)
    QTextCharFormat codeBlockFormat;
    QFont codeBlockFont = baseFont;
    codeBlockFormat.setFont(baseFont);
    codeBlockFormat.setForeground(Qt::gray);
    codeBlockFont.setFamily("Consolas");
    codeBlockFont.setPointSize(baseFont.pointSize());
    rule.pattern = QRegularExpression("```");
    rule.format = codeBlockFormat;
    m_highlightingRules.append(rule);

    // 5. 清單 (例如 - item 或 1. item)
    QTextCharFormat listFormat;
    QFont listFont = baseFont;
    listFont.setBold(true);
    listFormat.setFont(listFont);
    listFormat.setForeground(QColor(255, 165, 0)); // 橘色
    rule.pattern = QRegularExpression("^\\s*([\\*\\+\\-]\\s|\\d+\\.\\s.*)");
    rule.format = listFormat;
    m_highlightingRules.append(rule);

    // 6. 行內程式碼 (例如 `code`)
    QTextCharFormat inlineCodeFormat;
    QFont codeFont = baseFont;
    codeFont.setFamily("Consolas"); // 程式碼建議使用專業的等寬字體
    codeFont.setPointSize(baseFont.pointSize() * 0.8); // 程式碼字號略小一些
    inlineCodeFormat.setFont(codeFont);
    inlineCodeFormat.setBackground(QColor(230, 230, 230));
    rule.pattern = QRegularExpression("`([^`].*?)`");
    rule.format = inlineCodeFormat;
    m_highlightingRules.append(rule);

    // 7. 引用 (例如 > quote)
    QTextCharFormat blockquoteFormat;
    QFont bqFont = baseFont;
    bqFont.setItalic(true);
    blockquoteFormat.setFont(bqFont);
    blockquoteFormat.setForeground(Qt::darkGray);
    rule.pattern = QRegularExpression("^>.*");
    rule.format = blockquoteFormat;
    m_highlightingRules.append(rule);

    // 8. 連結文字 (例如 [Google])
    QTextCharFormat linkTextFormat;
    QFont linkFont = baseFont;
    linkFont.setUnderline(true);
    linkTextFormat.setFont(linkFont);
    linkTextFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression("\\[([^\\]]+)\\]");
    rule.format = linkTextFormat;
    m_highlightingRules.append(rule);

    // 9. 連結 URL (例如 (https://...))
    QTextCharFormat linkUrlFormat;
    linkUrlFormat.setFont(baseFont);
    linkUrlFormat.setForeground(Qt::gray);
    rule.pattern = QRegularExpression("\\(([^\\)]+)\\)");
    rule.format = linkUrlFormat;
    m_highlightingRules.append(rule);
}


// 這個函式會被 Qt 自動呼叫
void MarkdownHighlighter::highlightBlock(const QString &text)
{
    // 遍歷我們定義的所有規則
    for (const HighlightingRule &rule : m_highlightingRules) {
        // 在目前的文字行中，尋找所有匹配規則的子字串
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            // 為匹配到的子字串套用格式
            // match.capturedStart() 是子字串的起始位置
            // match.capturedLength() 是子字串的長度
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
