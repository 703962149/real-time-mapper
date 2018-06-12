#include "mainwindow.h"
#include "maindialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //MainDialog w;
    w.setWindowTitle("mapper");
    w.show();

    return a.exec();
}
