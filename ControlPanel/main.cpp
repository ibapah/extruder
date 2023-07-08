#include "mainwindow.h"
#include "version.h"
#include <QApplication>
#include <QMutex>
#include <iostream>

int main(int argc, char *argv[])
{
    int exitCode = 0;
    
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    exitCode = a.exec();

    return exitCode;
}
