// 在 mainwindow.cpp 中
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sstream>
#include <memory>

#include "maddy/parser.h"
#include "markdownhighlighter.h"
#include "maddy/parserconfig.h"

#include <QGraphicsDropShadowEffect>
#include <QWebEnginePage>
#include <QScrollBar>
#include <QTimer>
#include <QHBoxLayout>
#include <QSplitter>
#include <QWebEngineView>
#include <QScreen>
#include <QGuiApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QTextEdit>
#include <QCloseEvent>
#include <QInputDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_lastEditorScrollRatio = 0.0;

    m_editorFontSize = 12;
    m_previewFontSize = 12;

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int height = screenGeometry.height() * 0.8;
    int width = screenGeometry.width() * 0.8;
    resize(width, height);  //窗口大小


    QHBoxLayout *mainLayout = new QHBoxLayout(ui->centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    QFont baseFont("Arial", m_editorFontSize);
    m_editor = new QTextEdit(splitter);
    m_editor->setObjectName("editor");
    m_editor->setAutoFormatting(QTextEdit::AutoNone);
    m_editor->setFont(baseFont);

    m_highlighter = new MarkdownHighlighter(m_editor->document(), m_editor->font());

    m_preview = new QWebEngineView(splitter);
    m_preview->setObjectName("preview");
    //m_preview->page()->setBackgroundColor(Qt::transparent);

    mainLayout->addWidget(splitter);
    QList<int> initialSizes;
    initialSizes << 600 << 600;
    splitter->setSizes(initialSizes);

    connect(m_editor, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(m_editor->document(), &QTextDocument::modificationChanged, this, &MainWindow::onDocumentModified);
    connect(m_preview, &QWebEngineView::loadFinished, this, &MainWindow::onPreviewLoadFinished);
    m_previewUpdateTimer = new QTimer(this);

    m_previewUpdateTimer->setSingleShot(true); // 設定為單次觸發
    m_previewUpdateTimer->setInterval(500); // 設定延遲時間為 300 毫秒
    connect(m_previewUpdateTimer, &QTimer::timeout, this, &MainWindow::updatePreview);


    setupActions();
    loadCssTemplate();
    m_currentFilePath = ""; // 初始化檔案路徑為空
    updateWindowTitle(); // 設定初始視窗標題
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onTextChanged()
{
    m_previewUpdateTimer->start();
}


void MainWindow::setupActions()
{
    // --- 檔案功能表 ---
    QMenu *fileMenu = menuBar()->addMenu("檔案(&F)");

    // 新增
    QAction *newAction = new QAction("新增(&N)", this);
    newAction->setShortcut(QKeySequence::New); // 設定快速鍵 Ctrl+N
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

    // 開啟
    QAction *openAction = new QAction("開啟(&O)", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    // 儲存
    QAction *saveAction = new QAction("儲存(&S)", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);

    // 另存為
    QAction *saveAsAction = new QAction("另存為(&A)...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);

    // --- 將 Action 新增到功能表和工具列 ---
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);

    // --- 編輯功能表 ---
    QToolBar *editToolBar = addToolBar("Edit");
    editToolBar->setObjectName("MainToolBar");
    QMenu *editMenu = menuBar()->addMenu("編輯(&E)");

    // --- 編輯區字體控制 (使用 Lambda) ---
    QAction *editorZoomInAction = new QAction("放大編輯區", this);
    connect(editorZoomInAction, &QAction::triggered, this, [this]() {
        m_editorFontSize += 1; // 增加字號
        QFont f = m_editor->font();
        f.setPointSize(m_editorFontSize);
        m_editor->setFont(f);
        m_highlighter = new MarkdownHighlighter(m_editor->document(), f);
    });

    QAction *editorZoomOutAction = new QAction("縮小編輯區", this);
    connect(editorZoomOutAction, &QAction::triggered, this, [this]() {
        if (m_editorFontSize > 8) {
            m_editorFontSize -= 1; // 減小字號
            QFont f = m_editor->font();
            f.setPointSize(m_editorFontSize);
            m_editor->setFont(f);
            m_highlighter = new MarkdownHighlighter(m_editor->document(), f);
        }
    });

    // --- 預覽區字體控制 (使用 Lambda) ---
    QAction *previewZoomInAction = new QAction("放大預覽區", this);
    connect(previewZoomInAction, &QAction::triggered, this, [this]() {
        m_previewFontSize += 1;
        onTextChanged(); // 重新渲染預覽
    });

    QAction *previewZoomOutAction = new QAction("縮小預覽區", this);
    connect(previewZoomOutAction, &QAction::triggered, this, [this]() {
        if (m_previewFontSize > 8) {
            m_previewFontSize -= 1;
            onTextChanged(); // 重新渲染預覽
        }
    });

    // 建立新的 Action
    QAction *setEditorFontAction = new QAction("編輯區字體大小", this);
    connect(setEditorFontAction, &QAction::triggered, this, &MainWindow::setEditorFontSize);

    QAction *setPreviewFontAction = new QAction("預覽區字體大小", this);
    connect(setPreviewFontAction, &QAction::triggered, this, &MainWindow::setPreviewFontSize);

    // 將新的 Action 加入到「編輯」功能表中
    editMenu->addAction(setEditorFontAction);
    editMenu->addAction(setPreviewFontAction);

    editToolBar->addAction(editorZoomInAction);
    editToolBar->addAction(editorZoomOutAction);
    editToolBar->addAction(previewZoomInAction);
    editToolBar->addAction(previewZoomOutAction);

}



void MainWindow::updateWindowTitle()
{
    // 檢查文件是否已被修改
    bool isModified = m_editor->document()->isModified();

    QString title = m_currentFilePath.isEmpty()
                    ? "Untitled.md"
                    : QFileInfo(m_currentFilePath).fileName();

    // 如果已修改，就在檔名前面加上 '*'
    setWindowTitle(QString("%1%2 - Markdown Editor").arg(isModified ? "*" : "").arg(title));
}
void MainWindow::newFile()
{
    m_editor->clear();
    m_currentFilePath = "";
    updateWindowTitle();
}


void MainWindow::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "開啟檔案", "", "Markdown Files (*.md)");
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "錯誤", "無法開啟檔案: " + file.errorString());
        return;
    }

    QTextStream in(&file);
    m_editor->setText(in.readAll());
    file.close();
    m_editor->document()->setModified(false);

    m_currentFilePath = filePath;
    updateWindowTitle();


}

