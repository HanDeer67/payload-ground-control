#include "dataanalysishelper.h"
//#include "framehelper.h"
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QDebug>
#include <QThread>
#include <QtMath>
#include <QJSEngine>
#include <QJSValue>
#include <QDomDocument>

DataAnalysisHelper::DataAnalysisHelper(QObject *parent) : QObject(parent)
{
    frameHelper = new FrameHelper(this);
    dataTransfer = new DataTransfer(this);
    overThresholdCoords.reserve(imgSize * imgSize); // 预估 10% 的像素会超过阈值
    canParser1 = new CANParser();
}

///  ************************串口通信接收数据解析函数 ************************
// 对串口1数据进行所需的解析（收数据。3M 质心遥测）......
void DataAnalysisHelper::onDataAnalysisSerial1(){

    while(m_running){
        if (m_ringBuffer && !m_ringBuffer->isEmpty()) {
            // 该串口可能收到两种类型的帧数据，一种是以B9C0开头共计2+16+1=19字节的质心帧，一种是以B9D0开头共计2+2+1字节的应答帧。
            // 质心帧在每次一张图片传输完成后接收到，用于解析①质心XY②灰度最大值③灰度最小值④超阈值点个数⑤曝光时间⑥灰度累加和
            // 应答帧在每次发送指令后接收到，用于解析出发送的指令校验是否正确，①0A正确②A0不正确
            ///B9 C0 20 00 20 00 04 00 00 00 00 C8 00 C8 00 50 00 00 24
            /// B9 D0 0A 00 0A

            size_t available = m_ringBuffer->available();
//            qDebug()<<"available"<<available;
            size_t chunkSize = std::min(available, static_cast<size_t>(FRAME_SIZE_CENTROID));

            if (chunkSize > 0) {
                QByteArray frame = m_ringBuffer->pop(chunkSize);
                qDebug()<<"frame.size() "<<frame.size(); // 输出当前pop的数据长度
                processSerial1Frame(frame);  // 解析串口1数据帧
            } else {
                QThread::msleep(1);  // 避免忙等待
            }
        }
    }
}


// 对串口2数据进行所需的解析（收数据+发指令。115200 温度等遥测）......
void DataAnalysisHelper::onDataAnalysisSerial2(){
    // 开始对温度遥测数据进行解析
    while(m_running){
        if (m_ringBuffer && !m_ringBuffer->isEmpty()) {
            // 该串口可能收到一种类型的帧数据，以B9E0开头共计2+8+1=11字节的温度帧。
            ///E9 D0 0A 00 FF EC 00 00 FF FF F3
            /// 其中FF EC表示TEC温度，有符号整形，所以需要先判断第一个字节的第一位是1还是0，1表示负温度，0表示正温度

            size_t available = m_ringBuffer->available();
//            qDebug()<<"available"<<available;
            size_t chunkSize = std::min(available, static_cast<size_t>(FRAME_SIZE_TEM));

            if (chunkSize > 0) {
                QByteArray frame = m_ringBuffer->pop(chunkSize);
//                qDebug()<<"frame.size() "<<frame.size(); // 输出当前pop的数据长度
                processSerial2Frame(frame);  // 解析串口2数据帧
//                processLVDSFrame(frame); // 临时测试，使用COMII接收图像数据
            } else {
                QThread::msleep(1);  // 避免忙等待
            }
        }
    }
}

void DataAnalysisHelper::processSerial1Frame(QByteArray frame)
{
    serialBuffer1.append(frame);
    while (serialBuffer1.size() >= FRAME_SIZE_CENTROID) {
        // 1. 查找帧头
        int startIndex = -1;
        for (int i = 0; i<=serialBuffer1.size() - FRAME_SIZE_CENTROID; ++i) {
            if(static_cast<quint8>(serialBuffer1[i]) == 0xB9 &&
               static_cast<quint8>(serialBuffer1[i+1]) == 0xC0){
                startIndex = i;
                break;
            }
        }

        if(startIndex == -1){
            int keepSize = qMin(FRAME_SIZE_CENTROID-1, serialBuffer1.size());
            serialBuffer1 = serialBuffer1.right(keepSize);
            qDebug()<<"没有找到质心帧帧头";
            break;
        }

        qDebug()<<"startIndexCentroid:"<<startIndex;

        // 2. 丢弃质心帧头前的无效数据
        if(startIndex > 0){
            serialBuffer1.remove(0,startIndex);
            qDebug()<<"丢弃的质心帧字节数："<<startIndex<< "bytes";
        }

        // 3. 数据是否足够构成一帧
        if(serialBuffer1.size()<FRAME_SIZE_CENTROID){
            qDebug()<<"数据不足组成一帧19字节的质心帧,等待后续数据";
            break;
        }

        // 4. 提取帧存进缓存
        QByteArray frameData = serialBuffer1.left(FRAME_SIZE_CENTROID);
        centroidFrameAttrs.totalFrameNum += 1;
        centroidFrameAttrs.validFrameNum += 1;

        // 5. 检查校验和
        // 提取当前帧校验和
        QByteArray currentCheckSum  = frameData.mid(18,1);
        // 提取纯数据计算校验和
        QByteArray frameDataCheck = frameData.mid(2,16);
        QByteArray checkSum = frameHelper->frameChecksum(frameDataCheck,1);
        qDebug()<<"质心帧计算出的校验和"<<checkSum.toHex(' ').toUpper();
        qDebug()<<"质心帧接收到的校验和"<<currentCheckSum.toHex(' ').toUpper();
       if(checkSum != currentCheckSum ){
           // 当前帧校验和不对
           qDebug()<<"当前帧校验和不对";
           centroidFrameAttrs.errorFrameNum += 1;
           centroidFrameAttrs.validFrameNum -= 1;
       }

       // 6. 帧没有问题，开始提取信息
       quint16 centroidX = static_cast<quint16>(
                    (static_cast<quint8>(frameData[2]) << 8)  |
                    (static_cast<quint8>(frameData[3]))
               );

       quint16 centroidY = static_cast<quint16>(
                    (static_cast<quint8>(frameData[4]) << 8)  |
                    (static_cast<quint8>(frameData[5]))
               );

       quint16 maxGrayscale = static_cast<quint16>(
                    (static_cast<quint8>(frameData[6]) << 8)  |
                    (static_cast<quint8>(frameData[7]))
               );

       quint16 minGrayscale = static_cast<quint16>(
                    (static_cast<quint8>(frameData[8]) << 8)  |
                    (static_cast<quint8>(frameData[9]))
               );

       quint16 overThreshold = static_cast<quint16>(
                    (static_cast<quint8>(frameData[10]) << 8)  |
                    (static_cast<quint8>(frameData[11]))
               );

       quint16 exposureTime = static_cast<quint16>(
                    (static_cast<quint8>(frameData[12]) << 8)  |
                    (static_cast<quint8>(frameData[13]))
               );
       quint32 grayscaleSum = static_cast<quint32>(
                    (static_cast<quint8>(frameData[14]) << 24)  |
                    (static_cast<quint8>(frameData[15]) << 16)  |
                    (static_cast<quint8>(frameData[16]) << 8)   |
                    (static_cast<quint8>(frameData[17]))
               );

    centroidFrameAttrs.centroidX = centroidX / 32;
    centroidFrameAttrs.centroidY = centroidY / 32;
    centroidFrameAttrs.maxGrayscale = maxGrayscale;
    centroidFrameAttrs.minGrayscale = minGrayscale;
    centroidFrameAttrs.overThreshold = overThreshold;
    centroidFrameAttrs.exposureTime = exposureTime;
    centroidFrameAttrs.grayscaleSum = grayscaleSum;

    emit upgradeCentroidUi(centroidFrameAttrs);

    serialBuffer1 = serialBuffer1.mid(FRAME_SIZE_CENTROID); // 移除已处理部分
    if (serialBuffer1.size() < FRAME_SIZE_CENTROID) {
           break;  // 剩余数据不足，等待下次输入
       }
    }
}

