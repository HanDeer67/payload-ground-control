#include "canworker.h"


canWorker::canWorker(QObject *parent) : QObject(parent)
{
    canHelper1 = new canHelper(this);
    canParser = new CANParser();
}



bool canWorker::openDevice(DWORD deviceType, DWORD deviceIndex, DWORD Reserved)
{
    bool isSuc = canHelper1->OPEN_DEVICE(deviceType,deviceIndex,Reserved);
    if(!isSuc){
        qDebug()<<"打开设备失败!";
    }
    else {
        qDebug()<<"打开设备成功!";
    }
    return isSuc;
}

bool canWorker::closeDevice(DWORD deviceType, DWORD deviceIndex)
{
    bool isSuc = canHelper1->CLOSE_DEVICE(deviceType,deviceIndex);
    if(!isSuc){
        qDebug()<<"关闭设备失败!";
    }
    else {
        qDebug()<<"关闭设备成功!";
    }
    return isSuc;
}

bool canWorker::initCAN(DWORD DevType, DWORD DevIndex, DWORD CANIndex, PVCI_INIT_CONFIG pInitConfig)
{
    bool isSuc = canHelper1->INIT_CAN(DevType,DevIndex,CANIndex,pInitConfig);
    if(!isSuc){
        qDebug()<<"初始化CAN失败!";
    }
    else {
        qDebug()<<"初始化CAN成功!";
    }
    return isSuc;
}

bool canWorker::readBoardInfo(DWORD DevType, DWORD DevIndex, PVCI_BOARD_INFO pInfo)
{
    bool isSuc = canHelper1->READ_BOARD_INFO(DevType,DevIndex,pInfo);
    if(!isSuc){
        qDebug()<<"读取设备信息失败!";
    }
    else {
        qDebug()<<"读取设备信息成功!";
    }
    return isSuc;
}

bool canWorker::getReceiveNum(DWORD DevType, DWORD DevIndex, DWORD CANIndex)
{
    bool isSuc = canHelper1->GETRECEIVENUM(DevType,DevIndex,CANIndex);
    if(!isSuc){
        qDebug()<<"获取接收缓存数量失败!";
    }
    else {
        qDebug()<<"获取接收缓存数量成功!";
    }
    return isSuc;
}

bool canWorker::clearBuffer(DWORD DevType, DWORD DevIndex, DWORD CANIndex)
{
    bool isSuc = canHelper1->CLEARBUFFER(DevType,DevIndex,CANIndex);
    if(!isSuc){
        qDebug()<<"清空缓存失败!";
    }
    else {
        qDebug()<<"清空缓存成功!";
    }
    return isSuc;
}

bool canWorker::startCAN(DWORD DevType, DWORD DevIndex, DWORD CANIndex)
{
    bool isSuc = canHelper1->STARTCAN(DevType,DevIndex,CANIndex);
    if(!isSuc){
        qDebug()<<"开启CAN失败!";
    }
    else {
        qDebug()<<"开启CAN成功!";
    }
    return isSuc;
}

bool canWorker::resetCAN(DWORD DevType, DWORD DevIndex, DWORD CANIndex)
{
    bool isSuc = canHelper1->RESETCAN(DevType,DevIndex,CANIndex);
    if(!isSuc){
        qDebug()<<"复位失败!";
    }
    else {
        qDebug()<<"复位成功!";
    }
    return isSuc;
}

bool canWorker::transmit(DWORD DeviceType, DWORD DeviceInd, DWORD CANInd, PVCI_CAN_OBJ pSend, DWORD Length)
{
//    qDebug()<<"777DeviceType"<<DeviceType;
//    qDebug()<<"777DeviceInd"<<DeviceInd;
//    qDebug()<<"777CANInd"<<CANInd;
//    qDebug()<<"777Length"<<Length;

    bool isSuc = canHelper1->TRANSMIT(DeviceType,DeviceInd,CANInd,pSend,Length);
    if(!isSuc){
        qDebug()<<"发送失败!";
    }
    else {
        qDebug()<<"发送成功!";
    }
    return isSuc;
}

//bool canWorker::receive(DWORD DevType, DWORD DevIndex, DWORD CANIndex, PVCI_CAN_OBJ pReceive, ULONG Len, INT WaitTime)
//{
//    bool isSuc = canHelper1->RECEIVE(DevType,DevIndex,CANIndex,pReceive,Len, WaitTime);
//    if(!isSuc){
//        qDebug()<<"接收失败!";
//    }
//    else {
//        qDebug()<<"接收成功!";
//    }
//    // ⭐⭐⭐ 关键：把收到的CAN数据封包成 13 字节，送入 m_ringBuffer
//    for (UINT i = 0; i < Len; i++)
//    {
//        VCI_CAN_OBJ &obj = pReceive[i];

