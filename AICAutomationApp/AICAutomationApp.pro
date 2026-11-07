QT       += core gui widgets network
TARGET    = AICAutomation
TEMPLATE  = app
CONFIG   += c++11

SOURCES += main.cpp

# Link against the library
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../AICAutomationLib/release/ -lAICAutomationLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../AICAutomationLib/debug/ -lAICAutomationLib
else:unix: LIBS += -L$$OUT_PWD/../AICAutomationLib/ -lAICAutomationLib

INCLUDEPATH += $$PWD/../AICAutomationLib
DEPENDPATH  += $$PWD/../AICAutomationLib