// 从红外相机返回帧115200中解析温度信息
void DataAnalysisHelper::processSerial2Frame(QByteArray frame)
{
    qDebug()<<"processSerial2Frame";
    serialBuffer2.append(frame);
    while (serialBuffer2.size() >= FRAME_SIZE_TEM) {
        // 1. 查找帧头
        int startIndex = -1;
        for (int i = 0; i<=serialBuffer2.size() - FRAME_SIZE_TEM; ++i) {
            if(static_cast<quint8>(serialBuffer2[i]) == 0xB9 &&
               static_cast<quint8>(serialBuffer2[i+1]) == 0xE0){
                startIndex = i;
                break;
            }
        }

        if(startIndex == -1){
            int keepSize = qMin(FRAME_SIZE_TEM-1, serialBuffer2.size());
            serialBuffer2 = serialBuffer2.right(keepSize);
            qDebug()<<"没有找到温度帧帧头";
            break;
        }

        qDebug()<<"startIndexTem:"<<startIndex;

        // 2. 丢弃质心帧头前的无效数据
        if(startIndex > 0){
            serialBuffer2.remove(0,startIndex);
            qDebug()<<"丢弃的质心帧字节数："<<startIndex<< "bytes";
        }

        // 3. 数据是否足够构成一帧
        if(serialBuffer2.size()<FRAME_SIZE_TEM){
            qDebug()<<"数据不足组成一帧19字节的质心帧,等待后续数据";
            break;
        }

        // 4. 提取帧存进缓存
        QByteArray frameData = serialBuffer2.left(FRAME_SIZE_TEM);
        temFrameAttrs.totalFrameNum += 1;
        temFrameAttrs.validFrameNum += 1;

        // 额外添加关于指令是否正常的统计
        if(static_cast<quint8>(serialBuffer2[2]) == 0x0A){
            temFrameAttrs.totalUiParaNum +=1;
            temFrameAttrs.validUiParaNum +=1;
        }
        else {
            temFrameAttrs.totalUiParaNum +=1;
            temFrameAttrs.errorUiparaNum +=1;
        }

        // 5. 检查校验和
        // 提取当前帧校验和
        QByteArray currentCheckSum  = frameData.mid(18,1);
        // 提取纯数据计算校验和
        QByteArray frameDataCheck = frameData.mid(2,16);
        QByteArray checkSum = frameHelper->frameChecksum(frameDataCheck,1);
        qDebug()<<"质心帧计算出的校验和"<<checkSum.toHex(' ').toUpper();
        qDebug()<<"质心帧接收到的校验和"<<currentCheckSum.toHex(' ').toUpper();
       if(checkSum != currentCheckSum ){
           // 当前帧校验和不对
           qDebug()<<"当前帧校验和不对";
           temFrameAttrs.errorFrameNum += 1;
           temFrameAttrs.validFrameNum -= 1;
       }

       // 6. 帧没有问题，开始提取信息
       int temTEC = static_cast<qint16>(
                    (static_cast<qint8>(frameData[4]) << 8)  |
                    (static_cast<qint8>(frameData[5]))
               );
       int temFPGA = static_cast<qint16>(
                    (static_cast<qint8>(frameData[6]) << 8)  |
                    (static_cast<qint8>(frameData[7]))
               );

        temFrameAttrs.tempTEC = temTEC;
        temFrameAttrs.tempFPGA = temFPGA;

        emit upgradeTemUi(temFrameAttrs);

        serialBuffer2 = serialBuffer2.mid(FRAME_SIZE_TEM); // 移除已处理部分
        if (serialBuffer2.size() < FRAME_SIZE_TEM) {
               break;  // 剩余数据不足，等待下次输入
        }
    }
}


/// ************************板卡通信接收数据解析函数 ************************
// 执行*遥测*解析的代码......
//void DataAnalysisHelper::onDataAnalysisRS422(){

//    while(m_running){
//        if (m_ringBuffer && !m_ringBuffer->isEmpty()) {
//            // 该板卡可能收到三种类型的帧数据，一种是以B9C0开头共计2+16+1=19字节的质心帧，一种是以B9D0开头共计2+2+1字节的应答帧。
//            // 质心帧在每次一张图片传输完成后接收到，用于解析①质心XY②灰度最大值③灰度最小值④超阈值点个数⑤曝光时间⑥灰度累加和
//            // 应答帧在每次发送指令后接收到，用于解析出发送的指令校验是否正确，①0A正确②A0不正确
//            ///B9 C0 20 00 20 00 04 00 00 00 00 C8 00 C8 00 50 00 00 24
//            /// B9 D0 0A 00 0A

//            // 此外，地检还会接收到遥测参数帧，一般是以EB90开头
//            // 如何先判断当前帧是什么类型的帧再进行相应的解析呢

//            size_t available = m_ringBuffer->available();
////            qDebug()<<"available"<<available;
//            size_t chunkSize = std::min(available, static_cast<size_t>(19));

//            if (chunkSize > 0) {
//                QByteArray frame = m_ringBuffer->pop(chunkSize);
//                qDebug()<<"frame.size() "<<frame.size(); // 输出当前pop的数据长度
//                processRS422Frame(frame);  // 解析串口1数据帧
//            } else {
//                QThread::msleep(1);  // 避免忙等待
//            }
//        }
//    }
//}

void DataAnalysisHelper::onDataAnalysisRS422() {
    while (m_running) {
        if (m_ringBuffer && !m_ringBuffer->isEmpty()) {
            size_t available = m_ringBuffer->available();
//            qDebug()<<"m_ringBuffer size"<<available;

            // 至少要能看两个字节的帧头
            if (available < 2) {
                QThread::msleep(1);
                continue;
            }

            // pop前lengthHead个字节来判断帧类型
            QByteArray header = m_ringBuffer->peek(lengthHead);
            quint8 h0 = static_cast<quint8>(header[0]);
            quint8 h1 = static_cast<quint8>(header[1]);


            int frameLen = 0;
            enum FrameType { Unknown, Centroid, Ack, Telemetry } frameType = Unknown;

            qDebug()<<"h0"<<h0;
            qDebug()<<"h1"<<h1;
            qDebug()<<"h0_xml***"<<h0_xml;
            qDebug()<<"h1_xml***"<<h1_xml;

            if (h0 == 0xB9 && h1 == 0xC0) {
                frameLen = 19;              // 质心帧
                frameType = Centroid;
                qDebug()<<"质心帧";
            } else if (h0 == 0xB9 && h1 == 0xD0) {
                frameLen = 5;               // 应答帧
                frameType = Ack;
                qDebug()<<"应答帧";
            } else if (h0 == h0_xml && h1 == h1_xml) {
                frameLen =  frameTm.frameLength;              // 从xml文件中读取遥测帧长度
                frameType = Telemetry;
                qDebug()<<"遥测帧";
                qDebug()<<"遥测帧长度"<<frameLen;
            } else {
                // 未知帧头：丢掉一个字节再继续
                qDebug()<<"未知帧头";
                m_ringBuffer->pop(1);
                continue;
            }

            // 如果缓冲区数据不足整帧，就等下一轮
            if (available < static_cast<size_t>(frameLen)) {
                qDebug()<<"等待下一轮" ;
                QThread::msleep(1);
                continue;
            }

            // 取出整帧数据
            QByteArray frame = m_ringBuffer->pop(frameLen);
            qDebug()<<"frame"<<frame;

            // 根据类型解析
            switch (frameType) {
                case Centroid:
                    qDebug()<<"质心帧解析";
                    processRS422Frame(frame);       // 质心帧解析                   
                    break;
                case Ack:
                    qDebug()<<"应答帧解析";
                    processAckFrame(frame);         // 应答帧解析
                    break;
                case Telemetry:
                    qDebug()<<"遥测帧解析";
                    processTelemetryFrame(frame);   // 遥测帧解析
                    break;
                default:
                    break;
            }
        } else {
            QThread::msleep(1);  // 避免忙等待
        }
    }
}

void DataAnalysisHelper::processAckFrame(const QByteArray &frame) {
    // TODO: 应答帧解析逻辑
    qDebug() << "Ack Frame:" << frame.toHex(' ');
}

