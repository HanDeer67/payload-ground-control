#include "broadcastworker.h"
#include <QDebug>
#include <QThread>

BroadcastWorker::BroadcastWorker(QObject *parent) : QObject(parent)
{

}

void BroadcastWorker::updateOpenCloseSign(bool openClose, double delayTime)
{
    qDebug()<<"openClose"<<openClose;
    qDebug()<<"delayTime"<<delayTime;
    isRunning = openClose;
    delayTimeE = delayTime;

}

void BroadcastWorker::threadRun()
{
    while (!QThread::currentThread()->isInterruptionRequested()) {
        if (isRunning) {
            qDebug() << "当前正在执行广播";
            // ****************读取ui中plaintext中的数据并通过can发送*******************
            QByteArray dataToSend;
            {
                QMutexLocker locker(&m_mutex);
                dataToSend = m_broadcastData;
            }

            if (!dataToSend.isEmpty()) {
                qDebug() << "当前正在执行广播，数据长度:" << dataToSend.size();
                // 在这里通过 CAN 发送 dataToSend
            }
            QThread::msleep(static_cast<unsigned long>(delayTimeE));
        } else {
            QThread::msleep(10);
        }

    }
}


void BroadcastWorker::updateBroadcastData(const QByteArray &data)
{
    QMutexLocker locker(&m_mutex);
    m_broadcastData = data;   // 深拷贝，安全
}
