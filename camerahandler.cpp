#include "camerahandler.h"
#include <QDebug>

QByteArray *CameraHandler::streamDateBuf = nullptr;
QByteArray *CameraHandler::snapedPicsBuff = nullptr;
QByteArray *CameraHandler::streamHeadDataBuf = nullptr;
HKVSPlayer *CameraHandler::player = nullptr;

CameraHandler::CameraHandler():_lUserID(-1),_lPreviewHandler(-1)
{
    snapedPicsBuff = new QByteArray();

    player = new HKVSPlayer();


}
CameraHandler::~CameraHandler()
{
    if(_lUserID>=0)
        NET_DVR_Logout(_lUserID);
    if(!NET_DVR_Cleanup())
    {
        qDebug()<<"clean failed";
    }
    else
    {
        qDebug()<<"clean cuccess";
    }

    delete streamDateBuf;
    delete snapedPicsBuff;
    delete streamHeadDataBuf;
    delete player;
}

// 初始化设备并且登入
bool CameraHandler::InitAndLogin()
{
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


    _lUserID = NET_DVR_Login_V40(&struLoginInfo,&struDeviceInfo);

    if(_lUserID == -1)
    {
        qDebug()<<"login failed!";
        return false;
    }
    qDebug()<<"userid: "<<_lUserID<<" device type:"<<struDeviceInfo.struDeviceV30.wDevType<<" chanNumber:"<<struDeviceInfo.struDeviceV30.byChanNum<<" chanStart"<<struDeviceInfo.struDeviceV30.byStartChan<<" subproto"<<struDeviceInfo.struDeviceV30.bySubProto;
//    this->GetAbilityInfo();

    //获取人脸检测的配置，被且设置配置
    GetFacesnapConfig();//get and set facesnap config

    InitFaceDet();//init face det(snap) set callback and setup alarmchan

//   player->InitStreamPlayMode()
//    player->InitPlayer();
//    player->SnapManual();
    return true;
}


bool CameraHandler::OpenPreview(HWND hWind)
{
    NET_DVR_PREVIEWINFO struPreviewInfo;
    struPreviewInfo.dwStreamType = 1;//子流
    struPreviewInfo.dwLinkMode = 0;//TCP
    struPreviewInfo.hPlayWnd = hWind;
    struPreviewInfo.bBlocked = false;
    struPreviewInfo.lChannel = 1;
    _lPreviewHandler = NET_DVR_RealPlay_V40(_lUserID,&struPreviewInfo,&CameraHandler::PreviewDateCallback);
    if(_lPreviewHandler == -1)
        return false;
    else
        return true;
}

bool CameraHandler::StopPreview()
{
    auto result = NET_DVR_StopRealPlay(_lPreviewHandler);
    if(result == -1)
        return false;
    else
        return true;
}

// 初始化人脸检测、设置警报模式
bool CameraHandler::InitFaceDet()
{
//    if(NET_DVR_SetDeviceConfig(_lUserID,NET_DVR_SET_FACE_DETECT,1,))
    if(!NET_DVR_SetDVRMessageCallBack_V50(1,&CameraHandler::FaceDetCallback,NULL))
        return false;
    NET_DVR_SETUPALARM_PARAM *struSetupAlarmParam = new NET_DVR_SETUPALARM_PARAM();
    memset(struSetupAlarmParam,0,struSetupAlarmParam->dwSize);
    struSetupAlarmParam->dwSize = sizeof(NET_DVR_SETUPALARM_PARAM);
    struSetupAlarmParam->byLevel = 0;
    struSetupAlarmParam->byFaceAlarmDetection = 0;
    struSetupAlarmParam->byAlarmTypeURL = 0;//二进制传输

    auto alarmChanHandler = NET_DVR_SetupAlarmChan_V41(_lUserID,struSetupAlarmParam);
    delete struSetupAlarmParam;

    if(alarmChanHandler == -1)
    {
        qDebug()<<"SetupAlarmChan failed, error code:"<<NET_DVR_GetLastError();
        return false;
    }

    return true;

}