void DataAnalysisHelper::processTelemetryFrame(const QByteArray &frame) {
    // TODO: 遥测帧解析逻辑
    qDebug() << "Telemetry Frame:" << frame.toHex(' ');
    // 从paraQueueVector中获取数据长度、编码方式、combbox
    // 检查帧尾和校验和是否正确
    // 1. 帧尾
    QByteArray frameTail = frame.mid(frame.size()-lengthTail,-1);
    quint8 t0 = static_cast<quint8>(frameTail[0]);
    quint8 t1 = static_cast<quint8>(frameTail[1]);
//    qDebug()<<"t0"<<t0;
//    qDebug()<<"t1"<<t1;

    // ③帧尾
//    QString frameTail = frameTm.frameTail;
    QByteArray ba_frameTail = QByteArray::fromHex(frameTm.frameTail.toUtf8());
    quint8 t0_xml = static_cast<quint8>(ba_frameTail[0]);
    quint8 t1_xml = static_cast<quint8>(ba_frameTail[1]);

    if(t0 != t0_xml || t1 != t1_xml){
        qDebug()<<"帧尾不对";
        // 错误帧+1
        emit updateFrameCheckSignal(false);
        return;
    }
    // 2. 校验和
    // 提取当前帧校验和
    QByteArray currentCheckSum  = frame.mid(frame.size() - lengthHead - lengthTail, lengthHead);
    // 提取纯数据计算校验和
    QByteArray frameDataCheck = frame.mid(lengthHead, frame.size() - lengthHead - lengthTail - frameTm.checkSumNum); // 这里的4是帧头和帧尾占据的字节数,所以此版本必须局限在2字节的帧头
    // 和2字节的帧尾，如果协议发生变化，需要修改该代码
    QByteArray checkSum = frameHelper->frameChecksum(frameDataCheck, frameTm.checkSumNum);
    if(currentCheckSum != checkSum){
        qDebug()<<"遥测帧校验和不对";
        // 错误帧+1
        emit updateFrameCheckSignal(false);
        return;
    }
    // 正确帧+1
    emit updateFrameCheckSignal(true);

    // 先剔除帧头
    QByteArray frameDeal = frame.mid(lengthHead,-1);
    for(int i = 0; i < paraQueueVector.size();++i){
        const TMitemQueue &cmd = paraQueueVector.at(i);
        qDebug()  << "源码长度" << cmd.TMitemLength
                  << "参数编号" << cmd.TMitemNo
                  << "参数名称" << cmd.TMitemName
                  << "参数类型" << cmd.ParaType
                  << "编码方式" << cmd.EncodeType
                  << "备注" << cmd.TMitemRemarks
                  << "最小值"<<cmd.TMitemMinF
                  <<"最大值"<<cmd.TMitemMaxF
                 <<"符号数类型"<<cmd.TMsignedType;

        paraListUi.paraNo = cmd.TMitemNo; // 参数编号
        //根据源码长度从frameDeal中截取（pop）对应数据并根据编码方式解析
        int codeLength = cmd.TMitemLength.toInt();
        QString code = frameDeal.mid(0,codeLength).toHex().toUpper();
        int codeInt =  code.toInt(nullptr, 16);

        frameDeal.remove(0,codeLength);
        qDebug()<<"code:"<<code;
        paraListUi.code = code; // 源码

        /// 2026.2.27 新增符号数类型区分
        if(cmd.ParaType == "EDIT"){
            qDebug()<<"EDIT类型";
            // 根据编码方式计算
            paraListUi.value= applyRule(cmd.EncodeType,codeInt);
            // 检查数值是否符合xml限制，如果是，有效性置true，如果否，有效性置false
            double valueDouble = applyRule(cmd.EncodeType,codeInt).toDouble();
            double valueMin = cmd.TMitemMinF.toDouble();
            double valueMax = cmd.TMitemMaxF.toDouble();

            if(valueDouble>=valueMin && valueDouble<=valueMax){
                paraListUi.isValid = true;
            }
            else {
                paraListUi.isValid = false;
            }
        }

        else if(cmd.ParaType == "COMBOX"){
            qDebug()<<"COMBOX类型";
            // 根据源码查询Para中对应的名称,赋值给paraListUi.value
            bool isValid = false;
            for (int j = 0 ; j < cmd.TMparaList.size();++j) {
                qDebug()<<"cmd.TMparaList.at(j).initCode"<<cmd.TMparaList.at(j).initCode;
                qDebug()<<"code"<<code;

                if(cmd.TMparaList.at(j).initCode == code){
                    qDebug()<<"cmd.TMparaList.at(j).initCode == code*************";
                    paraListUi.value = cmd.TMparaList.at(j).paraName;
                    qDebug()<<"paraListUi.paraName*************"<<cmd.TMparaList.at(j).paraName;
                    isValid = true;
                    break;
                }
            }
            if(isValid){
                paraListUi.isValid = true;
            }
            else {
                paraListUi.isValid = false;
            }
            qDebug()<<"paraListUi.value"<<paraListUi.value;
            qDebug()<<"paraListUi.isValid"<<paraListUi.isValid;
        }
        else{
            qDebug()<<"当前xml文件中有未识别的参数类型（既不是“EDIT”也不是“COMBOX”）";
            return;
        }
        // 有效性

        // 创建一个结构体，存放①数值（根据编码方式计算）②combbox选项（根据源码选择）③源码（直接是截取的数据）④有效性（根据最大最小值判断）
        // 创建一个Vector，存放全部参数的结构体，然后一起传递给主UI
        paraListUiList.append(paraListUi);
    }
//    for (int j = 0;j<paraListUiList.size();++j) {
//        qDebug()<<"value:"<<paraListUiList.at(j).value;
//        qDebug()<<"code:"<<paraListUiList.at(j).code;
//        qDebug()<<"isValid:"<<paraListUiList.at(j).isValid;
//    }
    emit updateParaListUiSignal(paraListUiList);
    paraListUiList.clear();
    qDebug("更新遥测数据信号发送完毕");
}

void DataAnalysisHelper::processTelemetryFrameCAN(const QByteArray &frame) {
    // TODO: 遥测帧解析逻辑
    qDebug() << "processTelemetryFrameCAN";
    qDebug() << "Telemetry Frame:" << frame.toHex(' ');

    // 2. 校验和
    // 提取当前帧校验和
//    QByteArray currentCheckSum  = frame.mid(frame.size() - lengthHead - lengthTail, lengthHead);
    // 提取纯数据计算校验和
//    QByteArray frameDataCheck = frame.mid(lengthHead, frame.size() - lengthHead - lengthTail - frameTm.checkSumNum); // 这里的4是帧头和帧尾占据的字节数,所以此版本必须局限在2字节的帧头
    // 和2字节的帧尾，如果协议发生变化，需要修改该代码
//    QByteArray checkSum = frameHelper->frameChecksum(frameDataCheck, frameTm.checkSumNum);
//    if(currentCheckSum != checkSum){
//        qDebug()<<"遥测帧校验和不对";
//        // 错误帧+1
//        emit updateFrameCheckSignal(false);
//        return;
//    }
//    // 正确帧+1
//    emit updateFrameCheckSignal(true);

    // 先剔除帧头
    QByteArray frameDeal = frame.mid(10,-1);
    for(int i = 0; i < paraQueueVector.size();++i){
        const TMitemQueue &cmd = paraQueueVector.at(i);
        qDebug()  << "源码长度" << cmd.TMitemLength
                  << "参数编号" << cmd.TMitemNo
                  << "参数名称" << cmd.TMitemName
                  << "参数类型" << cmd.ParaType
                  << "编码方式" << cmd.EncodeType
                  << "备注" << cmd.TMitemRemarks
                  << "最小值"<<cmd.TMitemMinF
                  <<"最大值"<<cmd.TMitemMaxF
                  <<"符号数类型"<<cmd.TMsignedType;

        paraListUi.paraNo = cmd.TMitemNo; // 参数编号
        //根据源码长度从frameDeal中截取（pop）对应数据并根据编码方式解析
        int codeLength = cmd.TMitemLength.toInt();
        QString code = frameDeal.mid(0,codeLength).toHex().toUpper();
        int codeInt =  code.toInt(nullptr, 16);

        qDebug()<<"code:"<<code;
        int bitLen = 0;

        if(cmd.ByteInherit == "0"){
            // 继承上个字节，用于共享字节的遥测量
          frameDeal.remove(0,codeLength);
          bitLen = codeLength * 8;
        }
        else if(cmd.ByteInherit == "2"){
            // 重新规划CODE，根据bit
            // 获取字长
            bitLen = cmd.BitLength.toInt();
            qDebug()<<"字长****"<<bitLen;
            int checkBit = 0x00;
            switch(bitLen){
            case 0:
                checkBit = 0x00;
                break;
            case 1:
                checkBit = 0x01;
                break;
            case 2:
                checkBit = 0x03;
                break;
            case 3:
                checkBit = 0x07;
                break;
            case 4:
                checkBit = 0x0F;
                break;
            case 5:
                checkBit = 0x1F;
                break;
            case 6:
                checkBit = 0x3F;
                break;
            case 7:
                checkBit = 0x7F;
                break;
            case 8:
                checkBit = 0xFF;
                break;
            }
            // 获取位移
            int bitMove = cmd.BitOffset.toInt();
            int newCode = 0;
            newCode |= ((codeInt >> bitMove) & checkBit) ;
            qDebug()<<"checkBit"<<checkBit;
            qDebug()<<"bitMove"<<bitMove;
            int temp;
            temp = codeInt >> bitMove;
            qDebug()<<"codeInt >> bitMove"<<temp;

            QString newCodeString = QString::number(newCode);
            code = newCodeString;
            codeInt = newCode;
            qDebug()<<"newCodeString"<<newCodeString;
            frameDeal.remove(0,codeLength);
        }
        else{
            // 重新规划CODE，根据bit
            // 获取字长
            bitLen = cmd.BitLength.toInt();
            qDebug()<<"字长****"<<bitLen;
            int checkBit = 0x00;
            switch(bitLen){
            case 0:
                checkBit = 0x00;
                break;
            case 1:
                checkBit = 0x01;
                break;
            case 2:
                checkBit = 0x03;
                break;
            case 3:
                checkBit = 0x07;
                break;
            case 4:
                checkBit = 0x0F;
                break;
            case 5:
                checkBit = 0x1F;
                break;
            case 6:
                checkBit = 0x3F;
                break;
            case 7:
                checkBit = 0x7F;
                break;
            case 8:
                checkBit = 0xFF;
                break;
            }
            // 获取位移
            int bitMove = cmd.BitOffset.toInt();
            int newCode = 0;
            newCode |= ((codeInt >> bitMove) & checkBit) ;
            qDebug()<<"checkBit"<<checkBit;
            qDebug()<<"bitMove"<<bitMove;

            QString newCodeString = QString::number(newCode);
            code = newCodeString;
            codeInt = newCode;
            qDebug()<<"newCodeString"<<newCodeString;
        }

        paraListUi.code = code; // 源码

        /// 2026.2.27 新增符号数类型区分
        if(cmd.ParaType == "EDIT"){
            qDebug()<<"EDIT类型";

            qint64 realValue = 0;
//            int bitLen = cmd.BitLength.toInt();   // 位宽
            // ====== 区分有符号 / 无符号 ======
            if(cmd.TMsignedType == "1")   // 有符号数
            {
                realValue = dataTransfer->convertSigned(
                                static_cast<quint64>(codeInt),
                                bitLen);
            }
            else   // 无符号数
            {
                realValue = static_cast<qint64>(codeInt);
            }
            qDebug()<<"符号转换前数值"<<codeInt;
            qDebug()<<"符号转换后数值"<<realValue;

            // 根据编码方式计算
            paraListUi.value= applyRule(cmd.EncodeType,realValue);
            // 检查数值是否符合xml限制，如果是，有效性置true，如果否，有效性置false
            double valueDouble = paraListUi.value.toDouble();
            double valueMin = cmd.TMitemMinF.toDouble();
            double valueMax = cmd.TMitemMaxF.toDouble();

            if(valueDouble>=valueMin && valueDouble<=valueMax){
                paraListUi.isValid = true;
            }
            else {
                paraListUi.isValid = false;
            }
        }

        else if(cmd.ParaType == "COMBOX"){
            qDebug()<<"COMBOX类型";
            // 根据源码查询Para中对应的名称,赋值给paraListUi.value
            bool isValid = false;
            for (int j = 0 ; j < cmd.TMparaList.size();++j) {
                qDebug()<<"cmd.TMparaList.at(j).initCode"<<cmd.TMparaList.at(j).initCode;
                qDebug()<<"code"<<code;

                if(cmd.TMparaList.at(j).initCode == code){
                    qDebug()<<"cmd.TMparaList.at(j).initCode == code*************";
                    paraListUi.value = cmd.TMparaList.at(j).paraName;
                    qDebug()<<"paraListUi.paraName*************"<<cmd.TMparaList.at(j).paraName;
                    isValid = true;
                    break;
                }
            }
            if(isValid){
                paraListUi.isValid = true;
            }
            else {
                paraListUi.isValid = false;
            }
//            qDebug()<<"paraListUi.value"<<paraListUi.value;
//            qDebug()<<"paraListUi.isValid"<<paraListUi.isValid;
        }
        else{
            qDebug()<<"当前xml文件中有未识别的参数类型（既不是“EDIT”也不是“COMBOX”）";
            return;
        }
        // 有效性

        // 创建一个结构体，存放①数值（根据编码方式计算）②combbox选项（根据源码选择）③源码（直接是截取的数据）④有效性（根据最大最小值判断）
        // 创建一个Vector，存放全部参数的结构体，然后一起传递给主UI
        paraListUiList.append(paraListUi);
    }
//    for (int j = 0;j<paraListUiList.size();++j) {
//        qDebug()<<"value:"<<paraListUiList.at(j).value;
//        qDebug()<<"code:"<<paraListUiList.at(j).code;
//        qDebug()<<"isValid:"<<paraListUiList.at(j).isValid;
//    }
    emit updateParaListUiSignal(paraListUiList);
    paraListUiList.clear();
    qDebug("更新遥测数据信号发送完毕");
}

