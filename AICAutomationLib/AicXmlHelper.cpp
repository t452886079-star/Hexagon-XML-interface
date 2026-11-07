#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#include "AicXmlHelper.h"
#include <QDomDocument>
#include <QDomElement>

// ─── XML 模板常量 ───────────────────────────────────────────────────────────

static const char *XML_HEAD = "<?xml version='1.0' encoding='utf-8'?>";
static const char *XML_QUERY_PART = "<BSExecQuery><QueryID>PartList</QueryID></BSExecQuery>";
static const char *XML_QUERY_INSPECTION = "<BSExecQuery><QueryID>InspectionList</QueryID><Parameters><Part>%1</Part></Parameters></BSExecQuery>";
static const char *XML_CMD_INSPECT = "<BSExecCommand><CommandID>Inspect</CommandID><Part>%1</Part><Inspection>%2</Inspection><CommandParameters><Comments>%3</Comments></CommandParameters></BSExecCommand>";
static const char *XML_CMD_INSPECT_NO_COMMENT = "<BSExecCommand><CommandID>Inspect</CommandID><Part>%1</Part><Inspection>%2</Inspection></BSExecCommand>";
static const char *XML_CMD_INSPECT_CONTINUE = "<BSExecCommand><CommandID>Inspect</CommandID><Part>%1</Part><Inspection>%2</Inspection><Result>%3</Result><CommandParameters><Comments>%4</Comments></CommandParameters></BSExecCommand>";
static const char *XML_CMD_INSPECT_CONTINUE_NO_COMMENT = "<BSExecCommand><CommandID>Inspect</CommandID><Part>%1</Part><Inspection>%2</Inspection><Result>%3</Result></BSExecCommand>";
static const char *XML_CMD_CORRECT = "<BSExecCommand><CommandID>Correct</CommandID><Part>%1</Part><Result>%2</Result></BSExecCommand>";
static const char *XML_CMD_CALIBRATE = "<BSExecCommand><CommandID>Calibrate</CommandID></BSExecCommand>";
static const char *XML_CMD_EXPORT_RBS = "<BSExecCommand><CommandID>ExportRBS</CommandID><Part>%1</Part><Result>%2</Result></BSExecCommand>";
static const char *XML_CMD_REPORT_WITH = "<BSExecCommand><CommandID>Report</CommandID><Part>%1</Part><Result>%2</Result><CommandParameters><ExportPath>%3</ExportPath></CommandParameters></BSExecCommand>";
static const char *XML_CMD_REPORT_WITHOUT = "<BSExecCommand><CommandID>Report</CommandID><Part>%1</Part><Result>%2</Result></BSExecCommand>";

/**
 * @brief 默认构造函数
 */
AicXmlHelper::AicXmlHelper() {}

// ─── XML 构建方法 ───────────────────────────────────────────────────────────

/**
 * @brief 构建零件列表查询 XML
 */
QString AicXmlHelper::formatPartsQuery()
{
    return QString(XML_HEAD) + XML_QUERY_PART;
}

/**
 * @brief 构建检测项列表查询 XML
 */
QString AicXmlHelper::formatInspectionsQuery(const QString &part)
{
    return QString(XML_HEAD) + QString(XML_QUERY_INSPECTION).arg(part);
}

/**
 * @brief 构建检测命令 XML（Inspect）
 *        comment 为空时不包含 CommandParameters 节点，避免设备报错"参数单元不包含任何条目"
 */
QString AicXmlHelper::formatInspectCmd(const QString &part, const QString &inspection, const QString &comment)
{
    if (comment.isEmpty())
        return QString(XML_HEAD) + QString(XML_CMD_INSPECT_NO_COMMENT).arg(part, inspection);
    else
        return QString(XML_HEAD) + QString(XML_CMD_INSPECT).arg(part, inspection, comment);
}

/**
 * @brief 构建带结果的检测命令 XML（Inspect-Continue）
 *        comment 为空时不包含 CommandParameters 节点
 */
QString AicXmlHelper::formatInspectContinueCmd(const QString &part, const QString &inspection,
                                                const QString &result, const QString &comment)
{
    if (comment.isEmpty())
        return QString(XML_HEAD) + QString(XML_CMD_INSPECT_CONTINUE_NO_COMMENT).arg(part, inspection, result);
    else
        return QString(XML_HEAD) + QString(XML_CMD_INSPECT_CONTINUE).arg(part, inspection, result, comment);
}

