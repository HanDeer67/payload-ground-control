#include "lvdscardworker.h"
#include "lvdscardhelper.h"
#include <QObject>
#include "dataanalysishelper.h"

lvdsCardWorker::lvdsCardWorker(QObject *parent) : QObject(parent)
{
    card = new LVDSCardHelper(this); // 创建板卡对象
    datatransfer = new DataTransfer(this);
//    dataAnalysisHelper = new DataAnalysisHelper(this);

    // 错误警告信号
    connect(card,&LVDSCardHelper::RunError,this,&lvdsCardWorker::emitRunError);

    // 板卡接收信号
    connect(card,&LVDSCardHelper::onDataReady,this,&lvdsCardWorker::readyReceive);

    // 接收来自dataAnalysisHelper的图像信号
//    connect(dataAnalysisHelper,&DataAnalysisHelper::imageSignal,this,[=](QImage image){
//        emit imageToShowUiSignal(image);
//    });


}

// 板卡接收或发送帧帧格式设置
// 板卡发送（RS422）
const QByteArray lvdsCardWorker::COMMAND_FRAME_HEADER = QByteArray::fromHex("B9E0"); // 相机接收命令帧
const int lvdsCardWorker::COMMAND_FRAME_LENGTH = 18;
// 板卡接收（RS422）
const QByteArray lvdsCardWorker::DATA_FRAME_HEADER = QByteArray::fromHex("B9C0"); // 相机发送数据帧
const QByteArray lvdsCardWorker::RESPONSE_FRAME_HEADER = QByteArray::fromHex("B9D0"); // 相机发送应答帧
const int lvdsCardWorker::DATA_FRAME_LENGTH = 19;
const int lvdsCardWorker::RESPONSE_FRAME_LENGTH = 5;
// 板卡接收（LVDS）
const QByteArray lvdsCardWorker::IMG_FRAME_HEADER = QByteArray::fromHex("E225"); // 图像数据帧头
const QByteArray lvdsCardWorker::IMG_FRAME_TAIL = QByteArray::fromHex("FBFB"); // 图像数据帧尾
const int lvdsCardWorker::IMG_FRAME_LENGTH = 2048; // 图像数据帧长


/// lambda函数的qt5.13.0支持**************
void lvdsCardWorker::emitRunError(QString msg){
    emit RunError(msg);
}

void lvdsCardWorker::readyReceive(quint8 currentCardNo, quint8 channel,QByteArray data){
    // A系列地检
    if(cardNo0IsRs422){
        qDebug()<<"当前0号板卡为RS422";
        if(currentCardNo == Card1No) onRS422DataReceived(channel,data); // 开始对接收到的原始数据进行处理
        else if(currentCardNo == Card2No) onLVDSDataReceived(channel,data); // 开始对接收到的原始数据进行处理
    }
    // B系列地检
    else {
        qDebug()<<"当前0号板卡为LVDS";
        if(currentCardNo == Card2No) onRS422DataReceived(channel,data); // 开始对接收到的原始数据进行处理
        else if(currentCardNo == Card1No) onLVDSDataReceived(channel,data); // 开始对接收到的原始数据进行处理

    }
}

/// lambda函数的qt5.13.0支持**************

void lvdsCardWorker::updateCardNoInput(quint8 rs422N, quint8 lvdsNo){
    Card1No = rs422N;
    Card2No = lvdsNo;
}

// 获取板卡数量
int lvdsCardWorker::getCardCount(){
    int cardCount =card->getLVDSCardsCount();
    return cardCount;
}

// 设置板卡编号
void lvdsCardWorker::setCardNum(quint8 cardNum){
    card->setCurrentCardNo(cardNum);
};

// 获取板卡编号
quint8 lvdsCardWorker::getCardNum(bool chooseCard){
    if(chooseCard) return card->currentCardNo;
    else return card->currentCardNo;
}

// 获取通道编号
quint8 lvdsCardWorker::getChannelNum(bool chooseCard){
    if(chooseCard) return card->currentChannelNo;
    else return card->currentChannelNo;
}

// 打开板卡
void lvdsCardWorker::openCard(bool chooseCard){
    if(chooseCard) card->OpenLVDSCard();
    else card->OpenLVDSCard();
}

