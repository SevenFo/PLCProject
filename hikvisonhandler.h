#ifndef HIKVISONHANDLER_H
#define HIKVISONHANDLER_H

#include <QObject>
#include <QWidget>
#include "HCNetSDK.h"
#include "hkvsplayer.h"
#include <QXmlStreamReader>
#include <QObject>
#include <QByteArray>
#include <QTimer>
#include <QXmlStreamWriter>
#include "hikvisonhandlercstyelfuncs.h"
#include "newdatachecker.h"
#include "thread"

//extern void TestCFun();
class HikvisonHandler : public QObject
{
    Q_OBJECT
public:

    HikvisonHandler(QObject *parent = nullptr);
    virtual ~HikvisonHandler();

    bool SetupFaceDet();
    bool SetupPlayer();
    bool SetupRealPlay(WId winId = NULL);
    bool StopRealPlay();
    bool GetSnapedFaceImg(QByteArray *dist);
    bool StartDecode();


private:
    long userID;
    long playPort;
    long realPlayID;
    bool hasSetupRealPlay;
    bool runCheckStreamData;
    bool runCheckSnapedFaceImg;
    bool runCheckDecodedImgData;
    bool runCheckAudioData;

    QByteArray *snapedFaceImgData;
    QTimer *timerCheckHasNewSnapedImg;
    QThread *checkNewDataThread;
    NewDataChecker *newDataChecker;



    std::thread *checkStreamDataThread;
    std::thread *checkAudioDataThread;
    std::thread *checkImgDataThread;
    std::thread *checkSnapedFaceImgThread;


    void CheckSnapedImg();

    void CheckStreamData();
    void CheckAudioData();//decoded by player
    void CheckImgData();//decoded by player
    void CheckSnapedFaceImg();

signals:
    void newSnapedFaceImg(QByteArray *rawData);
    void StartInputStreamData();
    void GetAudioData();
    void HasNewData(char * data, size_t size);
    void HasNewAudioData(QByteArray audioData);
    void HasNewSnapedFaceImg(QByteArray faceData);
    void HasNewDecodedImgData(QByteArray imgData);

};




#endif // HIKVISONHANDLER_H
