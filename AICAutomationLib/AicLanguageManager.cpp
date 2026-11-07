#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#include "AicLanguageManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

// 静态单例实例初始化
AicLanguageManager *AicLanguageManager::s_instance = nullptr;

/**
 * @brief 获取单例实例，首次调用时自动创建，并注册默认语言
 */
AicLanguageManager *AicLanguageManager::instance()
{
    if (!s_instance) {
        s_instance = new AicLanguageManager(qApp);
        // 注册默认支持的语言
        s_instance->registerLanguage("en_US", "English");
        s_instance->registerLanguage("zh_CN", QString::fromUtf8("\xe7\xae\x80\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87"));
    }
    return s_instance;
}

/**
 * @brief 私有构造函数：初始化翻译器和默认翻译路径
 */
AicLanguageManager::AicLanguageManager(QObject *parent)
    : QObject(parent)
    , m_translator(new QTranslator(this))
    , m_translationPath(QCoreApplication::applicationDirPath() + QDir::separator() + "translations")
{
}

/**
 * @brief 析构函数，卸载并删除翻译器
 */
AicLanguageManager::~AicLanguageManager()
{
    if (m_translator) {
        qApp->removeTranslator(m_translator);
        delete m_translator;
        m_translator = nullptr;
    }
}

/**
 * @brief 设置语言：卸载旧翻译 → 加载新 .qm → 安装到应用 → 发射信号
 */
bool AicLanguageManager::setLanguage(const QString &locale)
{
    // 如果要切换的语言和当前相同，直接返回
    if (locale == m_currentLocale)
        return true;

    // 卸载当前翻译器
    qApp->removeTranslator(m_translator);

    // 传空则恢复默认语言
    if (locale.isEmpty()) {
        m_currentLocale.clear();
        emit languageChanged(m_currentLocale);
        return true;
    }

    // 搜索 .qm 文件：先查自定义路径，再查应用目录
    QString qmFile = m_translationPath + QDir::separator()
                     + QString("AICAutomationLib_%1.qm").arg(locale);

    if (!QFileInfo::exists(qmFile)) {
        // 备选：应用目录下直接搜索
        qmFile = QCoreApplication::applicationDirPath() + QDir::separator()
                 + QString("AICAutomationLib_%1.qm").arg(locale);
    }

    if (!QFileInfo::exists(qmFile)) {
        // 文件不存在，保持原语言
        return false;
    }

    // 加载并安装翻译
    if (!m_translator->load(qmFile)) {
        return false;
    }

    qApp->installTranslator(m_translator);
    m_currentLocale = locale;
    emit languageChanged(m_currentLocale);
    return true;
}

/**
 * @brief 返回当前语言区域代码
 */
QString AicLanguageManager::currentLanguage() const
{
    return m_currentLocale;
}

/**
 * @brief 返回所有已注册的语言列表
 */
QStringList AicLanguageManager::availableLanguages() const
{
    return m_languages.keys();
}

/**
 * @brief 注册一个可选语言到内部映射表
 */
void AicLanguageManager::registerLanguage(const QString &locale, const QString &displayName)
{
    m_languages.insert(locale, displayName);
}

/**
 * @brief 设置翻译文件的搜索目录
 */
void AicLanguageManager::setTranslationPath(const QString &path)
{
    m_translationPath = path;
}

/**
 * @brief 返回当前翻译文件搜索路径
 */
QString AicLanguageManager::translationPath() const
{
    return m_translationPath;
}

/**
 * @brief 根据 locale 获取显示名称，未注册则原样返回 locale
 */
QString AicLanguageManager::languageDisplayName(const QString &locale) const
{
    return m_languages.value(locale, locale);
}
