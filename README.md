# AICAutomation

Qt/C++ 上位机接口库，用于与**海克斯康（Hexagon）测量箱**进行 XML 通讯，对接其 BendingStudio XT / Automation 接口。请求与响应均为 XML 报文，底层以 HTTP POST 承载；库内封装了连接管理、命令构造、响应解析，并提供一个可直接嵌入宿主程序的 Qt 界面面板。

库以动态链接库（`AICAutomationLib.dll` / `.lib`）形式交付，对外只需包含 `AicAutomation.h` 一个头文件。

---

## 通讯方式

```
AICAutomation（Qt 上位机）
   │  HTTP POST  /  Content-Type: application/xml
   │  ← AicXmlHelper 构造请求 XML
   ▼
Hexagon 测量箱 XML 接口（默认端口 32459）
   │  响应 XML
   ▼
AicXmlHelper 解析 → Qt 信号 → 业务 / UI
```

网络操作全部异步（`QNetworkAccessManager`），配合心跳探测与断线重连，不阻塞 UI 线程。报文格式、命令与字段定义以仓库内的官方接口手册为准。

---

## 功能

| 模块 | 说明 |
| --- | --- |
| `AicConnectionManager` | 基于 `QNetworkAccessManager` 的异步 HTTP 通信，心跳探测、断线指数退避重连（上限 30 s）、区分主动断开与异常掉线 |
| `AicXmlHelper` | 构造查询/命令 XML，解析响应 XML，全部为静态方法 |
| `AicAutomationSingleton` | 全局单例入口，封装高层业务命令并转发结果信号 |
| `AicMainWindow` | 现成的 Qt 主窗口面板，可嵌入 `QWidget` 或作为独立窗口弹出 |
| `AicLanguageManager` | 中英文界面切换（`zh_CN` / `en_US`） |

支持的命令（对应官方手册）：`PartList`、`InspectionList`、`Inspect`、`Inspect-Continue`、`Correct`、`Calibrate`、`Report`、`ExportRBS`。也支持通过 `sendRequest()` 透传原始 XML。

响应解析已覆盖：

- **测量结果**：`ResultValue`、`MaxTolerance`、`IsCompleted`、`Component` 与 `SheathCriteria` 表格；按 `[0,4]` 区间自动划分 `WithinTolerance` / `WithinIntervention` / `AboveConcession` / `OutOfTolerance` 四级。
- **修正表**：`BE`、`Push`、`Bend`、`Rotate`、`Radius` 及各自偏差值。
- 通用字段：`QueryID` / `CommandID`、`ReturnCode`、`RetMsg`、结果 GUID。

---

## 环境要求

- Qt 5（`core gui widgets network xml`），C++11
- MSVC 2019 / Visual Studio 2019（x64），或 qmake + nmake
- 源码为 **UTF-8 with BOM**；MSVC 需开启 `/utf-8`（`.pro` 中已配置），否则中文会乱码

---

## 构建

**qmake**

```bash
qmake AICAutomation.pro
nmake                 # MSVC；MinGW 用 mingw32-make
```

只构建库：

```bash
qmake AICAutomationLib/AICAutomationLib.pro
nmake
```

**Visual Studio**

打开 `AICAutomation.sln`，选择 `x64 / Release` 生成解决方案。顶层 `.pro` 是 `subdirs` 工程，`AICAutomationApp` 依赖 `AICAutomationLib`，顺序已配置。

产物：`AICAutomationLib.dll`、`AICAutomationLib.lib` 和 `AICAutomationApp.exe`。运行/发布时把 DLL 与可执行文件放在同一目录。

---

## 集成

只需三样东西：`AicAutomation.h`（唯一头文件）、`AICAutomationLib.lib`（链接）、`AICAutomationLib.dll`（运行时）。

```cpp
#include "AicAutomation.h"

auto *aic = AicAutomationSingleton::instance();
aic->connectToHost("192.168.1.100", 32459);
aic->queryPartList();                       // 零件列表
aic->queryInspectionList(partGuid);         // 检测项列表
aic->commandInspect(partGuid, inspGuid);    // 触发检测
aic->commandCorrect(partGuid, resultGuid);  // 获取修正表
```

结果通过 Qt 信号异步返回，不会阻塞 UI：

```cpp
QObject::connect(aic, &AicAutomationSingleton::responseReceived,
                 [](const QString &tag, const QString &body, int httpCode) {
                     // 原始 XML；可用 AicXmlHelper 的静态方法解析
                 });
```

嵌入现成界面面板：

```cpp
AicMainWindow *panel = new AicMainWindow(centralWidget);
layout->addWidget(panel);
```

或作为独立窗口弹出（见 `example/example02.cpp`）。更多片段见 `example/` 目录。

---

## 连接配置

- 与 Hexagon 测量箱的 XML 接口通讯，默认端口 **32459**，请求地址 `http://<Host>:<Port>/`，`Content-Type: application/xml`。
- GUI 会从可执行文件同目录读写 `HostPort.ini`：

```ini
[Connection]
Host=192.168.1.12
Port=32459
```

`HostPort.ini` 含现场设备地址，已加入 `.gitignore`，不纳入版本库。

---

## 目录结构

```
AICAutomation/
├── AICAutomation.pro / AICAutomation.sln   # 顶层 subdirs / VS 解决方案
├── AICAutomationLib/                       # 核心 DLL
│   ├── AicAutomation.h                     # 总括头（对外唯一入口）
│   ├── AicTypes.h                          # 数据结构与公差级别枚举
│   ├── AicConnectionManager.*              # HTTP 通信、心跳、重连
│   ├── AicXmlHelper.*                      # XML 构建与响应解析
│   ├── AicAutomationSingleton.*            # 单例 + 高层命令
│   ├── AicMainWindow.*                     # Qt 界面面板
│   ├── AicLanguageManager.*                # 多语言
│   └── translations/                       # zh_CN / en_US 翻译文件
├── AICAutomationApp/                       # 可执行程序（main.cpp）
├── example/                                # 集成示例代码片段
├── STRUCTURE.md                            # 模块依赖与信号流
└── 编程.md                                 # 项目编码规范
```

完整的模块依赖关系与信号流见 [STRUCTURE.md](STRUCTURE.md)。

---

## 参考文档

| 文件 | 内容 |
| --- | --- |
| `Manual_Automation_EN.pdf` | 官方 Automation 接口手册（英文） |
| `BendingStudio_XT_接口手册_中文版.pdf` | 接口手册中文版 |
| `AICAutomationLib_DLL使用说明.docx` | 动态库集成说明 |
| `AICAutomation_检测报告.docx` | 检测报告 |

> 上述 PDF / DOCX 为海克斯康（Hexagon）发布的接口参考文档，版权归原发布方所有，仅用于集成开发参考。

---

## 许可证

[MIT](LICENSE) © 2026 t452886079-star
