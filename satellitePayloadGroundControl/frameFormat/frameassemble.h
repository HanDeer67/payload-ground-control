#ifndef FRAMEASSEMBLE_H
#define FRAMEASSEMBLE_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include "cardmanager.h"
class FrameAssemble : public QObject
{


    Q_OBJECT
public:
    //传输模式
    enum TransformType{LVDS,RS422,CAN};
    //0代表总帧数 1代表校验错误 2代表校验正确
    enum FrameMsgType{TOTAL_FRAME,CRC_ERROR,CRC_SUCCESS};

    //数据分段常量
    enum FrameSplitType{
        MIDDLE_DATA = 0b00,
        LAST_DATA = 0b01,
        FIRST_DATA = 0b10,
        SINGLE_DATA = 0b11,
    };

    enum RS422_APP_CMD_TYPE
    {
        RS422_CMD_FIXED_LENGTH = 0x41,          //定长指令
        RS422_CMD_CHANGE_LENGTH = 0x43,         //变长指令
        RS422_CMD_FPGA_RECONSITUTION = 0x46,    //FPGA重构指令
        RS422_CMD_TELEMETRY_REQUEST = 0x47,     //遥测请求
        RS422_CMD_ATP_BOARDCAST = 0x80,         //ATP广播
        RS422_CMD_PLATFORM_PARM = 0x81,         //平台参数
        RS422_CMD_SENSITIVE_DATA = 0xC7,        //敏感数据(位置数据)
        RS422_CMD_ENGINEERING_PARAM = 0xC0,     //工程参数
        RS422_CMD_RESPOND = 0xCF,               //响应帧
    };
    //定长指令
    enum RS422_APP_FIXED_CMD
    {
        RS422_CMD_FIXED_1 = 0x22,               //加电方式控制
        RS422_CMD_FIXED_2 = 0x26,               //粗跟踪限位控制指令
        RS422_CMD_FIXED_3 = 0x2A,               //粗跟踪模式控制指令
        RS422_CMD_FIXED_4 = 0x2C,               //粗跟踪控制参数调整
        RS422_CMD_FIXED_5 = 0x2E,               //精跟踪模式控制指令
        RS422_CMD_FIXED_6 = 0x60,               //超前压电1控制指令
        RS422_CMD_FIXED_7 = 0x62,               //超前压电2控制指令
        RS422_CMD_FIXED_8 = 0x64,               //超前压电3控制指令
        RS422_CMD_FIXED_9 = 0x30,               //跟踪相机工作参数
        RS422_CMD_FIXED_10 = 0x32,               //跟踪相机坏点位置
        RS422_CMD_FIXED_11 = 0x34,               //数据传输控制指令
        RS422_CMD_FIXED_12 = 0x36,               //指向角计算控制指令
        RS422_CMD_FIXED_13 = 0x38,               //信标光功率调整指令
        RS422_CMD_FIXED_14 = 0x3A,               //事件表清除指令
        RS422_CMD_FIXED_15 = 0x3E,               //紧急断电指令
        RS422_CMD_FIXED_16 = 0xC7,               //地面站位置信息
        RS422_CMD_FIXED_17 = 0x3C,               //旋转机构控制指令
        RS422_CMD_FIXED_18 = 0x55,               //芯片擦除指令
    };
    //变长指令
    enum RS422_APP_CHANGE_CMD
    {
        RS422_CMD_CHANGE_COMMAND_INJECTION = 0x7F,       //地面指令注入
        RS422_CMD_CHANGE_CURVE_INJECTION = 0x7C,        //曲线注入
    };
    //LVDS发送数据类型
    enum LVDS_APP_DATA_TYPE
    {
        LVDS_DATA_IMAGE = 0x0d03,   //图像数据
        LVDS_DATA_TRACE = 0x0d2c,   //图像数据
        LVDS_DATA_DSP = 0x0206,     //DSP重构数据
    };
    //功能定义
    enum RS422_APP_ATP_TYPE{
        ATP_TYPE_TIME = 0x30,           //时间广播
        ATP_TYPE_LOCATION = 0x1F,       //轨道数据
        ATP_TYPE_ATTITUDE = 0x1A,       //姿态数据

    };

    //功能定义
    enum RS422_LOCATION_TYPE{
        LOCATION_TYPE_LOCATION = 0x1C,           //敏感数据 位置数据

    };

