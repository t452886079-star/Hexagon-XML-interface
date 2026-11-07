QT       += core gui widgets network xml
TARGET    = AICAutomationLib
TEMPLATE  = lib
DEFINES  += AICAUTOMATIONLIB_LIBRARY

CONFIG   += c++11

# MSVC UTF-8 编译支持（解决中文乱码）
msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    AicXmlHelper.cpp \
    AicConnectionManager.cpp \
    AicMainWindow.cpp \
    AicAutomationSingleton.cpp \
    AicLanguageManager.cpp

HEADERS += \
    AICAutomationLib_global.h \
    AicTypes.h \
    AicAutomation.h \
    AicXmlHelper.h \
    AicConnectionManager.h \
    AicMainWindow.h \
    AicAutomationSingleton.h \
    AicLanguageManager.h

TRANSLATIONS += \
    translations/AICAutomationLib_en_US.ts \
    translations/AICAutomationLib_zh_CN.ts
