#include "xygifframe.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    XYGifFrame w;
    w.show();

    return a.exec();
}
