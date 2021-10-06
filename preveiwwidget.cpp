#include "preveiwwidget.h"
#include <QDebug>
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/core/cuda.hpp"
//#include "opencv2/cudaimgproc.hpp"
#include "chrono"
#include <QThread>


#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

PreveiwWidget::PreveiwWidget(long userID, QWidget *parent) :QWidget(parent), _openPreview(false),
    decodedImgW(0),decodedImgH(0),infedImgW(0),infedImgH(0),snapedImgW(0),snapedImgH(0),
    sendImg(true),isConnect(false),hasRecvImg(false)
{

    hkvision = new HikvisonHandler(this);

    _buttonLayout = new QHBoxLayout();
    imgsLayout = new QHBoxLayout();
    lineEditLayout = new QHBoxLayout();
    _pushbuttonDec = new QPushButton(parent);
    pushbuttonConnect = new QPushButton(parent);
    pushbuttonStartFaceDet = new QPushButton(this);
    labelSnapedFaceImg = new QLabel();
    labelDecodedImg = new QLabel();
    labelInfedImg = new QLabel();
    infedRawPic = new QByteArray();
    infedPic = new QImage();
    imgSender = new ImageSender();
    sendImgThread = new QThread();
    tcpSocket = new QTcpSocket();

    lineeditIP = new QLineEdit();
    lineeditPort = new QLineEdit();
    lineeditPassword = new QLineEdit();
    lineeditUserID = new QLineEdit();
    pushbuttonSetupCamera = new QPushButton();


    this->setWindowTitle("Preview Windows");
    this->resize(600,600);

    _frameDisplay.resize(480,480);


    //总布局，竖向
    _vBoxLayout.addLayout(lineEditLayout);
    _vBoxLayout.addWidget(labelInfedImg);
    _vBoxLayout.addLayout(_buttonLayout);
    _vBoxLayout.addLayout(imgsLayout);


    //按钮布局，横向
    _buttonLayout->addWidget(&_pushbuttonDisplayPrev);
    _buttonLayout->addWidget(_pushbuttonDec);
    _buttonLayout->addWidget(pushbuttonConnect);
    _buttonLayout->addWidget(pushbuttonStartFaceDet);

    //图像布局，横向

    imgsLayout->addWidget(&_frameDisplay);
    imgsLayout->addWidget(labelDecodedImg);
    imgsLayout->addWidget(labelSnapedFaceImg);

    //输入布局，横向

    lineEditLayout->addWidget(lineeditIP);
    lineeditIP->setText("172.20.21.88");
    lineEditLayout->addWidget(lineeditPort);
    lineeditPort->setText("6666");
    lineEditLayout->addWidget(lineeditUserID);
    lineeditUserID->setText("admin");
    lineEditLayout->addWidget(lineeditPassword);
    lineeditPassword->setText("hkvs123456");
    lineEditLayout->addWidget(pushbuttonSetupCamera);
    pushbuttonSetupCamera->setText("setup");




    this->setLayout(&_vBoxLayout);//设置preview widget的布局


    _frameDisplay.setWindowTitle("Preview");
    _pushbuttonDisplayPrev.setText("switch display");
    _pushbuttonDec->setText("Dec");
    pushbuttonConnect->setText("Connnect");
    pushbuttonStartFaceDet->setText("Face Det");
    labelSnapedFaceImg->setText("snaped face img");
    labelDecodedImg->setText("Decoded img");
    labelInfedImg->setText("Infed img");

    connect(&_pushbuttonDisplayPrev,&QPushButton::clicked,this,&PreveiwWidget::_SwitchPreview);
    connect(_pushbuttonDec,&QPushButton::clicked,this,&PreveiwWidget::ClickPushbuttonDec);
    connect(pushbuttonConnect,&QPushButton::clicked,this,&PreveiwWidget::ClickPushbuttonConnect);
    connect(pushbuttonStartFaceDet,&QPushButton::clicked,hkvision,&HikvisonHandler::SetupFaceDet);

    connect(lineeditIP,&QLineEdit::editingFinished,[=](){
       hkvision->host = lineeditIP->text();
    });
    hkvision->host = lineeditIP->text();
    connect(lineeditPort,&QLineEdit::editingFinished,[=](){
       hkvision->port = lineeditPort->text();
    });
    hkvision->port = lineeditPort->text();
    connect(lineeditUserID,&QLineEdit::editingFinished,[=](){
       hkvision->userid = lineeditUserID->text();
    });
    hkvision->userid = lineeditUserID->text();
    connect(lineeditPassword,&QLineEdit::editingFinished,[=](){
       hkvision->password = lineeditPassword->text();
    });
    hkvision->password = lineeditPassword->text();
    connect(pushbuttonSetupCamera,&QPushButton::clicked,hkvision,&HikvisonHandler::SetupCamera);

    connect(hkvision,&HikvisonHandler::HasNewSnapedFaceImg,this,&PreveiwWidget::DealNewSnapedFaceImg);
    connect(hkvision,&HikvisonHandler::HasNewDecodedImgData,this,&PreveiwWidget::DealNewDecodedImg);
    connect(hkvision,&HikvisonHandler::HasNewAudioData,this,&PreveiwWidget::DealNewAudioData);

    connect(this,&PreveiwWidget::RunSendImageThread,imgSender,&ImageSender::doWork);

    connect(tcpSocket,&QTcpSocket::connected,[=](){
        qDebug()<<"connected";
    });



}
PreveiwWidget::~PreveiwWidget()
{
    delete sendImgThread;

    delete hkvision;
    if(_openPreview)
        _SwitchPreview();
    delete _buttonLayout;
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
        else
        {
            _pushbuttonDisplayPrev.setText("打开失败");
        }

    }
    return false;
}