void DataAnalysisHelper::processTelemetryFrameCAN_slow(const QByteArray &frame) {
    // TODO: 遥测帧解析逻辑
    qDebug() << "Telemetry Frame:" << frame.toHex(' ');

    // 2. 校验和
    // 提取当前帧校验和
//    QByteArray currentCheckSum  = frame.mid(frame.size() - lengthHead - lengthTail, lengthHead);
    // 提取纯数据计算校验和
//    QByteArray frameDataCheck = frame.mid(lengthHead, frame.size() - lengthHead - lengthTail - frameTm.checkSumNum); // 这里的4是帧头和帧尾占据的字节数,所以此版本必须局限在2字节的帧头
    // 和2字节的帧尾，如果协议发生变化，需要修改该代码
//    QByteArray checkSum = frameHelper->frameChecksum(frameDataCheck, frameTm.checkSumNum);
//    if(currentCheckSum != checkSum){
//        qDebug()<<"遥测帧校验和不对";
//        // 错误帧+1
//        emit updateFrameCheckSignal(false);
//        return;
//    }
//    // 正确帧+1
//    emit updateFrameCheckSignal(true);

    // 先剔除帧头
    QByteArray frameDeal = frame.mid(10,-1);
    for(int i = 0; i < paraQueueVector_slow.size();++i){
        const TMitemQueue &cmd = paraQueueVector_slow.at(i);
        qDebug()  << "源码长度" << cmd.TMitemLength
                  << "参数编号" << cmd.TMitemNo
                  << "参数名称" << cmd.TMitemName
                  << "参数类型" << cmd.ParaType
                  << "编码方式" << cmd.EncodeType
                  << "备注" << cmd.TMitemRemarks
                  << "最小值"<<cmd.TMitemMinF
                  <<"最大值"<<cmd.TMitemMaxF
                 <<"符号数类型"<<cmd.TMsignedType;

        paraListUi_slow.paraNo = cmd.TMitemNo; // 参数编号
        //根据源码长度从frameDeal中截取（pop）对应数据并根据编码方式解析
        int codeLength = cmd.TMitemLength.toInt();
        QString code = frameDeal.mid(0,codeLength).toHex().toUpper();
        quint64 codeInt =  code.toULongLong(nullptr, 16);

        qDebug()<<"code:"<<code;
        int bitLen = 0;
        if(cmd.ByteInherit == "0"){
          frameDeal.remove(0,codeLength);
          bitLen = codeLength * 8;
        }
        else if(cmd.ByteInherit == "2"){
            // 重新规划CODE，根据bit
            // 获取字长
            bitLen = cmd.BitLength.toInt();
            qDebug()<<"字长****"<<bitLen;
            qint64 checkBit = 0x0000;
            switch(bitLen){
            case 0:
                checkBit = 0x0000;
                break;
            case 1:
                checkBit = 0x0001;
                break;
            case 2:
                checkBit = 0x0003;
                break;
            case 3:
                checkBit = 0x0007;
                break;
            case 4:
                checkBit = 0x000F;
                break;
            case 5:
                checkBit = 0x001F;
                break;
            case 6:
                checkBit = 0x003F;
                break;
            case 7:
                checkBit = 0x007F;
                break;
            case 8:
                checkBit = 0x00FF;
                break;
            case 9:
                checkBit = 0x01FF;
                break;
            case 10:
                checkBit = 0x03FF;
                break;
            case 11:
                checkBit = 0x07FF;
                break;
            case 12:
                checkBit = 0x0FFF;
                break;
            case 13:
                checkBit = 0x1FFF;
                break;
            case 14:
                qDebug()<<"case 14";
                checkBit = 0x3FFF;
                break;
            case 15:
                checkBit = 0x7FFF;
                break;
            case 16:
                checkBit = 0xFFFF;
                break;
            }
            // 获取位移
            qint64 bitMove = cmd.BitOffset.toInt();
            qint64 newCode = 0;
            newCode |= ((codeInt >> bitMove) & checkBit) ;
            qDebug()<<"checkBit"<<checkBit;
            qDebug()<<"bitMove"<<bitMove;
            int temp;
            temp = codeInt >> bitMove;
//            qDebug()<<"codeInt >> bitMove"<<temp;

            QString newCodeString = QString::number(newCode);
            code = newCodeString;
            codeInt = newCode;
            qDebug()<<"newCodeString"<<newCodeString;
            frameDeal.remove(0,codeLength);
        }
        else{
            // 重新规划CODE，根据bit
            // 获取字长
            bitLen = cmd.BitLength.toInt();
            qDebug()<<"字长****"<<bitLen;
            qint64 checkBit = 0x0000;
            switch(bitLen){
            case 0:
                checkBit = 0x0000;
                break;
            case 1:
                checkBit = 0x0001;
                break;
            case 2:
                checkBit = 0x0003;
                break;
            case 3:
                checkBit = 0x0007;
                break;
            case 4:
                checkBit = 0x000F;
                break;
            case 5:
                checkBit = 0x001F;
                break;
            case 6:
                checkBit = 0x003F;
                break;
            case 7:
                checkBit = 0x007F;
                break;
            case 8:
                checkBit = 0x00FF;
                break;
            case 9:
                checkBit = 0x01FF;
                break;
            case 10:
                checkBit = 0x03FF;
                break;
            case 11:
                checkBit = 0x07FF;
                break;
            case 12:
                checkBit = 0x0FFF;
                break;
            case 13:
                checkBit = 0x1FFF;
                break;
            case 14:
                qDebug()<<"case 14";
                checkBit = 0x3FFF;
                break;
            case 15:
                checkBit = 0x7FFF;
                break;
            case 16:
                checkBit = 0xFFFF;
                break;
            }
            // 获取位移
            quint64 bitMove = cmd.BitOffset.toInt();
            quint64 newCode = 0;
            newCode |= ((codeInt >> bitMove) & checkBit) ;
            qDebug()<<"checkBit"<<checkBit;
            qDebug()<<"bitMove"<<bitMove;

            QString newCodeString = QString::number(newCode);
            code = newCodeString;
            codeInt = newCode;
            qDebug()<<"newCodeString"<<newCodeString;
        }

        paraListUi_slow.code = code; // 源码
        /// 2026.2.27 新增符号数类型区分
        if(cmd.ParaType == "EDIT"){
            qDebug()<<"EDIT类型";

            qint64 realValue = 0;
//            bitLen = cmd.BitLength.toInt();   // 位宽
            qDebug() << "bitLen =" << bitLen;
            qDebug() << "TMsignedType =" << cmd.TMsignedType;
            // ====== 区分有符号 / 无符号 ======
            if(cmd.TMsignedType == "1")   // 有符号数
            {
                realValue = dataTransfer->convertSigned(
                                static_cast<quint64>(codeInt),
                                bitLen);
            }
            else   // 无符号数
            {
                realValue = static_cast<qint64>(codeInt);
            }
            qDebug()<<"符号转换前数值"<<codeInt;
            qDebug()<<"符号转换后数值"<<realValue;

            // 根据编码方式计算
            paraListUi_slow.value= applyRule(cmd.EncodeType,realValue);
            // 检查数值是否符合xml限制，如果是，有效性置true，如果否，有效性置false
            double valueDouble = paraListUi_slow.value.toDouble();
            double valueMin = cmd.TMitemMinF.toDouble();
            double valueMax = cmd.TMitemMaxF.toDouble();

            if(valueDouble>=valueMin && valueDouble<=valueMax){
                paraListUi_slow.isValid = true;
            }
            else {
                paraListUi_slow.isValid = false;
            }
        }
        else if(cmd.ParaType == "COMBOX"){
            qDebug()<<"COMBOX类型";
            // 根据源码查询Para中对应的名称,赋值给paraListUi.value
            bool isValid = false;
            for (int j = 0 ; j < cmd.TMparaList.size();++j) {
                qDebug()<<"cmd.TMparaList.at(j).initCode"<<cmd.TMparaList.at(j).initCode;
                qDebug()<<"code"<<code;

                if(cmd.TMparaList.at(j).initCode == code){
                    qDebug()<<"cmd.TMparaList.at(j).initCode == code*************";
                    paraListUi_slow.value = cmd.TMparaList.at(j).paraName;
                    qDebug()<<"paraListUi.paraName*************"<<cmd.TMparaList.at(j).paraName;
                    isValid = true;
                    break;
                }
            }
            if(isValid){
                paraListUi_slow.isValid = true;
            }
            else {
                paraListUi_slow.isValid = false;
            }
            qDebug()<<"paraListUi.value"<<paraListUi_slow.value;
            qDebug()<<"paraListUi.isValid"<<paraListUi_slow.isValid;
        }
        else{
            qDebug()<<"当前xml文件中有未识别的参数类型（既不是“EDIT”也不是“COMBOX”）";
            return;
        }
        // 有效性

        // 创建一个结构体，存放①数值（根据编码方式计算）②combbox选项（根据源码选择）③源码（直接是截取的数据）④有效性（根据最大最小值判断）
        // 创建一个Vector，存放全部参数的结构体，然后一起传递给主UI
        paraListUiList_slow.append(paraListUi_slow);
    }
//    for (int j = 0;j<paraListUiList.size();++j) {
//        qDebug()<<"value:"<<paraListUiList.at(j).value;
//        qDebug()<<"code:"<<paraListUiList.at(j).code;
//        qDebug()<<"isValid:"<<paraListUiList.at(j).isValid;
//    }
    emit updateParaListUiSignal_slow(paraListUiList_slow);
    paraListUiList_slow.clear();
    qDebug("更新遥测数据信号发送完毕");
}



