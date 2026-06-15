#include <QApplication>
#include <cstdlib>
#include <ctime>

#include "ui/qt/QtMainWindow.h"

// 流程：初始化随机种子 ──> 创建 QApplication 与主窗口 ──> 显示窗口并进入 Qt 事件循环
int main(int argc, char* argv[]) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    QApplication app(argc, argv);
    my_auto_arena::ui::QtMainWindow window(nullptr);
    window.show();
    return app.exec();
}