//        QByteArray frame;
//        frame.resize(13);

//        // 按你协议组织13字节（示例）
//        frame[0] = obj.ID & 0xFF;
//        frame[1] = (obj.ID >> 8) & 0xFF;
//        frame[2] = (obj.ID >> 16) & 0xFF;
//        frame[3] = (obj.ID >> 24) & 0xFF;
//        frame[4] = obj.ExternFlag;
//        frame[5] = obj.DataLen;

//        // 真实 payload 8 字节（6~13）
//        for(int j=0; j<8; j++){
//            frame[6 + j] = obj.Data[j];
//        }

//        // ⭐ push 到环形缓存
//        if(m_ringBuffer){
//            m_ringBuffer->push(frame);
//        }

//        qDebug() << "收到一帧 CAN，写入缓存，frame.size=" << frame.size();
//    }


//    return isSuc;
//}

void canWorker::setRingBuffer(RingBuffer *buffer)
{
    QMutexLocker locker(&m_mutex);  // 加锁保证线程安全
     if (m_ringBuffer != buffer) {
         m_ringBuffer = buffer;  // 更新缓冲区指针
         qDebug() << "RingBuffer updated in thread:" << QThread::currentThread();
     }
}

void canWorker::setRingBuffer_slow(RingBuffer *buffer)
{
    QMutexLocker locker(&m_mutex);  // 加锁保证线程安全
     if (m_ringBuffer_slow != buffer) {
         m_ringBuffer_slow = buffer;  // 更新缓冲区指针
         qDebug() << "RingBuffer updated in thread:" << QThread::currentThread();
     }
}

//// 开始执行板卡接收

//void canWorker::threadRun()
//{
//    VCI_CAN_OBJ recObj[nLen];
//    qDebug() << "m_runningCAN = " << (m_running ? "true" : "false");

//    while (!QThread::currentThread()->isInterruptionRequested()){
//        if(m_running){
//            int receiveCount = canHelper1->RECEIVE(nDeviceType, nDeviceInd, nCANInd, recObj,nLen,WaitTime);
//    //        qDebug()<<"接收数据长度："<<receiveCount;
//            if (receiveCount < 0){
//                qDebug() << "CAN 接收失败!";
//                QThread::msleep(1);
//                continue;
//            }
//            if (receiveCount == 0){
//                // 没有数据，不要疯狂空转
//                QThread::msleep(1);
//                continue;
//            }
//            // ⭐⭐⭐ 关键：把收到的CAN数据封包成 13 字节，送入 m_ringBuffer
//            for (int i = 0; i < receiveCount; i++)
//            {
//                VCI_CAN_OBJ &obj = recObj[i];

//                QByteArray frame(13,0);
//    //            frame.resize(13);

//                // 按你协议组织13字节（示例）
//                frame[0] = obj.ID & 0xFF;
//                frame[1] = (obj.ID >> 8) & 0xFF;
//                frame[2] = (obj.ID >> 16) & 0xFF;
//                frame[3] = (obj.ID >> 24) & 0xFF;
//                frame[4] = obj.ExternFlag;
//                frame[5] = obj.DataLen;

//                // 真实 payload 8 字节（6~13）
//                for(int j=0; j<8; j++){
//                    frame[6 + j] = obj.Data[j];
//                }

//                // ⭐ push 到环形缓存
//                if(m_ringBuffer){
//                    m_ringBuffer->push(frame);
//                }

//                qDebug() << "收到一帧 CAN，写入缓存，frame.size=" << frame.size();
//            }
//            QThread::msleep(1);   // 稍微让一下 CPU
//        }
//        QThread::msleep(5);   // 稍微让一下 CPU
//    }
//    qDebug() << "CAN 接收线程安全退出";
//}

