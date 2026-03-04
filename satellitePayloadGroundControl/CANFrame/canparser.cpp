#include "CANParser.h"
#include <cstring>
#include <qDebug>

CANParser::CANParser()
{
    reset();
}

void CANParser::reset()
{
    memset(frameReceived, 0, sizeof(frameReceived));
    lastSeq = -1;
//    fullTM.clear();
}

bool CANParser::pushFrame(const VCI_CAN_OBJ &obj)
{
    // 必须是扩展帧
    if (!obj.ExternFlag)
        return false;

    uint32_t id = obj.ID;

    // ==========================
    // 解析扩展帧 ID
    // ==========================

//    int srcAddr = (id >> 20) & 0x3F; // 源节点地址
    int flag = (id >> 11) & 0x03; // 最后一帧标志
    int seq  = (id >> 5)  & 0x3F; // 帧序号（0~63）
//    int flag = (id >> 8) & 0x03; // 最后一帧标志
//    int seq  = (id >> 2)  & 0x3F; // 帧序号（0~63）

    if (seq < 0 || seq >= 64)
        return false;

    // ==========================
    // 保存帧 payload
    // ==========================
    QByteArray payload(reinterpret_cast<const char*>(obj.Data), obj.DataLen);

    frameBuffer[seq] = payload;
    frameReceived[seq] = true;

    if (seq > lastSeq)
        lastSeq = seq;
//    qDebug()<<"------flag-------:"<<flag;
//    qDebug()<<"------seq-------:"<<seq;
//    qDebug()<<"------lastSeq---:"<<lastSeq;

    /// 先找到首帧（是否有必要？？）

    // ==========================
    // 若 flag == 2：最后一帧 → 开始拼包
    // ==========================
    if (flag == 2)
    {
        qDebug()<<"找到尾帧";
        fullTM.clear();

        for (int i = 0; i <= lastSeq; i++)
        {
            if (frameReceived[i])
                fullTM.append(frameBuffer[i]);
            qDebug()<<"fullTM.size"<<fullTM.size();
        }

        reset();   // 重置状态，等待下一包数据
        return true;
    }
    else {
        qDebug()<<"没有找到尾帧";
    }

    return false;
}



QByteArray CANParser::getFullTM() const
{
    return fullTM;
}
