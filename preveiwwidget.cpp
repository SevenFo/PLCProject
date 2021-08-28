#include "preveiwwidget.h"
#include <QDebug>
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "chrono"
#include <QThread>


#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

PreveiwWidget::PreveiwWidget(long userID, QWidget *parent) :QWidget(parent), _openPreview(false),
    decodedImgW(0),decodedImgH(0),runInfer(false),infedImgW(0),infedImgH(0),snapedImgW(0),snapedImgH(0),
    sendImg(true),isConnect(false),hasRecvImg(true)
{

    hkvision = new HikvisonHandler(this);

    _buttonLayout = new QHBoxLayout();
    imgsLayout = new QHBoxLayout();
    _pushbuttonSnap = new QPushButton(parent);
    _pushbuttonDec = new QPushButton(parent);
    pushbuttonConnect = new QPushButton(parent);
    pushbuttonSnapV2 = new QPushButton(parent);
    pushbuttonInf = new QPushButton(this);
    pushbuttonStartFaceDet = new QPushButton(this);
    labelSnapedFaceImg = new QLabel();
    labelDecodedImg = new QLabel();
    labelInfedImg = new QLabel();
    infedRawPic = new QByteArray();
    infedPic = new QImage();
    imgSender = new ImageSender();
    sendImgThread = new QThread();
    tcpSocket = new QTcpSocket();


//    imgSender->moveToThread(sendImgThread);
//    sendImgThread->start();



    this->setWindowTitle("Preview Windows");
    this->resize(600,600);

    _frameDisplay.resize(480,480);

    _vBoxLayout.addWidget(&_frameDisplay);
    _buttonLayout->addWidget(&_pushbuttonDisplayPrev);
    _buttonLayout->addWidget(_pushbuttonSnap);
    _buttonLayout->addWidget(_pushbuttonDec);
    _buttonLayout->addWidget(pushbuttonConnect);
    _buttonLayout->addWidget((pushbuttonSnapV2));
    _buttonLayout->addWidget(pushbuttonInf);
    _buttonLayout->addWidget(pushbuttonStartFaceDet);

    imgsLayout->addWidget(labelDecodedImg);
    imgsLayout->addWidget(labelSnapedFaceImg);
    imgsLayout->addWidget(labelInfedImg);

    _vBoxLayout.addLayout(_buttonLayout);
    _vBoxLayout.addLayout(imgsLayout);



    this->setLayout(&_vBoxLayout);


    _frameDisplay.setWindowTitle("Preview");
    _pushbuttonDisplayPrev.setText("switch display");
    _pushbuttonSnap->setText("Snap");
    _pushbuttonDec->setText("Dec");
    pushbuttonSnapV2->setText("SnapV2");
    pushbuttonConnect->setText("Connnect");
    pushbuttonInf->setText("INF");
    pushbuttonStartFaceDet->setText("Face Det");
    labelSnapedFaceImg->setText("snaped face img");
    labelDecodedImg->setText("Decoded img");
    labelInfedImg->setText("Infed img");

    connect(&_pushbuttonDisplayPrev,&QPushButton::clicked,this,&PreveiwWidget::_SwitchPreview);
    connect(_pushbuttonSnap,&QPushButton::clicked,this,&PreveiwWidget::ClickPushbuttonSnap);
    connect(_pushbuttonDec,&QPushButton::clicked,this,&PreveiwWidget::ClickPushbuttonDec);
    connect(pushbuttonSnapV2,&QPushButton::clicked,this,&PreveiwWidget::ClickPushbuttonSnapV2);
    connect(pushbuttonConnect,&QPushButton::clicked,this,&PreveiwWidget::ClickPushbuttonConnect);
    connect(pushbuttonInf,&QPushButton::clicked,this,&PreveiwWidget::ClickPushbuttonInf);
    connect(pushbuttonStartFaceDet,&QPushButton::clicked,hkvision,&HikvisonHandler::SetupFaceDet);

    connect(hkvision,&HikvisonHandler::HasNewSnapedFaceImg,this,&PreveiwWidget::DealNewSnapedFaceImg);
    connect(hkvision,&HikvisonHandler::HasNewDecodedImgData,this,&PreveiwWidget::DealNewDecodedImg);
    connect(hkvision,&HikvisonHandler::HasNewAudioData,this,&PreveiwWidget::DealNewAudioData);

    connect(this,&PreveiwWidget::RunSendImageThread,imgSender,&ImageSender::doWork);

    connect(tcpSocket,&QTcpSocket::connected,[=](){
        qDebug()<<"connected";
    });

//    if(!_camera.InitAndLogin())
//        return ;


}
PreveiwWidget::~PreveiwWidget()
{
    runInfer = false;
    delete sendImgThread;

    delete hkvision;
    if(_openPreview)
        _SwitchPreview();
    delete _buttonLayout;
    delete _pushbuttonSnap;
    delete _pushbuttonDec;
    delete infedRawPic;
    delete infedPic;




}

bool PreveiwWidget::_SwitchPreview()
{
    if(_openPreview)
    {
        //shutdown preview
        if(hkvision->StopRealPlay())
        {
            _openPreview = false;
            _pushbuttonDisplayPrev.setText("已经关闭了");
        }

    }
    else
    {
        //open display
        if(hkvision->SetupRealPlay(_frameDisplay.winId()))
        {
            _openPreview = true;
            _pushbuttonDisplayPrev.setText("已经打开了");
        }

    }
    return false;
}

