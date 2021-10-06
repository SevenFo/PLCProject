#ifndef PREVEIWWIDGET_H
#define PREVEIWWIDGET_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFrame>
#include <QTcpSocket>
#include "HCNetSDK.h"
class MainWindow;
#include "mainwindow.h"
#include "hikvisonhandler.h"
#include <QLabel>
#include "opencv2/opencv.hpp"
#include "thread"
#include <QThread>
#include "imagesender.h"


class PreveiwWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreveiwWidget(long userID,QWidget *parent = nullptr);
    virtual ~PreveiwWidget();

    HikvisonHandler *hkvision;

    bool _SwitchPreview();//切换是否打开实时预览

private:



    QVBoxLayout _vBoxLayout;
    QHBoxLayout *_buttonLayout;
    QHBoxLayout *imgsLayout;
    QHBoxLayout *lineEditLayout;
    QPushButton _pushbuttonDisplayPrev;
    QPushButton *_pushbuttonDec;
    QPushButton *pushbuttonConnect;
    QPushButton *pushbuttonInf; //推断
    QPushButton *pushbuttonStartFaceDet;
    QLabel *labelSnapedFaceImg;//显示snape face img
    QLabel *labelDecodedImg;
    QLabel *labelInfedImg;
    QTcpSocket *tcpSocket;
    QByteArray *infedRawPic;//推断出来的二进制图片数据
    std::vector<unsigned char> curruentImg;
    QImage *infedPic;//推断出来的图片数据
    QFrame _frameDisplay;//预览frame

    QLineEdit *lineeditIP;
    QLineEdit *lineeditPort;
    QLineEdit *lineeditUserID;
    QLineEdit *lineeditPassword;
    QPushButton *pushbuttonSetupCamera;



    int decodedImgW,infedImgW,snapedImgW;
    int decodedImgH,infedImgH,snapedImgH;

    bool _openPreview;
    bool runInfer;
    bool sendImg;
    bool isConnect,hasRecvImg;

    QThread *sendImgThread;
    ImageSender *imgSender;


    void ClickPushbuttonDec();
    void ClickPushbuttonConnect();
    void ClickPushbuttonInf();
    void readyReadSocket();
    void DealNewSnapedFaceImg(QByteArray imgdata);
    void DealNewDecodedImg(QByteArray imgdata);
    void DealNewAudioData(QByteArray audioData);
    void SendImg(bool *run,bool *sendImg, std::vector<unsigned char> *data);
signals:
    void RecvedNewInfedPic(QImage & infedPic);
    void RunSendImageThread();

};

#endif // PREVEIWWIDGET_H
