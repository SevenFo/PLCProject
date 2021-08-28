#ifndef HKVSPLAYER_H
#define HKVSPLAYER_H
#include "windows.h"
#include "PlayM4.h"
#include "putstreamdatethread.h"
#include <QByteArray>
#include <QThread>
#include <QFile>
#include <QTime>
class HKVSPlayer
{

    typedef void (CALLBACK* DecCBFun)(long nPort, char* pBuf, long nSize, FRAME_INFO* pFrameInfo, void* nUser, void* nReserved2);
    typedef void (CALLBACK* DisplayCBFun)(long nPort,char * pBuf,long nSize,long nWidth,long nHeight,long nStamp,long nType,void* nReserved);
public:
    HKVSPlayer();
    virtual ~HKVSPlayer();
    bool InitPlayer();
    bool InitStreamPlayMode(PBYTE pStreamHeadBuf,DWORD lSize ,HWND hWnd = NULL);
    bool SnapManual(QString fileName = "");
    void StartInputStreamDate();
    bool InputStreamData(QByteArray &streamData);
    static void VideoDateCallback(long nPort, char* pBuf, long nSize, FRAME_INFO* pFrameInfo, void* nUser, void* nReserved2);
    static void DisplayCallback(long nPort,char * pBuf,long nSize,long nWidth,long nHeight,long nStamp,long nType,void* nReserved);

    QByteArray *streamDateBuf;

    QByteArray *videoDateBuf;
    QByteArray *audioDateBuf;


    static QByteArray decDateBuf;
    static FRAME_INFO decDateInfo;
    static HKVSPlayer *thisPointer;
    static QFile *audioFile;
    static bool snap;//控制是否连续抓图


signals:
    void RecvDate(char *pBuf,long nSize, FRAME_INFO *pFrameInfo);


private:

    long _lPort;
    PutStreamDateThread *_inputStreamDate;


    /**
     * @brief InitStreamPlayMode 初始化流模式播放
     * @param pStreamHeadBuf 流数据的数据头缓冲区指针 lSize 缓冲区大小 hWnd 窗口句柄
     * @return 成功与否
     */

    //初始化连续抓图
    bool InitSnap(DisplayCBFun callback);
    bool StopSnap();


};

#endif // HKVSPLAYER_H
