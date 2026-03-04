#include "frameassemble.h"
#include <QtEndian>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include "parasetelemetryxml.h"
#include "frmshowmsg.h"
#include "app.h"
#include "savedatahelper.h"

//#include "serialporthelper.h"

#define SYNC_HEAD 0x499602D2
#define DEVICE_TYPE_ 0x0B
#define DEVICE_NUMBER 0xC2
#define LOAD_TYPE   0xA5
#define RESERVE 0xA5

#define FILE_SIZE (10*1024*1024)

//#define TEST

//定义数据净荷的数据长度
//#define LOAD_DATA_MAX_LENGTH 1597
#define LOAD_DATA_MAX_LENGTH 1597

#define LOAD_DATA_1_5K_LENGTH (1500)
#define LOAD_DATA_1_NK_LENGTH (1591-4)
#define LOAD_DATA_2_0K_LENGTH (2000)

//LVDS DSP包数据长度
#define LVDS_DSP_DATA_MAX_LENGTH 2036
//#define LVDS_DSP_DATA_MAX_LENGTH 8
#include <QMutex>
FrameAssemble *FrameAssemble::self = nullptr;
FrameAssemble *FrameAssemble::Instance()
{
    if (!self) {
        QMutex mutex;
        QMutexLocker locker(&mutex);
        if (!self) {
            self = new FrameAssemble;
        }
    }

    return self;
}

FrameAssemble::FrameAssemble(QObject *parent):QObject(parent)
{

    quint32 sync = SYNC_HEAD;
    sync = qbswap<quint32>(sync);
    m_RS422FrameHead.append((char*)&sync,sizeof(sync));


    m_LVDSFrameHead.append(0xE2);
    m_LVDSFrameHead.append(0x25);

    m_LVDSFrameTail.append(0xFB);
    m_LVDSFrameTail.append(0xFB);

    //设置精准度
    fpgaBinTimer.setTimerType(Qt::TimerType::PreciseTimer);
    fpgaBinTimer.setInterval(fpgaSendInterval);
    connect(&fpgaBinTimer,&QTimer::timeout,this,&FrameAssemble::fpgaBinTimeout);

    connect(CardManager::Instance(),&CardManager::onLVDSDataReady,this,&FrameAssemble::LVDSCardDataRecved);
    connect(CardManager::Instance(),&CardManager::onRS422DataReady,this,&FrameAssemble::RS422CardDataRecved);
    connect(CardManager::Instance(),&CardManager::onCanDataReady,this,&FrameAssemble::CANCardDataRecved);


//    connect(SerialPortHelper::Instance(),&SerialPortHelper::OnDataReady,this,&FrameAssemble::RS422CardDataRecvedSerial);

    //初始化低速LVDS定时器

//    lowLVDSWatcher = new QTimer(this);
//    lowLVDSWatcher->setInterval(5000);
//    lowLVDSWatcher->setTimerType(Qt::TimerType::PreciseTimer);

//    connect(lowLVDSWatcher,&QTimer::timeout,this,[=](){
//        //emit ;
//        qDebug()<<"低速LVDS接收完毕!";
//    });
}

void FrameAssemble::RS422ChangeFrameAssemble(QByteArray command, FrameAssemble::RS422_APP_CHANGE_CMD cmdType)
{
    QByteArray cmd;
    cmd.append(cmdType);
    //应用帧长度
    quint32 dataLength = command.length();
    dataLength = (dataLength &0xFFFFFF);
    cmd.append((dataLength&0xFF0000)>>16);
    cmd.append((dataLength&0xFF00)>>8);
    cmd.append((dataLength&0xFF));
    cmd.append(command);
    RS422AppDataFrameAssemble(cmd,RS422_CMD_CHANGE_LENGTH);
}

void FrameAssemble::RS422FixedFrameAssemble(QByteArray command, FrameAssemble::RS422_APP_FIXED_CMD cmdType)
{
    QByteArray cmd;
    cmd.append(cmdType);
    cmd.append(command);
    RS422AppDataFrameAssemble(cmd,RS422_CMD_FIXED_LENGTH);
}

void FrameAssemble::RS422FixedFrameAssemble(QByteArray command)
{
    QByteArray cmd;
    cmd.append(command);
    RS422AppDataFrameAssemble(cmd,RS422_CMD_FIXED_LENGTH);
}

void FrameAssemble::RS422AppDataFrameAssemble(QByteArray command,RS422_APP_CMD_TYPE cmdType)
{
    //应用层数据组装

    QByteArray cmd;
    cmd.append(cmdType);
    //应用帧长度
    quint32 dataLength = command.length();
    dataLength = (dataLength &0xFFFFFF);
    cmd.append((dataLength&0xFF0000)>>16);
    cmd.append((dataLength&0xFF00)>>8);
    cmd.append((dataLength&0xFF));
    cmd.append(command);
    //最后需要计算crc
    quint16 crcValue = getRS422SumCheckCode(cmd);
    //低字节序转高字节序
    crcValue = qbswap<quint16>(crcValue);
    cmd.append((char*)&crcValue,sizeof (crcValue));

    //qDebug()<<cmd.mid(0,10).toHex(' ');
    RS422PackCount = 0;
    RS422DataSubPackage(cmd);
}

