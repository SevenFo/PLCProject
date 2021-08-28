#ifndef CAMERAHANDLER_H
#define CAMERAHANDLER_H
#include "HCNetSDK.h"
#include "hkvsplayer.h"
#include <QXmlStreamReader>
#include <QObject>
#include <QByteArray>
#include <QXmlStreamWriter>
class CameraHandler
{
//    QObject
public:
    CameraHandler();
    virtual ~CameraHandler();
    bool InitAndLogin();
    bool InitPreview();

    bool OpenPreview();
    bool OpenPreview(HWND hWind);

    bool StopPreview();

    bool InitFaceDet();

    bool Snap(QString fileName = "");
    bool Snap(QByteArray &data);
    bool InitPlayer();
    QByteArray * GetSnapedPicsBuff();


    static QByteArray *streamDateBuf;
    static QByteArray *snapedPicsBuff;
    static QByteArray *streamHeadDataBuf;
private:

    //handler type:long
    long _lUserID;
    long _lPreviewHandler;



//    HKVSPlayer* player;

    void GetAbilityInfo();
    void GetFacesnapConfig();


    static HKVSPlayer *player;
    static void PreviewDateCallback(long lReanHandle, DWORD dwDataType,BYTE *pBuffer,DWORD dwBufSize, void *pUser);
    static void FaceDetCallback(  LONG               lCommand,
                                  NET_DVR_ALARMER    *pAlarmer,
                                  char               *pAlarmInfo,
                                  DWORD              dwBufLen,
                                  void               *pUser);


};


#endif // CAMERAHANDLER_H
