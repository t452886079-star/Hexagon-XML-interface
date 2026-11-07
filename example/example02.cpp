#include "AicAutomation.h"

// 点击某个按钮弹出
void MainWindow::onBtnOpenAIC()
{
    static AicMainWindow *aicWin = nullptr;
    if (!aicWin) {
        aicWin = new AicMainWindow(this);
        aicWin->setWindowFlags(Qt::Window);  // 独立窗口
    }
    aicWin->show();
    aicWin->raise();
}
