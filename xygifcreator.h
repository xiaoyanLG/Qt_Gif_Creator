#ifndef XYGIFCREATOR_H
#define XYGIFCREATOR_H

#include <QObject>

class Gif;
class XYGifCreator : public QObject
{
    Q_OBJECT
public:
    explicit XYGifCreator(QObject *parent = nullptr);
    ~XYGifCreator();

public slots:
    void begin(const QString &file, int width, int height, int delay = 0);
    void frame(const QImage &img);
    void end();

private:
    Gif  *mGif;
    int   mDelay;
};

#endif // XYGIFCREATOR_H
