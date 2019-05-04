#include "xygifcreator.h"
#include "gif.cpp"
#include <QImage>

XYGifCreator::XYGifCreator(QObject *parent)
    : QObject(parent)
{
    mGif = new Gif;
}

void XYGifCreator::begin(const QString &file, int width, int height)
{
    mGif->GifBegin(file.toUtf8().data(), static_cast<quint32>(width), static_cast<quint32>(height), 0);
}

void XYGifCreator::frame(const QImage &img)
{
    QImage img32 = img.convertToFormat(QImage::Format_RGBA8888);
    mGif->GifWriteFrame(img32.bits(), static_cast<quint32>(img32.width()), static_cast<quint32>(img32.height()), 0);
}

void XYGifCreator::end()
{
    mGif->GifEnd();
}
