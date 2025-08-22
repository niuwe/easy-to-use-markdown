#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTextEdit;
class QWebEngineView;
class QCloseEvent;
class QTimer;
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
    void updatePreview();
    void setEditorFontSize();
    void setPreviewFontSize();
    void onPreviewLoadFinished();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void updateWindowTitle();
    void setupActions();
    void applyEditorFontSize();
    void loadCssTemplate();

private:
    Ui::MainWindow *ui;
    QTextEdit *m_editor;
    QWebEngineView *m_preview;
    QString m_currentFilePath;
    MarkdownHighlighter *m_highlighter;
    int m_editorFontSize;
    int m_previewFontSize;
    QString m_cssTemplate;
    QTimer *m_previewUpdateTimer;
    qreal m_lastEditorScrollRatio;

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