void DataAnalysisHelper::getXmlParser(QVector<TMitemQueue> paraQueueVectorRec)
{
    qDebug()<<"paraQueueVectorRec.size"<<paraQueueVectorRec.size();
    qDebug()<< "paraQueueVector = paraQueueVectorRec;";
    paraQueueVector = paraQueueVectorRec;
    qDebug()<<"paraQueueVector.size"<<paraQueueVector.size();
}
void DataAnalysisHelper::getXmlParser_slow(QVector<TMitemQueue> paraQueueVectorRec)
{
    qDebug()<<"paraQueueVectorRec.size"<<paraQueueVectorRec.size();
    qDebug()<< "paraQueueVector = paraQueueVectorRec;";
    paraQueueVector_slow = paraQueueVectorRec;
    qDebug()<<"paraQueueVector.size"<<paraQueueVector_slow.size();
}

// 执行*图像*解析和高速数传的代码......
void DataAnalysisHelper::onDataAnalysisLVDS(){
//    qDebug()<<"onDataAnalysisLVDS ";
    /// 从环形缓冲区中读取数据并解析
    while (m_running) {  // 全局运行标志控制循环
        if (m_ringBuffer && !m_ringBuffer->isEmpty()) {
            // 按固定帧大小（如2048字节）读取，不足则等待或处理部分数据
            size_t available = m_ringBuffer->available();
            size_t chunkSize = std::min(available, static_cast<size_t>(2048));

            if (chunkSize > 0) {
                QByteArray frame = m_ringBuffer->pop(chunkSize);
//                qDebug()<<"frame.size() "<<frame.size(); // 输出当前pop的数据长度，调试发现，一般都小于2048
                processLVDSFrame(frame);  // 解析数据帧
            } else {
                QThread::msleep(1);  // 避免忙等待
            }
        }
    }
}

