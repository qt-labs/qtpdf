#include "mainwindow.h"
#include <QApplication>
#include <QUrl>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QStringList args = a.arguments();
    w.show();
    if (args.length() > 1)
        w.open(QUrl::fromUserInput(args[1]));

    return a.exec();
}
