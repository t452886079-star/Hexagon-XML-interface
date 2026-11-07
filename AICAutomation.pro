TEMPLATE = subdirs
SUBDIRS += \
    AICAutomationLib \
    AICAutomationApp

AICAutomationApp.depends = AICAutomationLib
