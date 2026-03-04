#include "lvdscardhelper.h"
#include "frameassemble.h"
#include<QDebug>

//#define LVDS_CARD_NO    0       //板卡号 根据自己实际定义
#define RCV_MAX_ST_CNT  32      //最大接收结构体个数

LVDSCardHelper::LVDSCardHelper(QObject *parent)
    : QObject{parent}
{

}


LVDSCardHelper::~LVDSCardHelper() {
    // 清理
}

// 发送数据
quint32 LVDSCardHelper::sendData(quint8 channel, BYTE* data, size_t dataSize)
{
    //超时等待,根据实际需要设置,LVDS_WAIT_INFINITE为一直等待
//    qDebug()<<"（helper）板卡发送数据："<<data;
    qDebug() << "（helper）板卡发送数据：" << QByteArray(reinterpret_cast<const char*>(data), dataSize).toHex(' ').toUpper();

    return LVDS_Send(currentCardNo, channel, data, dataSize, 100);
}

// 开始执行板卡接收
bool LVDSCardHelper::startReceive()
{
    qDebug() << "Sub-thread ID:" << QThread::currentThreadId();
    if(!LVDS_IsOpen(currentCardNo)){
        emit RunError("startLVDS：板卡未打开！");
        return false;
    }
    else{
        isExist = false; // 启动板卡时，将数据标记归置为false
        recieveStart(); // 开始运行
        return true;
    }
}

//// 板卡停止接收数据，直接停止板卡
//bool LVDSCardHelper::stopLVDS()
//{
//    return StartLVDSCard(false); // 板卡去使能就好了，板卡去使能后相应的while循环的判断标志位改变，while循环停止
//}

// 获取板卡数量
int LVDSCardHelper::getLVDSCardsCount()
{
    return LVDS_CountCards();
}

//发送或接收通道帧配置*******************?*********************************
bool LVDSCardHelper::configLVDSCardFrame(bool isRecv,quint8 channel){
    qDebug()<<"LVDSCardHelper channel"<<channel;
    ST_LVDS_FRM_CFG stFrmCfg;
    memset(&stFrmCfg,0,sizeof(stFrmCfg));
    stFrmCfg.bEnable    = TRUE;       //֡帧使能
    stFrmCfg.dwFrmLen   = 0x1000;     //֡帧长:0x1000 字节
    stFrmCfg.dwFrmTime  = 100;        //֡帧间隔:100个时钟周期
    stFrmCfg.dwFrmHead  = 0xEB90EB90;//֡帧头:0xEB90EB90
    stFrmCfg.dwFrmEnd   = 0x55AA55AA;//֡帧尾:0x55AA55AA

    return LVDS_SetChFrmCfg(currentCardNo,isRecv,channel,&stFrmCfg);//发送通道配置
}

// 打开板卡
bool LVDSCardHelper::OpenLVDSCard()
{
    qDebug()<<"OpenLvdscard :"<<currentCardNo;
    qDebug()<<"currentChannelNo :"<<currentChannelNo;
    if(!LVDS_IsOpen(currentCardNo))
    {
        isOpenCard = LVDS_Open(currentCardNo);//打开板卡
    }
    return isOpenCard;
}

// 关闭板卡
bool LVDSCardHelper::CloseLVDSCard()
{
    if(LVDS_IsOpen(currentCardNo))
    {
        isClosedCard = LVDS_Close(currentCardNo);//关闭板卡
    }
    return isClosedCard;
}

// 重置板卡
bool LVDSCardHelper::ResetLVDSCard()
{
    if(LVDS_IsOpen(currentCardNo))
    {
        isResetSucceed = LVDS_Reset(currentCardNo);//板卡复位
    }
    return isResetSucceed;
}

// 清空缓存
bool LVDSCardHelper::ClearLVDSCardBuff()
{
    if(LVDS_IsOpen(currentCardNo))
    {
        isClearFIFOSucceed = LVDS_ClrFIFO(currentCardNo);//该函数用于清空FIFO以及驱动中已经接收的数据
    }
    return isClearFIFOSucceed;
}

// 板卡使能和去使能（注意，板卡使能不等于开始抓取数据）
bool LVDSCardHelper::StartLVDSCard(bool isCardStart)
{
    if(LVDS_IsOpen(currentCardNo))
    {
        // 发送通道使能
        isStartOrCloseSucceed = LVDS_Start(currentCardNo,isCardStart);//启动（isCardStart = true）或停止（isCardStart = false）板卡
        return isStartOrCloseSucceed; // 无论是启动还是停止，有效就返回true
    }
    else if(isCardStart){
        emit RunError("StartLVDSCard：板卡未打开");
        return false;
    }
    else{
        return false;
    }
}

// 获取板卡开启状态
bool LVDSCardHelper::getLVDSCardOpenStatus()
{
    return LVDS_IsOpen(currentCardNo);
}

