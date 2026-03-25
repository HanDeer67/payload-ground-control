#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include "LVDS_DLL.h"
//#include "lvdscardhelper.h"
#include "lvdscardworker.h"
#include "serialportworker.h"
#include <QListWidget>
#include <QStackedWidget>
#include "xmlcommandparser.h"
#include "ringbuffer.h"
#include "framehelper.h"
#include "listwidgethelper.h"
#include <QList>
#include "Structs.h"
#include <QTimer>
#include <QCoreApplication>
#include <QPlainTextEdit>
#include "imagegraphicsview.h"
#include "dialogpara.h"
#include <QStandardItemModel>
#include "canworker.h"
#include "canframer.h"
#include <QButtonGroup>
#include "dialogcanframe.h"
#include "dialogcanframe2.h"
#include "canhelper.h"
#include "ControlCAN.h"
#include "broadcastworker.h"
#include "frmpagesdatabroadcast.h"


// 前向声明
class DataTransfer;  // 添加这行

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    SettingsUiToSub  settingsUiAttrs;

    void addTabItem(QListWidget* listWidget, const QIcon& icon, const QString& text);
    void addStackedPage(QStackedWidget* stackedWidget, const QString& pageName);
    void onTabItemClicked(QListWidgetItem* item);
    void onTabItemClicked_2(QListWidgetItem* item);
    void updateTabSelection(int selectedIndex);
    void updateTabSelection_2(int selectedIndex);


    void refreshSerialPorts();
    void serialOpenClose(bool serialChoose);
    void updatePortState(bool serialChoose, bool isOpen);
    void updatePortPara();
    void onSerialDataReceived();
    void dataReceivedRS422(QString hexString);
    void dataReceivedLVDS(QString hexString);
    void dataReceivedCAN(QString hexString);
    void cardOpenBtn(bool chooseCard);
    void qmessageBoxSet(QString errorString);
    std::tuple<quint8, quint8> getUiCardSet();

    //发送立即令
    void addCMDList(bool insertOrAdd); // 当insertOrAdd是true时插入指令到当前选中行的下一行，当insertOrAdd是false时添加指令到当前指令序列的最后一行
    void onListWidgetContextMenuRequested(const QPoint &pos);
    void sendCMD(int sendIndex, bool directOrList);
    void delCMDList();
    void clearCMDList();

    // 定义板卡接收还是发送
    bool isRecv = true; // 默认是接收模式

    bool isSaveDataSerial1 = false; // 是否需要保存Serial1数据
    bool isSaveDataSerial2 = false; // 是否需要保存Serial2数据
    QString fileNameSerial1;
    QString fileNameSerial2;

    QString saveDirSerial1Txt ;
    QString saveDirSerial2Txt ;
    QString saveDirSerial1Bin ;
    QString saveDirSerial2Bin ;

    bool isSaveDataRs422 = false; // 是否需要保存422数据
    bool isSaveDataLVDS = false; // 是否需要保存LVDS数据
    bool isSaveDataCAN = false;// 是否需要保存CAN数据
    QString fileNameRS422;
    QString fileNameLVDS;
    QString fileNameCAN;

    QString saveDirRS422Txt ;
    QString saveDirLVDSTxt ;
    QString saveDirCANTxt;
    QString saveDirRS422Bin ;
    QString saveDirLVDSBin ;
    QString saveDirCANBin;

    QString saveDirLVDSImgSingle ; // 单次保存LVDS图像的文件夹
    QString saveDirLVDSImgSpan ; // 连续保存LVDS图像的文件夹

    QString saveDirTM;
    QString saveDirTMAuto;
    QString saveFileNameTm; // 通过标志位判断是否保存，如果是的话，读取这个命名的文件开始写入
    QString saveFileNameTmAuto;
    bool isSaveTMFile = false;
    bool isSaveTMFile_slow = false;
    QTimer *m_saveTimer; // 保存遥测数据的指针

    bool isSaveImgLVDS = false; // 是否需要保存图像
    void saveImgSingle();
    void saveImgSpan();
    QString LVDSImgSpanName;

    int m_SendRow  = -1; // 当前要发送的指令行索引
    QVector<int> m_timeDelays; // 存放时间戳


    void showImage(QImage image);
    static QSize lastSize; // 记录上一次图像尺寸


    void readXML(QString xmlFiles, bool setCurrentToNew); // 加载XML文件，用于确定程序初始化时和后续载入XML文件时默认选中的指令组
    void readXML2(QString xmlFiles); // 加载用户配置好的指令序列XML文件
    void writeXML(QString xmlName); // 输出XML文件，根据指令行配置XML文件并输出，方便下次快速导入
    QList<Command> commandUi;
    // 创建一个用于缓存xml文件信息（doc）的容器，当对指令序列进行编辑修改时，这个容器中对应的doc也会被同步修改，当用户保存xml文件时直接从这个容器中取出并执行输出xml文件操作
    QList<QDomElement> docNodeInitList; // 软件启动时默认加载的指令xml文件指令列表（左侧），以及点击按钮“添加指令组”时用于保存Command节点的列表
    QList<QDomElement> docNodeList; // 用户点击按钮“导入XML”时用于保存Command节点的列表