//
/**********************************************
* @projectName   GroundTestSoftware
* @brief         通用传输方式 自动判断是独立包还是复合包 按照应用层数据组装完毕后 放入传输层进行分包发送
* @param         void
* @return        void
* @author        wangshan
* @date          2023-08-15
**********************************************/
void FrameAssemble::RS422DataSubPackage(QByteArray command)
{
    //根据传入进来的数据进行分包
    quint8 firstByte = 0;
    //默认无分段
    quint8 flag = FrameSplitType::SINGLE_DATA;
    int totalCount = 0;
    int hasSendCount = 0;
    if(command.length() % LOAD_DATA_MAX_LENGTH == 0)
    {
        totalCount = command.length() / LOAD_DATA_MAX_LENGTH;
    }
    else
    {
        totalCount = command.length() / LOAD_DATA_MAX_LENGTH+1;
    }

    while (command.length()>LOAD_DATA_MAX_LENGTH) {
        //第一包数据也就是首段
        if(flag==FrameSplitType::SINGLE_DATA)
        {
            flag = FrameSplitType::FIRST_DATA;
        }
        //其他时候是中间段
        else
        {
            flag = FrameSplitType::MIDDLE_DATA;
        }

        //组装第一个字节
        firstByte = (quint8)(flag<<6)+(quint8)(RS422PackCount&0x1F);

        QByteArray tempCmd = command.mid(0,LOAD_DATA_MAX_LENGTH);
        tempCmd.prepend(firstByte);
        RS422TransformFrameAssemble(tempCmd);
        command = command.mid(LOAD_DATA_MAX_LENGTH);
        RS422PackCount++;
        if(RS422PackCount>31)
        {
            RS422PackCount = 0;
        }
        hasSendCount++;
        emit hasSendRS422PackCount(totalCount,hasSendCount);
    }

    //如果是尾段
    if(flag != FrameSplitType::SINGLE_DATA)
    {
        //设置为尾段
        flag = FrameSplitType::LAST_DATA;
    }

    firstByte = (quint8)(flag<<6)+(quint8)(RS422PackCount&0x1F);
    command.prepend(firstByte);
    RS422TransformFrameAssemble(command);
    RS422PackCount++;
    if(RS422PackCount>31)
    {
        RS422PackCount = 0;
    }

    hasSendCount++;
    emit hasSendRS422PackCount(totalCount,hasSendCount);
}
/**********************************************
* @projectName   GroundTestSoftware
* @brief         只用来发送变长指令的内容
* @param         void
* @return        void
* @author        wangshan
* @date          2023-08-15
**********************************************/
void FrameAssemble::RS422DataSubPackageWithHead(FrameAssemble::RS422_APP_CHANGE_CMD cmdType, QByteArray command)
{
    RS422PackCount = 0;

    //根据传入进来的数据进行分包
    quint8 firstByte = 0;
    //默认无分段
    quint8 flag = FrameSplitType::SINGLE_DATA;
    int totalCount = 0;
    int hasSendCount = 0;
    if(command.length() % LOAD_DATA_1_NK_LENGTH == 0)
    {
        totalCount = command.length() / LOAD_DATA_1_NK_LENGTH;
    }
    else
    {
        totalCount = command.length() / LOAD_DATA_1_NK_LENGTH+1;
    }

    while (command.length()>LOAD_DATA_1_NK_LENGTH) {
        //第一包数据也就是首段
        if(flag==FrameSplitType::SINGLE_DATA)
        {
            flag = FrameSplitType::FIRST_DATA;
        }
        //其他时候是中间段
        else
        {
            flag = FrameSplitType::MIDDLE_DATA;
        }

        //组装第一个字节
        firstByte = (quint8)(flag<<6)+(quint8)(RS422PackCount&0x1F);
        QByteArray tempCmd;
        tempCmd.append(firstByte);

        //先组装变长指令的内容部分
        QByteArray content;
        //变长指令的类型
        content.append(cmdType);
        //3B的参数长度
        quint32 dataLength = LOAD_DATA_1_NK_LENGTH;
        content.append((dataLength&0xFF0000)>>16);
        content.append((dataLength&0xFF00)>>8);
        content.append((dataLength&0xFF));

        content.append(command.mid(0,LOAD_DATA_1_NK_LENGTH));

        //应用层组装
        QByteArray appCmd;
        //变长指令标识
        appCmd.append(RS422_CMD_CHANGE_LENGTH);

        //数据长度
        dataLength = content.length();
        appCmd.append((dataLength&0xFF0000)>>16);
        appCmd.append((dataLength&0xFF00)>>8);
        appCmd.append((dataLength&0xFF));
        //数据内容
        appCmd.append(content);
        //计算校验

        //最后需要计算crc
        quint16 crcValue = getRS422SumCheckCode(appCmd);
        //低字节序转高字节序
        crcValue = qbswap<quint16>(crcValue);
        appCmd.append((char*)&crcValue,sizeof (crcValue));

        qDebug()<<"content:"<<content.length();
        qDebug()<<"appCmd:"<<appCmd.length();

        tempCmd.append(appCmd);

        qDebug()<<"tempCmd:"<<tempCmd.length();

        RS422TransformFrameAssemble(tempCmd);
        command = command.mid(LOAD_DATA_1_NK_LENGTH);
        RS422PackCount++;
        if(RS422PackCount>31)
        {
            RS422PackCount = 0;
        }
        hasSendCount++;
        emit hasSendRS422PackCount(totalCount,hasSendCount);
    }

    //如果是尾段
    if(flag != FrameSplitType::SINGLE_DATA)
    {
        //设置为尾段
        flag = FrameSplitType::LAST_DATA;
    }

    firstByte = (quint8)(flag<<6)+(quint8)(RS422PackCount&0x1F);

    QByteArray tempCmd;
    tempCmd.append(firstByte);

    //先组装变长指令的内容部分
    QByteArray content;
    //变长指令的类型
    content.append(cmdType);
    //3B的参数长度
    quint32 dataLength = command.length();
    content.append((dataLength&0xFF0000)>>16);
    content.append((dataLength&0xFF00)>>8);
    content.append((dataLength&0xFF));

    content.append(command);

    //应用层组装
    QByteArray appCmd;
    //变长指令标识
    appCmd.append(RS422_CMD_CHANGE_LENGTH);

    //数据长度
    dataLength = content.length();
    appCmd.append((dataLength&0xFF0000)>>16);
    appCmd.append((dataLength&0xFF00)>>8);
    appCmd.append((dataLength&0xFF));
    //数据内容
    appCmd.append(content);
    //计算校验

    //最后需要计算crc
    quint16 crcValue = getRS422SumCheckCode(appCmd);
    //低字节序转高字节序
    crcValue = qbswap<quint16>(crcValue);
    appCmd.append((char*)&crcValue,sizeof (crcValue));

    tempCmd.append(appCmd);

    RS422TransformFrameAssemble(tempCmd);
    RS422PackCount++;
    if(RS422PackCount>31)
    {
        RS422PackCount = 0;
    }

    hasSendCount++;
    emit hasSendRS422PackCount(totalCount,hasSendCount);
}
/**********************************************
* @projectName   GroundTestSoftware
* @brief         应用层组装完毕后发送给传输层进行分包 目前这个不使用这种方式
* @param         void
* @return        void
* @author        wangshan
* @date          2023-08-17
**********************************************/
void FrameAssemble::RS422FPGADataSubPackage(QByteArray &command)
{
    //应用层数据组装
    fpgaProcessBin.clear();
    fpgaProcessBin.append(RS422_CMD_FPGA_RECONSITUTION);
    //应用帧长度
    quint32 dataLength = command.length();
    dataLength = (dataLength &0xFFFFFF);
    fpgaProcessBin.append((dataLength&0xFF0000)>>16);
    fpgaProcessBin.append((dataLength&0xFF00)>>8);
    fpgaProcessBin.append((dataLength&0xFF));
    fpgaProcessBin.append(command);
    //最后需要计算crc
    quint16 crcValue = getRS422SumCheckCode(fpgaProcessBin);
    //低字节序转高字节序
    crcValue = qbswap<quint16>(crcValue);
    fpgaProcessBin.append((char*)&crcValue,sizeof (crcValue));

    //判断大小
    //默认无分段 按照1.5K进行分包

    if(fpgaProcessBin.length() % LOAD_DATA_1_5K_LENGTH == 0)
    {
        fpgaBinTotalCount = fpgaProcessBin.length() / LOAD_DATA_1_5K_LENGTH;
    }
    else
    {
        fpgaBinTotalCount = fpgaProcessBin.length() / LOAD_DATA_1_5K_LENGTH+1;
        //填充0xFF
        QByteArray gap;
        gap.resize(LOAD_DATA_1_5K_LENGTH-(fpgaProcessBin.length() % LOAD_DATA_1_5K_LENGTH));
        gap.fill(0xff);
        fpgaProcessBin.append(gap);
    }

    fpgaBinTimer.start();
    isUpdating = true;
    RS422FPGAPackCount = 0;
    //初始化相关的变量
    fpgaBinFirstByte = 0;
    //先设置为独立帧
    fpgaBinFlag = FrameSplitType::SINGLE_DATA;
}
/**********************************************
* @projectName   GroundTestSoftware
* @brief         使用应用层分包
* @param         void
* @return        void
* @author        wangshan
* @date          2023-08-17
**********************************************/
void FrameAssemble::RS422FPGADataSubPackageWidthHead(QByteArray &command)
{
    //应用层数据组装
    fpgaProcessBin.clear();
    fpgaProcessBin.append(command);
    //判断大小
    //默认无分段 按照1.5K进行分包

    if(fpgaProcessBin.length() % LOAD_DATA_1_5K_LENGTH == 0)
    {
        fpgaBinTotalCount = fpgaProcessBin.length() / LOAD_DATA_1_5K_LENGTH;
    }
    else
    {
        fpgaBinTotalCount = fpgaProcessBin.length() / LOAD_DATA_1_5K_LENGTH+1;
        //填充0xFF
        QByteArray gap;
        gap.resize(LOAD_DATA_1_5K_LENGTH-(fpgaProcessBin.length() % LOAD_DATA_1_5K_LENGTH));
        gap.fill(0xff);
        fpgaProcessBin.append(gap);
    }

    fpgaBinTimer.start();
    isUpdating = true;
    RS422PackCount = 0;
    //初始化相关的变量
    fpgaBinFirstByte = 0;
    fpgaBinFlag = FrameSplitType::SINGLE_DATA;
}

void FrameAssemble::RS422TransformFrameAssemble(QByteArray command)
{
    QByteArray cmd;
    //同步头
    quint32 sync = SYNC_HEAD;
    sync = qbswap<quint32>(sync);
    cmd.append((char*)&sync,sizeof(sync));

    //帧长=数据部分长度+帧长2B+设备类型1B+设备编号1B+净荷1B+保留3B+2B的校验
    quint16 dataLength = command.length()+1+1+1+3+2+2;
    //低字节序转高字节序
    dataLength = qbswap<quint16>(dataLength);
    cmd.append((char*)&dataLength,sizeof(dataLength));

    //设备类型
    cmd.append(DEVICE_TYPE_);
    //设备编号
    cmd.append(DEVICE_NUMBER);
    //净荷类型
    cmd.append(LOAD_TYPE);
    //保留2B
    cmd.append(RESERVE);
    cmd.append(RESERVE);
    cmd.append(RESERVE);

    //组装数据部分
    cmd.append(command);
    //最后需要计算crc
    quint16 crcValue = getRS422SumCheckCode(cmd);
    //低字节序转高字节序
    crcValue = qbswap<quint16>(crcValue);
    cmd.append((char*)&crcValue,sizeof (crcValue));
    //qDebug()<<"Trans:"<<cmd.toHex(' ');

    historyCmd = cmd;

    TransformData(TransformType::RS422,cmd);
}

