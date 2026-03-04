#ifndef LVDSCARDHELPER_H
#define LVDSCARDHELPER_H

#include <QThread>
#include <QObject>
#include "LVDS_DLL.h"
#include <QDebug>

class LVDSCardHelper : public QObject
{
    Q_OBJECT // 包含信号的类必须包含QObject

public:
    enum CardType{LVDS_CARD,RS422_CARD,CAN_CARD}; // 枚举类型：枚举值默认从0开始递增，所以LVDS_CARD=0, RS422_CARD=1, CAN_CARD=2
    explicit LVDSCardHelper(QObject *parent = nullptr);
    ~LVDSCardHelper(); // 析构

public:
    // 获取ui界面关于板卡的参数：板卡编号、通道编号
    // void uiCardInit(quint8 currentCardNo,quint8 currentChannelN0);
    void uiCardInit(const std::tuple<UCHAR, BYTE> &cardSet);

    //当前板卡编号
    quint8 currentCardNo = 200;
    //当前通道编号
    quint8 currentChannelNo = 200;
    //当前通道编号  接收
//    quint8 currentChannelNoRec = 200;

public:
    // 获取唯一实例
    static LVDSCardHelper& Instance(); // 使用局部静态变量 instance 来保证 LVDSCardHelper 类的唯一实例，单例对象只能通过 Instance() 方法获取

    bool startReceive();
//    bool stopLVDS();

    //获取当前机器上的采集卡 是否存在采集卡
    int getLVDSCardsCount();
    //设置当前操作板卡编号
    void setCurrentCardNo(quint8 cardNo); // 注意：cardNo来自ui界面的combox
    //设置板卡类型
    void setCardType(CardType type);
    //获取板卡类型
    CardType getCardType();
    //设置通道和波特率
    void setChannelAndBaudRate(QList<QPair<quint8,quint32>> channelInfoList);
    //设置当前使用通道
    void setCurrentUseChannel(quint8 channel);
    //设置是否使用该卡
    void setEnable(bool enable);
    //打开板卡
    bool OpenLVDSCard(); // 此时只是板卡上电，并不进行其它操作，接收也未开启
    //关闭板卡
    bool CloseLVDSCard();
    //复位板卡
    bool ResetLVDSCard();
    //清空缓存
    bool ClearLVDSCardBuff();
    //启动板卡
    bool StartLVDSCard(bool isStart);
    //板卡是否上电
    bool getLVDSCardOpenStatus();
    // 板卡是否使能
    bool getLVDSCardStartStatus();
    //配置通道
    bool configCardChannel(quint8 currentCardNo,bool isRecv,quint8 channel,ST_LVDS_CFG stCfg);
    //发送或接收帧配置
    bool configLVDSCardFrame(bool isRecv,quint8 channel);

    void initCard();

    void uninitCard();

    bool checkLVDSCard();

    // 定义当前线程应该开还是关
    bool isT1Start = false;

    // 定义板卡使能还是非使能
    bool isCardStart = false;

    // 定义启动和关闭数据接收
    bool isQueryStart = false;



    //发送数据函数
    quint32 sendData(quint8 channel, BYTE* data, size_t dataSize);
    // quint32 sendData(QByteArray data);

//public slots:
//    void updateCard(quint8 cardNo, quint8 channelNo);


private:
    //板卡个数
    int localCardNums = 0;

    //当前板卡类型
    CardType currentCardType = CardType::RS422_CARD;

    //定义当前板卡的通道和对应的波特率
    QList<QPair<quint8, quint32>> channelInfoList;
    //是否使用该板卡默认不使用
    // bool enable = false;
    //定义板卡是否打开
    bool isOpenCard = false;
    //定义板卡是否关闭
    bool isClosedCard = false;
    // 定义板卡是否复位成功
    bool isResetSucceed = false;
    // 定义板卡是否清空FIFO缓存
    bool isClearFIFOSucceed;
    // 定义板卡发送通道使能是否成功
    bool isStartOrCloseSucceed;
    // 定义板卡接收通道使能是否成功
    bool isStartOrCloseSucceedRec;
    // 定义板卡缓存区是否有数据
    bool isExist = false;

    //是否初始化
    // bool isInit = false;

    ST_LVDS_DATA *pstInfo = nullptr;

    void recieveStart();


signals:
    //报错信息
    void RunError(QString msg); // 发送报错信息的信号
    void onDataReady(quint8 currentCardNo, quint8 channel,QByteArray data);
    // void onDataReady(quint8 channel,QByteArray data);
//    void onLVDSDataReady(quint8 channel,QByteArray data);
//    void onRS422DataReady(quint8 channel,QByteArray data);
//    void onCanDataReady(quint8 channel,QByteArray data);

//public slots:
//    void updateCard(quint8 cardNo, quint8 channelNo, quint8 channelNoRec) {
//        currentCardNo = cardNo;
//        currentChannelNo = channelNo;
//        currentChannelNoRec = channelNoRec;
//        qDebug() << "更新板卡号：" << currentCardNo;
//        qDebug() << "更新通道号：" << currentChannelNo;
//        qDebug() << "更新通道号：" << currentChannelNoRec;
//    }

};

#endif // LVDSCARDHELPER_H