//    QList<QString> *commandList = {};
    // commandList是指针类型，但用{}初始化后，它的值是**nullptr**（空指针）。
    // 后续调用commandList->append(text)和commandList->at(index)时，相当于对空指针解引用，这是C++中致命的错误，直接导致程序崩溃。
    QList<QString> commandList = {}; // 如果commandList的生命周期与当前类（如MainWindow）一致，直接定义为栈对象（非指针），自动管理内存
    // 而指针成员变量是否需要在析构函数中手动 delete，取决于这些指针指向的对象由谁负责分配和释放
    /**
     重要：如果以堆形式（指针）创建一个类对象，这个类对象在同时不满足以下条件时需要手动在析构函数中进行delete：
     1. 当前类并不是QObject的派生类，也就是没有继承自QObject
     2. 在初始化类对象时（new）没有指定父对象（this）
     当两个都不满足时，需要按照下面的方式进行析构：
     if (frameHelper) delete frameHelper;
     frameHelper = nullptr;

     此外，凡是以堆形式创建对象的，都需要在类的构造函数（cpp中）中初始化该对象，以new的形式，核心是用new分配内存。
     下面是一组完整的从指针声明、内存分配等到最后的内存释放的全过程
        // ①在类的成员变量中声明（.h文件）：
        QList<QString>* commandList; // 指针声明

        // ②在类的构造函数中初始化（.cpp文件）：
        commandList = new QList<QString>(); // 关键：用new分配内存

        // 连接信号槽（通过指针调用append）：
        connect(xmlcommandparser, &XMLCommandParser::updateUiCurComShowSignal, this, [=](QString text) {
            commandList->append(text); // 此时指针指向有效对象，可安全调用
        });

        // 使用时（先检查指针是否为空+索引是否有效）：
        if (commandList != nullptr && index >= 0 && index < commandList->size()) {
            ui->lb_current_cmd_name->setText(commandList->at(index));
        }

        // 在类的析构函数中释放内存（.cpp文件）：
        delete commandList; // 避免内存泄漏
        commandList = nullptr; // 防止野指针
    */

    // void LVDS_CountCards();

    //    void appendWithLimit(QPlainTextEdit *edit, const QString &text);
    //    void deleteXML(int xmlIndex);
    void saveDocNodeListToXML(const QString &filePath, QList<QDomElement> &docNodeList);
    void updateCommandAttribute(QDomElement &commandElem,
                                const QString &attrName,
                                const QString &newValue);

    void generatelLengthCode();

    ImageGraphicsView *view;
    QGraphicsPixmapItem *pixmapItem;
    QVector<QPoint> overThresholdCoords;

    void readTmXML(QString xmlFiles);
    void addRowToInterestTable(int row);
    void removeRowFromInterestTable(int row);

    QStandardItemModel *modelMain;
    QStandardItemModel *modelInterest;

    QStandardItemModel *modelMain_slow;
    QStandardItemModel *modelInterest_slow;

    void init();
    QVector<int> rowSort;
    QVector<int> rowSort_slow;
    int paraFrameCount = 0;
    int validFrameCount = 0;
    int invalidFrameCount = 0;

    void on_stopSaveButton_clicked();
    void addRowToInterestTable_slow(int row);
    void removeRowFromInterestTable_slow(int row);
    void readTmXML_slow(QString xmlFiles);

    void repeatSend();
    void repeatTransmit();
    QTimer *broadcastTimer = nullptr;
    bool broadcastSend = false;
    QTimer *tmRequestTimer = nullptr;
    QString canParaQStringFast = "001111";
    QString canParaQStringSlow = "001212";
    QString canParaQStringAll = "001313";
    bool fastSend = false;
    bool slowSend = false;
    bool allSend = false;

    void sendDataDirect(); // 调试助手页面的CAN页面直接发送数据时调用的函数
    void sendDataDirectInput(QString textInput, int  identifier);


    void sendDataDirectNew(QByteArray byteDataSend);