/**
 * @brief 构建修正命令 XML（Correct）
 */
QString AicXmlHelper::formatCorrectCmd(const QString &part, const QString &result)
{
    return QString(XML_HEAD) + QString(XML_CMD_CORRECT).arg(part, result);
}

/**
 * @brief 构建校准命令 XML（Calibrate）
 */
QString AicXmlHelper::formatCalibrateCmd()
{
    return QString(XML_HEAD) + XML_CMD_CALIBRATE;
}

/**
 * @brief 构建报告命令 XML（Report），可选导出路径
 */
QString AicXmlHelper::formatReportCmd(const QString &part, const QString &result, const QString &filename)
{
    if (filename.isEmpty())
        return QString(XML_HEAD) + QString(XML_CMD_REPORT_WITHOUT).arg(part, result);
    else
        return QString(XML_HEAD) + QString(XML_CMD_REPORT_WITH).arg(part, result, filename);
}

/**
 * @brief 构建导出 RBS 命令 XML（ExportRBS）
 */
QString AicXmlHelper::formatExportRBSCmd(const QString &part, const QString &result)
{
    return QString(XML_HEAD) + QString(XML_CMD_EXPORT_RBS).arg(part, result);
}

// ─── 响应解析方法 ───────────────────────────────────────────────────────────

/**
 * @brief 从响应 XML 的 Header 中提取请求类型标识
 */
QString AicXmlHelper::getRequestID(const QString &response)
{
    QDomDocument doc;
    if (!doc.setContent(response)) return QString();

    QDomElement root = doc.documentElement(); // <BSReturn>
    QDomElement header = root.firstChildElement("Header");
    if (header.isNull()) return QString();

    // 优先查 QueryID（查询响应）
    QDomElement queryEl = header.firstChildElement("BSExecQuery");
    if (!queryEl.isNull()) {
        QDomElement qid = queryEl.firstChildElement("QueryID");
        if (!qid.isNull()) return qid.text();
    }

    // 再查 CommandID（命令响应）
    QDomElement cmdEl = header.firstChildElement("BSExecCommand");
    if (!cmdEl.isNull()) {
        QDomElement cid = cmdEl.firstChildElement("CommandID");
        if (!cid.isNull()) return cid.text();
    }

    return QString();
}

/**
 * @brief 从响应 XML 中提取 ErrorCode 节点的值
 */
int AicXmlHelper::getRetCode(const QString &response)
{
    QDomDocument doc;
    if (!doc.setContent(response)) return -1;
    QDomElement el = doc.documentElement().firstChildElement("ErrorCode");
    if (!el.isNull()) return el.text().toInt();
    return -1;
}

/**
 * @brief 从响应 XML 中提取 Message 节点的文本
 */
QString AicXmlHelper::getRetMsg(const QString &response)
{
    QDomDocument doc;
    if (!doc.setContent(response)) return QString();
    QDomElement el = doc.documentElement().firstChildElement("Message");
    return el.isNull() ? QString() : el.text();
}

/**
 * @brief 检查 ReturnCode 节点是否为 "success"
 */
bool AicXmlHelper::cmdSuccessful(const QString &response)
{
    QDomDocument doc;
    if (!doc.setContent(response)) return false;
    QDomElement el = doc.documentElement().firstChildElement("ReturnCode");
    return el.text().startsWith("success");
}

/**
 * @brief 提取第一个 Object 节点下的 GUID 值
 */
QString AicXmlHelper::getResultGUID(const QString &response)
{
    QDomDocument doc;
    if (!doc.setContent(response)) return QString();
    QDomNodeList objects = doc.elementsByTagName("Object");
    if (objects.count() > 0) {
        QDomElement guid = objects.at(0).toElement().firstChildElement("GUID");
        return guid.isNull() ? QString() : guid.text();
    }
    return QString();
}

/**
 * @brief 提取第一个 Object 节点下的 ResultValue 数值
 */
