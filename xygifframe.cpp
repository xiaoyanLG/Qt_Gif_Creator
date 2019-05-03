#include "xygifframe.h"
#include "ui_xygifframe.h"
#include <QPainter>
#include <QFileDialog>
#include <QApplication>
#include <QScreen>
#include <QResizeEvent>
#include <QDesktopWidget>

XYGifFrame::XYGifFrame(QWidget *parent)
    : XYMovableWidget(parent),
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
    QString file = QFileDialog::getSaveFileName(this, "", "xiaoyan.gif");
    if (!file.isEmpty())
    {
        mTimer.start(ui->interval->value());
        mPixs = 0;
        ui->pushButton->setText(QStringLiteral("停止"));
        mGifCreator->begin(file.toUtf8().data(), ui->width->value(), ui->height->value());
    }
}

void XYGifFrame::stop()
{
    mTimer.stop();
    ui->pushButton->setText(QStringLiteral("启动"));
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

        ui->tips->setText(QStringLiteral("已保存 %1 张").arg(mPixs));
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