void FrameAssemble::LVDSDSPDataSubPackage(QByteArray command,LVDS_APP_DATA_TYPE dataType)
{

    if(command.length() % LVDS_DSP_DATA_MAX_LENGTH !=0)
    {
        int diffLength = LVDS_DSP_DATA_MAX_LENGTH-(command.length() % LVDS_DSP_DATA_MAX_LENGTH);
        QByteArray gap;
        gap.resize(diffLength);
        gap.fill(0xAA);
        command.append(gap);
    }

    int totalCount = command.length() / LVDS_DSP_DATA_MAX_LENGTH;
    int hasSendCount = 0;
    QByteArray cmd;
    //遥测0
    //版本号000
    quint8 version = 0;
    quint8 type = 0;
    quint16 title = 0;
    //根据数据类型判断是否有副导头
    //DSP重构数据没有副导头
    if(dataType != LVDS_DATA_DSP)
    {
        title |= (1<<11);
    }
    title |= (version&0x07)<<13;
    title |= (type&0x01)<<12;
    title = title | dataType;
    //低字节序转高字节序
    title = qbswap<quint16>(title);

    quint32 seq = 0;
    //根据数据长度判断是否是独立包还是组包
    quint16 packControl = 0;
    if(dataType == LVDS_DATA_DSP)
    {
        quint32 sumValue = DSPDataSumValue(command);
        quint32 xorValue = DSPDataXorValue(command);

        qDebug()<<"sumValue:"<<sumValue<<QString::number(sumValue,16);
        qDebug()<<"xorValue:"<<xorValue<<QString::number(xorValue,16);;

        //默认是独立包
        quint8 subPackFlag = 3;
        //需要分包
        while(command.length()>LVDS_DSP_DATA_MAX_LENGTH)
        {
            //如果进来是独立包则说明分包 并且是第一个源包
            if(subPackFlag == 3)
            {
                subPackFlag = 1;
            }
            //如果是第一源包 则说明是中间包
            else if(subPackFlag == 1)
            {
                subPackFlag = 0;
            }

            packControl |=subPackFlag<<14;
            packControl |=(seq&0x3FFF);

            cmd.append((char*)&title,sizeof(title));
            //低字节序转高字节序
            packControl = qbswap<quint16>(packControl);
            cmd.append((char*)&packControl,sizeof(packControl));

            //要减去1 该域表示如下:C＝字节数－1
            quint16 length = LVDS_DSP_DATA_MAX_LENGTH-1;
            length = qbswap<quint16>(length);
            cmd.append((char*)&length,sizeof(length));

            cmd.append(command.mid(0,LVDS_DSP_DATA_MAX_LENGTH));
            command = command.mid(LVDS_DSP_DATA_MAX_LENGTH);
            LVDSTransformFrameAssemble(cmd);
            cmd.clear();
            seq++;
            seq = seq % 16384;

            packControl = 0;

            hasSendCount++;
            emit hasSendDSPPackCount(totalCount,hasSendCount);
        }

        //在外面判断是否进入分包
        if(subPackFlag == 3)
        {
            subPackFlag = 1;
        }
        else if(subPackFlag == 1)
        {
            subPackFlag = 0;
        }

        packControl |=subPackFlag<<14;
        packControl |=(seq&0x3FFF);
        cmd.append((char*)&title,sizeof(title));
        //低字节序转高字节序
        packControl = qbswap<quint16>(packControl);
        cmd.append((char*)&packControl,sizeof(packControl));

         //要减去1 该域表示如下:C＝字节数－1
        quint16 length = LVDS_DSP_DATA_MAX_LENGTH-1;
        length = qbswap<quint16>(length);
        cmd.append((char*)&length,sizeof(length));

        //再次判断是否满足2036字节的整数倍
        int diffLength = LVDS_DSP_DATA_MAX_LENGTH - command.length();
        //qDebug()<<"最后一包:"<<diffLength;
        if(diffLength > 0)
        {
            for (int i = 0;i<diffLength;i++) {
                command.append(0xAA);
            }
        }

        cmd.append(command.mid(0,LVDS_DSP_DATA_MAX_LENGTH));
        command = command.mid(LVDS_DSP_DATA_MAX_LENGTH);
        LVDSTransformFrameAssemble(cmd);

        hasSendCount++;
        emit hasSendDSPPackCount(totalCount,hasSendCount);

        seq++;
        seq = seq % 16384;

        //另外对于DSP数据还需要发送最后的校验
        //最后一包
        if(subPackFlag != 3)
        {
            subPackFlag =2;
        }


        //发送尾帧
        cmd.clear();
        packControl = 0;
        packControl |=subPackFlag<<14;
        packControl |=(seq&0x3FFF);
        cmd.append((char*)&title,sizeof(title));
        //低字节序转高字节序
        packControl = qbswap<quint16>(packControl);
        cmd.append((char*)&packControl,sizeof(packControl));

         //要减去1 该域表示如下:C＝字节数－1

        length = LVDS_DSP_DATA_MAX_LENGTH-1;
        length = qbswap<quint16>(length);
        cmd.append((char*)&length,sizeof(length));

        sumValue = qbswap<quint32>(sumValue);
        xorValue = qbswap<quint32>(xorValue);

        QByteArray lastFrame;

        lastFrame.append((char*)&sumValue,sizeof(sumValue));
        lastFrame.append((char*)&xorValue,sizeof(xorValue));

        diffLength = LVDS_DSP_DATA_MAX_LENGTH - 8;
        QByteArray gap;
        gap.resize(diffLength);
        gap.fill(0xAA);
        lastFrame.append(gap);
        cmd.append(lastFrame);
        LVDSTransformFrameAssemble(cmd);
        //最后的帧尾不是数据因此不算进度
//        hasSendCount++;
//        emit hasSendDSPPackCount(totalCount,hasSendCount);
    }
    else if(dataType == LVDS_DATA_IMAGE)
    {

    }
    else if(dataType == LVDS_DATA_TRACE)
    {

    }

}

void FrameAssemble::LVDSTransformFrameAssemble(QByteArray command)
{
    QByteArray cmd;
    cmd.append(m_LVDSFrameHead);
    cmd.append(command);
    //最后需要计算crc
    quint16 crcValue = getLVDSSumCheckCode(command);
    //低字节序转高字节序
    crcValue = qbswap<quint16>(crcValue);
    cmd.append((char*)&crcValue,sizeof (crcValue));
    cmd.append(m_LVDSFrameTail);
    //qDebug()<<"LVDS:"<<cmd.toHex(' ');
    TransformData(TransformType::LVDS,cmd);
}

void FrameAssemble::TransformData(TransformType type, QByteArray command)
{
    //保存原始数据
    if(App::isSaveSendRawData)
    {
        SaveDataHelper::Instance()->saveSendRawData(command);
    }

#ifdef TEST
    qDebug()<<"TransformData:"<<type<<command.toHex('|');
#else

//    if(TransformType::LVDS == type)CardManager::Instance()->sendLVDSData(command);
//    else if(TransformType::RS422 == type)CardManager::Instance()->sendRS422Data(command);
//    else if(TransformType::CAN == type)CardManager::Instance()->sendRSCANData(command);
#endif

}

quint16 FrameAssemble::getRS422SumCheckCode(QByteArray command)
{
    quint16 sum = 0;
    for (int i = 0;i<command.length();i++) {
        sum+=(quint8)command.at(i);
    }
    return sum;
}

quint16 FrameAssemble::getLVDSSumCheckCode(QByteArray command)
{
    quint16 sum = 0;
    for (int i = 0;i<command.length();i++) {
        sum+=(quint8)command.at(i);
    }
    return sum;
}

quint16 FrameAssemble::rs422CheckCRC(const QByteArray &data)
{
    //校验计算方式是：数据域按单字节求和校验
    quint16 crc = 0;
    for(int i = 0;i<data.length()-2;i++)crc+=(quint8)data.at(i);
    return crc;
}