bool MainWindow::saveFile()
{
    // 如果檔案路徑為空，代表是新檔案，行為等同於「另存為」
    if (m_currentFilePath.isEmpty()) {
        return saveFileAs();
    }

    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "錯誤", "無法儲存檔案: " + file.errorString());
        return false;
    }

    QTextStream out(&file);
    out << m_editor->toPlainText();
    file.close();
    m_editor->document()->setModified(false);

    updateWindowTitle();
    return true;
}

bool MainWindow::saveFileAs()
{
    // 正确的过滤器字符串
    QString filePath = QFileDialog::getSaveFileName(this, "另存為", "", "Markdown Files (*.md)");
    if (filePath.isEmpty()) {
        return false; // 使用者取消
    }

    m_currentFilePath = filePath;
    return saveFile();
}

void MainWindow::onDocumentModified()
{
    // 當文件的修改狀態改變時，直接更新視窗標題即可
    updateWindowTitle();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 檢查文件是否有未儲存的變更
    if (m_editor->document()->isModified()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "檔案尚未儲存",
                                      "您有未儲存的變更，是否要在關閉前儲存？",
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            // 如果使用者選擇儲存
            if (saveFile()) {
                event->accept(); // 儲存成功，接受關閉事件
            } else {
                event->ignore(); // 儲存失敗 (例如使用者取消了另存為)，忽略關閉事件
            }
        } else if (reply == QMessageBox::Discard) {
            // 如果使用者選擇不儲存
            event->accept(); // 接受關閉事件
        } else { // reply == QMessageBox::Cancel
            // 如果使用者選擇取消
            event->ignore(); // 忽略關閉事件，視窗不會關閉
        }
    } else {
        // 如果沒有未儲存的變更
        event->accept(); // 直接接受關閉事件
    }
}

void MainWindow::applyEditorFontSize()
{
    QFont font = m_editor->font();
    font.setPointSize(m_editorFontSize);
    m_editor->setFont(font);

    // 重新建立 Highlighter 以套用新的基礎字體
    m_highlighter = new MarkdownHighlighter(m_editor->document(), font);
}

void MainWindow::setEditorFontSize()
{
    bool ok;
    int newSize = QInputDialog::getInt(this, "設定編輯區字體", "請輸入字體大小:",
                                       m_editorFontSize, 8, 72, 1, &ok);
    if (ok) {
        m_editorFontSize = newSize;
        applyEditorFontSize(); // 呼叫輔助函式來套用設定
    }
}

void MainWindow::setPreviewFontSize()
{
    bool ok;
    int newSize = QInputDialog::getInt(this, "設定預覽區字體", "請輸入字體大小:",
                                       m_previewFontSize, 8, 72, 1, &ok);
    if (ok) {
        m_previewFontSize = newSize;
        onTextChanged(); // 重新渲染預覽區以套用新的 CSS 字體大小
    }
}

void MainWindow::loadCssTemplate()
{
    // 從 Qt 資源系統中讀取 CSS 檔案
    QFile file(":/preview_style.css");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        m_cssTemplate = in.readAll();
        file.close();
    } else {
        qDebug() << "Error: Could not load CSS file from resources.";
    }
}

// 在 mainwindow.cpp 末尾新增
void MainWindow::updatePreview()
{
    QString markdownText = m_editor->toPlainText();

    QScrollBar *editorScrollBar = m_editor->verticalScrollBar();
    // 檢查最大值以避免除以零的錯誤
    if (editorScrollBar->maximum() > 0) {
        m_lastEditorScrollRatio = (double)editorScrollBar->value() / editorScrollBar->maximum();
    }

    // 使用 maddy 引擎進行轉換
    std::stringstream markdownStream(markdownText.toStdString());
    std::shared_ptr<maddy::ParserConfig> config = std::make_shared<maddy::ParserConfig>();
    config->enabledParsers &= ~maddy::types::EMPHASIZED_PARSER; // disable emphasized parser
    config->enabledParsers |= maddy::types::HTML_PARSER; // do not wrap HTML in paragraph
    std::shared_ptr<maddy::Parser> parser = std::make_shared<maddy::Parser>(config);
    std::string htmlString = parser->Parse(markdownStream);

   // QString wrappedHtml = QString("<div id=\"wrapper\"><div>%1</div></div>")
   //                         .arg(QString::fromStdString(htmlString));

    // 組合 CSS 並顯示
    QString finalCss = m_cssTemplate.arg(m_previewFontSize).arg(m_previewFontSize - 2);
    QString fullHtml = QString("<style>%1</style>").arg(finalCss)
                       + QString::fromStdString(htmlString);

    m_preview->setHtml(fullHtml);
}

void MainWindow::onPreviewLoadFinished()
{

    QString script = QString(
        "const scrollHeight = document.body.scrollHeight - window.innerHeight;"
        "window.scrollTo(0, scrollHeight * %1);"
    ).arg(m_lastEditorScrollRatio);

    m_preview->page()->runJavaScript(script);
}