    static FrameAssemble *Instance();
private:
    static FrameAssemble *self;
    explicit FrameAssemble(QObject *parent = nullptr);
public:
//    //RS422 通道编号
//    quint8 RS422ChannelNo = LVDS_DATA_CH1;
//    //LVDS 通道编号
//    quint8 LVDSChannelNo = LVDS_DATA_CH3;
//    //CAN 通道编号
//    quint8 CANChannelNo = LVDS_DATA_CH4;

    //变长指令组装
    void RS422ChangeFrameAssemble(QByteArray command,RS422_APP_CHANGE_CMD cmdType);
    //定长指令组装-应用在自定义发送定长数据，作为测试使用或自定义发送使用
    void RS422FixedFrameAssemble(QByteArray command,RS422_APP_FIXED_CMD cmdType);
    //定长指令组装-应用在模板自动组装的数据发送，具体指令模板已经明确
    void RS422FixedFrameAssemble(QByteArray command);

    //功能类定义
    //发送FPGA数据
    void RS422SendFPGABin(QString fileName,int timeSpan);
    //发送DSP数据
    void LVDSSendDSPBin(QString fileName);
    //发送遥测指令
    void RS422TelemetryRequestDataSend();
    //发送时间码
    void RS422TimeCodeDataSend(quint32 Second,quint32 us);
    //发送卫星轨道信息1Hz
    void RS422SatelliteOrbitDataSend(quint8 orbitMode,quint32 Second,quint16 ms,float x,float y,float z,float Vx,float Vy,float Vz);
    //发送卫星姿态信息
    void RS422SatelliteGestureDataSend(quint32 Second,quint16 us,float q1,float q2,float q3,float q4);
    //发送敏感数据 位置数据
    void RS422SensitiveDataSend(quint32 Second,quint16 ms,float X,float Y,float Z);
    //发送曲线数据 注入曲线
    void RS422CurveDataSend(QByteArray command);
    //发送地面指令注入
    void RS422CommandDataSend(QByteArray command);

    //发送指令序列
    void RS422InstructionSequenceSend(QByteArray command);
//    //设置LVDS的传输模式
//    void SetLVDSSpeedMode(bool isHigh);


    //测试
    void frameTest();
    //测试422数据解析
    void frameRS422Test();
    //测试帧率控制
    void frameTest2();

public:
    //获取RS422校验码
    static quint16 getRS422SumCheckCode(QByteArray command);
    //获取LVDS校验码
    static quint16 getLVDSSumCheckCode(QByteArray command);
    //RS422数据校验计算
    static quint16 rs422CheckCRC(const QByteArray &data);
    //LVDS数据校验计算
    static quint16 lvdsCheckCRC(const QByteArray &data);
    //重构数据的校验组装
    static quint32 DSPDataSumValue(QByteArray &data);
    static quint32 DSPDataXorValue(QByteArray &data);
private:
    //应用层数据组装
    void RS422AppDataFrameAssemble(QByteArray command,RS422_APP_CMD_TYPE cmdType);
    //LVDS处理数据部分
    void LVDSFrameProcess(const QByteArray &data);

    //RS422处理数据部分
    void RS422FrameProcess(const QByteArray &data);

    //CAN处理数据部分
    void CANFrameProcess(const QByteArray &data);

    //RS422数据分包
    void RS422DataSubPackage(QByteArray command);
    //RS422数据分包并没包携带数据类型 只用来发送变长指令
    void RS422DataSubPackageWithHead(RS422_APP_CHANGE_CMD cmdType,QByteArray command);

    //如果是FPGA程序则需要1s发送一次
    void RS422FPGADataSubPackage(QByteArray &command);
    //另外一种方式发送FPGA程序每次发送都添加标识和头部信息；
    void RS422FPGADataSubPackageWidthHead(QByteArray &command);
    //RS422传输层打包
    void RS422TransformFrameAssemble(QByteArray command);
    //LVDS数据分包发送DSP数据 目前LVDS只发送DSP数据 并且要求是低速模式
    void LVDSDSPDataSubPackage(QByteArray command,LVDS_APP_DATA_TYPE dataType);
    //LVDS传输层打包
    void LVDSTransformFrameAssemble(QByteArray command);
    //发送函数根据类型进行发送不同的板卡
    void TransformData(TransformType type,QByteArray command);
public:
    //处理图像数据
    void processImageData(QByteArray data);
    //处理跟踪数据
    void processTraceData(QByteArray data);
    //处理重构数据
    void processReconsitutionData(QByteArray data,int seqType);

