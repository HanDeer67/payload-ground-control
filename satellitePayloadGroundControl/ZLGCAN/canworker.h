// 周立功can卡上层函数库

#ifndef CANWORKER_H
#define CANWORKER_H

#include <QObject>
#include "canhelper.h"
#include <qDebug>
#include "ringbuffer.h"
#include <QThread>
#include "canparser.h"

class canWorker : public QObject
{
    Q_OBJECT
public:
    explicit canWorker(QObject *parent = nullptr);

public:

    canHelper *canHelper1;
    bool openDevice(DWORD deviceType, DWORD deviceIndex, DWORD Reserved);
    bool closeDevice(DWORD deviceType, DWORD deviceIndex);
    bool initCAN(DWORD DevType, DWORD DevIndex, DWORD CANIndex,
                 PVCI_INIT_CONFIG pInitConfig);
    bool readBoardInfo(DWORD DevType, DWORD DevIndex, PVCI_BOARD_INFO pInfo);
    bool getReceiveNum(DWORD DevType,DWORD DevIndex,DWORD CANIndex);
    bool clearBuffer(DWORD DevType,DWORD DevIndex,DWORD CANIndex);
    bool startCAN(DWORD DevType,DWORD DevIndex,DWORD CANIndex);
    bool resetCAN(DWORD DevType,DWORD DevIndex,DWORD CANIndex);
    bool transmit(DWORD DeviceType,DWORD DeviceInd,DWORD
                  CANInd,PVCI_CAN_OBJ pSend,DWORD Length);
//    bool receive(DWORD DevType, DWORD DevIndex, DWORD  CANIndex,
//                 PVCI_CAN_OBJ pReceive, ULONG Len, INT WaitTime);

    RingBuffer *m_ringBuffer;
    RingBuffer *m_ringBuffer_slow;
//    RingBuffer *m_ringBuffer_all;

    void setRingBuffer(RingBuffer *buffer);  // 关键函数声明
    void setRingBuffer_slow(RingBuffer *buffer);  // 关键函数声明
    QMutex m_mutex;  // 保护缓冲区指针的互斥锁



    bool m_running = false;


    DWORD  nDeviceType = 4; /* USBCAN-2A或USBCAN-2C或CANalyst-II */
    DWORD  nDeviceInd = 0; /* 第1个设备 */
    DWORD  nCANInd = 1;          /* 第1个通道 */
    ULONG  nLen = 2500; // 依照协议设置的最大帧数
    INT  WaitTime = 0; // 等待时间，保留

//    void startReceive();
//    void stopReceive();

    QByteArray frameRec;
    CANParser *canParser;

    bool canTmIsOpen = false;
    bool canTmIsOpen_slow = false;


    QString formatCanIdBinary(uint32_t id);
private:



signals:
    void dataToUiDirectCanWorker(QString dataReceivedString);

public slots:

    void startReceive() {
        m_running = true; }
    void stopReceive()  {
        m_running = false; }

    void threadRun();
    void updateCanTmIsOpen(bool canTmOpenClose);
    void updateCanTmIsOpen_slow(bool canTmOpenClose);

    void updateConfig(DWORD  nDeviceTypeIn, DWORD  nDeviceIndIn, DWORD  nCANIndIn);
};

#endif // CANWORKER_H