void PreveiwWidget::ClickPushbuttonSnap()
{
    _camera.Snap();
}
void PreveiwWidget::ClickPushbuttonSnapV2()
{
    QByteArray *jpgData = new QByteArray();
    _camera.Snap(*jpgData);

    delete jpgData;
}
void PreveiwWidget::ClickPushbuttonDec()
{
//    _camera.InitPlayer();
    //可以直接connect
    hkvision->StartDecode();
}
void PreveiwWidget::ClickPushbuttonConnect()
{
//    qDebug()<<"connecting";
//    connect(tcpSocket,&QTcpSocket::connected,[](){
//       qDebug()<<"connected！";
//    });
    connect(tcpSocket,&QTcpSocket::readyRead,this,&PreveiwWidget::readyReadSocket);
    tcpSocket->connectToHost("localhost",9999);
//    isConnect = true;
//    sendImgThread = new std::thread(std::bind(&PreveiwWidget::SendImg,this,&runInfer,&sendImg,&curruentImg));
//    emit RunSendImageThread();

}
void PreveiwWidget::ClickPushbuttonInf()
{
//    if(tcpSocket->isWritable())
//    {
//        tcpSocket->write(*_camera.GetSnapedPicsBuff());
//        qDebug("sended");
//    }
    runInfer = true;

}

void PreveiwWidget::readyReadSocket()
{

    infedRawPic->clear();
    char buff[10240] = {0};
    int size;
    do{
        size = (tcpSocket->read(buff,10240));
        infedRawPic->append(buff,size);
//        qDebug()<<"recv:"<<size;
    }while(size);

    std::vector<unsigned char> picVector(infedRawPic->begin(),infedRawPic->end());
    cv::Mat *dispic = new cv::Mat();
    cv::imdecode(picVector,cv::IMREAD_UNCHANGED,dispic);
    cv::Mat convetdPic;
    cv::cvtColor(*dispic,convetdPic,cv::COLOR_BGR2RGB);
    QImage qpic = QImage(convetdPic.data,convetdPic.cols,convetdPic.rows,QImage::Format_RGB888);
    delete dispic;
//    emit NewInfedImg(QPixmap::fromImage(qpic));
    this->labelInfedImg->setPixmap(QPixmap::fromImage(qpic));
    if(infedImgW!=convetdPic.cols || infedImgH!=convetdPic.rows)
    {
        infedImgW = convetdPic.cols;
        infedImgH = convetdPic.rows;
        this->labelInfedImg->resize(infedImgW,infedImgH);
    }
    hasRecvImg = true;
}


void PreveiwWidget::DealNewSnapedFaceImg(QByteArray imgdata)
{
    std::vector<unsigned char> picVector(imgdata.begin(),imgdata.end());
    auto pic = cv::imdecode(picVector,cv::IMREAD_UNCHANGED);
    cv::Mat miniPic,convetdPic;
    cv::resize(pic, miniPic, cv::Size(), 0.25, 0.25);
//    cv::imshow("snaped img",miniPic);
//    cv::waitKey(0);
    cv::cvtColor(miniPic,convetdPic,cv::COLOR_BGR2RGB);
    QImage qpic = QImage(convetdPic.data,convetdPic.cols,convetdPic.rows,QImage::Format_RGB888);
    this->labelSnapedFaceImg->setPixmap(QPixmap::fromImage(qpic));
    if(snapedImgW!=convetdPic.cols || snapedImgH!=convetdPic.rows)
    {
        snapedImgH = convetdPic.rows;
        snapedImgW = convetdPic.cols;
        this->labelSnapedFaceImg->resize(snapedImgW,snapedImgH);
    }
}

void PreveiwWidget::DealNewDecodedImg(QByteArray imgdata)
{
//    qDebug()<<"datasize:"<<imgdata.size();
    std::vector<unsigned char> picVector(imgdata.begin(),imgdata.end());
//    qDebug()<<"vector size:"<<picVector.size();
    long w,h;
    memcpy(&h,imgdata.data(),4);
    memcpy(&w,imgdata.data()+4,4);
    cv::Mat yuvPic;
    yuvPic.create(h*3.0/2.0,w,CV_8UC1);
    memcpy(yuvPic.data,imgdata.data(),imgdata.size()-8);
    cv::Mat jpgPic;
    cv::cvtColor(yuvPic,jpgPic,cv::COLOR_YUV2BGRA_YV12);

    cv::Mat miniPic,convetdPic;
    cv::resize(jpgPic, miniPic, cv::Size(), 0.25, 0.25);

    if(true)
    {
        cv::Mat m;
        cv::resize(jpgPic, m, cv::Size(), 0.5, 0.5);
        std::vector<int> params;
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(20);
        cv::imencode(".jpg",m,curruentImg,params);
        if(tcpSocket->isWritable() && hasRecvImg)
        {
            tcpSocket->write((char *)curruentImg.data(),curruentImg.size());
            hasRecvImg = false;
        }
    }


    cv::cvtColor(miniPic,convetdPic,cv::COLOR_BGR2RGB);
    QImage qpic = QImage(convetdPic.data,convetdPic.cols,convetdPic.rows,QImage::Format_RGB888);
    this->labelDecodedImg->setPixmap(QPixmap::fromImage(qpic));
    if(decodedImgW!=w || decodedImgH!=h)
    {
        decodedImgH = h;
        decodedImgW = w;
        this->labelDecodedImg->resize(convetdPic.cols,convetdPic.rows);
    }

}


void PreveiwWidget::SendImg(bool *run,bool *isSendImg, std::vector<unsigned char> *data)
{

}

void PreveiwWidget::DealNewAudioData(QByteArray audioData)
{
    //do something

}
