#include "xygifframe.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/gif.ico"));
    XYGifFrame w;
    w.show();

    return a.exec();
}
