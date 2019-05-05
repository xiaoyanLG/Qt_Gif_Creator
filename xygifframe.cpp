#include "xygifframe.h"
#include "xypackimage.h"
#include "ui_xygifframe.h"
#include <QPainter>
#include <QFileDialog>
#include <QApplication>
#include <QScreen>
#include <QResizeEvent>
#include <QDateTime>
#include <QMessageBox>

#ifdef Q_OS_WIN32
#include <windows.h>

static QImage getScreenImage(int x, int y, int width, int height)
{
    HDC hScrDC = ::GetDC(nullptr);
    HDC hMemDC = nullptr;

    BYTE *lpBitmapBits = nullptr;

    int nWidth = width;
    int nHeight = height;

    hMemDC = ::CreateCompatibleDC(hScrDC);

    BITMAPINFO bi;
    ZeroMemory(&bi, sizeof(BITMAPINFO));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = nWidth;
    bi.bmiHeader.biHeight = nHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;

    CURSORINFO hCur ;
    ZeroMemory(&hCur,sizeof(hCur));
    hCur.cbSize=sizeof(hCur);
    GetCursorInfo(&hCur);

    HBITMAP bitmap = ::CreateDIBSection(hMemDC, &bi, DIB_RGB_COLORS, (LPVOID*)&lpBitmapBits, nullptr, 0);
    HGDIOBJ oldbmp = ::SelectObject(hMemDC, bitmap);

    ::BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, x, y, SRCCOPY);
    DrawIconEx(hMemDC, hCur.ptScreenPos.x - x, hCur.ptScreenPos.y - y, hCur.hCursor, 0, 0, 0, nullptr, DI_NORMAL | DI_COMPAT);

    BITMAPFILEHEADER bh;
    ZeroMemory(&bh, sizeof(BITMAPFILEHEADER));
    bh.bfType = 0x4d42;  //bitmap
    bh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bh.bfSize = bh.bfOffBits + ((nWidth*nHeight)*3);

    int size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 3 * nWidth * nHeight;
    uchar *bmp = new uchar[size];
    uint offset = 0;
    memcpy(bmp, (char *)&bh, sizeof(BITMAPFILEHEADER));
    offset = sizeof(BITMAPFILEHEADER);
    memcpy(bmp + offset, (char *)&(bi.bmiHeader), sizeof(BITMAPINFOHEADER));
    offset += sizeof(BITMAPINFOHEADER);
    memcpy(bmp + offset, (char *)lpBitmapBits, 3 * nWidth * nHeight);

    ::SelectObject(hMemDC, oldbmp);
    ::DeleteObject(bitmap);
    ::DeleteObject(hMemDC);
    ::ReleaseDC(nullptr, hScrDC);
    QImage image = QImage::fromData(bmp, size);
    delete[] bmp;
    return image;
}
#endif

XYGifFrame::XYGifFrame(QWidget *parent)
    : XYMovableWidget(parent),
      mStartResize(false),
      ui(new Ui::XYGifFrame)
{
    ui->setupUi(this);
    mGifCreator = new XYGifCreator(this);
    mTimer.setSingleShot(false);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    connect(ui->width, SIGNAL(valueChanged(int)), this, SLOT(doResize()));
    connect(ui->height, SIGNAL(valueChanged(int)), this, SLOT(doResize()));
    connect(ui->gif, SIGNAL(clicked()), this, SLOT(active()));
    connect(ui->img, SIGNAL(clicked()), this, SLOT(packImages()));
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(frame()));
    connect(ui->save, SIGNAL(clicked()), this, SLOT(setGifFile()));
    connect(ui->quit, SIGNAL(clicked()), qApp, SLOT(quit()));
    connect(mGifCreator, &XYGifCreator::finished, this, [this](){
        this->ui->tips->setText(QStringLiteral("保存Gif完成！"));
    });

    ui->content->adjustSize();
    setMouseTracking(true);
    setMinimumSize(162, 60);
    ui->gif->setFocus();
}

XYGifFrame::~XYGifFrame()
{
    delete ui;
}

void XYGifFrame::doResize()
{
    QRect rect(pos(), QSize(ui->width->value(), ui->height->value()));
    rect.adjust(-3, -3, 3, ui->content->height() + 5);

    resize(rect.size());
}

