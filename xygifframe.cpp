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

XYGifFrame::XYGifFrame(QWidget *parent)
    : XYMovableWidget(parent),
      mStartResize(false),
      ui(new Ui::XYGifFrame)
{
    ui->setupUi(this);
    mGifCreator = new XYGifCreator(this);
    mTimer.setSingleShot(false);
    setWindowFlags(Qt::FramelessWindowHint);

    connect(ui->width, SIGNAL(valueChanged(int)), this, SLOT(doResize()));
    connect(ui->height, SIGNAL(valueChanged(int)), this, SLOT(doResize()));
    connect(ui->gif, SIGNAL(clicked()), this, SLOT(active()));
    connect(ui->img, SIGNAL(clicked()), this, SLOT(packImages()));
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(frame()));
    connect(ui->save, SIGNAL(clicked()), this, SLOT(setGifFile()));
    connect(ui->quit, SIGNAL(clicked()), qApp, SLOT(quit()));

    ui->content->adjustSize();
    setMouseTracking(true);
    setMinimumSize(162, 60);
    setFocus();
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
        mPixs = 0;
        ui->gif->setText(QStringLiteral("停止录制"));
        mGifCreator->begin(mGifFile.toUtf8().data(), ui->width->value(), ui->height->value(), 1);

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
}

void XYGifFrame::frame()
{
    auto screen = qApp->screenAt(pos());
    if (screen != nullptr)
    {
        QImage img = screen->grabWindow(0,
                                        x() + mRecordRect.x(),
                                        y() + mRecordRect.y(),
                                        mRecordRect.width(),
                                        mRecordRect.height()).toImage();
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
