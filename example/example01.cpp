#include "AicAutomation.h"

auto *aic = AicAutomationSingleton::instance();
aic->connectToHost("192.168.1.100", 32459);
aic->queryPartList();
