#include "xygifcreator.h"
#include "gif.cpp"
#include <QImage>

XYGifCreator::XYGifCreator(QObject *parent)
    : QObject(parent)
{
    mGif = new Gif;
    mDelay = 0;
}

XYGifCreator::~XYGifCreator()
{
    delete mGif;
}

void XYGifCreator::begin(const QString &file, int width, int height, int delay)
{
    mDelay = delay;
    mGif->GifBegin(file.toUtf8().data(), static_cast<quint32>(width), static_cast<quint32>(height), static_cast<quint32>(mDelay));
}

void XYGifCreator::frame(const QImage &img)
{
    // gif.cpp 文件有描述目前只能是RGBA8888图片格式，并且alpha没有被使用
    QImage img32 = img.convertToFormat(QImage::Format_RGBA8888);
    mGif->GifWriteFrame(img32.bits(), static_cast<quint32>(img32.width()), static_cast<quint32>(img32.height()), static_cast<quint32>(mDelay));
}

void XYGifCreator::end()
{
    mGif->GifEnd();
}
