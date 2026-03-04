/**
  时间：2025.8.6
  作者：XXH
  说明：这是一个专门用于定义结构体的头文件，没有与之对应的cpp文件
  */

#ifndef STRUCTS_H
#define STRUCTS_H
#include <QString>
#include <QStringList>
#include <QPoint>



//UI界面将用户设定的属性发送出去的结构体
struct SettingsUiToSub{
    int grayThrehold; // 灰度阈值设置
};

// LVDS图像帧解析时的属性结构体
struct ImageAttributes{
    int imgFrameNo = 0; // 图像帧编号
    QString imgFrameNoString;
    int totalImgNum = 0; // 总图像帧计数
    int validImgNum = 0; // 正确图像帧计数
    int errorImgNum = 0; // 错误图像帧计数，校验和不对
    int imgWinSize = 0; // 开窗大小
    int overThreshold = 0; // 超阈值点数量
    QString timeCode; // 时间码 解析成XXXXX 秒 XXXXX 毫秒

    QVector<QPoint> overThresholdCo;
};

// 相机质心帧属性结构体
struct CentroidFrameAttributes{
    double centroidX = 0.0; // X方向质心2字节
    double centroidY = 0.0; // Y方向质心2字节
    int maxGrayscale = 0; // 最大灰度值2字节
    int minGrayscale = 0; // 最小灰度值2字节
    int overThreshold = 0 ; // 超阈值点数量2字节
    int exposureTime = 0; // 曝光时间2字节
    quint32 grayscaleSum = 0; // 灰度累加和4字节

    int totalFrameNum = 0; // 总质心帧计数
    int validFrameNum = 0; // 正确质心帧计数
    int errorFrameNum = 0; // 错误质心帧计数，校验和不对


};

// 红外相机温度遥测帧属性结构体
struct TemperatureFrameAttributes{
    int tempTEC = 0; // TEC温度
    int tempFPGA = 0; // FPGA温度

    bool isUiParaValid = false;
    int totalUiParaNum = 0;
    int errorUiparaNum = 0;
    int validUiParaNum = 0;

    int totalFrameNum = 0; // 总温度帧计数
    int validFrameNum = 0; // 正确温度帧计数
    int errorFrameNum = 0; // 错误温度帧计数，校验和不对

};

// 这个结构体的目的是从xml中获取需要的参数
struct CommandAttribute {

    QString paraNo; // 参数编号
    QString paraName; // 参数名称
    QString paraValue; // 参数数值
    QString byteHex; // 源码
    QString byteNotes; // 备注

    QString paraType;       // 新增：EDIT or COMBOX

    // 新增字段（COMBOX类型专用）
    QStringList paraOptions; // 新增：COMBOX的所有选项（ParaSpecificationT）
    QStringList hexCodes;    // 每个选项对应的源码（ParaRawCodeH）
    QString paraIntCode;

    // 新增字段（EDIT类型专用）
    QString encodeRule;      // 编码规则（如 "x/8"），用户输入x，得到x/8
    QString decodeRule;      // 解码规则（如 "x*8"），看到源码y，得到x=y*8
    int byteLength;          // 字节长度（如 1, 2）
    QString minValue;        // 最小值（如 "64"）
    QString maxValue;        // 最大值（如 "512"）

    // 默认索引号
    int initIndex;
};

// 指令组结构体，加载到指令配置列表
struct Command {
    QString name; // 指令组名称
    int Index; // 指令组索引

    QString header; // 指令组帧头
    QString tail; // 指令组帧尾
    int checkSumLength = 1; // 校验和占据多少个字节

    QList<CommandAttribute> attributes; // 指令组内容
};

/// 指令序列保存xml文件结构体，加载到指令发送列表
struct CommandInfo{
    int index; // 当前指令在指令序列中的索引（行号）
    QString CommandTime;            //命令发送的时间戳
    QString CommandName;            //指令名称
    QString CommandCode;            //命令码 发送命令使用
    QString CommandRemark;          //这条命令的备注信息
    QList<CommandAttribute> paraInfo;      //参数信息 一个命令有多个数据域
};

struct CommandQueue{
    QString commandTime; // 指令时间延迟
    QString commandName ; // 指令名称
    QString commandCodeH; // 指令代码
    QString commandCode; // 完整的指令行28000000080800000E102020FFFF
    QString commandRemarks; // 备注
};




// 遥测解析结构体
struct TMparaQueue{
    QString paraName;
    QString initCode;
};

struct TMitemQueue{
    QString TMitemLength; // 遥测参数源码长度
    QString TMitemNo; // 遥测参数编号
    QString TMitemName ; // 遥测参数名称
    QString ParaType; // 遥测参数源码
    QString EncodeType; // 遥测参数解码规则（从十六位源码到十进制数值）
    // （COMBOX类型专用）
//    QString Para; // 遥测参数数值
    QString TMitemRemarks; // 备注
    QString TMitemMinF;
    QString TMitemMaxF;
    QString FrameLength;

    QList<TMparaQueue> TMparaList;

    QString ByteInherit; // 字节是否继承
    QString BitOffset;
    QString BitLength;

    QString TMsignedType;// 有符号数还是无符号数
};

struct ParaListUi{
    QString paraNo;
    QString value; // 数值（如果是EDIT，直接十进制数值，如果是COMBBOX，根据索引查询对应节点名称）
    QString code; // 源码
    bool isValid  = true; // 有效性
};

struct FrameTM{
    int frameLength = 0;
    QString frameHead;
    QString frameTail;
    int checkSumNum = 0;

    int frameLengthAll = 0;
    QString frameHeadAll;
    QString CutFront;
};

struct canFrameConfig{
    // 优先级
    uint32_t priority = 0;
    // 源节点地址
    uint32_t srcAddress = 1;
    // 组播标识
    uint32_t multicast = 0;
    // 目的节点地址
    uint32_t destAdrr = 10;
    // 功能码
    uint32_t funCode = 0;
};


#endif // STRUCTS_H

