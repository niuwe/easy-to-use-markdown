QT       += core gui webenginewidgets

# 針對 MSVC 編譯器，強制使用 UTF-8 編碼
win32-msvc {
    QMAKE_CXXFLAGS += /utf-8
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


CONFIG += c++14  # <-- 新增這一行，啟用 C++14 標準
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    markdownhighlighter.cpp

HEADERS += \
    maddy/blockparser.h \
    maddy/breaklineparser.h \
    maddy/checklistparser.h \
    maddy/codeblockparser.h \
    maddy/emphasizedparser.h \
    maddy/headlineparser.h \
    maddy/horizontallineparser.h \
    maddy/htmlparser.h \
    maddy/imageparser.h \
    maddy/inlinecodeparser.h \
    maddy/italicparser.h \
    maddy/latexblockparser.h \
    maddy/lineparser.h \
    maddy/linkparser.h \
    maddy/orderedlistparser.h \
    maddy/paragraphparser.h \
    maddy/parser.h \
    maddy/parserconfig.h \
    maddy/quoteparser.h \
    maddy/strikethroughparser.h \
    maddy/strongparser.h \
    maddy/tableparser.h \
    maddy/unorderedlistparser.h \
    mainwindow.h \
    markdownhighlighter.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD

DISTFILES +=

RESOURCES += \
    resouces.qrc
