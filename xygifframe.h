#ifndef XYGIFFRAME_H
#define XYGIFFRAME_H

#include "xymovablewidget.h"
#include "xygifcreator.h"
#include <QTimer>

namespace Ui {
class XYGifFrame;
}

class XYGifFrame : public XYMovableWidget
{
    Q_OBJECT
public:
    explicit XYGifFrame(QWidget *parent = nullptr);
	~XYGifFrame() override;

public slots:
    void doResize();
    void packImages();
    void setGifFile();
    void active();
    void start();
    void stop();
    void frame();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QImage getCurScreenImage();

    // 用于resize窗口
private:
    bool   mStartResize;
    QRect  mStartGeometry;

private:
    Ui::XYGifFrame *ui;
    QRect           mRecordRect;
    XYGifCreator   *mGifCreator;
    QTimer          mTimer;
    int             mPixs;
    QString         mGifFile;
};

#endif // XYGIFFRAME_H
