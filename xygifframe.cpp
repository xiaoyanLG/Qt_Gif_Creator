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
    hCur.cbSize = sizeof(CURSORINFO);
    GetCursorInfo(&hCur);

    ICONINFO IconInfo = {};
    if(GetIconInfo(hCur.hCursor, &IconInfo))
    {
        hCur.ptScreenPos.x -= IconInfo.xHotspot;
        hCur.ptScreenPos.y -= IconInfo.yHotspot;
        if(nullptr != IconInfo.hbmMask)
            DeleteObject(IconInfo.hbmMask);
        if(nullptr != IconInfo.hbmColor)
            DeleteObject(IconInfo.hbmColor);
    }

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
#else // linux x11
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
static void X11drawCursor(Display *display, QImage &image, int recording_area_x, int recording_area_y)
{
    // get the cursor
    XFixesCursorImage *xcim = XFixesGetCursorImage(display);
    if(xcim == NULL)
        return;

    // calculate the position of the cursor
    int x = xcim->x - xcim->xhot - recording_area_x;
    int y = xcim->y - xcim->yhot - recording_area_y;

    // calculate the part of the cursor that's visible
    int cursor_left = std::max(0, -x), cursor_right = std::min((int) xcim->width, image.width() - x);
    int cursor_top = std::max(0, -y), cursor_bottom = std::min((int) xcim->height, image.height() - y);

    unsigned int pixel_bytes, r_offset, g_offset, b_offset; // ARGB
    pixel_bytes = 4;
    r_offset = 2; g_offset = 1; b_offset = 0;

    // draw the cursor
    // XFixesCursorImage uses 'long' instead of 'int' to store the cursor images, which is a bit weird since
    // 'long' is 64-bit on 64-bit systems and only 32 bits are actually used. The image uses premultiplied alpha.
    for(int j = cursor_top; j < cursor_bottom; ++j) {
        unsigned long *cursor_row = xcim->pixels + xcim->width * j;
        uint8_t *image_row = (uint8_t*) image.bits() + image.bytesPerLine() * (y + j);
        for(int i = cursor_left; i < cursor_right; ++i) {
            unsigned long cursor_pixel = cursor_row[i];
            uint8_t *image_pixel = image_row + pixel_bytes * (x + i);
            int cursor_a = (uint8_t) (cursor_pixel >> 24);
            int cursor_r = (uint8_t) (cursor_pixel >> 16);
            int cursor_g = (uint8_t) (cursor_pixel >> 8);
            int cursor_b = (uint8_t) (cursor_pixel >> 0);
            if(cursor_a == 255) {
                image_pixel[r_offset] = cursor_r;
                image_pixel[g_offset] = cursor_g;
                image_pixel[b_offset] = cursor_b;
            } else {
                image_pixel[r_offset] = (image_pixel[r_offset] * (255 - cursor_a) + 127) / 255 + cursor_r;
                image_pixel[g_offset] = (image_pixel[g_offset] * (255 - cursor_a) + 127) / 255 + cursor_g;
                image_pixel[b_offset] = (image_pixel[b_offset] * (255 - cursor_a) + 127) / 255 + cursor_b;
            }
        }
    }
    // free the cursor
    XFree(xcim);
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

    ui->width->installEventFilter(this);
    ui->height->installEventFilter(this);
    connect(ui->width, SIGNAL(editingFinished()), this, SLOT(doResize()));
    connect(ui->height, SIGNAL(editingFinished()), this, SLOT(doResize()));
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
        if (mGifCreator->isRunning())
        {
            QMessageBox::warning(this, QStringLiteral("提醒"), QStringLiteral("正在保存Gif，请稍等！"));
            return;
        }
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
        mGifCreator->startThread();

        mGifCreator->begin(mGifFile.toUtf8().data(), ui->width->value(), ui->height->value(), 1);

        mPixs = 0;
        ui->gif->setText(QStringLiteral("停止录制"));

        frame();
        mTimer.start(static_cast<int>(1000.0 / ui->interval->value()));
        ui->width->setDisabled(true);
        ui->height->setDisabled(true);
        ui->interval->setDisabled(true);
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

    ui->width->setEnabled(true);
    ui->height->setEnabled(true);
    ui->interval->setEnabled(true);
    this->ui->tips->setText(QStringLiteral("请等待保存Gif..."));
}

void XYGifFrame::frame()
{
    QImage img = getCurScreenImage();
    if (!img.isNull())
    {
        mGifCreator->frame(img, ui->interval->value());
        mPixs++;

        ui->tips->setText(QStringLiteral("已保存 %1 张图片").arg(mPixs));
    }
}

bool XYGifFrame::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->width
            || watched == ui->height)
    {
        if (event->type() == QEvent::Wheel)
        {
            doResize();
        }
    }
    return XYMovableWidget::eventFilter(watched, event);
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

QImage XYGifFrame::getCurScreenImage()
{
    QImage img;
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
        img = getScreenImage(x() + mRecordRect.x(),
                                    y() + mRecordRect.y(),
                                    mRecordRect.width(),
                                    mRecordRect.height());
#else
        img = screen->grabWindow(0,
                                        x() + mRecordRect.x(),
                                        y() + mRecordRect.y(),
                                        mRecordRect.width(),
                                        mRecordRect.height()).toImage();

        // 需要系统是x11后端
        auto display = XOpenDisplay(NULL);
        X11drawCursor(display, img, x() + mRecordRect.x(), y() + mRecordRect.y());
        XCloseDisplay(display);
#endif
    }

    return img;
}