void DataAnalysisHelper::processRS422Frame(QByteArray frame)
{
    qDebug()<<"processRS422Frame";
    rs422Buffer.append(frame);
    while (rs422Buffer.size() >= FRAME_SIZE_CENTROID) {
        // 1. 查找帧头
        int startIndex = -1;
        for (int i = 0; i<=rs422Buffer.size() - FRAME_SIZE_CENTROID; ++i) {
            if(static_cast<quint8>(rs422Buffer[i]) == 0xB9 &&
               static_cast<quint8>(rs422Buffer[i+1]) == 0xC0){
                startIndex = i;
                break;
            }
        }

        if(startIndex == -1){
            int keepSize = qMin(FRAME_SIZE_CENTROID-1, rs422Buffer.size());
            rs422Buffer = rs422Buffer.right(keepSize);
            qDebug()<<"没有找到质心帧帧头";
            break;
        }

        qDebug()<<"startIndexCentroid:"<<startIndex;

        // 2. 丢弃质心帧头前的无效数据
        if(startIndex > 0){
            rs422Buffer.remove(0,startIndex);
            qDebug()<<"丢弃的质心帧字节数："<<startIndex<< "bytes";
        }

        // 3. 数据是否足够构成一帧
        if(rs422Buffer.size()<FRAME_SIZE_CENTROID){
            qDebug()<<"数据不足组成一帧19字节的质心帧,等待后续数据";
            break;
        }

        // 4. 提取帧存进缓存
        QByteArray frameData = rs422Buffer.left(FRAME_SIZE_CENTROID);
        centroidFrameAttrs.totalFrameNum += 1;
        centroidFrameAttrs.validFrameNum += 1;

        // 5. 检查校验和
        // 提取当前帧校验和
        QByteArray currentCheckSum  = frameData.mid(18,1);
        // 提取纯数据计算校验和
        QByteArray frameDataCheck = frameData.mid(2,16);
        QByteArray checkSum = frameHelper->frameChecksum(frameDataCheck,1);
        qDebug()<<"质心帧计算出的校验和"<<checkSum.toHex(' ').toUpper();
        qDebug()<<"质心帧接收到的校验和"<<currentCheckSum.toHex(' ').toUpper();
       if(checkSum != currentCheckSum ){
           // 当前帧校验和不对
           qDebug()<<"当前帧校验和不对";
           centroidFrameAttrs.errorFrameNum += 1;
           centroidFrameAttrs.validFrameNum -= 1;
       }

       // 6. 帧没有问题，开始提取信息
       quint16 centroidX = static_cast<quint16>(
                    (static_cast<quint8>(frameData[2]) << 8)  |
                    (static_cast<quint8>(frameData[3]))
               );

       quint16 centroidY = static_cast<quint16>(
                    (static_cast<quint8>(frameData[4]) << 8)  |
                    (static_cast<quint8>(frameData[5]))
               );

       quint16 maxGrayscale = static_cast<quint16>(
                    (static_cast<quint8>(frameData[6]) << 8)  |
                    (static_cast<quint8>(frameData[7]))
               );

       quint16 minGrayscale = static_cast<quint16>(
                    (static_cast<quint8>(frameData[8]) << 8)  |
                    (static_cast<quint8>(frameData[9]))
               );

       quint16 overThreshold = static_cast<quint16>(
                    (static_cast<quint8>(frameData[10]) << 8)  |
                    (static_cast<quint8>(frameData[11]))
               );

       quint16 exposureTime = static_cast<quint16>(
                    (static_cast<quint8>(frameData[12]) << 8)  |
                    (static_cast<quint8>(frameData[13]))
               );
       quint32 grayscaleSum = static_cast<quint32>(
                    (static_cast<quint8>(frameData[14]) << 24)  |
                    (static_cast<quint8>(frameData[15]) << 16)  |
                    (static_cast<quint8>(frameData[16]) << 8)   |
                    (static_cast<quint8>(frameData[17]))
               );

    centroidFrameAttrs.centroidX = centroidX / 32;
    centroidFrameAttrs.centroidY = centroidY / 32;
    centroidFrameAttrs.maxGrayscale = maxGrayscale;
    centroidFrameAttrs.minGrayscale = minGrayscale;
    centroidFrameAttrs.overThreshold = overThreshold;
    centroidFrameAttrs.exposureTime = exposureTime;
    centroidFrameAttrs.grayscaleSum = grayscaleSum;

    emit upgradeCentroidUi(centroidFrameAttrs);

    rs422Buffer = rs422Buffer.mid(FRAME_SIZE_CENTROID); // 移除已处理部分
    if (rs422Buffer.size() < FRAME_SIZE_CENTROID) {
           break;  // 剩余数据不足，等待下次输入
       }
    }
}


void DataAnalysisHelper::processLVDSFrame(QByteArray frame)
{
//    tempNum += frame.size();
//    qDebug()<<"tempNum"<<tempNum;

    lvdsBuffer.append(frame);

    /**
    当我没有使用环形缓存区和双子线程的生产者和消费者结构时，问题就出在下面，会丢失数据
    因为串口在不断地抓取数据，但是这里的处理过程较慢，过多的remove()和mid()函数
    相当于频繁的数据拷贝，计算复杂度是O(n)，会占据线程时间，这样数据来不及处理就容易丢失
    解决方案就是使用 环形缓冲区+双线程 结构设计，环形缓冲区避免了频繁的数据拷贝，双线程设计让
    数据抓取和数据处理分离开，环形缓冲区充当了两者之间的媒介，以避免线程阻塞的问题。
    */

    while (lvdsBuffer.size() >= FRAME_SIZE) {
        // 1. 查找帧头（0xE2 0x25）
        int startIndex = -1;
        for (int i = 0; i <= lvdsBuffer.size() - FRAME_SIZE; ++i) {
            if (static_cast<quint8>(lvdsBuffer[i]) == 0xE2 &&
                static_cast<quint8>(lvdsBuffer[i + 1]) == 0x25) {
                startIndex = i;
//                qDebug()<<"startIndex"<<startIndex;
                break;
            }
        }
        if (startIndex == -1) {
            // 保留最后FRAME_SIZE-1字节（可能包含半个帧头）
            int keepSize = qMin(FRAME_SIZE - 1, lvdsBuffer.size());
            lvdsBuffer = lvdsBuffer.right(keepSize);
            qDebug()<<"没找到帧头，丢弃部分数据";
            break;
        }

        qDebug()<<"startIndex"<<startIndex;

        // 2. 丢弃帧头前的数据
        if (startIndex > 0) {
            qDebug() << "丢弃无效头部数据:" << startIndex << "bytes";
                        lvdsBuffer.remove(0, startIndex);
        }

        // 3. 数据是否足够构成一帧
        if (lvdsBuffer.size() < FRAME_SIZE) {
            qDebug()<<"数据还不够2048字节，等待后续数据";
            // 数据还不够2048字节，等待后续数据
            break;
        }

        // 4. 检查帧完整性
        if (lvdsBuffer.size() < FRAME_SIZE) {
            break; // 等待更多数据
         }

        // 4. 校验帧尾（最后2字节 == FB FB）
        // 5. 验证帧尾 (FB FB)
        QByteArray frame = lvdsBuffer.left(FRAME_SIZE);
        bool validFrame = true;

        if (static_cast<quint8>(lvdsBuffer[2046]) != 0xFB ||
            static_cast<quint8>(lvdsBuffer[2047]) != 0xFB) {

            qDebug() << "无效帧尾, 丢弃整帧";
            lvdsBuffer.remove(0, FRAME_SIZE); // 丢弃整帧
            validFrame = false;
            continue; // 继续处理下一帧
        }

        // 接收到新帧
        if(imageNo != lvdsBuffer.mid(13,2)){
            qDebug()<<"检测到新图像帧数据";
            lvdsImgDataBuffer.clear();
            headFrameExist = false;

        }

        // 5. 校验帧编号（第16字节 == 0x00）检验首帧
        if(!headFrameExist){
            // 防御性检查（理论上不会触发）
            if (frame.size() < 16) {
                lvdsBuffer.remove(0, 2); // 仅丢弃帧头
                continue;
            }

            if (static_cast<quint8>(lvdsBuffer[15]) != 0x00) {
                qDebug()<<"不是首帧";
                // 非起始帧，丢掉帧头位置数据，从下一个位置继续找
//                lvdsBuffer = lvdsBuffer.mid(1);
                qDebug()<<"丢弃前的帧编号："<<static_cast<quint8>(lvdsBuffer[15]);
                lvdsBuffer.remove(0,FRAME_SIZE);
                qDebug()<<"丢弃后的帧编号："<<static_cast<quint8>(lvdsBuffer[15]);

//                break;
                continue; // 继续处理下一帧
            }
            else {
                // 找到了首帧
                qDebug()<<"找到了首帧";
                imgAttrs.totalImgNum  += 1;
                imgAttrs.validImgNum += 1; // 先默认这个图像帧是正确的，后续不正确的话再减掉
                headFrameExist = true; // 当前找到首帧了,也代表当前已经处于处理一帧图像的工作中
                imageNo = lvdsBuffer.mid(13,2); // 获取当前处理图像的编号:此参数只有在每次检测到00时会更新
                QByteArray timeCode = lvdsBuffer.mid(8,4); // 提取时间码
//                quint16 timeCodeS = (static_cast<quint8>(timeCode[0])<<8 | static_cast<quint8>(timeCode[1]));
                quint8 highS = static_cast<quint8>(timeCode[0]);
                quint8 lowS  = static_cast<quint8>(timeCode[1]);
                quint16 timeCodeS = static_cast<quint16>((highS << 8) | lowS);
                qDebug()<<"timeCodeS"<<timeCodeS; // 34731
                quint8 highMS = static_cast<quint8>(timeCode[2]);
                quint8 lowMS  = static_cast<quint8>(timeCode[3]);
                quint16 timeCodeMS = static_cast<quint16>((highMS << 8) | lowMS);
                double timeCodeMSDouble = timeCodeMS * 0.2;
                qDebug()<<"timeCodeMSDouble"<<timeCodeMSDouble; //
                QString timeCodeString = QString::number(timeCodeS) + "秒" + QString::number(timeCodeMSDouble) + "毫秒";
                qDebug()<<"timeCodeString"<<timeCodeString;
                imgAttrs.timeCode = timeCodeString;




//                QByteArray imageNo = lvdsBuffer.mid(13, 2);
//                quint16 frameNumber = (static_cast<quint8>(imageNo[0]) << 8) | static_cast<quint8>(imageNo[1]);
//                imgAttrs.imgFrameNo = frameNumber;
                imgAttrs.imgFrameNoString = imageNo.toHex(' ').toUpper();  // 注意 imgFrameNoStr 应该是 QString 或 QByteArray

                // 获取开窗大小
                qDebug()<<"imageNo"<<imageNo;
                imgSize = static_cast<int>(static_cast<quint8>(lvdsBuffer[16])) * 4; // 确定开窗大小
                imgAttrs.imgWinSize = imgSize;
                qDebug()<<"imgSize"<<imgSize;
                imgIsError = false; // 一张图像帧到来时先将错误帧标记重置为false，后续发现该图像帧有问题就置为true
            }
        }

        // 6. 提取帧存进缓存
        QByteArray frameData = lvdsBuffer.left(FRAME_SIZE);
        int frameNum = (imgSize * imgSize % frameSizePure == 0)? (imgSize * imgSize / frameSizePure ): (imgSize * imgSize / frameSizePure + 1);// 确定所需帧包数


        // 7. 检查校验和（从第18帧到第2044帧共2027字节）
        // 提取当前帧校验和
        QByteArray currentCheckSum = frameData.mid(2044,2);
        // 提取纯数据计算校验和
        QByteArray frameDataCheck = frameData.mid(2,2042);
        QByteArray checkSum =  frameHelper->frameChecksum(frameDataCheck,2);
         qDebug()<<"计算出的校验和"<<checkSum.toHex(' ').toUpper();
         qDebug()<<"接收到的校验和"<<currentCheckSum.toHex(' ').toUpper();
        if(checkSum != currentCheckSum ){
            // 当前帧校验和不对
            qDebug()<<"当前帧校验和不对";
            if(!imgIsError){
                imgIsError = true; // 证明当前图像有误
                imgAttrs.errorImgNum  += 1; // 错误图像帧计数+1
                imgAttrs.validImgNum  -= 1; // 正确图像帧计数-1，这里-1实际上是将收到首帧后无脑加的帧计数再去掉
            }
        }

////        QByteArray tempImageNo = frameData.mid(13,2);
////        if (imgIsError == true && tempImageNo == imageNo) {
////            lvdsImgDataBuffer.clear(); // 清空帧
////            break;
////        }


        //// 从这里开始，数据丢失
        // 如果帧没问题，存进图像数据缓存区
        QByteArray frameDataPure = frameData.mid(17,frameSizePure);
//        qDebug()<<"frameDataPure"<<frameDataPure.toHex(' ').toUpper();
        lvdsImgDataBuffer.append(frameDataPure);


        if (lvdsImgDataBuffer.size() >= imgSize * imgSize) {
            overThresholdCoords.clear();
//            qDebug() << "✅ 条件满足，准备拼图";
            QByteArray singleImgData = lvdsImgDataBuffer.mid(0, imgSize * imgSize);
//            qDebug() << "提取的图像数据大小:" << singleImgData.size();
            if (singleImgData.isEmpty()) {
                qDebug() << "❌ 警告：singleImgData 是空的！";
            }

            // 笔记：lvdsImgDataBuffer.at(i)本身获取的数据是只读的，是无法进行改值的
            // 统计超阈值点个数 阈值是 grayThrehold 直接使用
//                            qDebug()<<"isImgToPeaking"<<isImgToPeaking;
//                            qDebug()<<"peakingThrehold"<<peakingThrehold;
            for(int i = 0 ; i  < imgSize * imgSize; i++){
                quint8 pixel = static_cast<quint8>(singleImgData.at(i));
                if(pixel > grayThrehold){
                    imgAttrs.overThreshold += 1;
                    // 将超过阈值点的像素坐标保存起来
//                    qDebug()<<"isSaveCoordinate222"<<isSaveCoordinate;
                    if(isSaveCoordinate){
                        int x = i % imgSize;
                        int y = i / imgSize; // 在 C++ 里，/ 对于两个整数执行的是整除运算，不会进行四舍五入
//                        qDebug()<<"超阈值像素x坐标："<<x<<"超阈值像素y坐标："<<y;
                        overThresholdCoords.append(QPoint(x,y));
//                        qDebug()<<"isSaveCoordinate222";
                    }
                }
                if(isImgToPeaking && pixel < peakingThrehold){
                    singleImgData[i] = static_cast<char>(pixel * 0.1);
                }
            }


            imgAttrs.overThresholdCo = overThresholdCoords;
//            qDebug()<<"overThresholdCoords"<<overThresholdCoords.count();
            lvdsImgDataBuffer.clear();
            showGrayImage(singleImgData, imgSize);

            emit upgradeErrorImgNumUi(imgAttrs); // 更新上位机ui界面显示的图像计数
            imgAttrs.overThreshold = 0;
            qDebug()<<"imgAttrs.errorImgNum "<<imgAttrs.errorImgNum;
            /**
              跨线程传递自定义结构体 ImageAttributes 时必须正确注册类型，否则会导致 Qt 的元对象系统无法处理数据
              当信号通过 Qt::QueuedConnection（跨线程默认方式）传递自定义结构体时，必须使用 qRegisterMetaType注册该类型
              注册方式：在main文件中使用qRegisterMetaType<ImageAttributes>("ImageAttributes"); // 注册类型
            */
        }

        lvdsBuffer = lvdsBuffer.mid(FRAME_SIZE); // 移除已处理部分
        if (lvdsBuffer.size() < FRAME_SIZE) {
               break;  // 剩余数据不足，等待下次输入
        }
    }
}



