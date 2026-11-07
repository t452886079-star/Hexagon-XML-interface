#include "AicAutomation.h"

// 获取语言管理器实例
auto *langMgr = AicLanguageManager::instance();

// 注册语言（可选，已预注册 en_US 和 zh_CN）
langMgr->registerLanguage("ja_JP", "日本語");

// 设置翻译文件搜索路径（默认: 可执行文件目录/translations/）
langMgr->setTranslationPath("/path/to/translations");

// 切换语言
langMgr->setLanguage("zh_CN");   // 切到中文
langMgr->setLanguage("en_US");   // 切到英文
langMgr->setLanguage("");        // 恢复默认

// 监听语言变化（刷新 UI）
connect(langMgr, &AicLanguageManager::languageChanged,
        [](const QString &locale) {
            // 重新设置 UI 文字，或直接重建界面
        });

// 查询可用语言
QStringList langs = langMgr->availableLanguages();
QString displayName = langMgr->languageDisplayName("zh_CN");
