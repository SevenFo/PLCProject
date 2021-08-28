#include "imagesender.h"
#include <QThread>

ImageSender::ImageSender(QObject *parent) : QObject(parent),run(true),hasRecvImg(true),connected(false)
{
    tcpSocket = new QTcpSocket(this);
    infedRawPic = new QByteArray();


}

ImageSender::~ImageSender()
{

}
void ImageSender::DealRecvData()
{
    //接收推断结果的压缩图篇片
    infedRawPic->clear();
    char buff[10240] = {0};
    int size;
    do{
        size = (tcpSocket->read(buff,10240));
        infedRawPic->append(buff,size);
        qDebug()<<"recv:"<<size;
    }while(size);

    std::vector<unsigned char> picVector(infedRawPic->begin(),infedRawPic->end());
    cv::Mat *dispic = new cv::Mat();
    cv::imdecode(picVector,cv::IMREAD_UNCHANGED,dispic);
    cv::Mat convetdPic;
    cv::cvtColor(*dispic,convetdPic,cv::COLOR_BGR2RGB);
    QImage qpic = QImage(convetdPic.data,convetdPic.cols,convetdPic.rows,QImage::Format_RGB888);
    delete dispic;
    emit NewInfedImg(QPixmap::fromImage(qpic));
    hasRecvImg = true;
    //    emit PreveiwWidget::RecvedNewInfedPic(qpic);
}

void ImageSender::doWork()
{


    connect(tcpSocket,&QTcpSocket::connected,[=](){
        qDebug()<<"connected！";
        connected = true;
    });
    connect(tcpSocket,&QTcpSocket::readyRead,this,&ImageSender::DealRecvData);

//    tcpSocket->o
    qDebug()<<"run thread:"<<QThread::currentThreadId();
    tcpSocket->connectToHost("127.0.0.1",9999);
    qDebug()<<"wai connected";

    tcpSocket->waitForConnected();

    QThread::sleep(1);
    while(run)
    {
        if(!hasRecvImg)
        {
            qDebug("continue");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        qDebug()<<"write begin";
        qDebug()<<"thread size:"<<dataToSend.size();
        if(tcpSocket->isWritable() && dataToSend.size()>0)
        {
            qDebug()<<"try write";
            qDebug()<<tcpSocket->write((char *)dataToSend.data(),dataToSend.size());
//            qDebug()<<tcpSocket->write("??????????\0",10);
//            tcpSocket->waitForBytesWritten();
            hasRecvImg = false;
//            tcpSocket->waitForReadyRead();
        }
        else
        {
            qDebug()<<"socket unwirtable!";
        }
        qDebug()<<"write end";

    }



}