void DataAnalysisHelper::onDataAnalysisCAN()
{
    qDebug()<<"onDataAnalysisCAN";
    while (m_running) {
        if (m_ringBuffer && !m_ringBuffer->isEmpty()) {
            qDebug()<<"m_ringBuffer";
            size_t available = m_ringBuffer->available(); // 获取当前缓存区数据长度
            qDebug()<<"m_ringBuffer size"<<available;

            // 至少要能看两个字节的帧头,注意之所以取4字节是因为当遥测帧大于8字节时（一般都大于8字节），帧头前面有两个字节用于表示帧长度
            if (available < 4) {
                QThread::msleep(1);
                continue;
            }

            // pop前lengthHead个字节来判断帧类型
            QByteArray header = m_ringBuffer->peek(lengthHead_fast); // 帧头占据2字节
            quint8 h0 = static_cast<quint8>(header[2]); // 0个1字节代表帧长度（不包含最后三字节的校验）
            quint8 h1 = static_cast<quint8>(header[3]);


            int frameLen = 0;
            enum FrameType { Unknown, Centroid, Ack, Telemetry, Telemetry_slow, Telemetry_all} frameType = Unknown;

            qDebug()<<"h0"<<h0;
            qDebug()<<"h1"<<h1;
            qDebug()<<"h0_xml_fast***"<<h0_xml_fast;
            qDebug()<<"h1_xml_fast***"<<h1_xml_fast;
            qDebug()<<"h0_xml_slow***"<<h0_xml_slow;
            qDebug()<<"h1_xml_slow***"<<h1_xml_slow;
            qDebug()<<"h0_xml_all***"<<h0_xml_all;
            qDebug()<<"h1_xml_all***"<<h1_xml_all;

            if (h0 == h0_xml_fast && h1 == h1_xml_fast) {
                frameLen =  frameTm.frameLength;
                frameType = Telemetry;
                qDebug()<<"检测到快遥帧";
                qDebug()<<"快遥测帧长度"<<frameLen;

            } else if (h0 == h0_xml_slow && h1 == h1_xml_slow) {
                frameLen =  frameTm_slow.frameLength;
                frameType = Telemetry_slow;
                qDebug()<<"检测到慢遥帧";
                qDebug()<<"慢遥测帧长度"<<frameLen;
            }
            else if (h0 == h0_xml_all && h1 == h1_xml_all) {//通过xml获取还没有写
//                frameLen =  frameTm.frameLength + frameTm_slow.frameLength -5; // 原本各自有13字节的往外包数据，共23字节，但是快慢帧中只有21字节额外包数据
                frameLen =  frameTm.frameLengthAll; // 由XML提供总帧长
                frameType = Telemetry_all;
                qDebug()<<"检测到全部遥测帧";
                qDebug()<<"全部遥测帧长度"<<frameLen;
            }
            else {
                // 未知帧头：丢掉一个字节再继续
                qDebug()<<"未知帧头";
                m_ringBuffer->pop(1);
                continue;
            }

            // 如果缓冲区数据不足整帧，就等下一轮
            if (available < static_cast<size_t>(frameLen)) {
                qDebug()<<"等待下一轮" ;
                QThread::msleep(1);
                continue;
            }

            // 取出整帧数据
            QByteArray frame = m_ringBuffer->pop(frameLen);
            qDebug()<<"frame"<<frame;
            qDebug()<<"frame长度"<<frame.size();

            // 根据类型解析
            switch (frameType) {
                case Telemetry_all:
                    qDebug()<<"全部遥测帧解析...";
                    processCANFrame_all(frame);       // 全部帧解析
                    break;
                case Telemetry_slow:
                    qDebug()<<"慢遥帧解析...";
                    processCANFrame_slow(frame);         // 慢遥帧解析
                    break;
                case Telemetry:
                    qDebug()<<"快遥帧解析...";
                    processCANFrame(frame);   // 快遥帧解析
                    break;
                default:
                    break;
            }
        } else {
            QThread::msleep(1);  // 避免忙等待
        }


        if (m_ringBuffer_slow && !m_ringBuffer_slow->isEmpty()) {
            qDebug()<<"m_ringBuffer_slow";
            size_t available = m_ringBuffer_slow->available(); // 获取当前缓存区数据长度
//            qDebug()<<"m_ringBuffer size"<<available;

            // 至少要能看两个字节的帧头,注意之所以取4字节是因为当遥测帧大于8字节时（一般都大于8字节），帧头前面有两个字节用于表示帧长度
            if (available < 4) {
                QThread::msleep(1);
                continue;
            }

            // pop前lengthHead个字节来判断帧类型
            QByteArray header = m_ringBuffer_slow->peek(lengthHead_slow); // 帧头占据2字节
            quint8 h0 = static_cast<quint8>(header[2]); // 0个1字节代表帧长度（不包含最后三字节的校验）
            quint8 h1 = static_cast<quint8>(header[3]);


            int frameLen = 0;
            enum FrameType { Unknown, Centroid, Ack, Telemetry, Telemetry_slow, Telemetry_all} frameType = Unknown;

            qDebug()<<"h0"<<h0;
            qDebug()<<"h1"<<h1;
            qDebug()<<"h0_xml_fast***"<<h0_xml_fast;
            qDebug()<<"h1_xml_fast***"<<h1_xml_fast;
            qDebug()<<"h0_xml_slow***"<<h0_xml_slow;
            qDebug()<<"h1_xml_slow***"<<h1_xml_slow;

            if (h0 == h0_xml_fast && h1 == h1_xml_fast) {
                frameLen =  frameTm.frameLength;
                frameType = Telemetry;
                qDebug()<<"快遥帧";
                qDebug()<<"快遥测帧长度"<<frameLen;

            } else if (h0 == h0_xml_slow && h1 == h1_xml_slow) {
                frameLen =  frameTm_slow.frameLength;
                frameType = Telemetry_slow;
                qDebug()<<"慢遥帧";
                qDebug()<<"慢遥测帧长度"<<frameLen;
            }
            else {
                // 未知帧头：丢掉一个字节再继续
                qDebug()<<"未知帧头";
                m_ringBuffer_slow->pop(1);
                continue;
            }

            // 如果缓冲区数据不足整帧，就等下一轮
            if (available < static_cast<size_t>(frameLen)) {
                qDebug()<<"等待下一轮" ;
                QThread::msleep(1);
                continue;
            }

            // 取出整帧数据
            QByteArray frame = m_ringBuffer_slow->pop(frameLen);
            qDebug()<<"frame"<<frame;

            // 根据类型解析
            switch (frameType) {
                case Telemetry_all:
                    qDebug()<<"全部遥测帧解析";
                    processCANFrame_all(frame);       // 全部帧解析
                    break;
                case Telemetry_slow:
                    qDebug()<<"慢遥帧解析";
                    processCANFrame_slow(frame);         // 慢遥帧解析
                    break;
                case Telemetry:
                    qDebug()<<"快遥帧解析";
                    processCANFrame(frame);   // 快遥帧解析
                    break;
                default:
                    break;
            }
        } else {
            QThread::msleep(1);  // 避免忙等待
        }
    }
}