private:
    Ui::MainWindow *ui;



    FrameHelper *frameHelper;
//    ListWidgetHelper *listWidgetHelper;

    XMLCommandParser *xmlcommandparser;// xml指令剖析器
//    QString xmlPath = "XmlFiles/CMD/cmd_Camera.xml"; // 软件初始化时加载的默认xml文件——红外相机
//    QString xmlPathVisibleLight = "XmlFiles/CMD/cmd_Camera_Visible_Light.xml"; // 软件初始化时加载的默认xml文件——可见光相机
    QString basePath = QCoreApplication::applicationDirPath(); // exe 所在目录
//    QString xmlPath = basePath + "/XmlFiles/CMD/cmd_Camera.xml";
//    QString xmlPathVisibleLight = basePath + "/XmlFiles/CMD/cmd_Camera_Visible_Light.xml";
//    QString xmlTmPath = basePath + "/XmlFiles/TM/tm.xml";
//    QString xmlTmPath = basePath + "/XmlFiles/TM/p1_tm_can_fast_all_03.04.xml";
//    QString xmlTmPath_2 = basePath + "/XmlFiles/TM/p1_tm_can_slow_all_03.19.xml";
    QString configPath = basePath + "/ConfigFiles/";
    QString xmlTmPath = basePath + "/XmlFiles/TM/";
    QString xmlTmPath_2 = basePath + "/XmlFiles/TM/";
    QString CMDXmlFilesPath = basePath + "/XmlFiles/CMD/";
//    QString TMXmlFilesPath = basePath + "/XmlFiles/TM/";

    DataTransfer *dataTransfer;  // 添加 DataTransfer 对象
    SerialPortWorker *serialPortWorker1; // 串口管理对象1
    SerialPortWorker *serialPortWorker2; // 串口管理对象2
    QThread *t3;
    QThread *t4;

    // 创建帧检查对象
    DataAnalysisHelper *dataAnalysisHelper1;
    DataAnalysisHelper *dataAnalysisHelper2;
    DataAnalysisHelper *dataAnalysisHelperRS422;
    DataAnalysisHelper *dataAnalysisHelperLVDS;
    DataAnalysisHelper *dataAnalysisHelperCAN;

    QThread *t_dataAnalysis1; // 创建串口1数据处理线程
    QThread *t_dataAnalysis2; // 创建串口1数据处理线程
    QThread *t_dataAnalysisRS422; // 创建RS422数据处理线程
    QThread *t_dataAnalysisLVDS; // 创建LVDS数据处理线程
    QThread *t_dataAnalysisCAN; // 创建CAN数据处理线程

    // 创建环形缓冲区对象
    RingBuffer *ringBuffer1;
    RingBuffer *ringBuffer2;
    RingBuffer *ringBufferRS422;
    RingBuffer *ringBufferLVDS;
    RingBuffer *ringBufferCAN;
    RingBuffer *ringBufferCAN_slow;

    int MAX_LOG_LINES = 50;


