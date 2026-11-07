#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include <QApplication>
#include "AicAutomation.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("AIC Automation");
	
	// 获取语言管理器实例
	auto *langMgr = AicLanguageManager::instance();
	langMgr->setLanguage("zh_CN");   // 切到中文
    AicMainWindow w;
    w.show();

    return a.exec();
}