quint16 FrameAssemble::lvdsCheckCRC(const QByteArray &data)
{
    //检验校验码 校验中间数据帧中源包部分 2042个字节
    quint16 crc = 0;
    for (int i = 2;i< data.length()-4;i++)
    {
        crc+=(quint8)data.at(i);//此处要强制转为quint8 不然按照char计算 计算有问题；
    }
    return crc;
}

quint32 FrameAssemble::DSPDataSumValue(QByteArray &data)
{
    if(data.length() % 4 !=0)
    {
        qDebug()<<"data length is error!";
        return 0;
    }
    quint32 sum = 0;

    int span = 4;
    int spanCount = data.length()/span;

    for (int i = 0;i<spanCount;i++) {
        sum+=(quint32)(((quint8)data.at(i*span+0)<<24)+((quint8)data.at(i*span+1)<<16)+((quint8)data.at(i*span+2)<<8)+(quint8)data.at(i*span+3));
    }
    return sum;
}

quint32 FrameAssemble::DSPDataXorValue(QByteArray &data)
{
    if(data.length() % 4 !=0)
    {
        qDebug()<<"data length is error!";
        return 0;
    }
    quint32 sum = 0;
    int span = 4;
    int spanCount = data.length()/span;

    for (int i = 0;i<spanCount;i++) {
        sum^=(quint32)(((quint8)data.at(i*span+0)<<24)+((quint8)data.at(i*span+1)<<16)+((quint8)data.at(i*span+2)<<8)+(quint8)data.at(i*span+3));
    }
    return sum;
}

void FrameAssemble::frameTest()
{
    QByteArray cmd;
    //发送定长指令


    //发送变长指令
    connect(this,&FrameAssemble::hasSendDSPPackCount,this,[=](int total,int hasSend)
    {
        qDebug()<<"BSP:"<<total<<hasSend;
    });

    //发送变长指令
    connect(this,&FrameAssemble::hasSendFPGAPackCount,this,[=](int total,int hasSend)
    {
        qDebug()<<"FPGA:"<<total<<hasSend;
    });

    //发送遥测数据
    cmd.clear();
    //FrameAssemble::Instance()->RS422AppDataFrameAssemble(cmd,FrameAssemble::RS422_CMD_TELEMETRY_REQUEST);
    //发送时间码 8个字节
    cmd.clear();
    //添加时间广播数据类型
    //cmd.append(0x30);
    //RS422AppDataFrameAssemble(cmd,FrameAssemble::RS422_CMD_ATP_BOARDCAST);
    cmd.resize(10);
    cmd.fill(0x11);
//    LVDSDataSubPackage(cmd,LVDS_DATA_DSP);
    //QFileDialog::getExistingDirectory(nullptr, tr("选择文件夹"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QString path = QFileDialog::getOpenFileName(nullptr,"打开文件","","bin (*.bin);;bit (*.bit)");
    if(!path.isEmpty())RS422SendFPGABin(path,1000);


}

void FrameAssemble::frameRS422Test()
{
    QString hexData = "49 96 02 D2 00 9D 0B C2 5A 5A 5A 5A C0 C0 00 00 8C 02 50 00 00 00 13 00 80 00 80 00 00 00 00 00 39 AA 74 33 88 E6 74 55 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00\
            00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 C9 00 1B 00 1B 0C 8F 0A BF 0F 83 08 A8 05 44 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF 0D 0B 6C 11 28";

    QByteArray data = QByteArray::fromHex(hexData.toLocal8Bit());
    RS422FrameProcess(data);
}

void FrameAssemble::frameTest2()
{
    //实现内容有:读取图像数据 每次读取一帧 根据帧率进行设置定时器进行读取 保存内容包含分辨率：2字节+2字节+图像内容


    QByteArray allData;
    QFile file1("frame.txt");
    if(file1.open(QIODevice::ReadOnly))
    {
        allData = file1.readAll();
        file1.close();
    }

    allData = allData.replace(' ',"");
    QByteArray aa = QByteArray::fromHex(allData);
    qDebug()<<"aa:"<<aa.length();

//    QByteArray aa = hexStringtoByteArray(allData);

    qDebug()<<"aa:"<<aa.length();

    qDebug()<<"一帧图像总长度:"<<aa.length()<<"多少帧:"<<aa.length()/2048;
    QByteArray frameTitle,frameTail;
    frameTitle.append(0xE2);
    frameTitle.append(0x25);

    frameTail.append(0xFB);
    frameTail.append(0xFB);

    int h =aa.indexOf(frameTitle);
    int t = aa.indexOf(frameTail);

    int count = 0;
    while(h>=0 && t>=0)
    {
        count++;
        QByteArray frame = aa.mid(h,2048);

        qDebug()<<"frameCount:"<<count;
        LVDSFrameProcess(frame);
        aa=aa.mid(h+2048);
        h =aa.indexOf(frameTitle);
        t = aa.indexOf(frameTail);
    }
    qDebug()<<"frameCount:"<<count;


    return;
    //读取前四字节
    QFile file("Image.dat");
    //只是读取一个数据帧并不是一幅图像

    //保存的时候 点击开始录像后 从下一帧图像数据获取的数据开始进行保存 显示图像的类中添加一个原始数据保存的数组
    if(file.open(QIODevice::ReadOnly))
    {
//        int count = 0;
//        while (!file.atEnd()) {
//            count++;
//        }
        QByteArray data = file.read(2048);   // 读取2字节文件数据
        //qDebug()<<"count:"<<count<<file.size()<<file.size()/2048.0;

        //qDebug()<<data.length()<<data.mid(0,10).toHex('|');

        LVDSFrameProcess(data);
       file.close();
    }

}

void FrameAssemble::RS422TelemetryRequestDataSend()
{
    QByteArray cmd;
    cmd.clear();
    RS422AppDataFrameAssemble(cmd,FrameAssemble::RS422_CMD_TELEMETRY_REQUEST);
}

void FrameAssemble::RS422TimeCodeDataSend(quint32 Second,quint32 us)
{
    QByteArray cmd;
    cmd.clear();
    //时间广播
    cmd.append(ATP_TYPE_TIME);
    //秒
    Second = qbswap<quint32>(Second);
    cmd.append((char*)&Second,sizeof(Second));
    //微秒
    us = (us &0xFFFFFF);
    cmd.append((us&0xFF0000)>>16);
    cmd.append((us&0xFF00)>>8);
    cmd.append((us&0xFF));
    RS422AppDataFrameAssemble(cmd,FrameAssemble::RS422_CMD_ATP_BOARDCAST);
}

void FrameAssemble::RS422SatelliteOrbitDataSend(quint8 orbitMode,quint32 Second,quint16 ms,float x,float y,float z,float Vx,float Vy,float Vz)
{
    QByteArray cmd;
    cmd.clear();
    //轨道广播
    cmd.append(ATP_TYPE_LOCATION);
    cmd.append(orbitMode);
    //秒
    Second = qbswap<quint32>(Second);
    cmd.append((char*)&Second,sizeof(Second));
    //毫秒
    ms = qbswap<quint16>(ms);
    cmd.append((char*)&ms,sizeof(ms));
    //位置单精度浮点数
    //注意此处是否需要转换大小端 如果需要大端则需要进行转换
    cmd.append((char*)&x,sizeof(x));
    cmd.append((char*)&y,sizeof(y));
    cmd.append((char*)&z,sizeof(z));
    cmd.append((char*)&Vx,sizeof(Vx));
    cmd.append((char*)&Vy,sizeof(Vy));
    cmd.append((char*)&Vz,sizeof(Vz));

    RS422AppDataFrameAssemble(cmd,FrameAssemble::RS422_CMD_ATP_BOARDCAST);
}

