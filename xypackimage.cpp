#include "xypackimage.h"
#include "xygifcreator.h"
#include "ui_xypackimage.h"
#include <QFileDialog>
#include <QDateTime>
#include <QThread>
#include <QProgressBar>
#include <QMessageBox>
#include <QDebug>

XYPackImage::XYPackImage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::XYPackImage)
{
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    mWorkThread = new Work;
    connect(this, &XYPackImage::pickImags, mWorkThread, &Work::pickImags);

    connect(mWorkThread, &Work::progress, ui->progressBar, &QProgressBar::setValue);
}

XYPackImage::~XYPackImage()
{
    delete mWorkThread;
    delete ui;
}

void XYPackImage::on_pushButton_2_clicked()  // 添加图片
{
    QStringList files = QFileDialog::getOpenFileNames(this, "", "", "Images (*.png *.jpg)");

    for (int i = 0; i < files.size(); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon(files.at(i)));
        item->setText(files.at(i));
        ui->listWidget->addItem(item);
    }
}

void XYPackImage::on_pushButton_clicked()   // 开始打包
{
    QStringList images;
    for (int row = 0; row < ui->listWidget->count(); ++row)
    {
        images << ui->listWidget->item(row)->text();
    }

    if (images.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("提醒"), QStringLiteral("请先选择图片！"));
        return;
    }

    if (!mGifFile.isEmpty())
    {
        int w = ui->width->value();
        int h = ui->height->value();
        int delay = ui->delay->value();

        emit pickImags(mGifFile, images, w, h, delay);
    }
    else
    {
        QMessageBox::warning(this, QStringLiteral("提醒"), QStringLiteral("请先设置保存gif的位置！"));
    }
}

Work::Work()
{
    mThread = new QThread;
    mThread->start();
    moveToThread(mThread);
}

Work::~Work()
{
    if (mThread->isRunning()) {
        mThread->quit();
        mThread->wait();
    }

    delete mThread;
}

void Work::pickImags(const QString &file, const QStringList &imgs, int w, int h, int delay)
{
    XYGifCreator gif;
    gif.begin(file.toUtf8().data(), w, h, 1);
    emit progress(0);
    for (int i = 0; i < imgs.size(); ++i)
    {
        QImage img(imgs.at(i));
        img = img.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        gif.frame(img, delay);

        progress(static_cast<int>((i + 1.0) / imgs.size() * 100));
    }
    gif.end();

    emit progress(100);
}

void XYPackImage::on_pushButton_3_clicked()
{
    mGifFile = QFileDialog::getSaveFileName(this, "",
                                            QString("xy-%1.gif").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss")));
}
