#ifndef IMAGESENDER_H
#define IMAGESENDER_H

#include <QObject>
#include <QTcpSocket>
#include <QImage>
#include <QByteArray>
#include <vector>
#include <opencv2/opencv.hpp>
#include <QPixmap>

class ImageSender : public QObject
{
    Q_OBJECT
public:
    explicit ImageSender(QObject *parent = nullptr);
    virtual ~ImageSender();

    bool run,hasRecvImg,connected;
    QByteArray *infedRawPic;
    std::vector<unsigned char> dataToSend;

    void doWork();
    void DealRecvData();


private:
    QTcpSocket *tcpSocket;



signals:
    void Done();
    void NewInfedImg(QPixmap img);


};

#endif // IMAGESENDER_H