void CameraHandler::PreviewDateCallback(long lReanHandle, DWORD dwDataType,BYTE *pBuffer,DWORD dwBufSize, void *pUser)
{
    switch (dwDataType) {
    case NET_DVR_SYSHEAD:{
        qDebug()<<"system head date";
        CameraHandler::streamHeadDataBuf = new QByteArray((char *)pBuffer,dwBufSize);
        break;
    }
    case NET_DVR_STREAMDATA:{
//        qDebug()<<"stream date";
        CameraHandler::streamDateBuf = new QByteArray((char *)pBuffer,dwBufSize);
        player->InputStreamData(*streamDateBuf);
        break;
    }
    case NET_DVR_AUDIOSTREAMDATA:{qDebug()<<"audio date";break;}
    case NET_DVR_PRIVATE_DATA:{qDebug()<<"private date";break;}
    default:{qDebug()<<"unknown date";break;}
    }
}
void CameraHandler::FaceDetCallback(  LONG               lCommand,
                                              NET_DVR_ALARMER    *pAlarmer,
                                              char               *pAlarmInfo,
                                              DWORD              dwBufLen,
                                              void               *pUser)
{
    NET_VCA_FACESNAP_RESULT struFacenapResult;
    NET_DVR_FACE_DETECTION struFaceDetection;
    QFile f = QFile("pic.jpg");
    switch(lCommand)
    {
    //人脸抓拍
    case COMM_UPLOAD_FACESNAP_RESULT:{
        qDebug()<<"Get Face Message (snap)";
        memcpy_s(&struFacenapResult,sizeof(struFacenapResult),pAlarmInfo,((NET_VCA_FACESNAP_RESULT*)pAlarmInfo)->dwSize);
        qDebug()<<"dwFaceScore:"<<struFacenapResult.dwFaceScore<<"dwFacePicLen:"<<struFacenapResult.dwFacePicLen<<"byUploadEventDataType:"<<struFacenapResult.byUploadEventDataType<<"fStayDuration:"<<struFacenapResult.fStayDuration<<"dwBackgroundPicLen:"<<struFacenapResult.dwBackgroundPicLen<<"struRect: width:"<<struFacenapResult.struRect.fWidth<<"height:"<<struFacenapResult.struRect.fHeight<<"x:"<<struFacenapResult.struRect.fX<<"y:"<<struFacenapResult.struRect.fY;
        snapedPicsBuff->clear();
        CameraHandler::snapedPicsBuff->append((char *)struFacenapResult.pBuffer1,struFacenapResult.dwFacePicLen);
        qDebug()<<"pic buff size:"<<snapedPicsBuff->size();

        f.open(QIODevice::WriteOnly);
        f.write(*CameraHandler::snapedPicsBuff);
        f.close();

        break;
    //人脸侦测
    case COMM_ALARM_FACE_DETECTION:{
            qDebug()<<"Get Face Message (detection)";
        memcpy_s(&struFaceDetection,sizeof(struFaceDetection),pAlarmInfo,((NET_DVR_FACE_DETECTION*)pAlarmInfo)->dwSize);
        break;
        }
    }
    default:{qDebug()<<"use less msg";break;}
    }

}

void CameraHandler::GetAbilityInfo()
{
    QString in = QString();
//    in->resize(100);
    QXmlStreamWriter inputXml(&in);
//    inputXml.setAutoFormatting(true);
    inputXml.writeStartDocument();
    inputXml.writeStartElement("VcaChanAbility");

    inputXml.writeAttribute("version","2.0");
//    inputXml.writeTextElement("channelNO","1");
    inputXml.writeTextElement("channelNO","1");
    inputXml.writeEndElement();
    inputXml.writeEndDocument();
    qDebug()<<in;
    char outBuffer[10240];
    if(NET_DVR_GetDeviceAbility(_lUserID,VCA_DEV_ABILITY,(char *)in.toStdString().c_str(),in.size(),outBuffer,10240))
    {
        QString out = QString(QLatin1String(outBuffer));
        qDebug()<<outBuffer;
    }
    else
    {
        qDebug()<<"failed, error code:"<<NET_DVR_GetLastError();
    }
}

