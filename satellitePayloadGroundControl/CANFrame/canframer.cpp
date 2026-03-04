#include "CANFramer.h"
#include <cstring>
#include <qDebug>

std::vector<VCI_CAN_OBJ> CANFramer::frameCAN(const QByteArray &tmData, canFrameConfig canFrameConfigInput)
{
    std::vector<VCI_CAN_OBJ> frames;

    int totalLen = tmData.size();
    int frameCount = (totalLen + 7) / 8; // 向上取整
    qDebug()<<"totalLen"<<totalLen; // 127
    qDebug()<<"frameCount"<<frameCount; // 16

    for(int i = 0; i < frameCount; i++)
    {
        VCI_CAN_OBJ obj;
        memset(&obj, 0, sizeof(obj));

        // -------------------
        // 构造 29bit 扩展帧 ID
        // -------------------
        // 数据优先级    ID28-ID27   // 00b、01b、10b、11b
        uint32_t priority = canFrameConfigInput.priority; // 遥测10b
        // 源节点地址    ID26-ID21
        uint32_t srcAddr = canFrameConfigInput.srcAddress; // 000001b 星务
        // 组播标识      ID20-ID19
        uint32_t multicast = canFrameConfigInput.multicast; //  00b 点对点传输
        // 目的节点地址   ID18-ID13
        uint32_t destinationNodeAddress = canFrameConfigInput.destAdrr; // 001010b 正X星遥星间激光载荷
        // 帧序号标志位       ID12-ID11
        uint32_t flag = 0; // 最后一帧标志
        if(frameCount == 1){
            flag = 3; // 11b  03H
        }
        else if(i == 0){ // 首帧
            flag = 1; // 01b  01H
        }
        else if(i == frameCount - 1){ // 尾帧
            flag = 2; // 10b  02H
        }
        else{ // 中间帧
            flag = 0; // 00b  00H
        }

        // 帧序号      ID10-ID5
        uint32_t frameNo = i;
        // 功能码      ID4-ID0
        uint32_t funcCode =canFrameConfigInput.funCode;

        uint32_t id = 0;

//        id |= (0x00 & 0x0) << 29;   // ID28~ID27 优先级
        id |= (priority & 0x03) << 27;   // ID28~ID27 优先级
        id |= (srcAddr & 0x3F) << 21;   // ID26~ID21 源节点地址
        id |= (multicast & 0x03) << 19;   // ID20~ID19 组播标识
        id |= (destinationNodeAddress & 0x3F) << 13;   // ID18~ID13 目的节点地址
        id |= (flag & 0x03)   << 11;    // ID12~ID11 √ 帧标志
        id |= (frameNo & 0x3F)   << 5;     // ID10~ID5 √ 帧序号
        id |= (funcCode & 0x1F)   << 0;     // ID4~ID0 √ 功能码

//        id |= (priority & 0x03) << 27;            // ID28~ID27  (shift = 27)
//        id |= (srcAddr & 0x3F) << 21;             // ID26~ID21  (shift = 21)
//        id |= (multicast & 0x03) << 19;           // ID20~ID19  (shift = 19)
//        id |= (destinationNodeAddress & 0x3F) << 13; // ID18~ID13 (shift = 13)
//        id |= (flag & 0x03) << 11;                // ID12~ID11  (shift = 11)
//        id |= (frameNo & 0x3F) << 5;              // ID10~ID5   (shift = 5)
//        id |= (funcCode & 0x1F) << 0;             // ID4~ID0    (shift = 0)

        obj.ID = id & 0x1FFFFFFF;

//        obj.ID = 0x00215801;
//        obj.ID = id<<3;

        qDebug().noquote() << QString("ID = 0x%1 (%2)")
                              .arg(obj.ID, 8, 16, QChar('0'))
                              .arg(obj.ID);

        qDebug().noquote() << QString("ID = 0x%1")
                              .arg(obj.ID, 8, 16, QChar('0')).toUpper();

        qDebug().noquote() << QString("ID = %1 0x%2")
                              .arg(formatCanIdBinary(obj.ID))  // 二进制分组显示
                              .arg(obj.ID, 8, 16, QChar('0')).toUpper();
//        uint32_t id_raw = obj.ID & 0x1FFFFFFF;
//        uint32_t id_left = id_raw << 3;
//        uint32_t id_left = id << 3;

//        qDebug().noquote()
//            << QString("ID(left) = 0x%1")
//               .arg(id_left, 8, 16, QChar('0')).toUpper();

//        uint32_t id_raw = id & 0x1FFFFFFF; // 29bit 逻辑 ID
//        obj.ID = id_raw << 3;              // ★ 左对齐




//        qDebug().noquote()
//            << QString("ID(raw,29b) = 0x%1").arg(id_raw, 0, 16).toUpper();

//        qDebug().noquote()
//            << QString("ID(left,reg) = 0x%1").arg(id_left, 8, 16, QChar('0')).toUpper();

//        qDebug().noquote()
//            << QString("ID(raw,bin)  = %1").arg(id_raw, 29, 2, QChar('0'));

//        qDebug().noquote()
//            << QString("ID(left,bin) = %1").arg(id_left, 32, 2, QChar('0'));


        obj.ExternFlag = 1;  // 扩展帧（0表示普通帧）
        obj.RemoteFlag = 0;  // 数据帧（1表示远程帧）
        obj.SendType   = 1; // 正常发送（1表示单次发送）

        qDebug()<<i<<"---flag---:"<<flag;
        qDebug()<<frameNo<<"---frameNo---:"<<frameNo;
        // -------------------
        // 填充 8 字节 payload
        // -------------------
        int offset = i * 8;
        int len = std::min(8, totalLen - offset);

        obj.DataLen = len;
        qDebug()<<"DLC"<<len;
        memcpy(obj.Data, tmData.constData() + offset, len);

        frames.push_back(obj);
    }

    return frames;
}

QString CANFramer::formatCanIdBinary(uint32_t id) {
    QString binary = QString("%1").arg(id, 32, 2, QChar('0'));
    // 每4位加空格：0000 0001 0000 1010...
    for (int i = 4; i < 32; i += 5) {
        binary.insert(i, ' ');
    }
    return binary;
}
