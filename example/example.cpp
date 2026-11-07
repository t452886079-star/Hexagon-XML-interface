#include "AicAutomation.h"

// 在你的窗口里
QWidget *centralWidget = new QWidget(this);
QVBoxLayout *layout = new QVBoxLayout(centralWidget);

// 直接创建，传 parent 即可嵌入
AicMainWindow *aicPanel = new AicMainWindow(centralWidget);
layout->addWidget(aicPanel);

setCentralWidget(centralWidget);
