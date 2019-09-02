#include <QtGui/QGuiApplication>
#include<QApplication>
#include"mainwindow.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow* window = new MainWindow();
    window->showMaximized();
    return a.exec();
}