void FrameAssemble::RS422SatelliteGestureDataSend(quint32 Second,quint16 us,float q1,float q2,float q3,float q4)
{
    QByteArray cmd;
    cmd.clear();
    //姿态广播
    cmd.append(ATP_TYPE_ATTITUDE);
    //秒
    Second = qbswap<quint32>(Second);
    cmd.append((char*)&Second,sizeof(Second));
    //毫秒
    us = qbswap<quint16>(us);
    cmd.append((char*)&us,sizeof(us));

    //注意此处是否需要转换大小端 如果需要大端则需要进行转换
    cmd.append((char*)&q1,sizeof(q1));
    cmd.append((char*)&q2,sizeof(q2));
    cmd.append((char*)&q3,sizeof(q3));
    cmd.append((char*)&q4,sizeof(q4));

    RS422AppDataFrameAssemble(cmd,FrameAssemble::RS422_CMD_ATP_BOARDCAST);
}
//发送指令序列
void FrameAssemble::RS422InstructionSequenceSend(QByteArray command)
{
    QByteArray cmd;
    quint32 dataLength = command.length();
    dataLength = qbswap<quint32>(dataLength);
    cmd.append((char*)&dataLength,sizeof(dataLength));
    cmd.append(command);
    RS422DataSubPackage(cmd);
}

void FrameAssemble::RS422SensitiveDataSend(quint32 Second,quint16 ms,float X,float Y,float Z)
{
    QByteArray cmd;
    cmd.clear();
    //敏感数据 指令名称：位置数据
    cmd.append(LOCATION_TYPE_LOCATION);
    //秒
    //quint32 Second = 0;
    Second = qbswap<quint32>(Second);
    cmd.append((char*)&Second,sizeof(Second));
    //毫秒
    //quint16 ms = 0;
    ms = qbswap<quint16>(ms);
    cmd.append((char*)&ms,sizeof(ms));
    //位置单精度浮点数 float
    //注意此处是否需要转换大小端 如果需要大端则需要进行转换
    cmd.append((char*)&X,sizeof(X));
    cmd.append((char*)&Y,sizeof(Y));
    cmd.append((char*)&Z,sizeof(Z));


    RS422AppDataFrameAssemble(cmd,FrameAssemble::RS422_CMD_SENSITIVE_DATA);
}

void FrameAssemble::RS422CurveDataSend(QByteArray command)
{
//    QByteArray cmd;
//    cmd.clear();
//    //敏感数据 指令名称：位置数据
//    cmd.append(RS422_CMD_CHANGE_CURVE_INJECTION);
//    //应用帧长度3字节
//    quint32 dataLength = command.length();
//    dataLength = (dataLength &0xFFFFFF);
//    cmd.append((dataLength&0xFF0000)>>16);
//    cmd.append((dataLength&0xFF00)>>8);
//    cmd.append((dataLength&0xFF));
//    cmd.append(command);
//    //注入曲线 采用变长指令发送
//    RS422AppDataFrameAssemble(cmd,FrameAssemble::RS422_CMD_CHANGE_LENGTH);

    RS422DataSubPackageWithHead(RS422_CMD_CHANGE_CURVE_INJECTION,command);
}

void FrameAssemble::RS422CommandDataSend(QByteArray command)
{
//    QByteArray cmd;
//    cmd.clear();
//    //敏感数据 指令名称：位置数据
//    cmd.append(RS422_CMD_CHANGE_COMMAND_INJECTION);
//    //应用帧长度3字节
//    quint32 dataLength = command.length();
//    dataLength = (dataLength &0xFFFFFF);
//    cmd.append((dataLength&0xFF0000)>>16);
//    cmd.append((dataLength&0xFF00)>>8);
//    cmd.append((dataLength&0xFF));
//    cmd.append(command);
    //注入曲线 采用变长指令发送
    //RS422AppDataFrameAssemble(cmd,FrameAssemble::RS422_CMD_CHANGE_LENGTH);

    RS422DataSubPackageWithHead(RS422_CMD_CHANGE_COMMAND_INJECTION,command);
}

void FrameAssemble::LVDSFrameProcess(const QByteArray &data)
{

    if(App::isLowSpeedMode)
    {
        //低速数据处理
        totalLowLVDSFrames++;
        //接收到数据后通过信号发送给低速界面更新包数
        emit lowLVDSData(data);
    }
    else
    {
        totalLVDSImageFrames++;
    }


    emit SendLVDSFrameComeIn(TOTAL_FRAME);

    //qDebug()<<"LVDSFrameProcess!"<<data.length()<<data.mid(0,20).toHex('|');
    if(data.length()<2048)
    {
        qDebug()<<"图像帧长度不够2048字节!帧异常不做处理!";
        if(!App::isLowSpeedMode)totalLVDSImageFailedFrames++;
        else totalLowLVDSFailedFrames++;
        emit SendLVDSFrameComeIn(CRC_ERROR);
        return;
    }

    //数据部分2042个字节
    if((quint8)data.at(0) == 0xE2 && (quint8)data.at(1) == 0x25 && (quint8)data.at(data.length()-2) == 0xFB && (quint8)data.at(data.length()-1) == 0xFB)
    {

        emit SendLVDSFrameComeIn(CRC_SUCCESS);
        //校验LVDS CRC 两个字节的CRC高字节在前
        //quint16 realCrc = (quint8)data.at(data.length()-4)*256+(quint8)data.at(data.length()-3);
        //quint16 lvdsCRC = lvdsCheckCRC(data);

        //qDebug()<<realCrc<<lvdsCRC;
        //if(lvdsCRC != realCrc)
        //由于设备的特殊原因 不对CRC进行校验 CRC肯定不对;
        if(0)
        {
            //qDebug()<<"LVDS数据CRC错误!"<<"clc:"<<realCrc <<"lvds:"<<lvdsCRC;
            //校验错误加一
            if(App::isLowSpeedMode)totalLowLVDSFailedFrames++;
            else totalLVDSImageFailedFrames++;
            //return;
        }
        else
        {
            if(App::isLowSpeedMode) totalLowLVDSSuccessFrames++;
            else totalLVDSImageSuccessFrames++;
        }

        //主导头 6字节
        //版本号3bit，固定为000，表示CCSDC版本。 不做解析
        int version = (data.at(2) & 0xE0)>>5;
        //if(version == 0)ui->lb_lvds_version->setText("CCSDC");

        //类型 固定为0 标识遥测包
        int type = (data.at(2) & 0x10)>>4;
        //if(0 == type)ui->lb_lvds_type->setText("遥测包");

        //是否是副导头
        int isDeputy = (data.at(2) & 0x08)>>3;

        //if(isDeputy == 1)ui->lb_lvds_sub_flag->setText("有副导头");

        //APID 包标识
        int apid = ((quint8)data.at(2))<<8 | (quint8)data.at(3);

        //ui->lb_lvds_app_flag->setText("0X"+data.mid(2,2).toHex());

        int seqType =  (data.at(4) & 0xC0) >> 6;
        //是一组的第一个源包
        if(seqType == 2)
        {
            //ui->lb_lvds_seq->setText("第一包");
        }
        //是一组的相连源包
        else if(seqType == 0)
        {
            //ui->lb_lvds_seq->setText("相连包");
        }
        //是一组的最后源包
        else if(seqType == 1)
        {
            //ui->lb_lvds_seq->setText("最后包");
        }
        //独立包，不属于源包组的源包
        else if(seqType == 3)
        {
            //ui->lb_lvds_seq->setText("独立包");
        }

        //源包序列计数
        int seq = (quint8)((data.at(4)& 0x3F)<< 8) + (quint8)data.at(5);

        //ui->lb_lvds_seq_count->setText(QString::number(seq));

        //2字节，包长定义了数据域字节长度，包长数值＝数据域字节数－1
        int packLength = (quint8)data.at(6)*256+(quint8)data.at(7);

        //ui->lb_lvds_pack_length->setText(QString::number(packLength));
        //2035

        QByteArray subData = data.mid(8,4);
        //2B秒 2B毫秒 统一用毫秒
        double timeCode = ((quint8)subData.at(0)*256+(quint8)subData.at(1))*1000+((quint8)subData.at(2)*256+(quint8)subData.at(3))*0.2;
        //ui->le_lvds_time_code->setText(QString::number(timeCode,'f',1));
        qDebug()<<"源包序列计数:"<<seq<<"packLength:"<<packLength<<"时间码:"<<timeCode;

        //图像数据包
        if(apid == 0x0d03)
        {
            processImageData(data);
            //qDebug()<<"图像数据包!";
        }
        //跟踪数据包
        else if(apid == 0x0d2c)
        {
            processTraceData(data);
            //qDebug()<<"跟踪数据包!";
        }
        //DSP重构数据包
        else if(apid == 0x0206)
        {
            processReconsitutionData(data,seqType);
        }
        //默认为图像
        else
        {
            qDebug()<<"apid is error:"<<apid;
        }
    }
    else
    {

        //帧头帧尾不正确
        //emit SendLVDSImageFrameStatusInfo();
        qDebug()<<"frameHead or frameTail error!"<<data.length()<<data.mid(20).toHex(' ');
        if(App::isLowSpeedMode)totalLowLVDSFailedFrames++;
        else totalLVDSImageFailedFrames++;
    }

}



