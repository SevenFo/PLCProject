#ifndef PUTSTREAMDATETHREAD_H
#define PUTSTREAMDATETHREAD_H
#include "windows.h"
#include "PlayM4.h"
#include <QObject>
#include <QByteArray>

class PutStreamDateThread : public QObject
{
    Q_OBJECT
public:
    explicit PutStreamDateThread(long port,QObject *parent = nullptr);
    virtual ~PutStreamDateThread();

    /**
     * @brief doWork 独立的线程，向流里面转载数据
     * @param buf
     */
    void doWork(QByteArray *buf);
    void StopInput();
    void StartInput();
private:

    long _lPort;
    bool _bInputDate;

signals:
    void Done();
};

#endif // PUTSTREAMDATETHREAD_H