void canWorker::threadRun()
{
    VCI_CAN_OBJ recObj[nLen];

    qDebug() << "m_runningCAN = " << (m_running ? "true" : "false");

    while (!QThread::currentThread()->isInterruptionRequested())
    {
        if (!m_running)
        {
            QThread::msleep(5);
            continue;
        }

        // 从设备读取 CAN 帧
        int receiveCount = canHelper1->RECEIVE(
            nDeviceType, nDeviceInd, nCANInd,
            recObj, nLen, WaitTime
        ); // 从缓存中抓取的结构体数量

        if (receiveCount < 0)
        {
            qDebug() << "CAN 接收失败!";
            QThread::msleep(1);
            continue;
        }

        if (receiveCount == 0)
        {
            QThread::msleep(1);
//            qDebug() << "等待数据!";
            continue;
        }

        // 把收到的 CAN 帧 push 到 ringbuffer
        for (int i = 0; i < receiveCount; i++)
        {
            qDebug()<<"收到数据，正在解析";
            VCI_CAN_OBJ &obj = recObj[i];
            qDebug().noquote() << QString("ID = 0x%1 (%2)")
                                  .arg(obj.ID, 8, 16, QChar('0'))
                                  .arg(obj.ID);

            qDebug().noquote() << QString("ID = 0x%1")
                                  .arg(obj.ID, 8, 16, QChar('0')).toUpper();

            qDebug().noquote() << QString("ID = %1 0x%2")
                                  .arg(formatCanIdBinary(obj.ID))  // 二进制分组显示
                                  .arg(obj.ID, 8, 16, QChar('0')).toUpper();

            // 这里开始调用函数进行解包，返回的是一个纯字节文件
            bool tempL = canParser->pushFrame(obj);
            if(tempL){
                frameRec.append(canParser->getFullTM());
            }
//            for (int i = 0; i < frameRec.size(); ++i) {
//                qDebug().noquote()
//                    << QString("%1")
//                           .arg(static_cast<quint8>(frameRec.at(i)), 2, 16, QChar('0'))
//                           .toUpper();
//            }
            QString dataStr;

            for (int j = 0; j < frameRec.size(); ++j) {
                dataStr += QString("%1 ")
                               .arg(static_cast<quint8>(frameRec.at(j)), 2, 16, QChar('0'))
                               .toUpper();
            }

            emit dataToUiDirectCanWorker(dataStr.trimmed());

            // 检查帧指令码，解析是快遥还是慢遥
            // 1.直接检查第三第四字节是0011H（快）还是0012H（慢）
            if (frameRec.size() < 4) {
                qDebug() << "数据太短";
                continue;
            }
            // 方法1：逐字节组合
            uint16_t header = (static_cast<uint8_t>(frameRec[2]) << 8) |
                               static_cast<uint8_t>(frameRec[3]);


            if (header == 0x0011 && canTmIsOpen) {
                qDebug() << "前两个字节是 0011H——快遥";
                // 等待127字节
//                if(frameRec.size() == 127 ){
                    if (m_ringBuffer)
                        m_ringBuffer->push(frameRec);
//                }
            }
            else if (header == 0x0012 && canTmIsOpen_slow) {
                qDebug() << "前两个字节是 0012H——慢遥";
                // 等待227字节
//                if(frameRec.size() == 227){
                    if (m_ringBuffer_slow)
                        m_ringBuffer_slow->push(frameRec);
//                }
            }
            else if(header == 0x0013 && canTmIsOpen && canTmIsOpen_slow){
                qDebug() << "前两个字节是 0013H——快慢遥";
                if (m_ringBuffer)
                    m_ringBuffer->push(frameRec);
            }
            else {
                qDebug() << "前两个字节不是 0011H/0012H——不明帧或未打开遥测请求";
            }
            // ⬇ 放入你自己的环形缓存（CANParser 会从缓存里取）
//            if (m_ringBuffer)
//                m_ringBuffer->push(frameRec);
        }
        frameRec.clear();
        QThread::msleep(1);
    }

    qDebug() << "CAN 接收线程安全退出";
}



/**
绿色（如森林精灵般灵动充盈）
春天（日光轻柔，明灭光影之间如梦似幻）
猫猫（午后慵懒的猫猫，喜欢你的蹭蹭）
巧克力（巧克力般纯净绵密的香气，淡淡的苦涩中其实藏着独特的香甜）
*/


void canWorker::updateCanTmIsOpen(bool canTmOpenClose)
{
    qDebug()<<"updateCanTmIsOpen";
    canTmIsOpen = canTmOpenClose;
}

void canWorker::updateCanTmIsOpen_slow(bool canTmOpenClose)
{
    canTmIsOpen_slow = canTmOpenClose;
}

void canWorker::updateConfig(DWORD nDeviceTypeIn, DWORD nDeviceIndIn, DWORD nCANIndIn)
{
    nDeviceType = nDeviceTypeIn;
    nDeviceInd = nDeviceIndIn;
    nCANInd = nCANIndIn;
}

QString canWorker::formatCanIdBinary(uint32_t id) {
    QString binary = QString("%1").arg(id, 32, 2, QChar('0'));
    // 每4位加空格：0000 0001 0000 1010...
    for (int i = 4; i < 32; i += 5) {
        binary.insert(i, ' ');
    }
    return binary;
}