void FrameAssemble::RS422FrameProcess(const QByteArray &data)
{
    //目前不知道采集卡采集上来的数据是整帧数据还是 一帧的一部分数据 RS422
    //目前按照来数据则为一帧数据
    //emit recvRS422Data(data);

    //是否遥测数据和响应帧同时发送过来导致解析不了

    //保存RS422的原始数据
    if(App::isSaveRecvRawData)
    {
        SaveDataHelper::Instance()->saveRs422RawData(data);
    }

    if(data.startsWith(m_RS422FrameHead) && data.length()>=14)
    {
        //先判断长度
        int frameLength = (quint8)data.at(4)*256+(quint8)data.at(5);

        //说明接收的一帧数据没有接收完毕
        if(frameLength+4>data.length())
        {
            qDebug()<<"frameLength+4>data.length()"<<frameLength+4<<data.length();
            return;
        }


        //进行校验对比
        quint16 frameCrcValue = (quint8)data.at(data.length()-2)*256+(quint8)data.at(data.length()-1);
        quint16 clcCrcValue = rs422CheckCRC(data);
        if(frameCrcValue != clcCrcValue)
        {
            qDebug()<<"frame crc error!"<<"帧中校验数据:"<<frameCrcValue<<"计算校验数据:"<<clcCrcValue;
            emit recvRS422CrCCheck(false);
            return;
        }
        //发送校验正确
        emit recvRS422CrCCheck(true);

        //开始解析
        quint8 flag = (quint8)data.at(12);
        flag = flag >> 6;

        //int startIndex  = 13;
        //int len = frameLength-11;
        int startIndex  = 13;
        int len = frameLength-11;
        //如果是单帧则直接处理
        if(flag == 3)
        {
            //RS422Data.append(data.mid(startIndex,len));
            RS422Data.append(data);
        }
        //如果是首帧
        else if(flag == 2)
        {
            RS422Data.append(data.mid(startIndex,len));
            return;
        }
        //中间帧
        else if(flag == 0)
        {
            RS422Data.append(data.mid(startIndex,len));
            return;
        }
        //如果是尾帧
        else if(flag == 1)
        {
            RS422Data.append(data.mid(startIndex,len));
        }

        //此处处理接收到的RS422数据  目前接收到的有:工程参数、响应帧
        switch ((quint8)RS422Data.at(13)) {
        case RS422_CMD_FIXED_LENGTH:
            break;
        case RS422_CMD_CHANGE_LENGTH:
            break;
        case RS422_CMD_FPGA_RECONSITUTION:
            break;
        case RS422_CMD_TELEMETRY_REQUEST:
            break;
        case RS422_CMD_ATP_BOARDCAST:
            break;
        case RS422_CMD_SENSITIVE_DATA:
            break;
        //工程参数
        case RS422_CMD_ENGINEERING_PARAM: 
        {
            //QByteArray arr = RS422Data.mid(1);
            emit sendRS422TelemetryRequest(RS422Data);
            //processRS422TelemetryRequest(arr);
            break;
        }
        case RS422_CMD_RESPOND:
        {
            //主要处理响应帧 并计数
            //QByteArray arr = RS422Data.mid(1);
            emit sendRS422Respond(RS422Data);
            //processRS422Respond(arr);
            break;
        }
        default:
            break;
        }

        //处理完毕后进行清空
        RS422Data.clear();

    }
}

void FrameAssemble::CANFrameProcess(const QByteArray &data)
{

}

//void FrameAssemble::SetLVDSSpeedMode(bool isHigh)
//{
//    highSpeedMode = isHigh;
//}
void FrameAssemble::processImageData(QByteArray data)
{
    //取出包副导头信息共9个字节
    //图像数据包：9字节  跟踪数据包：8字节
    //只需处理副导头中的后5个字节
    QByteArray subData = data.mid(12,5);
    int index = 0;
    //相机标识符
    //粗跟踪相机
    quint8 flag = (quint8)subData.at(index);
    if( flag == 0x23)
    {
        //ui->lb_lvds_camera_flag->setText("粗跟踪相机");
    }
    //精跟踪相机
    else if(flag == 0x26)
    {
        //ui->lb_lvds_camera_flag->setText("精跟踪相机");
    }
    //qDebug()<<"camera flag:"<<subData.mid(4,1).toHex('|');

    //图像帧编号
    //15bit
    int frameNum15Bit = ((quint8)subData.at(index+1)<<7) | (quint8)(subData.at(index+2) & 0xFE)>>1;
    //12bit
    int frameNum12Bit = ((quint8)subData.at(index+1)<<4) | (quint8)(subData.at(index+2) & 0xF0)>>4;

    //ui->le_lvds_frame_num->setText(QString::number(frameNum12Bit));

    if(frameNum12Bit == 0x29)
    {
        qDebug()<<"BMP图像";
    }
    //帧内编号
    quint16 frameInterNum15Bit = ((quint8)subData.at(index+2)& 0x01 <<8) | (quint8)(subData.at(index+3));
    quint16 frameInterNum12Bit = ((quint8)subData.at(index+2)& 0x0F <<8) | (quint8)(subData.at(index+3));

    //ui->lb_lvds_frame_in_num ->setText(QString::number(frameInterNum12Bit));
    //开窗大小
    int windowSize = (quint8)subData.at(index+4)*4;
    //ui->lb_windows_size->setText(QString::number(windowSize));


    qDebug()<<"帧编号:"<<frameNum15Bit<<"帧内编号:"<<frameInterNum15Bit<<"开窗大小:"<<windowSize<<"当前宽度:"<<ImageWidth;


    //数据域的数据分为图像数据、跟踪数据以及重构数据，其中图像数据及跟踪数据包含副导头信息，重构数据不包含副导头信息

//    //在此处添加 如果帧内编号为0 并且开窗大小和预设的不一致则 自动更改 删除以前的内容并重新分配内容
//    if(windowSize != ImageWidth && frameInterNum15Bit == 0)
//    {

//        ImageWidth = windowSize;
//        ImageHeight = windowSize;
//        OneImageDataCount = ImageWidth*ImageHeight;
//        qDebug()<<"图像尺寸变动!"<<"现在尺寸:"<<ImageWidth<<"图像尺寸:"<<windowSize<<OneImageDataCount;
//        imageData.clear();
//    }

//    //第0帧则清空
//    else if(frameInterNum15Bit == 0)
//    {
//        imageData.clear();
//        OneImageDataCount = ImageWidth*ImageHeight;
//        qDebug()<<"frameInterNum15Bit == 0:"<<OneImageDataCount;
//    }


//    //剩下的是图像数据
//    if(OneImageDataCount>=2027)
//    {
//        imageData.append(data.mid(8+9,2027));
//        OneImageDataCount-=2027;
//        qDebug()<<"OneImageDataCount:"<<OneImageDataCount;
//    }
//    else
//    {
//        qDebug()<<"最后一帧";
//        QByteArray timeData = data.mid(8,4);
//        QString strTimeCode = timeData.toHex(' ').toUpper().replace(' ',"");
//        emit SendLVDSFrameInfo1(strTimeCode);
//        emit SendLVDSFrameInfo2(frameNum15Bit,frameInterNum15Bit,windowSize);
//        QString imageFlag = QString("相机标识%1_图像编号%2").arg(flag).arg(frameNum15Bit,4,16,QLatin1Char('0'));
//        imageData.append(data.mid(17,OneImageDataCount));
//        emit SendLVDSImage(imageData,ImageWidth,ImageHeight,imageFlag);
//        OneImageDataCount = ImageWidth*ImageHeight;
//        imageData.clear();
//    }


    //在此处添加 如果帧内编号为0 并且开窗大小和预设的不一致则 自动更改 删除以前的内容并重新分配内容
    if(windowSize != ImageWidth && frameInterNum15Bit == 0)
    {

        ImageWidth = windowSize;
        ImageHeight = windowSize;
        OneImageDataCount = ImageWidth*ImageHeight;
        //qDebug()<<"图像尺寸变动!"<<"现在尺寸:"<<ImageWidth<<"图像尺寸:"<<windowSize<<OneImageDataCount;
        imageData.clear();
    }

    //第0帧则清空
    else if(frameInterNum15Bit == 0)
    {

        QByteArray timeData = data.mid(8,4);
        QString strTimeCode = timeData.toHex(' ').toUpper().replace(' ',"");
        emit SendLVDSFrameInfo1(strTimeCode);
        emit SendLVDSFrameInfo2(frameNum15Bit,frameInterNum15Bit,windowSize);
        QString strFrameNum = QString("%1").arg(frameNum15Bit,4,16,QLatin1Char('0'));
        QString imageFlag = QString("相机标识%1_图像编号%2").arg(flag).arg(strFrameNum.toUpper());
        if(imageData.length() < ImageWidth*ImageHeight)
        {
            int gapLen = ImageWidth*ImageHeight - imageData.length();
            if(gapLen>0)
            {
                QByteArray gapA;
                gapA.resize(gapLen);
                gapA.fill(0);
                imageData.append(gapA);
            }
        }
        emit SendLVDSImage(imageData,ImageWidth,ImageHeight,imageFlag);

        imageData.clear();
        OneImageDataCount = ImageWidth*ImageHeight;
        qDebug()<<"frameInterNum15Bit == 0:"<<OneImageDataCount;
    }


    imageData.append(data.mid(8+9,2027));
    OneImageDataCount-=2027;

}