//    // 下面代码有潜在的不稳定性，既没有寻找帧头、也没有检查校验和，仅仅是把希望寄托在127字节上面
//    while (m_running)
//    {
//        if (m_ringBuffer && m_ringBuffer->available() >= 127)
//        {
//            // 从环形缓存拿出一帧 CAN 数据
//            QByteArray rawFrame = m_ringBuffer->pop(127);
////            qDebug() << "rawFrame:" << rawFrame.toHex(' ');
//            processCANFrame(rawFrame);
//        }
//        else
//        {
//            QThread::msleep(1);
//        }

//        if (m_ringBuffer_slow && m_ringBuffer_slow->available() >= 227)
//        {
//            // 从环形缓存拿出一帧 CAN 数据
//            QByteArray rawFrame = m_ringBuffer_slow->pop(227);
////            qDebug() << "rawFrame:" << rawFrame.toHex(' ');
//            processCANFrame_slow(rawFrame);
//        }
//        else
//        {
//            QThread::msleep(1);
//        }
//    }


/// 快遥帧处理
void DataAnalysisHelper::processCANFrame(QByteArray frame)
{
    QString dataReceivedString = dataTransfer->Bytearray2string(frame);
    emit dataToUiDirect(dataReceivedString); // 主页面显示串口接收
    processTelemetryFrameCAN(frame);
}
/// 慢遥帧处理
void DataAnalysisHelper::processCANFrame_slow(QByteArray frame)
{
    QString dataReceivedString = dataTransfer->Bytearray2string(frame);
    emit dataToUiDirect(dataReceivedString); // 主页面显示串口接收
    processTelemetryFrameCAN_slow(frame);
}
/// 快慢遥帧处理
void DataAnalysisHelper::processCANFrame_all(QByteArray frame)
{
    QString dataReceivedString = dataTransfer->Bytearray2string(frame);
    emit dataToUiDirect(dataReceivedString); // 主页面显示串口接收
    // 根据帧长划分出快遥帧和慢遥帧两个帧分别送入相应的处理函数
    // 获取两个帧的前后长度，前帧是CutFront，来自xml
    int frame1Len = frameTm.CutFront.toInt();
    QByteArray frame1 = frame.left(frame1Len);
    QByteArray frame2 = frame.mid(frame1Len);
    processTelemetryFrameCAN(frame1);
    // 由于frame1前10字节要剔除，符合规范，但是frame2只有6个字节需要去除，所以给它补4个字节
    frame2.prepend(QByteArray(4, 0xFB));
    processTelemetryFrameCAN_slow(frame2);
}

void DataAnalysisHelper::updataUiSettingsRec(const SettingsUiToSub &settingsUi)
{
    grayThrehold = settingsUi.grayThrehold;
    qDebug()<<"grayThrehold: "<<grayThrehold;
}

void DataAnalysisHelper::showGrayImage(const QByteArray &singleImgData, int imgSize)
{
    qDebug() << "showGrayImage singleImgData size:" << singleImgData.size();
    // 1. 创建 QImage，格式为 8bit 灰度图
    QImage image(reinterpret_cast<const uchar*>(singleImgData.constData()),
                 imgSize,
                 imgSize,
                 QImage::Format_Grayscale8);

    // 2. 拷贝一份（防止单独缓存区释放影响图像）
    QImage imageCopy = image.copy();

    // 3. 显示图像
    emit imageSignal(imageCopy);

}

void DataAnalysisHelper::setRingBuffer(RingBuffer *buffer)
{
    QMutexLocker locker(&m_mutex);  // 加锁保证线程安全
     if (m_ringBuffer != buffer) {
         m_ringBuffer = buffer;  // 更新缓冲区指针
         qDebug() << "RingBuffer updated in thread:" << QThread::currentThread();
     }
}

void DataAnalysisHelper::setRingBuffer_slow(RingBuffer *buffer)
{
    QMutexLocker locker(&m_mutex);  // 加锁保证线程安全
     if (m_ringBuffer_slow != buffer) {
         m_ringBuffer_slow = buffer;  // 更新缓冲区指针
         qDebug() << "RingBuffer_slow updated in thread:" << QThread::currentThread();
     }
}

void DataAnalysisHelper::imgToPeaking(bool isImgToPeakingUi, int peakingThresholdUi)
{

    isImgToPeaking = isImgToPeakingUi;
    peakingThrehold = peakingThresholdUi;
//    qDebug()<<"isImgToPeaking2"<<isImgToPeaking;
    //    qDebug()<<"peakingThrehold2"<<peakingThrehold;
}

void DataAnalysisHelper::updateSaveCoordinate(bool isSaveCoordinateUi)
{
    isSaveCoordinate = isSaveCoordinateUi;
    qDebug()<<"updateSaveCoordinate isSaveCoordinate "<<isSaveCoordinate;
}

// 根据xml中的TMitemEncodeTypeT中的公式解析，下面的函数需要包含TMitemEncodeTypeT中可能涉及到的所有运算规则，比如 +-*/({[log()
QString DataAnalysisHelper::applyRule(const QString &rule, double value) {
    QJSEngine engine;
    // 给表达式里用到的变量 x 赋值
    engine.globalObject().setProperty("x", value);

    // 计算表达式
    QJSValue result = engine.evaluate(rule);

    if (result.isError()) {
        return "EvalError: " + result.toString();
    }

    // 转成字符串返回，这里保留 6 位小数（可调整）
    return QString::number(result.toNumber(), 'f', 6);
}
