#include "hikvisonhandler.h"
#include "opencv2/opencv.hpp"
HikvisonHandler::HikvisonHandler(QObject *parent):QObject(parent),hasSetupRealPlay(false),runCheckStreamData(true),runCheckSnapedFaceImg(true),
    runCheckDecodedImgData(true),runCheckAudioData(true)
{
    TestCFun();

    timerCheckHasNewSnapedImg = new QTimer(this);
    snapedFaceImgData =  new QByteArray();


    NET_DVR_Init();
    NET_DVR_SetExceptionCallBack_V30(0,NULL,NULL,NULL);

    NET_DVR_USER_LOGIN_INFO struLoginInfo;
    memset(&struLoginInfo,0,sizeof(struLoginInfo));

    strcpy_s(struLoginInfo.sDeviceAddress,"192.168.1.64");
    struLoginInfo.wPort=8000;
    strcpy_s(struLoginInfo.sUserName ,"admin"); //设备登录用户名
    strcpy_s(struLoginInfo.sPassword,"hkvs123456"); //设备登录密码

    NET_DVR_DEVICEINFO_V40 struDeviceInfo;
    memset(&struDeviceInfo,0,sizeof(struDeviceInfo));


    userID = NET_DVR_Login_V40(&struLoginInfo,&struDeviceInfo);

    if(userID == -1)
    {
        qDebug()<<"login failed!";
        return;
    }
    qDebug()<<"userid: "<<userID<<" device type:"<<struDeviceInfo.struDeviceV30.wDevType<<" chanNumber:"<<struDeviceInfo.struDeviceV30.byChanNum<<" chanStart"<<struDeviceInfo.struDeviceV30.byStartChan<<" subproto"<<struDeviceInfo.struDeviceV30.bySubProto;

    qDebug()<<"setup camera success";

    if(!PlayM4_GetPort(&playPort))
    {
        qDebug()<<"get player port failed error code:"<<PlayM4_GetLastError(playPort);
        return;
    }
    qDebug()<<"player port:"<<playPort;

    if(!PlayM4_SetStreamOpenMode(playPort,0))//此模式（默认）下, 会尽量保正实时性, 防止数据阻塞; 而且数据检查严格
    {
        qDebug()<<"set stream mode failed error code:"<<PlayM4_GetLastError(playPort);
        return;
    }
    qDebug()<<"setup player success";



}
HikvisonHandler::~HikvisonHandler()
{
    runCheckDecodedImgData = false;
    runCheckSnapedFaceImg =  false;
    runCheckStreamData = false;
    runCheckAudioData = false;
    checkAudioDataThread->join();
    checkImgDataThread->join();
    checkSnapedFaceImgThread->join();
    checkStreamDataThread->join();
}

bool HikvisonHandler::SetupFaceDet()
{
    if(!hasSetupRealPlay)
    {
        qDebug()<<"pls setup real play firstly";
        return false;
    }
    if(!NET_DVR_SetDVRMessageCallBack_V50(1,&FaceDetAlarmCallback,NULL))
        return false;

    NET_DVR_SETUPALARM_PARAM *struSetupAlarmParam = new NET_DVR_SETUPALARM_PARAM();
    memset(struSetupAlarmParam,0,struSetupAlarmParam->dwSize);
    struSetupAlarmParam->dwSize = sizeof(NET_DVR_SETUPALARM_PARAM);
    struSetupAlarmParam->byLevel = 0;
    struSetupAlarmParam->byFaceAlarmDetection = 0;
    struSetupAlarmParam->byAlarmTypeURL = 0;//二进制传输

    auto alarmChanHandler = NET_DVR_SetupAlarmChan_V41(userID,struSetupAlarmParam);
    delete struSetupAlarmParam;

    if(alarmChanHandler == -1)
    {
        qDebug()<<"SetupAlarmChan failed, error code:"<<NET_DVR_GetLastError();
        return false;
    }
//    connect(this,&HikvisonHandler::HasNewSnapedFaceImg,[=](auto imgdata){
//        qDebug()<<"new snaped face img take 100s"<<QThread::currentThreadId();
//        qDebug()<<"imgs size:"<<snapedFaceImgs.size();
//        QByteArray img = imgdata;
//        qDebug()<<"img len:"<<img.size();
//        img.clear();
//        qDebug()<<"img len:"<<img.size();
//        snapedFaceImgMutex.unlock();
//        Sleep(100000);
//        qDebug()<<"dealing end";
//    });
    qDebug()<<"seted face det"<<QThread::currentThreadId();
    checkSnapedFaceImgThread = new std::thread(std::bind(&HikvisonHandler::CheckSnapedFaceImg,this));

//    connect(timerCheckHasNewSnapedImg,&QTimer::timeout,[=](){
//        if(!havNewSnapedFaceImg)
//        {
//            return ;
//        }
//        snapedFaceImgData->clear();
//        snapedFaceImgData->append((char *)snapedFaceImgBuff,snapedFaceImgBuffSize);
//        havNewSnapedFaceImg = false;
//        emit newSnapedFaceImg(snapedFaceImgData);//当有新的照片进来的时候发射信息
//    });

//    timerCheckHasNewSnapedImg->start(500);//0.5s检查 一次有没有新的人脸照片
    return true;
}

bool HikvisonHandler::SetupRealPlay(WId winId)
{
    NET_DVR_PREVIEWINFO struPreviewInfo;
    struPreviewInfo.dwStreamType = 1;//子流
    struPreviewInfo.dwLinkMode = 0;//TCP
    struPreviewInfo.hPlayWnd = (HWND) winId;
    struPreviewInfo.bBlocked = false;
    struPreviewInfo.lChannel = 1;
    realPlayID = NET_DVR_RealPlay_V40(userID,&struPreviewInfo,&RealDataCallback);
    if(realPlayID == -1)
    {
        hasSetupRealPlay = false;
//        return false;
    }
    else
    {
        qDebug()<<"open real stream success";
        hasSetupRealPlay = true;
        return true;
    }



    return true;
}

