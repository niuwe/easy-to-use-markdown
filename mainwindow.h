#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTextEdit;
class QWebEngineView;
class QCloseEvent;

class MarkdownHighlighter;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void onTextChanged();
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void onDocumentModified();
    void FontSizeSet();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupFileActions();
    void updateWindowTitle();
    void setupEditActions();

private:
    Ui::MainWindow *ui;
    QTextEdit *m_editor;
    QWebEngineView *m_preview;
    QString m_currentFilePath;
    MarkdownHighlighter *m_highlighter;
    int m_currentFontSize;
protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
