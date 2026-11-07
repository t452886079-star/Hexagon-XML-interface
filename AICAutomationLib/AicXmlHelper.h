#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#ifndef AICXMLHELPER_H
#define AICXMLHELPER_H

#include <QString>
#include <QPair>
#include <QList>
#include <QVector>
#include "AICAutomationLib_global.h"
#include "AicTypes.h"

// ─── AicXmlHelper ──────────────────────────────────────────────────────────

/**
 * @brief AIC XML 辅助工具类
 *        提供构建 AIC 设备查询/命令 XML 的静态方法，
 *        以及解析设备响应 XML 的工具函数。
 */
class AICAUTOMATIONLIBSHARED_EXPORT AicXmlHelper
{
public:
    /**
     * @brief 默认构造函数
     */
    AicXmlHelper();

    // ─── XML 构建 ─────────────────────────────────────────────────────────

    /**
     * @brief 构建零件列表查询 XML
     * @return 完整的 PartList 查询 XML 字符串
     */
    static QString formatPartsQuery();

    /**
     * @brief 构建指定零件的检测项列表查询 XML
     * @param part 零件 GUID
     * @return 完整的 InspectionList 查询 XML 字符串
     */
    static QString formatInspectionsQuery(const QString &part);

    /**
     * @brief 构建检测命令 XML（Inspect）
     * @param part       零件 GUID
     * @param inspection 检测项 GUID
     * @param comment    备注信息
     * @return 完整的 Inspect 命令 XML 字符串
     */
    static QString formatInspectCmd(const QString &part, const QString &inspection, const QString &comment);

    /**
     * @brief 构建带结果的检测命令 XML（Inspect-Continue）
     * @param part       零件 GUID
     * @param inspection 检测项 GUID
     * @param result     检测结果 GUID
     * @param comment    备注信息
     * @return 完整的 InspectContinue 命令 XML 字符串
     */
    static QString formatInspectContinueCmd(const QString &part, const QString &inspection,
                                            const QString &result, const QString &comment);

    /**
     * @brief 构建修正命令 XML（Correct）
     * @param part   零件 GUID
     * @param result 结果 GUID
     * @return 完整的 Correct 命令 XML 字符串
     */
    static QString formatCorrectCmd(const QString &part, const QString &result);

    /**
     * @brief 构建校准命令 XML（Calibrate）
     * @return 完整的 Calibrate 命令 XML 字符串
     */
    static QString formatCalibrateCmd();

    /**
     * @brief 构建报告命令 XML（Report），可选导出路径
     * @param part       零件 GUID
     * @param result     结果 GUID
     * @param filename   导出文件路径（为空则不带 ExportPath 参数）
     * @return 完整的 Report 命令 XML 字符串
     */
    static QString formatReportCmd(const QString &part, const QString &result, const QString &filename = QString());

    /**
     * @brief 构建导出 RBS 命令 XML（ExportRBS）
     * @param part   零件 GUID
     * @param result 结果 GUID
     * @return 完整的 ExportRBS 命令 XML 字符串
     */
    static QString formatExportRBSCmd(const QString &part, const QString &result);

    // ─── 响应解析 ─────────────────────────────────────────────────────────

    /**
     * @brief 从响应 XML 的 Header 中提取请求类型标识
     *        查询响应返回 QueryID（如 "PartList"、"InspectList"）
     *        命令响应返回 CommandID（如 "Inspect"、"Correct"、"Calibrate"）
     * @param response 设备响应 XML 字符串
     * @return 请求类型字符串，解析失败返回空字符串
     */
    static QString getRequestID(const QString &response);

    /**
     * @brief 从响应 XML 中提取错误码
     * @param response 设备响应 XML 字符串
     * @return 错误码（-1 表示解析失败）
     */
    static int getRetCode(const QString &response);

    /**
     * @brief 从响应 XML 中提取消息文本
     * @param response 设备响应 XML 字符串
     * @return 消息内容，解析失败返回空字符串
     */
    static QString getRetMsg(const QString &response);

