#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#ifndef AICTYPES_H
#define AICTYPES_H

#include <QString>
#include <QVector>

// ─── 公差水平枚举（手册 表22 / 表23）────────────────────────────────────────

/**
 * @brief 公差水平分类，对应手册 2.4.1 Result evaluation 表22/表23
 *
 *        ResultValue 范围 [0, 4]，数值越小越好：
 *        - [0, 1]  WithinTolerance    → 公差范围内，OK
 *        - (1, 2]  WithinIntervention → 干预限内，OK 但有偏差
 *        - (2, 3]  AboveConcession    → 超出公差但在让步限内，NOT OK
 *        - (3, 4]  OutOfTolerance     → 超出公差范围，NOT OK
 *
 *        IsCompleted = YES  时：评估整根管件所有分量（表22）
 *        IsCompleted = NO   时：仅评估已测量的部分管段（表23）
 */
enum class ToleranceLevel
{
    WithinTolerance,     ///< ResultValue ∈ [0, 1]，MaxTolerance ∈ [0, intervention limit]
    WithinIntervention,  ///< ResultValue ∈ (1, 2]，MaxTolerance ∈ (intervention limit, 100%]
    AboveConcession,     ///< ResultValue ∈ (2, 3]，MaxTolerance ∈ (100%, concession limit]
    OutOfTolerance       ///< ResultValue ∈ (3, 4]，MaxTolerance ∈ (concession limit, +∞]
};

// ─── 测量结果数据结构（手册 2.2.3.3.1）─────────────────────────────────────

/**
 * @brief 管件分量数据行，对应手册 表18 RowType - Component
 */
struct ComponentRow
{
    QString compName;     ///< 分量名称 (CompName)
    double  length;       ///< 展开长度 [mm] (Length)
    double  lengthDev;    ///< 展开长度偏差 [mm] (LengthDev)
    double  endToEnd;     ///< 端到端距离 [mm] (EndToEnd)
    double  endToEndDev;  ///< 端到端距离偏差 [mm] (EndToEndDev)
};

/**
 * @brief 护套判定准则数据行，对应手册 表18 RowType - SheathCriteria
 */
struct SheathCriteriaRow
{
    QString critName;    ///< 判定准则名称 (CritName)
    QString compName;    ///< 分量名称 (CompName)
    QString startType;   ///< 起始类型：CutLength 或 CutInGage (StartType)
    double  startValue;  ///< 起始切割值 [mm] (StartValue)
    QString endType;     ///< 结束类型：CutLength 或 CutInGage (EndType)
    double  endValue;    ///< 结束切割值 [mm] (EndValue)
};

/**
 * @brief 完整测量结果，对应手册 2.2.3.3.1 Measurement Result
 *
 *        包含 ResultValue、MaxTolerance、IsCompleted 以及
 *        Component / SheathCriteria 表格数据。
 *
 *        IsCompleted = YES 时，数据代表整根管件（表22）；
 *        IsCompleted = NO  时，数据仅代表已测量的部分管段（表23）。
 */
struct MeasurementResult
{
    QString resultGUID;                   ///< 结果 GUID
    QString resultName;                   ///< 结果名称
    QString resultPath;                   ///< 结果路径
    double  resultValue;                  ///< 测量值 [0.0, 4.0]
    double  maxTolerance;                 ///< 最大公差利用率 [%]
    bool    isCompleted;                  ///< 是否所有分量均完成测量
    ToleranceLevel toleranceLevel;        ///< 公差水平分类（自动填充）
    QVector<ComponentRow>      components;       ///< 分量数据列表
    QVector<SheathCriteriaRow> sheathCriteria;   ///< 护套判定准则列表
};

// ─── 修正数据结构（手册 2.2.3.4.2）─────────────────────────────────────────

/**
 * @brief 修正数据行结构体，对应手册 2.2.3.4.2 中 Correction Table 的一行
 */
struct CorrectionRow
{
    int    be;          ///< 弯曲点编号 (BE)
    double push;        ///< 推进值 (Push)
    double pushDev;     ///< 推进偏差 (PushDev)
    double bend;        ///< 弯曲角度 (Bend)
    double bendDev;     ///< 弯曲偏差 (BendDev)
    double rotate;      ///< 旋转角度 (Rotate)
    double rotateDev;   ///< 旋转偏差 (RotateDev)
    double radius;      ///< 弯曲半径 (Radius)
    double radiusDev;   ///< 半径偏差 (RadiusDev)
};

#endif // AICTYPES_H
