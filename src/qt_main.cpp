#include <QApplication>
#include <cstdlib>
#include <ctime>

#include "ui/qt/QtMainWindow.h"

int main(int argc, char* argv[]) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    QApplication app(argc, argv);
    my_auto_arena::ui::QtMainWindow window(nullptr);
    window.show();
    return app.exec();
}
