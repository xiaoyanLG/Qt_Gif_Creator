#ifndef XYPACKIMAGE_H
#define XYPACKIMAGE_H

#include <QDialog>

namespace Ui {
class XYPackImage;
}

class QThread;
class Work: public QObject
{
    Q_OBJECT
public:
    Work();
    ~Work();

signals:
    void progress(int progress);

public slots:
    void pickImags(const QString &file, const QStringList &imgs, int w, int h, int delay);

private:
    QThread  *mThread;
};

class XYPackImage : public QDialog
{
    Q_OBJECT

public:
    explicit XYPackImage(QWidget *parent = nullptr);
    ~XYPackImage();

signals:
    void pickImags(const QString &file, const QStringList &imgs, int w, int h, int delay);

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::XYPackImage *ui;
    Work            *mWorkThread;
    QString          mGifFile;
};

#endif // XYPACKIMAGE_H