bool HikvisonHandler::StopRealPlay()
{
    auto result = NET_DVR_StopRealPlay(realPlayID);
    if(result == -1)
    {
//        hasSetupRealPlay = true;
        return false;
    }
    else
    {
        hasSetupRealPlay = false;
        return true;
    }
}

bool HikvisonHandler::GetSnapedFaceImg(QByteArray *dist)
{
    if(!havNewSnapedFaceImg)
    {
        return false;
    }
    dist->clear();
    dist->append(*snapedFaceImgData);
    return true;
}

bool HikvisonHandler::SetupPlayer()
{
    return true;
}

bool HikvisonHandler::StartDecode()
{
    checkStreamDataThread = new std::thread(std::bind(&HikvisonHandler::CheckStreamData,this));
    checkAudioDataThread = new std::thread(std::bind(&HikvisonHandler::CheckAudioData,this));
    checkImgDataThread = new std::thread(std::bind(&HikvisonHandler::CheckImgData,this));

    connect(this,&HikvisonHandler::HasNewData,[=](char * data,size_t size){
       if(!PlayM4_InputData(playPort,(PBYTE)data,size))
       {
           qDebug()<<"input stream data failed error code::"<<PlayM4_GetLastError(playPort);
           return;
       }
    });

    if(!streamHeadDataBuffSize)//检查是否接收到数据头
    {
        qDebug()<<"pls open real play firstly";
        return false;
    }

    if(!PlayM4_OpenStream(playPort,streamHeadDataBuff,streamHeadDataBuffSize,SOURCE_BUF_MAX))
    {
        qDebug()<<"PlayM4_OpenStream failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }


    if(!PlayM4_Play(playPort,NULL))
    {
        qDebug()<<"PlayM4_Play failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }

    if(!PlayM4_SetDecCallBackExMend(playPort,&DecodedDataCallback,NULL,0,0))
    {
        qDebug()<<"PlayM4_SetDecCallBackExMend failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }

    if(!PlayM4_PlaySound(playPort))
    {
        qDebug()<<"PlayM4_PlaySound failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }
    if(!PlayM4_SetVolume(playPort,0))
    {
        qDebug()<<"PlayM4_SetVolume failed error code:"<<PlayM4_GetLastError(playPort);
        return false;
    }
    qDebug()<<"decoder checked";

    hasStartDecode = true;
}

void HikvisonHandler::CheckStreamData()
{
    qDebug()<<"run thread:"<<QThread::currentThreadId() <<runCheckStreamData;
    streamDataCondi.wakeAll();
    while(runCheckStreamData)
    {
        dataMutex.lock();
        checkerReady = true;
        streamNewDataCondi.wait(&dataMutex);//释放锁，等待新数据
        checkerReady = false;
        emit HasNewData((char *)streamDataBuff,streamDataBuffSize);//得到新数据，发射信号
        dataMutex.unlock();
        //此时，callback还没lock，checker unlock，没有人拥有mutex
        streamDataCondi.notify_one();
    }
    qDebug()<<"stop stream checker";
}

void HikvisonHandler::CheckAudioData()
{
    qDebug()<<"run thread:"<<QThread::currentThreadId();
//    audioDataCondi.wakeAll();
    while(runCheckAudioData)
    {
        audioDataMutex.lock();
        while(audioDatas.isEmpty())
        {
//            qDebug()<<"snaped face img buff is empty wait";
           audioDataEmptyCondi.wait(&audioDataMutex);
        }
        //have data
        QByteArray data = audioDatas.takeFirst();
        audioDataMutex.unlock();
        emit HasNewAudioData(data);
        audioDataFullCondi.notify_one();
        //通知完过后，回调函数线程被唤醒，但要等待check线程wait之后他才能lock（反正回调函数执行过快，在checkaudiowait之前就notify），才能通知checkemit
        //正常应该是若缓冲区是满的，则callback wati等待emit，如果缓冲区是空的，说明check执行的比callback快，就没必要wait了
        //对于check来说，若缓冲区是空的，则等待，非则就不需要等待
        //在这里是不管怎么样check都等待，即默认callback的速度比checker慢
    }
    qDebug()<<"stop audio checker";
}

void HikvisonHandler::CheckImgData()
{
    qDebug()<<"run thread:"<<QThread::currentThreadId();
//    audioDataCondi.wakeAll();
    while(runCheckDecodedImgData)
    {
        decodedImgMutex.lock();
        while(decodedImgs.isEmpty())
        {
           decodedImgEmptyCondi.wait(&decodedImgMutex);
        }
        //have data
        QByteArray data = decodedImgs.takeFirst();
        decodedImgMutex.unlock();
        emit HasNewDecodedImgData(data);
        decodedImgFullCondi.notify_one();
    }
    qDebug()<<"stop img checker";
}

void HikvisonHandler::CheckSnapedFaceImg()
{
    while(runCheckSnapedFaceImg)
    {
        snapedFaceImgMutex.lock();
        while(snapedFaceImgs.isEmpty())
        {
            snapedFaceImgEmptyCondi.wait(&snapedFaceImgMutex);
        }
        //have data
        auto data = snapedFaceImgs.takeFirst();
        snapedFaceImgMutex.unlock();
        emit HasNewSnapedFaceImg(data);
        snapedFaceImgFullCondi.notify_one();
    }
    qDebug()<<"stop snap checker";
}