//    QSerialPort serialPort;  // 串口对象

    lvdsCardWorker *cardWorker1; //板卡管理对象
    lvdsCardWorker *cardWorker2; //板卡管理对象
    QThread *t1;
    QThread *t2;


    void statusBarShow(bool chooseCard,QString showString);
    void statusBarShowSerial(bool chooseSerial,QString showString);
    void updatePlainText(bool serialChoose, QString textToShowDirect);
    void updateCheckboxesForTab(const QString &tabName);

    QStringList pendingData_LVDS;  // 数据缓冲区
    QStringList pendingData_RS422;  // 数据缓冲区
    QTimer updateTimer_LVDS;       // 定时器
    QTimer updateTimer_RS422;   // 定时器
    QStringList pendingData_Serial1;  // 数据缓冲区
    QStringList pendingData_Serial2;  // 数据缓冲区
    QTimer updateTimer_Serial1;       // 定时器
    QTimer updateTimer_Serial2;       // 定时器

    QStringList pendingData_CAN;  // 数据缓冲区
    QTimer updateTimer_CAN;       // 定时器


    DialogPara *dialogPara;
    DialogCanFrame *dialogCanFrame;
    DialogCanFrame2 *dialogCanFrame2;
    canFrameConfig canFrameConfigUi; // 通信接口调试页面帧配置
    canFrameConfig canFrameConfigUi2; // 指令配置及发送页面帧配置
    canFrameConfig canFrameConfig_tmRequest; // 遥测请求指令帧配置

    canFrameConfig canFrameConfig_timeMulticast; // 时间广播
    canFrameConfig canFrameConfig_attiBroadcast; // 姿态组播
    canFrameConfig canFrameConfig_busiMulticast; // 业务组播



    // can卡管理对象
    canWorker *canWorker1;
    QThread *t5; // can卡管理线程
    DWORD  nDeviceType = 4; /* USBCAN-2A或USBCAN-2C或CANalyst-II */
    DWORD  nDeviceInd = 0; /* 第1个设备 */
    DWORD  nCANInd = 1;          /* 第1个通道 */
    VCI_INIT_CONFIG vic;

    CANFramer *canFrameHelper;

    void transmitDirectCan(QString canDataToTransmit);

    bool sendData1(UINT channel, UINT ID, BYTE remoteFlag, BYTE externFlag, const unsigned char *data, BYTE len);

    BroadcastWorker *broadcastWorker;
    QThread *t6;


signals:

    /// 串口信号
    void initSerialSignal(QString portName,int baudRate, int dataBit, int parity, int stopBit); // 初始化串口1
    void openCloseSignal(); // 打开关闭串口1
    void writeDataSignal(QByteArray byteDataSend); // 写数据到串口1
    void initSerialSignal_2(QString portName,int baudRate, int dataBit, int parity, int stopBit); // 初始化串口2
    void openCloseSignal_2(); // 打开关闭串口2
    void writeDataSignal_2(QByteArray byteDataSend); // 写数据到串口2

    void closeSerialSignal();
    void closeSerialSignal_2();
    void imgToPeakingSignal(bool isImgToPeaking, int peakingThreshold);

    void updateConfigCANSignal(DWORD nDeviceTypeIn, DWORD nDeviceIndIn, DWORD nCANIndIn);




    /// 板卡信号
    void cardNoConfigSignal(bool no0IsRs422);

    void hexStringDataSiganl(QString hexStringData);
    // void cardOpenBtnSignal(ST_LVDS_CFG stCfg); // 点击板卡打开按钮时，对板卡的通道进行配置的信号
    // 将当前ui界面选择的板卡号和通道号信息传输给lvdscardhelper
    void cardSetSignal(quint8 cardNo, quint8 channelNo, quint8 channelNoRec);
    void lvdsStartStopSignal(bool isStart);

    void setChannelNumSignal(bool chooseCard, quint8 channelNum);
    void updateIsT1StartSignal(bool chooseCard, bool isT1Start);

    void startLVDSSignal(bool chooseCard); // 使能
    void stopLVDSSignal(bool chooseCard); // 去使能
    void resetCardSignal(bool chooseCard); //复位
    void clearFIFOSignal(bool chooseCard); // 清空缓存
    void openCardSignal(bool chooseCard); // 打开板卡
    void closeCardSignal(bool chooseCard); // 关闭板卡
    void setChannelFrame(bool chooseCard); // 通道帧配置
    void setChannelPara(bool chooseCard, bool isRecv,quint8 channel,ST_LVDS_CFG stCfg); // 通道参数配置

    void updateWorkerCardNum1(bool rss422No, bool lvdsNo);
    void updateWorkerCardNum2(bool rss422No, bool lvdsNo);

    void updateSaveCoordinateSignal(bool checked);
    void canTmIsOpeUpdateSignal(bool canTmOpenClose);
    void canTmIsOpeUpdateSignal_slow(bool canTmOpenClose);

    void openCloseBroadcastSignal(bool openOrClose, double delayTimeUi);
    void updateBroadcastDataSignal(QByteArray data);



private slots:
    void handleRS422Data(quint8 channel, QString dataString);
    void handleLVDSData(quint8 channel, QString dataString);

    void runError(QString errorString);
    void updateStatusBar(bool chooseCard,QString errorText);
    /*
    void startCommandSequence();
    void sendCommandWithDelay();
    */

};
#endif // MAINWINDOW_H