void PreveiwWidget::ClickPushbuttonDec()
{
    //可以直接connect
    //开始解码
    hkvision->StartDecode();
}
void PreveiwWidget::ClickPushbuttonConnect()
{
    //连接本地服务器
    tcpSocket->connectToHost("localhost",9999);

    connect(tcpSocket,&QTcpSocket::readyRead,this,&PreveiwWidget::readyReadSocket);
    hasRecvImg = true;
}

void PreveiwWidget::readyReadSocket()
{

    infedRawPic->clear();
    char buff[102400] = {0};
    int size;
    do{
        size = (tcpSocket->read(buff,102400));
        infedRawPic->append(buff,size);
        qDebug()<<"recv:"<<size;
    }while(size);

    std::vector<unsigned char> picVector(infedRawPic->begin(),infedRawPic->end());
    cv::Mat *dispic = new cv::Mat();
    cv::imdecode(picVector,cv::IMREAD_UNCHANGED,dispic);

    cv::imshow("dispic",*dispic);
    cv::waitKey();

    cv::Mat convetdPic;
    cv::cvtColor(*dispic,convetdPic,cv::COLOR_BGR2RGB);

    QImage qpic = QImage(convetdPic.data,convetdPic.cols,convetdPic.rows,QImage::Format_RGB888);
    delete dispic;
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
    std::vector<unsigned char> picVector(imgdata.begin(),imgdata.end());

    long w,h;

    memcpy(&h,imgdata.data(),4);
    memcpy(&w,imgdata.data()+4,4);

    cv::Mat yuvPic;
    yuvPic.create(h*3.0/2.0,w,CV_8UC1);

    memcpy(yuvPic.data,imgdata.data()+8,imgdata.size()-8);

    cv::Mat jpgPic,convetdPic;
    cv::cvtColor(yuvPic,jpgPic,cv::COLOR_YUV2BGRA_YV12);

//    cv::Mat miniPic,convetdPic;
//    cv::resize(jpgPic, miniPic, cv::Size(), 0.25, 0.25);
#define COMP
#ifdef COMP
    if(true)
    {
//        cv::Mat m;
//        cv::resize(jpgPic, m, cv::Size(), 0.5, 0.5);
        std::vector<int> params;
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(100);
        if(tcpSocket->isWritable() && hasRecvImg)
        {
            cv::imencode(".jpg",jpgPic,curruentImg,params);
            tcpSocket->write((char *)curruentImg.data(),curruentImg.size());
            hasRecvImg = false;
        }
    }
#endif


//    cv::cvtColor(jpgPic,convetdPic,cv::COLOR_BGR2RGB);
//    QImage qpic = QImage(convetdPic.data,convetdPic.cols,convetdPic.rows,QImage::Format_RGB888);
//    this->labelDecodedImg->setPixmap(QPixmap::fromImage(qpic));
//    if(decodedImgW!=w || decodedImgH!=h)
//    {
//        decodedImgH = h;
//        decodedImgW = w;
//        this->labelDecodedImg->resize(convetdPic.cols,convetdPic.rows);
//    }

}


void PreveiwWidget::SendImg(bool *run,bool *isSendImg, std::vector<unsigned char> *data)
{

}

void PreveiwWidget::DealNewAudioData(QByteArray audioData)
{
    //do something

}