// 关闭板卡
bool lvdsCardWorker::closeCard(bool chooseCard){
    if(chooseCard) return card->CloseLVDSCard();
    else return card->CloseLVDSCard();
}

// 复位板卡
bool lvdsCardWorker::resetCard(bool chooseCard){
    if(chooseCard) return card->ResetLVDSCard();
    else return card->ResetLVDSCard();
}

// 清空缓存
bool lvdsCardWorker::clearFIFO(bool chooseCard){
    if(chooseCard) return card->ClearLVDSCardBuff();
    else return card->ClearLVDSCardBuff();
}

// 设置板卡通道号
void lvdsCardWorker::setChannelNum(bool chooseCard, quint8 channelNum){
    if(chooseCard) card->setCurrentUseChannel(channelNum);
    else card->setCurrentUseChannel(channelNum);
};

// 板卡通道帧配置
void lvdsCardWorker::configLVDSCardFrame(bool chooseCard){
    if(chooseCard){
        qDebug()<<"lvdsCardWorker channel"<<card->currentChannelNo;
        bool frameSetRS422 = card->configLVDSCardFrame(false,card->currentChannelNo);
        bool frameSetRecRS422 = card->configLVDSCardFrame(true,card->currentChannelNo);
        if(frameSetRS422 == false || frameSetRecRS422 == false){
            emit updateBarStatusSignal(true, "RS422板卡发送通道或接收通道帧配置失败！");
        }
        else{
            emit updateBarStatusSignal(true, "RS422板卡通道帧配置成功！");
        }
    }
    else {
        bool frameSetLVDS = card->configLVDSCardFrame(false,card->currentChannelNo);
        bool frameSetRecLVDS = card->configLVDSCardFrame(true,card->currentChannelNo);
        if(frameSetLVDS == false || frameSetRecLVDS == false){
            emit updateBarStatusSignal(false, "LVDS板卡发送通道或接收通道帧配置失败！");
        }
        else{
            emit updateBarStatusSignal(false, "LVDS板卡通道帧配置成功！");
        }
    }
}

// 板卡通道参数配置
bool lvdsCardWorker::configChannelPara(bool chooseCard, bool isRecv,quint8 channel,ST_LVDS_CFG stCfg){
    if(chooseCard) return card->configCardChannel(card->currentCardNo,isRecv,channel,stCfg);
    else return card->configCardChannel(card->currentCardNo,isRecv,channel,stCfg);
}

bool lvdsCardWorker::getLVDSCardOpenStatus(bool chooseCard){
    if(chooseCard) return card->getLVDSCardOpenStatus();
    else return card->getLVDSCardOpenStatus();
}

// 板卡使能
bool lvdsCardWorker::StartCard(bool chooseCard){
    // 更新标志位
    updateIsTStart(chooseCard, true); // 开始接收数据，更新标志位
    if(chooseCard) {
        card->StartLVDSCard(true);
        return card->StartLVDSCard(true);
    }
    else{
        card->StartLVDSCard(true);
        return card->StartLVDSCard(true);
    }
}

// 板卡去使能
bool lvdsCardWorker::StopCard(bool chooseCard){
    updateIsTStart(chooseCard, false); // 开始接收数据，更新标志位
    if(chooseCard) {
        card->StartLVDSCard(false);
        return card->StartLVDSCard(false);
    }
    else{
        card->StartLVDSCard(false);
        return card->StartLVDSCard(false);
    }
}

/*
// 开始接收数据
bool lvdsCardWorker::startLVDS(bool chooseCard){
    updateIsT1Start(chooseCard, true); // 开始接收数据，更新标志位
    if(chooseCard){
        return card1->startLVDS();
    }
    else{
        return card2->startLVDS();
    }
}

// 停止接收
bool lvdsCardWorker::stopLVDS(bool chooseCard){
    updateIsT1Start(chooseCard, false); // 更新标志位
    // 发送一个信号到ui界面，告诉上位机可以关闭线程了
    if(chooseCard) {
        card1->stopLVDS();
    }
    else {
        card2->stopLVDS();
    }
    if(card1->getLVDSCardStartStatus()==false && card2->getLVDSCardStartStatus()==false){
        emit stopThreadRec();
    }
    if(chooseCard)  return card1->stopLVDS();
    else return card2->stopLVDS();
} */