void XYGifFrame::packImages()
{
    XYPackImage img(this);
    img.exec();
}

void XYGifFrame::setGifFile()
{
    mGifFile = QFileDialog::getSaveFileName(this, "", QString("xy-%1.gif").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss")));

    ui->save->setToolTip(mGifFile);
}

void XYGifFrame::active()
{
    if (!mTimer.isActive())
    {
        start();
    }
    else
    {
        stop();
    }
}

void XYGifFrame::start()
{
    if (!mGifFile.isEmpty())
    {
        if (!mGifCreator->begin(mGifFile.toUtf8().data(), ui->width->value(), ui->height->value(), 1))
        {
            QMessageBox::warning(this, QStringLiteral("提醒"), QStringLiteral("正在保存Gif，请稍等！"));
            return;
        }

        mPixs = 0;
        ui->gif->setText(QStringLiteral("停止录制"));

        frame();
        mTimer.start(static_cast<int>(1000.0 / ui->interval->value()));
    }
    else
    {
        QMessageBox::warning(this, QStringLiteral("提醒"), QStringLiteral("请先设置保存gif的位置！"));
    }
}

void XYGifFrame::stop()
{
    mTimer.stop();
    ui->gif->setText(QStringLiteral("开始录制"));
    mGifCreator->end();

    this->ui->tips->setText(QStringLiteral("请等待保存Gif..."));
}

void XYGifFrame::frame()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    auto screen = qApp->screenAt(pos());
#else
    QScreen *screen = nullptr;
    foreach (screen, qApp->screens())
    {
        if (screen->geometry().contains(pos()))
        {
            break;
        }
    }
#endif
    if (screen != nullptr)
    {
#ifdef Q_OS_WIN32
        QImage img = getScreenImage(x() + mRecordRect.x(),
                                    y() + mRecordRect.y(),
                                    mRecordRect.width(),
                                    mRecordRect.height());
#else
        QImage img = screen->grabWindow(0,
                                        x() + mRecordRect.x(),
                                        y() + mRecordRect.y(),
                                        mRecordRect.width(),
                                        mRecordRect.height()).toImage();
#endif
        mGifCreator->frame(img, ui->interval->value());
        mPixs++;

        ui->tips->setText(QStringLiteral("已保存 %1 张图片").arg(mPixs));
    }
}

void XYGifFrame::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::gray);
}

void XYGifFrame::resizeEvent(QResizeEvent *)
{
    QRect rect = this->rect();
    rect.adjust(3, 3, -3, -(ui->content->height() + 5));
    mRecordRect = rect;

    ui->width->setValue(mRecordRect.width());
    ui->height->setValue(mRecordRect.height());

    QRegion region(this->rect());
    setMask(region.xored(mRecordRect));

    ui->content->move(width() - ui->content->width() - 3, height() - ui->content->height() - 3);
}

void XYGifFrame::mousePressEvent(QMouseEvent *event)
{
    QRect rect(QPoint(width() - 3, height() - 3), QSize(3, 3));
    if (rect.contains(event->pos()) && !mTimer.isActive())
    {
        mStartResize = true;
        mStartGeometry = QRect(event->globalPos(), size());
    }
    else
    {
        XYMovableWidget::mousePressEvent(event);
    }
}

void XYGifFrame::mouseReleaseEvent(QMouseEvent *event)
{
    mStartResize = false;
    setCursor(Qt::ArrowCursor);
    XYMovableWidget::mouseReleaseEvent(event);
}

void XYGifFrame::mouseMoveEvent(QMouseEvent *event)
{
    QRect rect(QPoint(width() - 3, height() - 3), QSize(3, 3));

    if (mStartResize)
    {
        QPoint ch = event->globalPos() - mStartGeometry.topLeft();
        resize(mStartGeometry.size() + QSize(ch.x(), ch.y()));
    }
    else if (rect.contains(event->pos()) && !mTimer.isActive())
    {
        setCursor(Qt::SizeFDiagCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        XYMovableWidget::mouseMoveEvent(event);
    }
}

void XYGifFrame::wheelEvent(QWheelEvent *event)
{
    if (event->delta() < 0)
    {
        ui->content->move(ui->content->x() + 6, ui->content->y());
    }
    else
    {
        ui->content->move(ui->content->x() - 6, ui->content->y());
    }

    XYMovableWidget::wheelEvent(event);
}
