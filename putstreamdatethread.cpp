#include "putstreamdatethread.h"
#include <QDebug>

PutStreamDateThread::PutStreamDateThread(long port,QObject *parent) :QObject(parent),_lPort(port),_bInputDate(true)
{

}
PutStreamDateThread::~PutStreamDateThread()
{

}

void PutStreamDateThread::doWork(QByteArray *buf)
{
    while(_bInputDate)
    {
        auto result = PlayM4_InputData(_lPort,(PBYTE)buf->data(),buf->size());
        if(!result)
        {
            auto errorCode = PlayM4_GetLastError(_lPort);
            qDebug()<<"input stream date failed error code:"<<errorCode;
            if(errorCode == PLAYM4_BUF_OVER)
            {
                Sleep(2);
                continue;
            }
            else
                break;
        }
    }
    emit Done();
}
void PutStreamDateThread::StopInput()
{
    _bInputDate = false;
}

void PutStreamDateThread::StartInput()
{
    _bInputDate = true;
}