void lvdsCardWorker::updateIsTStart(bool chooseCard, bool isTStart){
    if(chooseCard) card->isT1Start = isTStart;
    else card->isT1Start = isTStart;
}

bool lvdsCardWorker::getLVDSCardStartStatus(bool chooseCard){
    if(chooseCard) return card->getLVDSCardStartStatus();
    else return card->getLVDSCardStartStatus();
}

// 发送数据
quint32 lvdsCardWorker::sendData(bool chooseCard, quint8 channel, BYTE* data, size_t dataSize)
{
    if(chooseCard) return card->sendData(channel, data, dataSize);
    else return card->sendData(channel, data, dataSize);
}

void lvdsCardWorker::onRS422DataReceived(quint8 channel,QByteArray data){
    /// data 是 QByteArray，但它存储的是字符串形式的十六进制字符，而不是实际的二进制数据
    /// 如果 data 存储的是：QByteArray data = "EB900012";
    /// 那么它实际存储的是 ASCII 字符串：
    /// 'E' (0x45)  'B' (0x42)  '9' (0x39)  '0' (0x30)  '0' (0x30)  '0' (0x30)  '1' (0x31)  '2' (0x32)
    QByteArray dataRS422 = QByteArray::fromHex(data); /// 这行代码很重要，是确保输出正确非ASCII码十六进制的关键
//    QString hexStringRS422 = data2.toHex(' ').toUpper();
    QString hexStringRS422 = datatransfer->Bytearray2string(dataRS422);
    qDebug()<<"hexStringRS422:"<<hexStringRS422;

    emit onRS422DataToUi(channel,hexStringRS422);  // 先把数据发送给主ui直接显示
    // 将数据不断写入环形缓冲区
    if (m_ringBuffer) {
            m_ringBuffer->push(dataRS422);  // 非阻塞写入数据
        }

//    dataAnalysisHelper->onDataAnalysisRS422(hexStringRS422, data); // 开始对数据进行处理和分析以获取需要的数据
}

void lvdsCardWorker::onLVDSDataReceived(quint8 channel,QByteArray data){

    QByteArray dataLVDS = QByteArray::fromHex(data); /// 这行代码很重要，是确保输出正确非ASCII码十六进制的关键
//    QString hexStringLVDS = data2.toHex(' ').toUpper();
    QString hexStringLVDS = datatransfer->Bytearray2string(dataLVDS);
    qDebug()<<"hexStringLVDS:"<<hexStringLVDS;
    emit onLVDSDataToUi(channel,hexStringLVDS); // 先把数据发送给主ui直接显示

    // 将数据不断写入环形缓冲区
    if (m_ringBuffer) {
            m_ringBuffer->push(dataLVDS);  // 非阻塞写入数据
        }

//    dataSaveTo2048.append(dataLVDS);
//    while(dataSaveTo2048.size()>=2048){
//        QByteArray data2048 = dataSaveTo2048.mid(0,2048);
//        dataAnalysisHelper->onDataAnalysisLVDS(hexStringLVDS,data2048);
//        // 删除前2048字节（直接修改原对象）
//        dataSaveTo2048.remove(0, 2048);
//    }

//    dataAnalysisHelper->onDataAnalysisLVDS(hexStringLVDS, dataLVDS); // 开始对数据进行处理和分析以获取需要的数据 // 处理这帧
}

//void lvdsCardWorker::clearBuffer()
//{
//    dataAnalysisHelper->lvdsBuffer.clear();
//    dataAnalysisHelper->lvdsImgDataBuffer.clear();
//    dataSaveTo2048.clear();

//}

void lvdsCardWorker::threadRun(){
    // 获取当前的两个板卡的使能状态，只要是处于使能状态，就开始执行数据接收
    bool isCardStart = getLVDSCardStartStatus(true);
    if(isCardStart){
        card->startReceive();
    }
}

void lvdsCardWorker::setRingBuffer(RingBuffer *buffer)
{
    QMutexLocker locker(&m_mutex);  // 加锁保证线程安全
     if (m_ringBuffer != buffer) {
         m_ringBuffer = buffer;  // 更新缓冲区指针
         qDebug() << "RingBuffer updated in thread:" << QThread::currentThread();
     }
}

void lvdsCardWorker::GTChoise(bool cardNo0IsRs422Get)
{
    cardNo0IsRs422 = cardNo0IsRs422Get;
}

