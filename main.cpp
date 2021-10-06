#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    std::cout<<"begin mainwindows"<<std::endl;

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