// 获取板卡使能状态
bool LVDSCardHelper::getLVDSCardStartStatus()
{
    return (LVDS_GetStart(currentCardNo) == 0x01); //0x00：未启动或未打开板卡，0x01：启动状态
}

// 通道帧配置
bool LVDSCardHelper::configCardChannel(quint8 currentCardNoInput,bool isRecv,quint8 channel,ST_LVDS_CFG stCfg)
{
    this->currentCardNo = currentCardNoInput;
    return LVDS_SetCfg(currentCardNo,isRecv,channel,&stCfg);//发送通道配置
}

/*
// 板卡初始化（未使用）
void LVDSCardHelper::uiCardInit(const std::tuple<UCHAR, BYTE> &cardSet){
    UCHAR cardNo;
    BYTE channelNo;
    std::tie(cardNo, channelNo) = cardSet; // 解包 tuple
    this->currentCardNo = cardNo;
    this->currentChannelNo = channelNo;
    qDebug()<<"uiCardInit函数获取的板卡号："<<cardNo;
    qDebug()<<"uiCardInit函数获取的号："<<channelNo;
};



// 未使用
void LVDSCardHelper::setCardType(LVDSCardHelper::CardType type)
{
    currentCardType = type;
}
// 未使用
LVDSCardHelper::CardType LVDSCardHelper::getCardType()
{
    return currentCardType;
}
// 未使用
void LVDSCardHelper::setChannelAndBaudRate(QList<QPair<quint8, quint32>> channelInfoList)
{
    this->channelInfoList = channelInfoList; //一般情况下，加不加this->效果是一样的，但是这里由于传入的参数和成员变量名相同，为了区分
}
*/

// 设置当前板卡号
void LVDSCardHelper::setCurrentCardNo(quint8 cardNo)
{
    localCardNums = LVDS_CountCards();

    if(cardNo>=localCardNums)
    {
        qDebug()<<"该编号板卡不存在!"<<localCardNums<<cardNo;
        return;
    }
    currentCardNo = cardNo;
}

// 设置通道号
void LVDSCardHelper::setCurrentUseChannel(quint8 channel)
{
    currentChannelNo = channel;
}


// 板卡开始接收
void LVDSCardHelper::recieveStart()
{
    qDebug()<<"recieveStart！";
    //在这里面跑接收
    quint32 dwRcvCnt = 0;//接收计数
    quint32 dwSTCnt = 0;//查询结构体个数

    isQueryStart = LVDS_QueryStart(currentCardNo);//启动所有通道的数据接收，注意，通道必须是接收通道
    if(isQueryStart){
        qDebug()<<"数据接收启动成功！";
    }
    else{
        qDebug()<<"数据接收启动失败！";
    }

    while (isT1Start && !isExist) // 当板卡处于使能状态且有数据存在时，执行while循环
    {
        dwSTCnt = LVDS_GetCount(currentCardNo);//查询缓存中已接收的结构体个数
        qDebug()<<"进入if语句之前缓存中已接收的结构体个数:"<<dwSTCnt;
        if (dwSTCnt > 0)
        {
            qDebug()<<"进入if语句之后缓存中已接收的结构体个数:"<<"dwSTCnt:"<<dwSTCnt;
            dwSTCnt = dwSTCnt > RCV_MAX_ST_CNT?RCV_MAX_ST_CNT:dwSTCnt;
//            qDebug()<<"校验后已接收的结构体个数:"<<"dwSTCnt2:"<<dwSTCnt;
            pstInfo = (ST_LVDS_DATA*)malloc(sizeof(ST_LVDS_DATA)*RCV_MAX_ST_CNT);
            qDebug()<<pstInfo;

            DWORD dwRetCnt = LVDS_ReadData(currentCardNo, pstInfo, dwSTCnt);
            for (DWORD i = 0; i < dwRetCnt; i++)
            {
                qDebug() << "ucBuf address:" << static_cast<void*>(pstInfo[i].ucBuf);
                qDebug() << "emType:" << pstInfo[i].emType << "usLen:" << pstInfo[i].usLen;
                QByteArray data = QByteArray::fromRawData(reinterpret_cast<const char*>(pstInfo[i].ucBuf), pstInfo[i].usLen).toHex();
                qDebug() << "Data: " <<data;
                qDebug() << "DataSize: " <<data.size();
                emit onDataReady(currentCardNo,pstInfo[i].emType,data); // 发送带有板卡号的载有数据的信号
                if (pstInfo[i].usLen == 0) continue;
            }
            free(pstInfo); // 释放内存
            pstInfo = nullptr;
        }
        else
        {
            //有可能导致接收卡顿
            QThread::msleep(1);
//            QThread::usleep(100); // 改为微秒级休眠，减少延迟
        }
    }
}