void FrameAssemble::processTraceData(QByteArray data)
{
    //qDebug()<<data.toHex(' ');

    //剩下4个字节都是0xaa; 先处理288个字节
    QString msg;

    QByteArray sourceData = data.mid(8+8,288);

    for (int i =0;i<14;i++) {
        int index = i*20;
        //粗跟踪指向角1(方位轴、俯仰轴)
        int RoughTrackingPointingAngleX1 = (quint8)sourceData.at(index)*256+ (quint8)sourceData.at(index+1);
        int RoughTrackingPointingAngleY1 = (quint8)sourceData.at(index+2)*256+ (quint8)sourceData.at(index+3);
        //粗跟踪质心数据1(X方向、Y方向)
        int RoughTrackingCentroidX1 = (quint8)sourceData.at(index+4)*256+ (quint8)sourceData.at(index+5);
        int RoughTrackingCentroidY1 = (quint8)sourceData.at(index+6)*256+ (quint8)sourceData.at(index+7);
        //粗跟踪预指向角1(方位轴、俯仰轴)
        int RoughTrackingPrePointtingAngleX1 = (quint8)sourceData.at(index+8)*256+ (quint8)sourceData.at(index+9);
        int RoughTrackingPrePointtingAngleY1 = (quint8)sourceData.at(index+10)*256+ (quint8)sourceData.at(index+11);

        //粗跟踪超前瞄准1(X方向、Y方向)
        int RoughTrackingAimingX1 = (quint8)sourceData.at(index+12)*256+ (quint8)sourceData.at(index+13);
        int RoughTrackingAimingY1 = (quint8)sourceData.at(index+14)*256+ (quint8)sourceData.at(index+15);

        //时间码1(2Byte的秒值、2Byte的毫秒值) 已经转换为毫秒
        int timeSec = ((quint8)sourceData.at(index+16)*256+(quint8)sourceData.at(index+17));
        int timeMs =((quint8)sourceData.at(index+18)*256+(quint8)sourceData.at(index+19));//*0.2;
        msg+=QString::number(RoughTrackingPointingAngleX1)+",";
        msg+=QString::number(RoughTrackingPointingAngleY1)+",";
        msg+=QString::number(RoughTrackingCentroidX1)+",";
        msg+=QString::number(RoughTrackingCentroidY1)+",";
        msg+=QString::number(RoughTrackingPrePointtingAngleX1)+",";
        msg+=QString::number(RoughTrackingPrePointtingAngleY1)+",";
        msg+=QString::number(RoughTrackingAimingX1)+",";
        msg+=QString::number(RoughTrackingAimingY1)+",";
        msg+=QString::number(timeSec)+",";
        msg+=QString::number(timeMs)+",";
    }

    QByteArray sourceData2 = data.mid(304,288);

    for (int i =0;i<217;i++)
    {
        int index = i*8;
        //精跟踪质心数据1(X方向、Y方向)
        int FineTrackingCentroidX1 = (quint8)sourceData.at(index+0)*256+ (quint8)sourceData.at(index+1);
        int FineTrackingCentroidY1 = (quint8)sourceData.at(index+2)*256+ (quint8)sourceData.at(index+3);
        //
        int timeSec = ((quint8)sourceData.at(index+4)*256+(quint8)sourceData.at(index+5));
        int timeMs = ((quint8)sourceData.at(index+6)*256+(quint8)sourceData.at(index+7));



        msg+=QString::number(FineTrackingCentroidX1)+",";
        msg+=QString::number(FineTrackingCentroidY1)+",";
        msg+=QString::number(timeSec)+",";
        //最后一包
        if(i == 216)
        {
            msg+=QString::number(timeMs);
        }
        else
        {
           msg+=QString::number(timeMs)+",";
        }
    }


    //将这个msg写入到csv文件中
    SaveDataHelper::Instance()->saveLowLVDSData(msg);
    qDebug()<<msg;
}

void FrameAssemble::processReconsitutionData(QByteArray data,int seqType)
{
    //DSP重构数据帧不包含副导头信息，单帧的有效数据长度为2036Bytes
    QByteArray sourceData = data.mid(8,2036);

    //判断最后一帧
    //是一组的第一个源包
    if(seqType == 2)
    {
        //ui->lb_lvds_seq->setText("第一包");
    }
    //是一组的相连源包
    else if(seqType == 0)
    {
        //ui->lb_lvds_seq->setText("相连包");
    }
    //是一组的最后源包
    else if(seqType == 1)
    {
        //ui->lb_lvds_seq->setText("最后包");
    }
    //独立包，不属于源包组的源包
    else if(seqType == 3)
    {
        //ui->lb_lvds_seq->setText("独立包");
    }

    //累计数据后保存
}

QByteArray FrameAssemble::hexStringtoByteArray(QString hex)
{
    QByteArray ret;
    hex=hex.trimmed();
    formatString(hex,2,' ');
    QStringList sl=hex.split(" ");
    foreach(QString s,sl)
    {
        if(!s.isEmpty())
            ret.append((char)s.toInt(0,16)&0xFF);
    }
    return ret;
}

void FrameAssemble::formatString(QString &org, int n, const QChar &ch)
{
    int size= org.size();
    int space= qRound(size*1.0/n+0.5)-1;
    if(space<=0)
        return;
    for(int i=0,pos=n;i<space;++i,pos+=(n+1))
    {
        org.insert(pos,ch);
    }
}