double AicXmlHelper::getResultValue(const QString &response)
{
    QDomDocument doc;
    if (!doc.setContent(response)) return -1.0;
    QDomNodeList objects = doc.elementsByTagName("Object");
    if (objects.count() > 0) {
        QDomElement val = objects.at(0).toElement().firstChildElement("ResultValue");
        return val.isNull() ? -1.0 : val.text().toDouble();
    }
    return -1.0;
}

/**
 * @brief 提取第一个 Object 节点下的 MaxTolerance 数值
 *        对应手册 2.2.3.3.1：返回最大公差利用率（百分比）
 */
double AicXmlHelper::getMaxTolerance(const QString &response)
{
    QDomDocument doc;
    if (!doc.setContent(response)) return -1.0;
    QDomNodeList objects = doc.elementsByTagName("Object");
    if (objects.count() > 0) {
        QDomElement val = objects.at(0).toElement().firstChildElement("MaxTolerance");
        return val.isNull() ? -1.0 : val.text().toDouble();
    }
    return -1.0;
}

/**
 * @brief 提取第一个 Object 节点下的 IsCompleted 属性
 *        手册 2.2.3.3.1：
 *        - "yes" → 整根管件所有分量均完成测量（表22）
 *        - "no"  → 仅测量了部分管段（表23）
 */
bool AicXmlHelper::isCompleted(const QString &response)
{
    QDomDocument doc;
    if (!doc.setContent(response)) return false;
    QDomNodeList objects = doc.elementsByTagName("Object");
    if (objects.count() > 0) {
        QDomElement val = objects.at(0).toElement().firstChildElement("IsCompleted");
        if (val.isNull()) return false;
        return val.text().trimmed().toLower() == "yes";
    }
    return false;
}

/**
 * @brief 根据 ResultValue 分类公差水平（手册 表22 / 表23）
 *
 *        ResultValue 范围 [0, 4]：
 *        - [0, 1]  → WithinTolerance     (OK)
 *        - (1, 2]  → WithinIntervention  (OK，有偏差)
 *        - (2, 3]  → AboveConcession     (NOT OK，让步限内)
 *        - (3, 4]  → OutOfTolerance      (NOT OK)
 */
ToleranceLevel AicXmlHelper::classifyTolerance(double resultValue)
{
    if (resultValue <= 1.0)
        return ToleranceLevel::WithinTolerance;
    else if (resultValue <= 2.0)
        return ToleranceLevel::WithinIntervention;
    else if (resultValue <= 3.0)
        return ToleranceLevel::AboveConcession;
    else
        return ToleranceLevel::OutOfTolerance;
}

/**
 * @brief 公差水平枚举转可读字符串
 */
QString AicXmlHelper::toleranceLevelString(ToleranceLevel level)
{
    switch (level) {
    case ToleranceLevel::WithinTolerance:    return QStringLiteral("WithinTolerance");
    case ToleranceLevel::WithinIntervention: return QStringLiteral("WithinIntervention");
    case ToleranceLevel::AboveConcession:    return QStringLiteral("AboveConcession");
    case ToleranceLevel::OutOfTolerance:     return QStringLiteral("OutOfTolerance");
    }
    return QStringLiteral("Unknown");
}

/**
 * @brief 遍历所有 Object 节点，提取 (Name, GUID) 对作为零件列表
 */
QList<QPair<QString, QString>> AicXmlHelper::getPartList(const QString &response)
{
    QList<QPair<QString, QString>> parts;
    QDomDocument doc;
    if (!doc.setContent(response)) return parts;
    QDomNodeList objects = doc.elementsByTagName("Object");
    for (int i = 0; i < objects.count(); ++i) {
        QDomElement obj = objects.at(i).toElement();
        QString name = obj.firstChildElement("Name").text();
        QString guid = obj.firstChildElement("GUID").text();
        parts.append(qMakePair(name, guid));
    }
    return parts;
}

/**
 * @brief 遍历所有 Object 节点，提取 (Name, GUID) 对作为检测项列表
 */
QList<QPair<QString, QString>> AicXmlHelper::getInspectionList(const QString &response)
{
    QList<QPair<QString, QString>> inspections;
    QDomDocument doc;
    if (!doc.setContent(response)) return inspections;
    QDomNodeList objects = doc.elementsByTagName("Object");
    for (int i = 0; i < objects.count(); ++i) {
        QDomElement obj = objects.at(i).toElement();
        QString name = obj.firstChildElement("Name").text();
        QString guid = obj.firstChildElement("GUID").text();
        inspections.append(qMakePair(name, guid));
    }
    return inspections;
}

