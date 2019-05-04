#include "xygifframe.h"
#include "ui_xygifframe.h"
#include <QPainter>
#include <QFileDialog>
#include <QApplication>
#include <QScreen>
#include <QResizeEvent>
#include <QDesktopWidget>
#include <QDateTime>
#include <QDebug>

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
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(active()));
    connect(ui->pushButton_2, SIGNAL(clicked()), qApp, SLOT(quit()));
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(frame()));

    setMouseTracking(true);
}

void XYGifFrame::doResize()
{
    QRect rect(pos(), QSize(ui->width->value(), ui->height->value()));
    rect.adjust(-3, -3, 3, 30);

    resize(rect.size());
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
    QString file = QFileDialog::getSaveFileName(this, "", QString("xy-%1.gif").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss")));
    if (!file.isEmpty())
    {
        mTimer.start(ui->interval->value());
        mPixs = 0;
        ui->pushButton->setText(QStringLiteral("停止录制"));
        mGifCreator->begin(file.toUtf8().data(), ui->width->value(), ui->height->value());
    }
}

void XYGifFrame::stop()
{
    mTimer.stop();
    ui->pushButton->setText(QStringLiteral("开始录制"));
    mGifCreator->end();
}

void XYGifFrame::frame()
{
    auto screen = qApp->screenAt(pos());
    if (screen != nullptr)
    {
        QRect rect = this->rect();
        rect.adjust(3, 3, -3, -30);
        rect.translate(pos());
        QImage img = screen->grabWindow(qApp->desktop()->winId(), rect.x(), rect.y(), rect.width(), rect.height()).toImage();
        mGifCreator->frame(img);
        mPixs++;

        ui->tips->setText(QStringLiteral("已保存 %1 张图片").arg(mPixs));
    }
}

void XYGifFrame::paintEvent(QPaintEvent *)
{
    QRect rect = this->rect();
    QRegion r1(rect);
    QRegion r2(rect.adjusted(3, 3, -3, -30));
    QRegion r3 = r1.xored(r2);
    setMask(r3);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect, Qt::gray);
}

void XYGifFrame::resizeEvent(QResizeEvent *)
{
    QRect rect = this->rect();
    rect.adjust(3, 3, -3, -30);

    ui->width->setValue(rect.width());
    ui->height->setValue(rect.height());
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