void FrameAssemble::fpgaBinTimeout()
{
    //如果数据长度大于1.5K说明不是独立包 是复合包
    if(fpgaProcessBin.length()>LOAD_DATA_1_5K_LENGTH)
    {
        //第一包数据也就是首段
        if(fpgaBinFlag == FrameSplitType::SINGLE_DATA)
        {
            fpgaBinFlag = FrameSplitType::FIRST_DATA;
        }
        //其他时候是中间段
        else
        {
            fpgaBinFlag = FrameSplitType::MIDDLE_DATA;
        }

        //组装第一个字节
        fpgaBinFirstByte = (quint8)(fpgaBinFlag<<6)+(quint8)(RS422FPGAPackCount&0x1F);

        QByteArray tempCmd;

        //应用层的数据类型
        tempCmd.append(RS422_CMD_FPGA_RECONSITUTION);
        //应用帧长度
        quint32 dataLength = LOAD_DATA_1_5K_LENGTH;
        dataLength = (dataLength &0xFFFFFF);
        tempCmd.append((dataLength&0xFF0000)>>16);
        tempCmd.append((dataLength&0xFF00)>>8);
        tempCmd.append((dataLength&0xFF));
        tempCmd.append(fpgaProcessBin.mid(0,LOAD_DATA_1_5K_LENGTH));
        //最后需要计算crc
        quint16 crcValue = getRS422SumCheckCode(tempCmd);
        //低字节序转高字节序
        crcValue = qbswap<quint16>(crcValue);
        tempCmd.append((char*)&crcValue,sizeof (crcValue));
        //在头部添加分包标识
        tempCmd.prepend(fpgaBinFirstByte);
        //开始传输
        RS422TransformFrameAssemble(tempCmd);


        fpgaProcessBin = fpgaProcessBin.mid(LOAD_DATA_1_5K_LENGTH);
        RS422FPGAPackCount++;
        //qDebug()<<"RS422FPGAPackCount:"<<RS422FPGAPackCount;
        if(RS422FPGAPackCount>31)
        {
            RS422FPGAPackCount = 0;
        }
        fpgaBinHasSendCount++;
        emit hasSendFPGAPackCount(fpgaBinTotalCount,fpgaBinHasSendCount);
    }
    else
    {
        //如果是尾段
        if(fpgaBinFlag != FrameSplitType::SINGLE_DATA)
        {
            //设置为尾段
            fpgaBinFlag = FrameSplitType::LAST_DATA;
        }

        fpgaBinFirstByte = (quint8)(fpgaBinFlag<<6)+(quint8)(RS422FPGAPackCount&0x1F);

        QByteArray tempCmd;
        //应用层的数据类型
        tempCmd.append(RS422_CMD_FPGA_RECONSITUTION);
        //应用帧长度
        quint32 dataLength = LOAD_DATA_1_5K_LENGTH;
        dataLength = (dataLength &0xFFFFFF);
        tempCmd.append((dataLength&0xFF0000)>>16);
        tempCmd.append((dataLength&0xFF00)>>8);
        tempCmd.append((dataLength&0xFF));
        tempCmd.append(fpgaProcessBin.mid(0,LOAD_DATA_1_5K_LENGTH));
        //最后需要计算crc
        quint16 crcValue = getRS422SumCheckCode(tempCmd);
        //低字节序转高字节序
        crcValue = qbswap<quint16>(crcValue);
        tempCmd.append((char*)&crcValue,sizeof (crcValue));
        //添加分包标识
        tempCmd.prepend(fpgaBinFirstByte);

        RS422TransformFrameAssemble(tempCmd);
        RS422FPGAPackCount++;
        if(RS422FPGAPackCount>31)
        {
            RS422FPGAPackCount = 0;
        }

        fpgaBinHasSendCount++;
        emit hasSendFPGAPackCount(fpgaBinTotalCount,fpgaBinHasSendCount);

        //最后一包发送完毕则
        fpgaBinTimer.stop();
        isUpdating = false;
    }
}

void FrameAssemble::LVDSCardDataRecved(quint8 channel, QByteArray data)
{
    //是否保存LVDS数据
    if(App::isSaveRecvRawData)
    {
        //保存原始数据
        SaveDataHelper::Instance()->saveLVDSRawData(data);
    }


    qDebug()<<"LVDSCardDataRecved begin:"<<channel<<data.length();
//    while(data.length()>=2048)
//    {
//        QByteArray imageData = data.mid(0,2048);
//        LVDSFrameProcess(imageData);
//        data = data.mid(2048);
//    }
//    return;
    mImageBuffer.append(data);
    bool done = false;
    while (!done)
    {
        int startFlag = mImageBuffer.indexOf(m_LVDSFrameHead);
        int endFlag = mImageBuffer.indexOf(m_LVDSFrameTail);
        //qDebug()<<"flag:"<<startFlag<<endFlag;
        //开头结尾都找到
        if(startFlag>=0 && endFlag>=0)
        {
            //如果是尾部在头部之前说明是上一帧不完整删除找到头部之前的数据
            if(endFlag<=startFlag)
            {
                mImageBuffer = mImageBuffer.mid(startFlag);
                //qDebug()<<"endFlag<=startFlag"<<startFlag<<endFlag;
                continue;
            }
            //如果数据长度不是2048个字节数据则抛弃这包数据
            if(qAbs(endFlag-startFlag) != (2048-2))
            {
                mImageBuffer = mImageBuffer.mid(endFlag+2);
                //qDebug()<<"(endFlag-startFlag) != (2048-2)"<<startFlag<<endFlag<<qAbs(endFlag-startFlag)<<(2048-2);
                continue;
            }

            //取2048个字节数据进行处理
            QByteArray imageData = mImageBuffer.mid(startFlag,2048);
            LVDSFrameProcess(imageData);
            mImageBuffer =  mImageBuffer.mid(endFlag+2);
        }
        //只找到开头则说明后续数据还没有到来
        else if(startFlag>=0)
        {
            mImageBuffer =  mImageBuffer.mid(startFlag);
            //qDebug()<<"startFlag>=0"<<mImageBuffer.toHex(' ');
            done = true;
        }
        //只找到结尾则将结尾之前的数据全部删除
        else if(endFlag>=0)
        {

            mImageBuffer = mImageBuffer.mid(endFlag+2);
            //qDebug()<<"endFlag>=0"<<mImageBuffer.toHex(' ');
            done = true;
        }
        else
        {
            //qDebug()<<"no h no t done!!!!!"<<mImageBuffer.length();
            mImageBuffer.clear();

            done = true;
        }
    }
    //如果处理完后
    if(mImageBuffer.length()>=100000)
    {
        //qDebug()<<"mImageBuffer.length()>=100000!!!";
        mImageBuffer.clear();
    }
    qDebug()<<"LVDSCardDataRecved end:"<<channel<<data.length();
}

void FrameAssemble::RS422CardDataRecved(quint8 channel, QByteArray data)
{
    //RS422通道数据处理
    //if(channel == RS422ChannelNo)
    {
        RS422FrameProcess(data);
    }
}
/**********************************************
* @projectName   GroundTestSoftware
* @brief         摘该函数应该是接收串口数据后自动进行拆包 根据头部和数据部分
* @param         void
* @return        void
* @author        wangshan
* @date          2023-08-16
**********************************************/
void FrameAssemble::RS422CardDataRecvedSerial(QByteArray data)
{
    RS422DataBuffer.append(data);
    //qDebug()<<data.length()<<data.toHex('|');

    if(RS422DataBuffer.startsWith(m_RS422FrameHead) && RS422DataBuffer.length()>=14)
    {
        //先判断长度
        int frameLength = (quint8)RS422DataBuffer.at(4)*256+(quint8)RS422DataBuffer.at(5);

        if(frameLength+4>RS422DataBuffer.length())
        {
            //qDebug()<<RS422DataBuffer.length()<<frameLength+4;
            return;
        }

        RS422FrameProcess(RS422DataBuffer.mid(0,frameLength+4));

        RS422DataBuffer = RS422DataBuffer.mid(frameLength+4);
    }
}

void FrameAssemble::CANCardDataRecved(quint8 channel, QByteArray data)
{
     //CAN 通道数据处理
    //if(channel == CANChannelNo)
    {
        CANFrameProcess(data);
    }
}

//要求1s下发一包
void FrameAssemble::RS422SendFPGABin(QString fileName,int timeSpan)
{
    if(isUpdating || fpgaBinTimer.isActive())
    {
        //提示正咋升级中
        qDebug()<<"正在升级中,请等待升级完毕!";
        return;
    }

    fpgaBinTimer.setInterval(timeSpan);

    //1.打开文件 读取文件
    QFile file(fileName);
    //大小大于10M后提示报错
    if(file.size()>FILE_SIZE)
    {
        qDebug() << "file size is too big!"<<fileName;
        return;
    }

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Cannot open file for reading:"<<fileName;
        return;
    }

    //记录文件内容
    QByteArray fileBin = file.readAll();
    //RS422FPGADataSubPackage(fileBin);
    RS422FPGADataSubPackageWidthHead(fileBin);
}

void FrameAssemble::LVDSSendDSPBin(QString fileName)
{
    //1.打开文件 读取文件
    QFile file(fileName);
    //大小大于10M后提示报错
    if(file.size()>FILE_SIZE)
    {
        qDebug() << "file size is too big!"<<fileName;
        return;
    }
    qDebug()<<"file size:"<<file.size();

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Cannot open file for reading:"<<fileName;
        return;
    }

    //记录文件内容
    QByteArray fileBin = file.readAll();

    LVDSDSPDataSubPackage(fileBin,LVDS_DATA_DSP);
}
