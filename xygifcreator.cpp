#include "xygifcreator.h"
#include "gif.cpp"
#include <QImage>

XYGifCreator::XYGifCreator(QObject *parent)
    : QObject(parent)
{
    mGif = new Gif_H;
}

void XYGifCreator::begin(const QString &file, int width, int height)
{
    mGif->GifBegin(file.toUtf8().data(), static_cast<quint32>(width), static_cast<quint32>(height), 0);
}

void XYGifCreator::frame(const QImage &img)
{
    if (mMutex.tryLock())
    {
        mGif->GifWriteFrame(img.bits(), static_cast<quint32>(img.width()), static_cast<quint32>(img.height()), 0);
        mMutex.unlock();
    }
}

void XYGifCreator::end()
{
    mGif->GifEnd();
}
