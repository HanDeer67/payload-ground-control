#ifndef LVDSCARDWORKER_H
#define LVDSCARDWORKER_H

#include <QObject>
#include "lvdscardhelper.h"
#include "datatransfer.h"
#include "dataanalysishelper.h"

class lvdsCardWorker : public QObject
{
    Q_OBJECT
public:
    explicit lvdsCardWorker(QObject *parent = nullptr);

    int getCardCount(); // 获取板卡数
    void setCardNum(quint8 cardNum); // 设置板卡号
    void setChannelNum(bool chooseCard, quint8 channelNum); // 设置通道号
    void openCard(bool chooseCard); // 打开板卡 chooseCard是标志位，true代表RS422板卡，false代表LVDS板卡
    bool closeCard(bool chooseCard); // 关闭板卡
    bool resetCard(bool chooseCard); // 复位板卡
    bool clearFIFO(bool chooseCard); // 清空缓存
    void configLVDSCardFrame(bool chooseCard);
    bool configChannelPara(bool chooseCard, bool isRecv,quint8 channel,ST_LVDS_CFG stCfg);
    bool getLVDSCardOpenStatus(bool chooseCard);
    bool isTStart = false; // 定义当前线程应该开还是关
    bool StartCard(bool chooseCard); // 板卡使能
    bool StopCard(bool chooseCard); // 板卡去使能
//    bool startLVDS(bool chooseCard);
//    bool stopLVDS(bool chooseCard);
    void updateIsTStart(bool chooseCard, bool isTStart); // 改变循环接收数据标志位，true代表开启接收，false代表结束接收
    bool getLVDSCardStartStatus(bool chooseCard);

    void onRS422DataReceived(quint8 channel,QByteArray data); // 接收板卡原始数据并初步处理成字节形式
    void onLVDSDataReceived(quint8 channel,QByteArray data); // 接收板卡原始数据并初步处理成字节形式

//    void clearBuffer(); // 清空缓存区，用于关闭程序时防止内存泄漏

//    void setCardNo(quint8 cardNoRSS422, quint8 cardNoLVDS); // 分别对RS422和LVDS板卡进行编号
//    void setChannelNo(quint8 channelNoRSS422, quint8 channelNoLVDS); // 分别对RS422和LVDS进行通道选择

    quint8 getCardNum(bool chooseCard);
    quint8 getChannelNum(bool chooseCard);
    quint32 sendData(bool chooseCard, quint8 channel, BYTE* data, size_t dataSize);

    void updateCardNoInput(quint8 rs422No, quint8 lvdsNo);

    void threadRun();

    quint8 Card1No = 0;
    quint8 Card2No = 1;

    // 板卡接收或发送帧帧格式设置
    // 板卡发送（RS422）
    static const QByteArray COMMAND_FRAME_HEADER; // 指令帧头
    static const int COMMAND_FRAME_LENGTH; // 指令帧长
    // 板卡接收（RS422）
    static const QByteArray DATA_FRAME_HEADER; // 数据帧头
    static const QByteArray RESPONSE_FRAME_HEADER; // 响应帧头
    static const int DATA_FRAME_LENGTH; // 数据帧长
    static const int RESPONSE_FRAME_LENGTH; // 响应帧长
    // 板卡接收（LVDS）
    static const QByteArray IMG_FRAME_HEADER; // 图像数据帧头
    static const QByteArray IMG_FRAME_TAIL; // 图像数据帧尾
    static const int IMG_FRAME_LENGTH; // 图像数据帧长

    QByteArray CHECKSUM_16; // 单字节校验和（用于RS422数据）
    QByteArray CHECKSUM_32; // 双字节校验和（用于LVDS数据）

    QByteArray dataSaveTo2048;

    RingBuffer *m_ringBuffer;
    void setRingBuffer(RingBuffer *buffer);  // 关键函数声明

    bool cardNo0IsRs422 = true; // A系列地检的0号板卡是RS422，1号板卡是LVDS，B系列地检与之相反
    void GTChoise(bool cardNo0IsRs422Get);


private:

    LVDSCardHelper *card; // 板卡对象
    DataTransfer *datatransfer;
//    DataAnalysisHelper *dataAnalysisHelper;

    QMutex m_mutex;  // 保护缓冲区指针的互斥锁




signals:
//    void getLVDSCardsCountSignal(int cardCount);
    void updateBarStatusSignal(bool chooseCard, QString errorText);
    void stopThreadRec();
    //报错信息
    void RunError(QString msg); // 发送报错信息的信号

    void onLVDSDataToUi(quint8 channel,QString LVDSData); // 输出原始数据的字节形式
    void onRS422DataToUi(quint8 channel,QString RS422Data);// 输出原始数据的字节形式
    void onLVDSAnalysisToUi(quint8 channel,QByteArray data);// 输出数据处理和解析后所需的输出（图像、高速数传）
    void onRS422AnalysisToUi(quint8 channel,QByteArray data);// 输出数据处理和解析后所需的输出（遥测显示）

    void imageSignal(QImage image);


public slots:
    /// qt5.13.0的lambda函数支持
    void emitRunError(QString msg);
    void readyReceive(quint8 currentCardNo, quint8 channel, QByteArray data);
};

#endif // LVDSCARDWORKER_H
