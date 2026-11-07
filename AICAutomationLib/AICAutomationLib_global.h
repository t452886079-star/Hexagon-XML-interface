#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#ifndef AICAUTOMATIONLIB_GLOBAL_H
#define AICAUTOMATIONLIB_GLOBAL_H

#include <QtCore/qglobal.h>

/**
 * @brief 动态库导出/导入宏定义
 *        编译 AICAutomationLib 时定义 AICAUTOMATIONLIB_LIBRARY，使用 Q_DECL_EXPORT；
 *        外部引用时未定义该宏，自动切换为 Q_DECL_IMPORT。
 */
#if defined(AICAUTOMATIONLIB_LIBRARY)
#  define AICAUTOMATIONLIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define AICAUTOMATIONLIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // AICAUTOMATIONLIB_GLOBAL_H
