#ifndef XYGIFCREATOR_H
#define XYGIFCREATOR_H

#include <QObject>

class XYGif;
class XYGifCreator : public QObject
{
    Q_OBJECT
public:
    explicit XYGifCreator(QObject *parent = nullptr);
    ~XYGifCreator();

signals:
    void finished();

public slots:
    bool begin(const QString &file, int width, int height, int delay = 0, Qt::ConnectionType type = Qt::AutoConnection);
    bool frame(const QImage &img, int delay = 0, Qt::ConnectionType type = Qt::AutoConnection);
    bool end(Qt::ConnectionType type = Qt::AutoConnection);

private:
    XYGif  *mGif;
    int     mWidth;
    int     mHeight;

};

#endif // XYGIFCREATOR_H