// ─── 测量结果完整解析（手册 2.2.3.3.1）──────────────────────────────────────

/**
 * @brief 解析 Inspect 命令返回的完整测量结果
 *
 *        响应结构示例（手册 2.2.3.4.2 第1条）：
 *        <BSReturn>
 *          <Header>...</Header>
 *          <ReturnCode>success</ReturnCode>
 *          <Payload>
 *            <Object>
 *              <Type>Result</Type>
 *              <Name>2019/09/02-11:29:42</Name>
 *              <GUID>1E8D0F7E-...</GUID>
 *              <Path>/parts/.../results/...obj</Path>
 *              <ResultValue>4.00</ResultValue>
 *              <MaxTolerance>1356.55</MaxTolerance>
 *              <IsCompleted>yes</IsCompleted>
 *              <Table>
 *                <Name>Result</Name>
 *                <Data>
 *                  <Row id="Component">
 *                    <Column id="CompName">PBRRComponent</Column>
 *                    <Column id="Length">1339.39</Column>
 *                    <Column id="LengthDev">18.62</Column>
 *                    <Column id="EndToEnd">511.23</Column>
 *                    <Column id="EndToEndDev">-5.40</Column>
 *                  </Row>
 *                  <Row id="SheathCriteria">
 *                    <Column id="CritName">Sheath deviation</Column>
 *                    <Column id="CompName">PBRRComponent</Column>
 *                    <Column id="StartType">CutInGage</Column>
 *                    <Column id="StartValue">7.31</Column>
 *                    <Column id="EndType">CutLength</Column>
 *                    <Column id="EndValue">-21.60</Column>
 *                  </Row>
 *                </Data>
 *              </Table>
 *            </Object>
 *          </Payload>
 *        </BSReturn>
 */
MeasurementResult AicXmlHelper::parseMeasurementResult(const QString &response)
{
    MeasurementResult result = {};
    result.resultValue  = -1.0;
    result.maxTolerance = -1.0;
    result.isCompleted  = false;
    result.toleranceLevel = ToleranceLevel::OutOfTolerance;

    QDomDocument doc;
    if (!doc.setContent(response))
        return result;

    QDomElement root = doc.documentElement();

    // 检查 ReturnCode
    QDomElement retCode = root.firstChildElement("ReturnCode");
    if (retCode.isNull() || retCode.text() != "success")
        return result;

    // 定位到 Payload > Object
    QDomElement payload = root.firstChildElement("Payload");
    if (payload.isNull())
        return result;

    QDomElement object = payload.firstChildElement("Object");
    if (object.isNull())
        return result;

    // 提取基础属性
    result.resultGUID = object.firstChildElement("GUID").text();
    result.resultName = object.firstChildElement("Name").text();
    result.resultPath = object.firstChildElement("Path").text();

    // ResultValue
    QDomElement rvEl = object.firstChildElement("ResultValue");
    if (!rvEl.isNull())
        result.resultValue = rvEl.text().toDouble();

    // MaxTolerance
    QDomElement mtEl = object.firstChildElement("MaxTolerance");
    if (!mtEl.isNull())
        result.maxTolerance = mtEl.text().toDouble();

    // IsCompleted
    QDomElement icEl = object.firstChildElement("IsCompleted");
    if (!icEl.isNull())
        result.isCompleted = (icEl.text().trimmed().toLower() == "yes");

    // 自动计算公差水平
    if (result.resultValue >= 0.0)
        result.toleranceLevel = classifyTolerance(result.resultValue);

    // 解析 Table > Data
    QDomElement table = object.firstChildElement("Table");
    if (table.isNull())
        return result;

    QDomElement data = table.firstChildElement("Data");
    if (data.isNull())
        return result;

    QDomNodeList rowNodes = data.childNodes();
    for (int i = 0; i < rowNodes.count(); ++i) {
        QDomElement rowEl = rowNodes.at(i).toElement();
        if (rowEl.tagName() != "Row")
            continue;

        QString rowId = rowEl.attribute("id");

        if (rowId == "Component") {
            // ── Component 行 ──
            ComponentRow comp = {};
            QDomNodeList cols = rowEl.childNodes();
            for (int j = 0; j < cols.count(); ++j) {
                QDomElement col = cols.at(j).toElement();
                if (col.tagName() != "Column") continue;
                QString id  = col.attribute("id");
                QString val = col.text();

                if (id == "CompName")       comp.compName    = val;
                else if (id == "Length")     comp.length      = val.toDouble();
                else if (id == "LengthDev")  comp.lengthDev   = val.toDouble();
                else if (id == "EndToEnd")   comp.endToEnd    = val.toDouble();
                else if (id == "EndToEndDev") comp.endToEndDev = val.toDouble();
            }
            result.components.append(comp);
        }
        else if (rowId == "SheathCriteria") {
            // ── SheathCriteria 行 ──
            SheathCriteriaRow sc = {};
            QDomNodeList cols = rowEl.childNodes();
            for (int j = 0; j < cols.count(); ++j) {
                QDomElement col = cols.at(j).toElement();
                if (col.tagName() != "Column") continue;
                QString id  = col.attribute("id");
                QString val = col.text();

                if (id == "CritName")       sc.critName   = val;
                else if (id == "CompName")  sc.compName   = val;
                else if (id == "StartType") sc.startType  = val;
                else if (id == "StartValue") sc.startValue = val.toDouble();
                else if (id == "EndType")   sc.endType    = val;
                else if (id == "EndValue")  sc.endValue   = val.toDouble();
            }
            result.sheathCriteria.append(sc);
        }
    }

    return result;
}

