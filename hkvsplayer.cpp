#include "hkvsplayer.h"
#include <QDebug>


QByteArray HKVSPlayer::decDateBuf = QByteArray();
//HKVSPlayer* HKVSPlayer::thisPointer = nullptr;
FRAME_INFO HKVSPlayer::decDateInfo = FRAME_INFO();
bool HKVSPlayer::snap = false;
QFile *HKVSPlayer::audioFile = nullptr;


HKVSPlayer::HKVSPlayer():_lPort(-1)
{
    streamDateBuf = new QByteArray(2048,0);
    videoDateBuf = new QByteArray();
    audioDateBuf = new QByteArray();
//    _inputStreamDateThread = new QThread();

    audioFile = new QFile("recordAudio.pcm");
    audioFile->open(QIODevice::WriteOnly);

}
HKVSPlayer::~HKVSPlayer()
{
    audioFile->close();
    delete streamDateBuf;
//    delete _inputStreamDateThread;
    delete videoDateBuf;
    delete audioDateBuf;
    delete audioFile;
//    delete _decDateBuf;
//    delete _decDateInfo;
}
bool HKVSPlayer::InitPlayer()
{
    if(!PlayM4_GetPort(&_lPort))
    {
        qDebug()<<"get player port failed error code:"<<PlayM4_GetLastError(_lPort);
        return false;
    }
    qDebug()<<"player port:"<<_lPort;
    return true;
}
bool HKVSPlayer::InitStreamPlayMode(PBYTE pStreamHeadBuf,DWORD lSize ,HWND hWnd)
{
    if(_lPort == -1)
    {
        if(!InitPlayer())
            return false;
    }


    if(!PlayM4_SetStreamOpenMode(_lPort,0))//此模式（默认）下, 会尽量保正实时性, 防止数据阻塞; 而且数据检查严格
    {
        qDebug()<<"set stream mode failed error code:"<<PlayM4_GetLastError(_lPort);
        return false;
    }

    if(!PlayM4_OpenStream(_lPort,pStreamHeadBuf,lSize,SOURCE_BUF_MAX))
    {
        qDebug()<<"PlayM4_OpenStream failed error code:"<<PlayM4_GetLastError(_lPort);
        return false;
    }

    if(!PlayM4_Play(_lPort,hWnd))
    {
        qDebug()<<"PlayM4_Play failed error code:"<<PlayM4_GetLastError(_lPort);
        return false;
    }
    if(!PlayM4_PlaySound(_lPort))
    {
        qDebug()<<"PlayM4_PlaySound failed error code:"<<PlayM4_GetLastError(_lPort);
        return false;
    }
    if(!PlayM4_SetVolume(_lPort,0))
    {
        qDebug()<<"PlayM4_SetVolume failed error code:"<<PlayM4_GetLastError(_lPort);
        return false;
    }

    if(!PlayM4_SetDecCallBackExMend(_lPort,&HKVSPlayer::VideoDateCallback,NULL,0,0))
    {
        qDebug()<<"PlayM4_Play failed error code:"<<PlayM4_GetLastError(_lPort);
        return false;
    }
    qDebug()<<"InitStreamPlayMode success";
    return true;

}


//*******连续抓图***********
//设置显示的回调函数，该回调函数会返回当前显示的图像的相关信息，根据这个来抓图
bool HKVSPlayer::InitSnap(DisplayCBFun callback){
    if(!PlayM4_SetDisplayCallBack(_lPort,callback))
    {
        qDebug()<<"set display callbcak failed error code::"<<PlayM4_GetLastError(_lPort);
        return false;
    }
    return true;
}

bool HKVSPlayer::StopSnap(){
    if(!PlayM4_SetDisplayCallBack(_lPort,NULL))
    {
        qDebug()<<"stop display callbcak failed error code::"<<PlayM4_GetLastError(_lPort);
        return false;
    }
    return true;
}
//******************



bool HKVSPlayer::SnapManual(QString fileName)
{
    long high,width;
    QByteArray buf;
    if(!PlayM4_GetPictureSize(_lPort,&width,&high))
    {
        qDebug()<<"get picture size failed error code:"<<PlayM4_GetLastError(_lPort);
        return false;
    }
    buf.resize(high*width*3/2);
    DWORD picSize;
    if(!PlayM4_GetJPEG(_lPort,(PBYTE)buf.data(),buf.size(),&picSize))
    {
        qDebug()<<"snap picture failed error code:"<<PlayM4_GetLastError(_lPort);
        return false;
    }
    // save to file
    QFile picFile;
    if(fileName == "")
    {
        QFile picFile = QFile(QTime::currentTime().toString()+"_"+QString::number(width)+"_"+QString::number(high)+".jpg");
    }
    else
    {
        QFile picFile = QFile(fileName);
    }
    auto fileSize = picFile.write(buf);
    if(fileSize == -1){
        qDebug()<<"write file failed";
    }
    else
        qDebug()<<"write file: "<<fileSize;

    return true;
}


void HKVSPlayer::VideoDateCallback(long nPort, char *pBuf, long nSize, FRAME_INFO *pFrameInfo, void *nUser, void *nReserved2)
{
    //数据复制操作
//    memcpy_s(dateBuff,10240,pBuf,nSize);
//    HKVSPlayer::decDateBuf.resize(nSize);
    HKVSPlayer::decDateBuf = QByteArray(pBuf,nSize);
    memcpy_s(&HKVSPlayer::decDateInfo,sizeof(FRAME_INFO),pFrameInfo,sizeof(FRAME_INFO));
    qDebug()<<"copied video data info";
    qDebug()<<"width: "<<pFrameInfo->nWidth<<" height:"<<pFrameInfo->nHeight<<" rate:"<<pFrameInfo->nFrameRate<<" frameNumber:"<<pFrameInfo->dwFrameNum<<" type:"<<pFrameInfo->nType <<"stamp:"<<pFrameInfo->nStamp;
    switch(pFrameInfo->nType)
    {
    case T_AUDIO16:{
        qDebug()<<"audio data";
        audioFile->write(decDateBuf);
        break;
    }
    case  T_YV12:{
        qDebug()<<"video data";
        break;
    }
    default:
    {
        qDebug()<<"other data";
        break;
    }
    }
}

void HKVSPlayer::DisplayCallback(long nPort, char *pBuf, long nSize, long nWidth, long nHeight, long nStamp, long nType, void *nReserved)
{
    //连续抓图
    if(HKVSPlayer::snap)
    {
        qDebug()<<"Snaped";
        //todo
    }
}

bool HKVSPlayer::InputStreamData(QByteArray &streamData)
{
    auto result = PlayM4_InputData(_lPort, (PBYTE)streamData.data(), streamData.size());

    if(!result)
    {
        qDebug()<<"input stream data failed error code::"<<PlayM4_GetLastError(_lPort);
        return false;
    }
    else return true;
}