void CameraHandler::GetFacesnapConfig()
{
    NET_VCA_FACESNAPCFG *struFacesnapConfig = new NET_VCA_FACESNAPCFG();
    DWORD * lpConfigSize = new DWORD;
    if(!NET_DVR_GetDVRConfig(_lUserID,NET_DVR_GET_FACESNAPCFG,1,struFacesnapConfig,sizeof(NET_VCA_FACESNAPCFG),lpConfigSize))
    {
        qDebug()<<"failed, error code:"<<NET_DVR_GetLastError();
    }

    qDebug()<<"snaptime"<<struFacesnapConfig->bySnapTime<<"threshold"<<struFacesnapConfig->bySnapThreshold<<"pic type:"<<struFacesnapConfig->struPictureParam.wPicSize<<"open rule"<<struFacesnapConfig->struRule->byActive<<"dwFaceFilteringTime:"<<struFacesnapConfig->dwFaceFilteringTime<<" bySnapInterval:"<<struFacesnapConfig->bySnapInterval<<"dwValidFaceTime:"<<struFacesnapConfig->dwValidFaceTime<<"dwUploadInterval"<<struFacesnapConfig->dwUploadInterval;

//    struFacesnapConfig->bySnapThreshold = 10;
//    struFacesnapConfig->bySensitive = 5;
//    struFacesnapConfig->dwFaceFilteringTime = 0;
//    struFacesnapConfig->bySnapInterval = 1;//抓拍间隔，单位：帧
//    struFacesnapConfig->dwValidFaceTime = 1;//有效人脸最短持续时间，单位：秒
//    struFacesnapConfig->dwUploadInterval = 1;//人脸抓拍统计数据上传间隔时间，单位：秒，默认为900秒
//    struFacesnapConfig->struRule->byActive = true;

    if(!NET_DVR_SetDVRConfig(_lUserID,NET_DVR_SET_FACESNAPCFG,1,struFacesnapConfig,struFacesnapConfig->dwSize))
    {
        qDebug()<<"failed, error code:"<<NET_DVR_GetLastError();
    }
    qDebug()<<"set face snap config success!";

    delete struFacesnapConfig;
    delete lpConfigSize;


}

bool CameraHandler::Snap(QString fileName)
{
    //*************设备抓图，不需要调用播放器的SDK**********
    //JPEG图像信息结构体。
    NET_DVR_JPEGPARA jpegInfo = NET_DVR_JPEGPARA();
    jpegInfo.wPicQuality = 1;//较好
    jpegInfo.wPicSize = 5;//HD
    QFile *picFile;
    char *picBuff = new char[1280*720*3/2];
    DWORD picSize;
    if(fileName == "")
    {
        picFile = new QFile(QString::number( QTime::currentTime().msecsSinceStartOfDay())+"_"+QString::number(1280)+"_"+QString::number(720)+".jpg");
    }
    else
        picFile = new QFile("");
    if(!NET_DVR_CaptureJPEGPicture_NEW(_lUserID,1,&jpegInfo,picBuff,1280*720*3/2,&picSize))
    {
        qDebug()<<"capture jpeg failed error code:"<<NET_DVR_GetLastError();
        return false;
    }
    qDebug()<<"file name:"<<picFile->fileName();
    auto openresult = picFile->open(QIODevice::WriteOnly);
    if(!openresult)
    {
        qDebug()<<"open file failed";
        return false;
    }
    auto result = picFile->write(picBuff,picSize);
    if(result ==-1)
    {
        qDebug()<<"wirte file failed";
        return false;
    }
    qDebug()<<"pic file size:"<<result;
    return true;
}

bool CameraHandler::Snap(QByteArray &data)
{
    //*************设备抓图，不需要调用播放器的SDK**********
    //JPEG图像信息结构体。
    NET_DVR_JPEGPARA jpegInfo = NET_DVR_JPEGPARA();
    jpegInfo.wPicQuality = 1;//较好
    jpegInfo.wPicSize = 5;//HD
    char *picBuff = new char[1280*720*3/2];
    DWORD picSize;

    if(!NET_DVR_CaptureJPEGPicture_NEW(_lUserID,1,&jpegInfo,picBuff,1280*720*3/2,&picSize))
    {
        qDebug()<<"capture jpeg failed error code:"<<NET_DVR_GetLastError();
        return false;
    }

    data = QByteArray(picBuff,picSize);
    qDebug()<<"pic data size:"<<picSize;

    return true;
}

bool CameraHandler::InitPlayer()
{
    if(!player->InitStreamPlayMode((PBYTE)CameraHandler::streamHeadDataBuf->data(),CameraHandler::streamHeadDataBuf->size()))
    {
        qDebug()<<"init player failed";
        return false;
    }
    return true;
}

QByteArray *CameraHandler::GetSnapedPicsBuff()
{
    return snapedPicsBuff;
}
