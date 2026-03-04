#ifndef CAN_PARSER_HPP
#define CAN_PARSER_HPP

#include "ControlCAN.h"
#include <QByteArray>

class CANParser
{
public:
    CANParser();

    // 输入一帧 CAN，对其进行解析
    // 若返回 true，表示已经完成一次完整遥测包
    bool pushFrame(const VCI_CAN_OBJ &obj);

    // 返回完整 TM（flag=1 时才能调用）
    QByteArray getFullTM() const;

private:
    void reset();

private:
    QByteArray frameBuffer[64]; // 每帧最多 8 字节，序号 0~63
    bool frameReceived[64];
    int lastSeq;
    QByteArray fullTM; // 完整遥测包
};

#endif // CAN_PARSER_HPP