// ─── Correct 响应解析（手册 2.2.3.3.2 / 2.2.3.4.2）─────────────────────────

/**
 * @brief 解析 Return of command Correct 的响应 XML
 *
 *        提取 Payload 中 Correction Table 的所有行数据
 *        （BE, Push, PushDev, Bend, BendDev, Rotate, RotateDev, Radius, RadiusDev）
 */
QVector<CorrectionRow> AicXmlHelper::parseCorrectResponse(const QString &response)
{
    QVector<CorrectionRow> rows;

    QDomDocument doc;
    if (!doc.setContent(response))
        return rows;

    // 1. 检查 ReturnCode 是否为 success
    QDomElement root = doc.documentElement();
    QDomElement retCode = root.firstChildElement("ReturnCode");
    if (retCode.isNull() || retCode.text() != "success")
        return rows;

    // 2. 定位到 Payload > Object > Table > Data
    QDomElement payload = root.firstChildElement("Payload");
    if (payload.isNull())
        return rows;

    QDomElement object = payload.firstChildElement("Object");
    if (object.isNull())
        return rows;

    QDomElement table = object.firstChildElement("Table");
    if (table.isNull())
        return rows;

    QDomElement data = table.firstChildElement("Data");
    if (data.isNull())
        return rows;

    // 3. 遍历所有 Row 节点
    QDomNodeList rowNodes = data.childNodes();
    for (int i = 0; i < rowNodes.count(); ++i) {
        QDomElement rowEl = rowNodes.at(i).toElement();
        if (rowEl.tagName() != "Row")
            continue;

        CorrectionRow row = {};

        // 遍历该行的所有 Column 节点
        QDomNodeList cols = rowEl.childNodes();
        for (int j = 0; j < cols.count(); ++j) {
            QDomElement col = cols.at(j).toElement();
            if (col.tagName() != "Column")
                continue;

            QString id  = col.attribute("id");
            QString val = col.text();

            if (id == "BE")         row.be        = val.toInt();
            else if (id == "Push")      row.push      = val.toDouble();
            else if (id == "PushDev")   row.pushDev   = val.toDouble();
            else if (id == "Bend")      row.bend      = val.toDouble();
            else if (id == "BendDev")   row.bendDev   = val.toDouble();
            else if (id == "Rotate")    row.rotate    = val.toDouble();
            else if (id == "RotateDev") row.rotateDev = val.toDouble();
            else if (id == "Radius")    row.radius    = val.toDouble();
            else if (id == "RadiusDev") row.radiusDev = val.toDouble();
        }

        rows.append(row);
    }

    return rows;
}
