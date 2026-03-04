#ifndef BROADCASTWORKER_H
#define BROADCASTWORKER_H

#include <QObject>
#include <QMutexLocker>

class BroadcastWorker : public QObject
{
    Q_OBJECT
public:
    explicit BroadcastWorker(QObject *parent = nullptr);

public:
    void updateOpenCloseSign(bool openClose, double delayTime);
    void threadRun();

    bool isRunning = false;
    double delayTimeE = 100;

    QByteArray m_broadcastData;
    QMutex m_mutex;
    void updateBroadcastData(const QByteArray &data);


signals:

public slots:
};

#endif // BROADCASTWORKER_H
