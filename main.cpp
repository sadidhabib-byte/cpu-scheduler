#include <QApplication>

#include <cstdlib>
#include <ctime>

#include "mainwindow.h"

int main(int argc, char* argv[]) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    QApplication app(argc, argv);
    app.setStyle(QStringLiteral("Fusion"));
    QApplication::setApplicationName(QStringLiteral("cpu-scheduler"));
    QApplication::setOrganizationName(QStringLiteral("cpu-scheduler"));

    MainWindow window;
    window.show();
    return app.exec();
}
