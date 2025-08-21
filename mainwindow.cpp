// 在 mainwindow.cpp 中
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sstream>
#include <memory>
#include "maddy/parser.h"
#include "markdownhighlighter.h"

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

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int height = screenGeometry.height() * 0.7;
    int width = screenGeometry.width() * 0.7;
    resize(width, height);  //窗口大小
    m_currentFontSize = 20; //初始字體大小

    QHBoxLayout *mainLayout = new QHBoxLayout(ui->centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    QFont baseFont("Arial", m_currentFontSize);
    m_editor = new QTextEdit(splitter);
    m_editor->setObjectName("editor");
    m_editor->setAutoFormatting(QTextEdit::AutoNone);
    m_editor->setFont(baseFont);

    m_highlighter = new MarkdownHighlighter(m_editor->document(), m_editor->font());

    m_preview = new QWebEngineView(splitter);
    m_preview->setObjectName("preview");

    mainLayout->addWidget(splitter);
    QList<int> initialSizes;
    initialSizes << 600 << 600;
    splitter->setSizes(initialSizes);

    connect(m_editor, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(m_editor->document(), &QTextDocument::modificationChanged, this, &MainWindow::onDocumentModified);

    setupFileActions();
    setupEditActions();
    m_currentFilePath = ""; // 初始化檔案路徑為空
    updateWindowTitle(); // 設定初始視窗標題
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTextChanged()
{
    const QString css = QString(R"(
        <style>
            body {
                font-family: Arial, sans-serif;
                font-size: %1pt;
                line-height: 1.6;
            }
            blockquote {
                border-left: 4px solid #dfe2e5;
                padding: 0 1em;
                color: #6a737d;
            }
            pre, code {
                font-family: Consolas;
                font-size: %2pt;
                background-color: #f6f8fa;
                border-radius: 3px;
            }
            pre {
                padding: 16px;
                overflow: auto;
            }
            code {
                padding: .2em .4em;
            }
        </style>
    )").arg(m_currentFontSize).arg(m_currentFontSize - 2);

    // 1. 從編輯器獲取純文字 Markdown 原始碼
    QString markdownText = m_editor->toPlainText();

    // --- 使用 maddy 庫進行轉換 ---
    std::stringstream markdownStream(markdownText.toStdString());
    auto parser = std::make_shared<maddy::Parser>();
    std::string htmlString = parser->Parse(markdownStream);

    // 2. 將 CSS 樣式表和轉換後的 HTML 內容組合起來
    QString fullHtml = css + QString::fromStdString(htmlString);

    // 3. 將最終生成的、帶有樣式的 HTML 顯示在右側預覽視窗
    m_preview->setHtml(fullHtml);

}

void MainWindow::setupFileActions()
{
    // 建立功能表
    QMenu *fileMenu = menuBar()->addMenu("檔案(&F)");
    // --- 建立 QAction ---
    // QAction 是 "指令" 的抽象，可以同時放在功能表和工具列中

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
/*
    toolbar->addAction(newAction);
    toolbar->addAction(openAction);
    toolbar->addAction(saveAction);
    toolbar->addAction(saveAsAction);
*/
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

void MainWindow::setupEditActions()
{
    QMenu *editMenu = menuBar()->addMenu("編輯(&E)");
    //QToolBar *toolbar = addToolBar("Edit");

    // 調整字體大小 - 這就是你寫的程式碼
    QAction *fontSizeAction = new QAction("字體大小", this);
    // 可以在這裡為 Action 設定一個圖示 (可選)
    // fontSizeAction->setIcon(QIcon(":/icons/font-size.png"));
    connect(fontSizeAction, &QAction::triggered, this, &MainWindow::FontSizeSet);

    editMenu->addAction(fontSizeAction);
    //toolbar->addAction(fontSizeAction);
}

void MainWindow::FontSizeSet()
{
    bool ok;
    // 彈出一個整數輸入對話方塊
    int newSize = QInputDialog::getInt(this,
                                       "設定字體大小",
                                       "請輸入新的字體大小:",
                                       m_currentFontSize, // 預設值
                                       8,               // 最小值
                                       72,              // 最大值
                                       1,               // 步長
                                       &ok);

    if (ok && newSize > 0) {
        m_currentFontSize = newSize;

        QFont font = m_editor->font();
        font.setPointSize(m_currentFontSize);  //使字體大小看起來一樣
        m_editor->setFont(font);

        // 1. 刪除舊的 highlighter 物件以避免内存洩漏
        delete m_highlighter;
        // 2. 用新的字體重新建立一個 highlighter
        m_highlighter = new MarkdownHighlighter(m_editor->document(), font);

        // 更新右側預覽區
        onTextChanged();
    }
}
