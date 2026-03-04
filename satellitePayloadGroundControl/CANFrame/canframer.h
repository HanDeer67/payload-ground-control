#ifndef CAN_FRAMER_HPP
#define CAN_FRAMER_HPP

#include "ControlCAN.h"   // 周立功 CAN 设备头文件
#include <QByteArray>
#include <vector>
#include "Structs.h"

class CANFramer
{
public:
    // 输入：完整 TM（二进制数据）
    // 输出：分帧后的 VCI_CAN_OBJ 列表
    static std::vector<VCI_CAN_OBJ> frameCAN(const QByteArray &tmData, canFrameConfig canFrameConfigInput);
    static QString formatCanIdBinary(uint32_t id);

};

#endif // CAN_FRAMER_HPP
