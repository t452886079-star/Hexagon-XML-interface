# AICAutomation 项目结构

```
AICAutomation/
├── AICAutomation.pro                  # 顶层 subdirs 项目
├── AICAutomation.sln                  # VS2019 解决方案
├── .gitignore
├── STRUCTURE.md                       # 本文件
├── Manual_Automation_EN.pdf           # BendingStudio XT 接口手册
│
├── AICAutomationLib/                  # 核心库 (DLL)
│   ├── AICAutomationLib.pro           # 库项目文件
│   ├── AICAutomationLib_global.h      # DLL 导出/导入宏
│   ├── AicTypes.h                     # 数据结构定义 (CorrectionRow, MeasurementResult 等)
│   ├── AicAutomation.h               # 总括头文件 (一键包含全部公共接口)
│   │
│   ├── AicConnectionManager.h         # HTTP 通信层 (连接/重连/心跳)
│   ├── AicConnectionManager.cpp
│   │
│   ├── AicXmlHelper.h                 # XML 构建 + 响应解析
│   ├── AicXmlHelper.cpp
│   │
│   ├── AicAutomationSingleton.h       # 全局单例入口 (连接管理 + 业务命令)
│   ├── AicAutomationSingleton.cpp
│   │
│   ├── AicMainWindow.h               # Qt GUI 主窗口
│   ├── AicMainWindow.cpp
│   │
│   ├── AicLanguageManager.h           # 多语言管理器 (单例)
│   ├── AicLanguageManager.cpp
│   │
│   └── translations/                  # 翻译文件
│       ├── AICAutomationLib_en_US.ts
│       └── AICAutomationLib_zh_CN.ts
│
└── AICAutomationApp/                  # 可执行程序
    ├── AICAutomationApp.pro           # App 项目文件
    └── main.cpp                       # 程序入口
```

## 模块依赖关系

```
AicAutomation (总括头)
  ├── AICAutomationLib_global.h
  ├── AicTypes.h
  ├── AicLanguageManager.h
  ├── AicXmlHelper.h  → AicTypes.h
  ├── AicConnectionManager.h
  ├── AicAutomationSingleton.h → AicTypes.h, AicConnectionManager(前向声明)
  └── AicMainWindow.h → AicXmlHelper.h, AicConnectionManager(前向声明)

main.cpp → AicAutomation.h
```

## 信号流

```
AicConnectionManager::responseReceived(tag, body, httpCode)
  ├→ AicMainWindow::onResponseReceived → autoExtractFromResponse
  │    ├→ PartList        → partListReceived
  │    ├→ InspectionList  → inspectionListReceived
  │    ├→ Inspect         → inspectResult + inspectMeasurementResult
  │    ├→ Correct         → correctResult + m_correctionRows 缓存
  │    └→ 其他命令        → 对应结果信号
  ├→ AicAutomationSingleton (lambda) → Correct 自动解析 → correctResult
  └→ AicAutomationSingleton::responseReceived 转发 (原始)
```
