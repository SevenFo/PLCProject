#include "newdatachecker.h"
#include "hikvisonhandlercstyelfuncs.h"
NewDataChecker::NewDataChecker(QObject *parent) : QObject(parent),runCheckStreamData(true),runCheckAudioData(true)
{

}
NewDataChecker::~NewDataChecker(){

}
void NewDataChecker::CheckStreamData()
{
    qDebug()<<"CheckStreamDataRun";
    streamDataCondi.wakeAll();
    while(runCheckStreamData)
    {

        dataMutex.lock();
        qDebug()<<"checker ready and wait";
        checkerReady = true;
        streamNewDataCondi.wait(&dataMutex);//释放锁，等待新数据
        checkerReady = false;
        emit HasNewData((char *)streamDataBuff,streamDataBuffSize);//得到新数据，发射信号
        dataMutex.unlock();
        //此时，callback还没lock，checker unlock，没有人拥有mutex
        streamDataCondi.notify_one();
        qDebug()<<"notify callback";
    }
    emit DoneCheckStreamData();
}

void NewDataChecker::StopCheckStreamData()
{
    runCheckStreamData = false;
}

