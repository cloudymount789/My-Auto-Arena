#include <QApplication>

#include "ui/qt/QtMainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    my_auto_arena::ui::QtMainWindow window(nullptr);
    window.show();
    return app.exec();
}
