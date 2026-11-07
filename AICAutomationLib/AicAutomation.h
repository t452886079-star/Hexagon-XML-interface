#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#ifndef AICAUTOMATION_H
#define AICAUTOMATION_H

/**
 * @brief AIC Automation 库总括头文件
 *        使用方只需 #include "AicAutomation.h" 即可引入库的全部公共接口，
 *        包括全局宏定义、XML 辅助工具、连接管理器、单例入口和主窗口。
 */
#include "AICAutomationLib_global.h"
#include "AicLanguageManager.h"
#include "AicXmlHelper.h"
#include "AicConnectionManager.h"
#include "AicAutomationSingleton.h"
#include "AicMainWindow.h"

#endif // AICAUTOMATION_H