    /**
     * @brief 判断命令是否执行成功（ReturnCode == "success"）
     * @param response 设备响应 XML 字符串
     * @return true 成功，false 失败或解析失败
     */
    static bool cmdSuccessful(const QString &response);

    /**
     * @brief 从响应 XML 中提取第一个 Object 的 GUID
     * @param response 设备响应 XML 字符串
     * @return GUID 字符串，未找到则返回空字符串
     */
    static QString getResultGUID(const QString &response);

    /**
     * @brief 从响应 XML 中提取第一个 Object 的 ResultValue
     * @param response 设备响应 XML 字符串
     * @return 结果数值（double），解析失败返回 -1.0
     */
    static double getResultValue(const QString &response);

    /**
     * @brief 从响应 XML 中提取第一个 Object 的 MaxTolerance
     * @param response 设备响应 XML 字符串
     * @return 最大公差利用率（%），解析失败返回 -1.0
     */
    static double getMaxTolerance(const QString &response);

    /**
     * @brief 从响应 XML 中提取第一个 Object 的 IsCompleted 属性
     *        手册 2.2.3.3.1：
     *        - "yes" 表示整根管件所有分量均完成测量（表22）
     *        - "no"  表示仅测量了部分管段（表23）
     * @param response 设备响应 XML 字符串
     * @return true = yes，false = no 或解析失败
     */
    static bool isCompleted(const QString &response);

    /**
     * @brief 根据 ResultValue 判断公差水平，对应手册 表22/表23
     *        - [0, 1]  → WithinTolerance     (OK)
     *        - (1, 2]  → WithinIntervention  (OK，有偏差)
     *        - (2, 3]  → AboveConcession     (NOT OK，让步限内)
     *        - (3, 4]  → OutOfTolerance      (NOT OK)
     * @param resultValue 测量结果值 [0.0, 4.0]
     * @return 公差水平枚举
     */
    static ToleranceLevel classifyTolerance(double resultValue);

    /**
     * @brief 将公差水平枚举转为可读字符串
     * @param level 公差水平枚举
     * @return 如 "WithinTolerance"、"OutOfTolerance" 等
     */
    static QString toleranceLevelString(ToleranceLevel level);

    /**
     * @brief 从响应 XML 中提取零件列表（Name-GUID 对）
     * @param response 设备响应 XML 字符串
     * @return 零件列表，每个元素为 (Name, GUID) 对
     */
    static QList<QPair<QString, QString>> getPartList(const QString &response);

    /**
     * @brief 从响应 XML 中提取检测项列表（Name-GUID 对）
     * @param response 设备响应 XML 字符串
     * @return 检测项列表，每个元素为 (Name, GUID) 对
     */
    static QList<QPair<QString, QString>> getInspectionList(const QString &response);

    // ─── 测量结果完整解析（手册 2.2.3.3.1）────────────────────────────────

    /**
     * @brief 解析 Inspect 命令返回的完整测量结果
     *
     *        提取 ResultValue、MaxTolerance、IsCompleted 以及
     *        Component / SheathCriteria 表格数据，并自动计算 toleranceLevel。
     *
     *        IsCompleted = YES 时，数据代表整根管件（表22）；
     *        IsCompleted = NO  时，数据仅代表已测量的部分管段（表23）。
     *
     * @param response 设备响应 XML 字符串
     * @return MeasurementResult 结构体，解析失败时 resultValue = -1.0
     */
    static MeasurementResult parseMeasurementResult(const QString &response);

    // ─── Correct 响应解析（手册 2.2.3.3.2 / 2.2.3.4.2）───────────────────

    /**
     * @brief 解析 Return of command Correct 的响应 XML
     *        提取 Payload 中 Correction Table 的所有行数据
     *        （BE, Push, PushDev, Bend, BendDev, Rotate, RotateDev, Radius, RadiusDev）
     *
     * @param response  设备响应 XML 字符串
     * @return 修正数据行列表，解析失败返回空列表
     */
    static QVector<CorrectionRow> parseCorrectResponse(const QString &response);
};

#endif // AICXMLHELPER_H