    //测试使用
    static QByteArray hexStringtoByteArray(QString hex);
    static void formatString(QString &org, int n=2, const QChar &ch=QChar(' '));

signals:
    void hasSendDSPPackCount(int totalCount,int count);
    void hasSendFPGAPackCount(int totalCount,int count);
    void hasSendRS422PackCount(int totalCount,int count);
    //发送图像
    void SendLVDSImage(QByteArray imageData,int imageWidth,int imageHeight,QString imageFlag);
    //发送当前帧的信息
    void SendLVDSFrameInfo1(int seq,double timeCode);
    void SendLVDSFrameInfo1(QString msg);
    void SendLVDSFrameInfo2(int frameNum,int frameInNum,int windowSize);
    //0代表总帧数 1代表校验错误 2代表校验正确
    void SendLVDSFrameComeIn(int type);
    //定义发送图像帧LVDS校验错误统计
    void SendLVDSImageFrameStatusInfo(quint32 totalFrames,quint32 successFrames, quint32 failedFrames);
    void SendLVDSLowFrameStatusInfo(quint32 totalFrames,quint32 successFrames, quint32 failedFrames);

    //发送接收到遥测数据的信号
    void recvRS422Data(const QByteArray &data);
    void recvRS422CrCCheck(bool isRight);

    //响应帧处理
    void sendRS422Respond(QByteArray &data);
    //遥测数据处理
    void sendRS422TelemetryRequest(QByteArray &data);

    //将数据发送到低速处理界面
    void lowLVDSData(const QByteArray &data);

public slots:
    void fpgaBinTimeout();
    //采集卡接收到的数据处理
    void LVDSCardDataRecved(quint8 channel,QByteArray data);
    void RS422CardDataRecved(quint8 channel,QByteArray data);
    void RS422CardDataRecvedSerial(QByteArray data);
    void CANCardDataRecved(quint8 channel,QByteArray data);

private:

    //FPGA发送相关变量
    quint8 fpgaBinFirstByte = 0;
    quint8 fpgaBinFlag = 0;
    int fpgaBinTotalCount = 0;
    int fpgaBinHasSendCount = 0;
    //保存历史数据
    QByteArray historyCmd;

    //RS422帧头和帧尾
    QByteArray m_RS422FrameHead;
    //QByteArray m_RS422FrameTail;
    QByteArray RS422Data;
    QByteArray RS422DataBuffer;

    QByteArray m_LVDSFrameHead;
    QByteArray m_LVDSFrameTail;

    //LVDS图像数据大小
    int ImageWidth = 256;
    int ImageHeight = 256;

    //当前图像数据宽高
//    int CurrentImageWidth = ImageWidth;
//    int CurrentImageHeigth = ImageHeight;
    int OneImageDataCount = ImageWidth*ImageHeight;

    //视频接收后的图像数据
    QByteArray imageData;
    //图像数据buffer
    QByteArray mImageBuffer;
    //RS422发送包序号
    quint8 RS422PackCount = 0;
    //发送FPGA的
    quint8 RS422FPGAPackCount = 0;


    //来个定时器 1s发送一包FPGA的bin数据
    QTimer fpgaBinTimer;
    QByteArray fpgaProcessBin;

    //正在升级中
    bool isUpdating = false;
    //发送即那个
    int fpgaSendInterval = 1000;

    //定义图像数据LVDS帧统计信息

    quint32 totalLVDSImageFrames = 0;
    quint32 totalLVDSImageSuccessFrames = 0;
    quint32 totalLVDSImageFailedFrames = 0;
    //低速LVDS帧统计信息
    quint32 totalLowLVDSFrames = 0;
    quint32 totalLowLVDSSuccessFrames = 0;
    quint32 totalLowLVDSFailedFrames = 0;

    //添加一个定时器来监控低速接收部分
    //QTimer * lowLVDSWatcher = nullptr;
};

#endif // FRAMEASSEMBLE_H
