#ifndef NEWDATACHECKER_H
#define NEWDATACHECKER_H

#include <QObject>

class NewDataChecker : public QObject
{
    Q_OBJECT
public:
    explicit NewDataChecker(QObject *parent = nullptr);
    virtual ~NewDataChecker();

    void CheckStreamData();
    void CheckAudioData();
    void StopCheckAudioData();
    void StopCheckStreamData();

private:
    bool runCheckStreamData;
    bool runCheckAudioData;
signals:
    void HasNewData(char *data,size_t size);
    void HasNewAudioData(char *data,size_t size);
    void DoneCheckStreamData();
};

#endif // NEWDATACHECKER_H
