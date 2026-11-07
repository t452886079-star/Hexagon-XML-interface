#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#ifndef AICLANGUAGEMANAGER_H
#define AICLANGUAGEMANAGER_H

#include <QObject>
#include <QTranslator>
#include <QMap>
#include "AICAutomationLib_global.h"

/**
 * @brief AIC 多语言管理器（单例）
 *        负责加载/切换翻译语言，对外提供 setLanguage() 接口。
 *
 *        使用方式：
 *          AicLanguageManager::instance()->setLanguage("zh_CN");  // 切换中文
 *          AicLanguageManager::instance()->setLanguage("en_US");  // 切换英文
 *          AicLanguageManager::instance()->setLanguage("");       // 恢复默认语言
 *
 *        翻译文件（.qm）默认从以下路径搜索：
 *          1. 可执行文件同级目录/translations/
 *          2. Qt 标准翻译路径
 *          3. 自定义路径（通过 setTranslationPath() 设置）
 */
class AICAUTOMATIONLIBSHARED_EXPORT AicLanguageManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 获取单例实例
     * @return AicLanguageManager 的唯一实例指针
     */
    static AicLanguageManager *instance();

    /**
     * @brief 设置当前语言，立即生效
     * @param locale 语言区域代码，如 "zh_CN"、"en_US"、"ja_JP"；
     *               传空字符串 "" 则卸载翻译，恢复默认语言
     * @return true 切换成功，false 未找到对应翻译文件
     */
    bool setLanguage(const QString &locale);

    /**
     * @brief 获取当前生效的语言区域代码
     * @return 当前 locale 字符串，空字符串表示默认语言
     */
    QString currentLanguage() const;

    /**
     * @brief 获取所有已注册的语言列表
     * @return locale 列表，如 ["en_US", "zh_CN"]
     */
    QStringList availableLanguages() const;

    /**
     * @brief 注册一个可选语言
     * @param locale      语言区域代码（如 "zh_CN"）
     * @param displayName 显示名称（如 "简体中文"）
     */
    void registerLanguage(const QString &locale, const QString &displayName);

    /**
     * @brief 设置翻译文件（.qm）的搜索路径
     * @param path 包含 .qm 文件的目录绝对路径
     */
    void setTranslationPath(const QString &path);

    /**
     * @brief 获取当前翻译文件搜索路径
     * @return 翻译文件目录路径
     */
    QString translationPath() const;

    /**
     * @brief 获取指定语言的显示名称
     * @param locale 语言区域代码
     * @return 显示名称，未注册时返回 locale 本身
     */
    QString languageDisplayName(const QString &locale) const;

signals:
    /**
     * @brief 语言切换完成信号，UI 可连接此信号刷新界面文字
     * @param locale 新的语言区域代码（空字符串表示默认语言）
     */
    void languageChanged(const QString &locale);

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit AicLanguageManager(QObject *parent = nullptr);

    /**
     * @brief 析构函数，清理翻译器
     */
    ~AicLanguageManager();

    Q_DISABLE_COPY(AicLanguageManager)

    static AicLanguageManager *s_instance;  ///< 单例静态实例指针
    QTranslator *m_translator;              ///< 当前加载的翻译器
    QString m_currentLocale;                ///< 当前语言区域代码
    QString m_translationPath;              ///< 翻译文件搜索路径
    QMap<QString, QString> m_languages;     ///< 已注册语言列表（locale -> 显示名称）
};

#endif // AICLANGUAGEMANAGER_H
