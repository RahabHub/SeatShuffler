/**
 * @file main.cpp
 * @brief SeatShuffler 应用程序入口
 */
#include <QApplication>

#include "src/ui/page/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QApplication::setApplicationName("SeatShuffler");
    QApplication::setOrganizationName("SeatShuffler");
    QApplication::setApplicationVersion("0.1.0");

    seat::MainWindow mainWindow;
    mainWindow.show();

    return QApplication::exec();

}
