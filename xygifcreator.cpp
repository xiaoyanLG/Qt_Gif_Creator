#include "xygifcreator.h"
#include <QImage>
#include <QThread>

#include "gif.h"

class XYGif : public QObject
{
    Q_OBJECT
public:
    explicit XYGif()
    {
        moveToThread(&mWorkThread);
    }
    ~XYGif()
    {
        if (mWorkThread.isRunning())
        {
            mWorkThread.quit();
            mWorkThread.wait();
        }
    }

public slots:
    void begin(const QString &file, int width, int height, int delay)
    {
        GifBegin(&mGifWriter, file.toUtf8().data(), static_cast<quint32>(width), static_cast<quint32>(height), static_cast<quint32>(delay));
    }
    void frame(const QImage &img, int width, int height, int delay)
    {
        GifWriteFrame(&mGifWriter, img.bits(),
                      static_cast<quint32>(qMin(width, img.width())),
                      static_cast<quint32>(qMin(height, img.height())),
                      static_cast<quint32>(100.0 / delay));
    }
    void end()
    {
        GifEnd(&mGifWriter);

        mWorkThread.quit();
    }

private:
    GifWriter mGifWriter;
    QThread   mWorkThread;

    friend class XYGifCreator;
};

XYGifCreator::XYGifCreator(QObject *parent)
    : QObject(parent)
{
    mGif = new XYGif;
    connect(&mGif->mWorkThread, &QThread::finished, this, &XYGifCreator::finished);
}

XYGifCreator::~XYGifCreator()
{
    delete mGif;
}

void XYGifCreator::startThread()
{
    mGif->mWorkThread.start();
}

bool XYGifCreator::isRunning()
{
    return mGif->mWorkThread.isRunning();
}

void XYGifCreator::begin(const QString &file, int width, int height, int delay, Qt::ConnectionType type)
{
    mWidth  = width;
    mHeight = height;

    QMetaObject::invokeMethod(mGif, "begin", type,
                              QGenericReturnArgument(),
                              Q_ARG(QString, file),
                              Q_ARG(int, width),
                              Q_ARG(int, height),
                              Q_ARG(int, delay));
}

void XYGifCreator::frame(const QImage &img, int delay, Qt::ConnectionType type)
{
    // gif.cpp 文件有描述目前只能是RGBA8888图片格式，并且alpha没有被使用
    QImage img32 = img.convertToFormat(QImage::Format_RGBA8888);

    QMetaObject::invokeMethod(mGif, "frame", type,
                              QGenericReturnArgument(),
                              Q_ARG(QImage, img32),
                              Q_ARG(int, mWidth),
                              Q_ARG(int, mHeight),
                              Q_ARG(int, delay));
}

void XYGifCreator::end(Qt::ConnectionType type)
{
    QMetaObject::invokeMethod(mGif, "end", type);
}

#include "xygifcreator.moc"
