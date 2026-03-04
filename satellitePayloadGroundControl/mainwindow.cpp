#include "mainwindow.h"
#include "datatransfer.h"
#include "ui_mainwindow.h"
#include "LVDS_DLL.h"
#include <QDebug>
#include <QtSerialPort>
#include <QMessageBox>
#include "lvdscardworker.h"
#include "TabItem.h" // 包含自定义项头文件
#include <QGraphicsPixmapItem>
#include <QFileDialog>
//#include "imagegraphicsview.h"
#include "tabitem.h"
#include "dataanalysishelper.h"
#include "dialogpara.h"
#include <QStandardItemModel>



/// 这边认为板卡及其编号是内部固定的，不可更改的，即1号卡就是LVDS卡，0号卡就是RS422卡


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "Main thread ID:" << QThread::currentThreadId();

    // 初始化表格
    // 在MainWindow::MainWindow()中添加：
    ui->tableWidget_cmd_immediately->setColumnCount(4); // 设置4列（指令序列、时间戳、备注）
    ui->tableWidget_cmd_immediately->setHorizontalHeaderLabels({
        "时间戳（秒）",    // 列0
        "名称",// 列1
        "源码"  ,       // 列2
        "备注"         // 列3
    });

    // ① 拉伸列（列2）：平分剩余空间，占满页面
    ui->tableWidget_cmd_immediately->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    // ② 内容适配列：允许用户手动调整，初始时适配内容
    ui->tableWidget_cmd_immediately->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->tableWidget_cmd_immediately->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->tableWidget_cmd_immediately->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);

    // 允许用户拖动列分隔线调整列宽
    ui->tableWidget_cmd_immediately->horizontalHeader()->setSectionsMovable(true);

    ///**********************************第三方函数库对象****************************

    // 创建 DataTransfer 对象
    dataTransfer = new DataTransfer(this);
    frameHelper = new FrameHelper(this);
//    listWidgetHelper = new ListWidgetHelper(this);

    /// **********************************界面初始化****************************
    setWindowTitle("地检主控程序-P1");
    QIcon iconApp(":/Icon/game_hat_mario_retro_super_video_icon_183171.png");
    setWindowIcon(iconApp);
    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    QLabel *infoLabel = new QLabel(this);
    infoLabel->setText("milurx@163.com | XuXiaohan | Version 1.0.5.260227 | 2026.2.27"); // 1.0.4 通过串口II获取图像 1.0.5 修复xml不匹配等多个问题
    infoLabel->setFont(QFont("Times New Roman", 12));
    infoLabel->setStyleSheet("color: gray;");
    statusBar->addPermanentWidget(infoLabel);

//    ui->tabWidget_main->setTabIcon(0, QIcon(":/Icon/3592835-general-hierachy-map-office-site-structure_107768.png"));
//    ui->tabWidget_main->setTabIcon(1, QIcon(":/Icon/3592869-compose-create-edit-edit-file-office-pencil-writing-creative_107746.png"));
//    ui->tabWidget_main->setTabIcon(2, QIcon(":/Icon/3592864-general-graph-increase-office-rise-statistics-success_107790.png"));
//    ui->tabWidget_main->setTabIcon(3, QIcon(":/Icon/3592837-camera-image-photo-photography-shot_107774.png"));
    // ---------------------- 左侧listWidget设置 ----------------------
     // 1. 去掉默认样式（边框、选中虚线框）

     // 2. 添加标签项（顺序对应右侧页面顺序）
     addTabItem(ui->listWidget, QIcon(":/Icon/3592835-general-hierachy-map-office-site-structure_107768.png"), "接口设置");      // 项1：设置页面
     addTabItem(ui->listWidget, QIcon(":/Icon/3592869-compose-create-edit-edit-file-office-pencil-writing-creative_107746.png"), "指令发送");  // 项2：指令发送页面
     addTabItem(ui->listWidget, QIcon(":/Icon/3592864-general-graph-increase-office-rise-statistics-success_107790.png"), "遥测显示");    // 项3：数据接收页面
     addTabItem(ui->listWidget, QIcon(":/Icon/3592837-camera-image-photo-photography-shot_107774.png"), "图像显示");        // 项4：图像页面
     addTabItem(ui->listWidget, QIcon(":/Icon/3592824-airplane-airplane-mode-general-office-plane-transport-travel_107771.png"), "广播");        // 项5：图像页面

     // 3. 默认选中第一个项
     // 程序打开时，检查当前的中心页面处于什么选项，根据中心页面选项决定ui->listWidget当前选中项
     int curIndex = ui->stackedWidget->currentIndex();
     ui->listWidget->setCurrentRow(curIndex);

     // ---------------------- 右侧stackedWidget设置 ----------------------
     // 1. 添加页面（顺序对应左侧项顺序）
     addStackedPage(ui->stackedWidget, "接口设置页面");  // 页面1：对应「接口设置」项
     addStackedPage(ui->stackedWidget, "指令发送页面");  // 页面2：对应「指令发送」项
     addStackedPage(ui->stackedWidget, "遥测显示页面");  // 页面3：对应「遥测显示」项
     addStackedPage(ui->stackedWidget, "图像显示页面");  // 页面4：对应「图像显示」项
     addStackedPage(ui->stackedWidget, "广播页面");  // 页面5：对应「广播」项

    connect(ui->listWidget, &QListWidget::itemClicked, this, &MainWindow::onTabItemClicked);

    // 取消列表滑块
    ui->listWidget_2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget_2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 切换命令组
    connect(ui->listWidget_2, &QListWidget::itemClicked, this, &MainWindow::onTabItemClicked_2);
    // 右键点击时可执行删除当前命令组
    ui->listWidget_2->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget_2, &QListWidget::customContextMenuRequested,
            this, &MainWindow::onListWidgetContextMenuRequested);

    // 对输入框格式进行限制
    QIntValidator *intValidator = new QIntValidator(this);
    intValidator->setBottom(0);
    intValidator->setTop(255);
    ui->lineEdit_grayThre_2->setValidator(intValidator);

    ui->comboBox_cardCom->addItem("擦除指令");
    ui->comboBox_cardCom->addItem("开始重构");
    ui->comboBox_cardCom->addItem("重载指令");
    ui->comboBox_cardCom->addItem("更改flash寄存器");
    ui->comboBox_cardCom->setCurrentIndex(0);

    /// ****************************can帧配置弹窗*********************************
    dialogCanFrame = new DialogCanFrame(this);
    dialogCanFrame2 = new DialogCanFrame2(this);
    connect(ui->pushButton_canFrameConfig,&QPushButton::clicked,this,[=](){
        dialogCanFrame->exec();
    });
    connect(ui->pushButton_canFrameConfig_2,&QPushButton::clicked,this,[=](){
        dialogCanFrame2->exec();
    });

    /// 加载xml文件，创建指令配置表格
    xmlcommandparser = new XMLCommandParser(this);
    // 更新ui界面的当前指令显示
    connect(xmlcommandparser,&XMLCommandParser::updateUiCurComShowSignal,this,[=](QString text){
        commandList.append(text);
    });


    /// **********************************串口****************************
    /// Serial1
    saveDirSerial1Txt = QCoreApplication::applicationDirPath() + "/DataCapture/Serial1/txt/";
    saveDirSerial1Bin = QCoreApplication::applicationDirPath() + "/DataCapture/Serial1/bin/";
    QDir dirSerial1Txt(saveDirSerial1Txt);
    if (!dirSerial1Txt.exists()) {
        if (!dirSerial1Txt.mkpath(".")) {  // 创建多级目录
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirSerial1Txt);
            return;
        }
    }
    QDir dirSerial1Bin(saveDirSerial1Bin);
    if (!dirSerial1Bin.exists()) {
        if (!dirSerial1Bin.mkpath(".")) {  // 创建多级目录
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirSerial1Bin);
            return;
        }
    }

    /// Serial2
    saveDirSerial2Txt = QCoreApplication::applicationDirPath() + "/DataCapture/Serial2/txt/";
    saveDirSerial2Bin = QCoreApplication::applicationDirPath() + "/DataCapture/Serial2/bin/";
    QDir dirSerial2Txt(saveDirSerial2Txt);
    if (!dirSerial2Txt.exists()) {
        if (!dirSerial2Txt.mkpath(".")) {  // 创建多级目录
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirSerial2Txt);
            return;
        }
    }
    QDir dirSerial2Bin(saveDirSerial2Bin);
    if (!dirSerial2Bin.exists()) {
        if (!dirSerial2Bin.mkpath(".")) {  // 创建多级目录
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirSerial2Bin);
            return;
        }
    }

    // 控件格式设置
    ui->pushButton_serialOpenClose->setCheckable(false);
    ui->pushButton_serialOpenClose_2->setCheckable(false);
    ui->pushButton_saveDataSerial->setCheckable(true);
    ui->pushButton_saveDataSerial_2->setCheckable(true);

    // 设置最大行数并禁用撤销/重做
//    ui->plainTextEdit_receive_serial->setMaximumBlockCount(MAX_LOG_LINES);
//    ui->plainTextEdit_receive_serial->setUndoRedoEnabled(false);
//    ui->plainTextEdit_receive_serial_2->setMaximumBlockCount(MAX_LOG_LINES);
//    ui->plainTextEdit_receive_serial_2->setUndoRedoEnabled(false);

//    /// lVDS图像框格式

    ui->graphicsView->setScene(new QGraphicsScene(this));
//    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
//    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // 初始化波特率选项
    QList<qint32> baudRates = QSerialPortInfo::standardBaudRates();

    int index115200 = baudRates.indexOf(115200);

    foreach (qint32 baud, baudRates) {
        ui->comboBox_baudRate->addItem(QString::number(baud), baud);
        ui->comboBox_baudRate_2->addItem(QString::number(baud), baud);
    }

    ui->comboBox_baudRate->addItem("2000000",2000000);
    ui->comboBox_baudRate->addItem("3000000",3000000);
    ui->comboBox_baudRate->addItem("48000000",48000000);
    int index3M = ui->comboBox_baudRate->findData(3000000); // 根据用户数据找
    ui->comboBox_baudRate->setCurrentIndex(index3M); // 串口1默认波特率3M
    ui->comboBox_baudRate_2->addItem("2000000",2000000);
    ui->comboBox_baudRate_2->addItem("3000000",3000000);
    ui->comboBox_baudRate_2->addItem("48000000",48000000);
    ui->comboBox_baudRate_2->setCurrentIndex(index115200); // 串口2默认波特率115200

    // 数据位
    ui->comboBox_dataBit->addItem("5", QSerialPort::Data5);
    ui->comboBox_dataBit->addItem("6", QSerialPort::Data6);
    ui->comboBox_dataBit->addItem("7", QSerialPort::Data7);
    ui->comboBox_dataBit->addItem("8", QSerialPort::Data8);
    ui->comboBox_dataBit->setCurrentIndex(3); // 默认8位
    ui->comboBox_dataBit_2->addItem("5", QSerialPort::Data5);
    ui->comboBox_dataBit_2->addItem("6", QSerialPort::Data6);
    ui->comboBox_dataBit_2->addItem("7", QSerialPort::Data7);
    ui->comboBox_dataBit_2->addItem("8", QSerialPort::Data8);
    ui->comboBox_dataBit_2->setCurrentIndex(3); // 默认8位

    // 校验位
    ui->comboBox_parity->addItem("None", QSerialPort::NoParity);
    ui->comboBox_parity->addItem("Even", QSerialPort::EvenParity);
    ui->comboBox_parity->addItem("Odd", QSerialPort::OddParity);
    ui->comboBox_parity->addItem("Mark", QSerialPort::MarkParity);
    ui->comboBox_parity->addItem("Space", QSerialPort::SpaceParity);
    ui->comboBox_parity->setCurrentIndex(2); // 默认ODD校验
    ui->comboBox_parity_2->addItem("None", QSerialPort::NoParity);
    ui->comboBox_parity_2->addItem("Even", QSerialPort::EvenParity);
    ui->comboBox_parity_2->addItem("Odd", QSerialPort::OddParity);
    ui->comboBox_parity_2->addItem("Mark", QSerialPort::MarkParity);
    ui->comboBox_parity_2->addItem("Space", QSerialPort::SpaceParity);
    ui->comboBox_parity_2->setCurrentIndex(2); // 默认ODD校验

    // 停止位
    ui->comboBox_stopBits->addItem("1", QSerialPort::OneStop);
    ui->comboBox_stopBits->addItem("2", QSerialPort::TwoStop);
    ui->comboBox_stopBits->setCurrentIndex(1); // 默认停止位2
    ui->comboBox_stopBits_2->addItem("1", QSerialPort::OneStop);
    ui->comboBox_stopBits_2->addItem("2", QSerialPort::TwoStop);
    ui->comboBox_stopBits_2->setCurrentIndex(1); // 默认停止位2

    // 刷新串口列表
    refreshSerialPorts();

    // 创建串口管理对象
    serialPortWorker1 = new SerialPortWorker(); // 创建串口管理对象：这里包括串口数据的接收，以及将其存进环形缓冲区
    serialPortWorker2 = new SerialPortWorker(); // 创建串口管理对象

    // 创建数据处理对象
    dataAnalysisHelper1 = new DataAnalysisHelper();
    dataAnalysisHelper2 = new DataAnalysisHelper();
    // 创建环形缓冲区对象
    ringBuffer1  = new RingBuffer(10 * 1024 * 1024); // 共享环形缓冲区对象：10MB
    ringBuffer2  = new RingBuffer(10 * 1024 * 1024); // 共享环形缓冲区对象：10MB
    ringBufferRS422  = new RingBuffer(10 * 1024 * 1024); // 共享环形缓冲区对象：10MB
    ringBufferLVDS  = new RingBuffer(10 * 1024 * 1024); // 共享环形缓冲区对象：10MB
    ringBufferCAN  = new RingBuffer(10 * 1024 * 1024); // 共享环形缓冲区对象：10MB
    ringBufferCAN_slow  = new RingBuffer(10 * 1024 * 1024); // 共享环形缓冲区对象：10MB

    // 将这个缓冲区同时传递给串口对象（push数据）和数据处理对象（pop数据）
    serialPortWorker1->setRingBuffer(ringBuffer1);
    serialPortWorker2->setRingBuffer(ringBuffer2);
    dataAnalysisHelper1->setRingBuffer(ringBuffer1);
    dataAnalysisHelper2->setRingBuffer(ringBuffer2);

    // 创建数据处理线程
    t_dataAnalysis1 = new QThread(this);
    t_dataAnalysis2 = new QThread(this);
    dataAnalysisHelper1->moveToThread(t_dataAnalysis1);
    dataAnalysisHelper2->moveToThread(t_dataAnalysis2);
    // 当线程t_dataAnalysis启动时，dataAnalysisHelper从环形缓冲区中抓取数据然后解析

    connect(t_dataAnalysis1,&QThread::started,dataAnalysisHelper1,&DataAnalysisHelper::onDataAnalysisSerial1);
//    connect(t_dataAnalysis2,&QThread::started,dataAnalysisHelper2,&DataAnalysisHelper::onDataAnalysisRS422); // 临时测试用，用于使用串口2调试RS422通道接收遥测
    // 所以如果想要使用COMII来测试遥测显示页面，可以将上面这一行解除注释，但是注意，当上面这一行解除注释时，下面的一行槽函数就无法运行了
    connect(t_dataAnalysis2,&QThread::started,dataAnalysisHelper2,&DataAnalysisHelper::onDataAnalysisLVDS);// 临时测试用，用于使用串口2调试LVDS通道接收图像
//    connect(t_dataAnalysis1,&QThread::started,dataAnalysisHelper1,&DataAnalysisHelper::onDataAnalysisLVDS);// 注意，目前无法使用串口1调试LVDS通道接收图像
//    connect(t_dataAnalysis2,&QThread::started,dataAnalysisHelper2,&DataAnalysisHelper::onDataAnalysisSerial2);


    t_dataAnalysis1->start();
    dataAnalysisHelper1->startProcessing();
    t_dataAnalysis2->start();
    dataAnalysisHelper2->startProcessing();


//    // 清空缓存区
//    serialPortWorker1->clearFIFO();
//    serialPortWorker2->clearFIFO();

    // 创建串口线程对象
    t3 = new QThread(this);
    t4 = new QThread(this);

    // 启动线程
    serialPortWorker1->moveToThread(t3);
    serialPortWorker2->moveToThread(t4);
    t3->start();
    t4->start();

    /// 串口槽函数
    // 初始化串口
    connect(this,&MainWindow::initSerialSignal,this,[=](QString portName,int baudRate, int dataBit, int parity, int stopBit){
        serialPortWorker1->initSerial(portName, baudRate, dataBit, parity, stopBit);
    });
    connect(this,&MainWindow::initSerialSignal_2,this,[=](QString portName,int baudRate, int dataBit, int parity, int stopBit){
        serialPortWorker2->initSerial(portName, baudRate, dataBit, parity, stopBit);
    });

    // 打开关闭串口
    connect(this,&MainWindow::openCloseSignal,serialPortWorker1,&SerialPortWorker::openCloseSerial);
    connect(this,&MainWindow::openCloseSignal_2,serialPortWorker2,&SerialPortWorker::openCloseSerial);
    // 打开关闭串口按钮
    connect(ui->pushButton_serialOpenClose,&QPushButton::clicked,this,[=](){
        serialOpenClose(true); // 串口1控制
    }); // 打开串口
    connect(ui->pushButton_serialOpenClose_2,&QPushButton::clicked,this,[=](){
        serialOpenClose(false); // 串口2控制
    }); // 打开串口

    // ui显示串口打开状态更新
    connect(serialPortWorker1,&SerialPortWorker::updatePortStateSignal,this,[=](bool portStatus){
        updatePortState(true, portStatus);
    });
    connect(serialPortWorker2,&SerialPortWorker::updatePortStateSignal,this,[=](bool portStatus){
        updatePortState(false, portStatus);
    });

    connect(ui->pushButton_clearSend_serial,&QPushButton::clicked,ui->plainTextEdit_send_serial,&QPlainTextEdit::clear);
    connect(ui->pushButton_clearReceive_serial,&QPushButton::clicked,ui->plainTextEdit_receive_serial,&QPlainTextEdit::clear);
    connect(ui->pushButton_clearSend_serial_2,&QPushButton::clicked,ui->plainTextEdit_send_serial_2,&QPlainTextEdit::clear);
    connect(ui->pushButton_clearReceive_serial_2,&QPushButton::clicked,ui->plainTextEdit_receive_serial_2,&QPlainTextEdit::clear);


    /// 串口checkbox状态改变时*直接修改串口配置*然后重新打开串口*无需再手动点击串口打开按钮*
    connect(this,&MainWindow::closeSerialSignal,serialPortWorker1,&SerialPortWorker::closeSerial);
    connect(this,&MainWindow::closeSerialSignal_2,serialPortWorker2,&SerialPortWorker::closeSerial);
    connect(ui->comboBox_serial,&QComboBox::currentTextChanged,this,[=](){
        if(ui->pushButton_serialOpenClose->text() == "关闭串口"){
            // 先关闭串口
            emit closeSerialSignal();
//            serialPortWorker1->closeSerial(); // 会导致跨线程调用的崩溃
            serialOpenClose(true);
        }
    });
    connect(ui->comboBox_baudRate,&QComboBox::currentTextChanged,this,[=](){
        if(ui->pushButton_serialOpenClose->text() == "关闭串口"){
            // 先关闭串口
            emit closeSerialSignal();
            serialOpenClose(true);
        }
    });
    connect(ui->comboBox_dataBit,&QComboBox::currentTextChanged,this,[=](){
        if(ui->pushButton_serialOpenClose->text() == "关闭串口"){
            // 先关闭串口
            emit closeSerialSignal();
            serialOpenClose(true);
        }
    });
    connect(ui->comboBox_parity,&QComboBox::currentTextChanged,this,[=](){
        if(ui->pushButton_serialOpenClose->text() == "关闭串口"){
            // 先关闭串口
            emit closeSerialSignal();
            serialOpenClose(true);
        }
    });
    connect(ui->comboBox_stopBits,&QComboBox::currentTextChanged,this,[=](){
        if(ui->pushButton_serialOpenClose->text() == "关闭串口"){
            // 先关闭串口
            emit closeSerialSignal();
            serialOpenClose(true);
        }
    });
    connect(ui->comboBox_serial_2,&QComboBox::currentTextChanged,this,[=](){
        if(ui->pushButton_serialOpenClose_2->text() == "关闭串口"){
            emit closeSerialSignal_2();
            serialOpenClose(false);
        }
    });
    connect(ui->comboBox_baudRate_2,&QComboBox::currentTextChanged,this,[=](){
        if(ui->pushButton_serialOpenClose_2->text() == "关闭串口"){
            emit closeSerialSignal_2();
            serialOpenClose(false);
        }
    });
    connect(ui->comboBox_dataBit_2,&QComboBox::currentTextChanged,this,[=](){
        if(ui->pushButton_serialOpenClose_2->text() == "关闭串口"){
            emit closeSerialSignal_2();
            serialOpenClose(false);
        }
    });
    connect(ui->comboBox_parity_2,&QComboBox::currentTextChanged,this,[=](){
        if(ui->pushButton_serialOpenClose_2->text() == "关闭串口"){
            emit closeSerialSignal_2();
            serialOpenClose(false);
        }
    });
    connect(ui->comboBox_stopBits_2,&QComboBox::currentTextChanged,this,[=](){
        if(ui->pushButton_serialOpenClose_2->text() == "关闭串口"){
            emit closeSerialSignal_2();
            serialOpenClose(false);
        }
    });


    // 发送数据
    connect(ui->pushButton_send_serial,&QPushButton::clicked,this,[=](){
        // 准备数据
        QString tempStringData = ui->plainTextEdit_send_serial->toPlainText();
        QByteArray byteDataSend = dataTransfer->string2Bytearray(tempStringData);
//        qDebug()<<"byteDataToSend1："<<byteDataSend;
        emit writeDataSignal(byteDataSend);
    });
    connect(this,&MainWindow::writeDataSignal,serialPortWorker1,[=](QByteArray byteDataSend){
        qDebug()<<"byteDataSend connect"<<byteDataSend;
        serialPortWorker1->writeData(byteDataSend); // 这里是将数据直接原封不动地通过串口写出，后面当具体发送指令时，需要先为配置好的指令添加帧头、校验和等必备帧格式框架后再写入串口
    });
    connect(ui->pushButton_send_serial_2,&QPushButton::clicked,this,[=](){
        // 准备数据
        QString tempStringData = ui->plainTextEdit_send_serial_2->toPlainText();
        QByteArray byteDataSend = dataTransfer->string2Bytearray(tempStringData);
//        qDebug()<<"byteDataToSend2："<<byteDataSend;
        emit writeDataSignal_2(byteDataSend);
    });
    connect(this,&MainWindow::writeDataSignal_2,serialPortWorker2,[=](QByteArray byteDataSend){
        serialPortWorker2->writeData(byteDataSend);
    });

    // 接收数据
    // 直接显示
    connect(serialPortWorker1,&SerialPortWorker::dataToUiDirect,this,[=](QString textToShowDirect){
        updatePlainText(true, textToShowDirect);
    });
    connect(serialPortWorker2,&SerialPortWorker::dataToUiDirect,this,[=](QString textToShowDirect){
        updatePlainText(false, textToShowDirect);
    });



    /// 开始保存数据到txt
    connect(ui->pushButton_saveDataSerial,&QPushButton::clicked,this,[=](){
        if(ui->pushButton_saveDataSerial->isChecked()){
            // 检查串口打开状态
            bool serialIsOpen = serialPortWorker1->serialPort->isOpen(); // 其实是比较危险的操作，属于跨线程调用
            if(serialIsOpen){
                qDebug()<<"串口被正常打开";
                ui->pushButton_saveDataSerial->setChecked(true);
                ui->pushButton_saveDataSerial->setText("等待");
                // 获取当前时间作为文件名
                QDateTime currentTime = QDateTime::currentDateTime();
                fileNameSerial1 = currentTime.toString("yyyyMMdd_hhmmss") +"Serial1"+ ".txt";
                statusBarShowSerial(true,"准备保存数据");
                ui->pushButton_saveDataSerial->setChecked(true);
                // 查询保存时间
                double saveTime = ui->lineEdit_saveDataTimeSerial->text().toDouble();
                isSaveDataSerial1 = true; // 开始保存
                // 启动定时器，在指定时间后停止保存
                QTimer::singleShot(static_cast<int>(saveTime * 1000), this, [this](){
                    isSaveDataSerial1 = false; // 停止保存
                    ui->pushButton_saveDataSerial->setChecked(false);
                    ui->pushButton_saveDataSerial->setText("开始");
                    QMessageBox::information(this, "保存完成", "数据已保存到文件");
                });
            }
            else{
                qmessageBoxSet("Serial1未打开！");
                statusBarShowSerial(true,"Serial1未打开！");
                ui->pushButton_saveDataSerial->setChecked(false);
            }
        }
        else{
            qmessageBoxSet("请等待当前数据保存完毕！");
            ui->pushButton_saveDataSerial->setChecked(true);
            ui->pushButton_saveDataSerial->setText("等待");
        }
    });

    connect(ui->pushButton_saveDataSerial_2,&QPushButton::clicked,this,[=](){
        if(ui->pushButton_saveDataSerial_2->isChecked()){
            // 检查串口打开状态
            bool serialIsOpen = serialPortWorker2->serialPort->isOpen(); // 其实是比较危险的操作，属于跨线程调用
            if(serialIsOpen){
                ui->pushButton_saveDataSerial_2->setChecked(true);
                ui->pushButton_saveDataSerial_2->setText("等待");
                // 获取当前时间作为文件名
                QDateTime currentTime = QDateTime::currentDateTime();
                fileNameSerial2 = currentTime.toString("yyyyMMdd_hhmmss") +"Serial2"+ ".txt";
                statusBarShowSerial(false,"准备保存数据");
                ui->pushButton_saveDataSerial_2->setChecked(true);
                // 查询保存时间
                double saveTime = ui->lineEdit_saveDataTimeSerial_2->text().toDouble();
                isSaveDataSerial2 = true; // 开始保存
                // 启动定时器，在指定时间后停止保存
                QTimer::singleShot(static_cast<int>(saveTime * 1000), this, [this](){
                    isSaveDataSerial2 = false; // 停止保存
                    ui->pushButton_saveDataSerial_2->setChecked(false);
                    ui->pushButton_saveDataSerial_2->setText("开始");
                    QMessageBox::information(this, "保存完成", "数据已保存到文件");
                });
            }
            else{
                qmessageBoxSet("Serial2未打开！");
                statusBarShowSerial(false,"Serial2未打开！");
                ui->pushButton_saveDataSerial_2->setChecked(false);
            }
        }
        else{
            qmessageBoxSet("请等待当前数据保存完毕！");
            ui->pushButton_saveDataSerial_2->setChecked(true);
            ui->pushButton_saveDataSerial_2->setText("等待");
        }
    });

    // 临时测试用，使用串口1模拟RS422板卡，用于调试可见光相机
    connect(dataAnalysisHelper1,&DataAnalysisHelper::upgradeCentroidUi,this,[=](const CentroidFrameAttributes &attrs){
        qDebug()<<"质心帧解析参数到达main函数";
        ui->lineEdit_centroidX->setText(QString::number(attrs.centroidX));
        ui->lineEdit_centroidY->setText(QString::number(attrs.centroidY));
        ui->lineEdit_minGray->setText(QString::number(attrs.minGrayscale));
        ui->lineEdit_maxGray->setText(QString::number(attrs.maxGrayscale));
        ui->lineEdit_overGrayscaleNum->setText(QString::number(attrs.overThreshold));
        ui->lineEdit_exposureTime->setText(QString::number(attrs.exposureTime));
        ui->lineEdit_grayscaleSum->setText(QString::number(attrs.grayscaleSum));
        ui->lineEdit_totalCentroidFameNum->setText(QString::number(attrs.totalFrameNum));
        ui->lineEdit_validCentroidFameNum->setText(QString::number(attrs.validFrameNum));
        ui->lineEdit_errorCentroidFameNum->setText(QString::number(attrs.errorFrameNum));
    });

    connect(dataAnalysisHelper2,&DataAnalysisHelper::upgradeTemUi,this,[=](const TemperatureFrameAttributes &attrs){
        qDebug()<<"温度帧解析参数到达main函数";
        ui->lineEdit_temTEC->setText(QString::number(attrs.tempTEC));
        ui->lineEdit_temFPGA->setText(QString::number(attrs.tempFPGA));
        ui->lineEdit_totalTemParaNum->setText(QString::number(attrs.totalUiParaNum));
        ui->lineEdit_errorTemParaNum->setText(QString::number(attrs.errorUiparaNum));
        ui->lineEdit_validTemParaNum->setText(QString::number(attrs.validUiParaNum));
    });

    /// **********************************板卡****************************
    /// RS422
    saveDirRS422Txt = QCoreApplication::applicationDirPath() + "/DataCapture/RS422/txt/";
    saveDirRS422Bin = QCoreApplication::applicationDirPath() + "/DataCapture/RS422/bin/";
    QDir dirRS422Txt(saveDirRS422Txt);
    if (!dirRS422Txt.exists()) {
        if (!dirRS422Txt.mkpath(".")) {  // 创建多级目录
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirRS422Txt);
            return;
        }
    }
    QDir dirRS422Bin(saveDirRS422Bin);
    if (!dirRS422Bin.exists()) {
        if (!dirRS422Bin.mkpath(".")) {  // 创建多级目录
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirRS422Bin);
            return;
        }
    }

    /// LVDS
    saveDirLVDSTxt = QCoreApplication::applicationDirPath() + "/DataCapture/LVDS/txt/";
    saveDirLVDSBin = QCoreApplication::applicationDirPath() + "/DataCapture/LVDS/bin/";
    QDir dirLVDSTxt(saveDirLVDSTxt);
    if (!dirLVDSTxt.exists()) {
        if (!dirLVDSTxt.mkpath(".")) {  // 创建多级目录
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirLVDSTxt);
            return;
        }
    }
    QDir dirLVDSBin(saveDirLVDSBin);
    if (!dirLVDSBin.exists()) {
        if (!dirLVDSBin.mkpath(".")) {  // 创建多级目录
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirLVDSBin);
            return;
        }
    }

    // 创建保存图片的路径
    saveDirLVDSImgSingle = QCoreApplication::applicationDirPath() + "/ImageCapture/ImgSingle/"; // 单次点击按钮保存图片文件夹
    saveDirLVDSImgSpan = QCoreApplication::applicationDirPath() + "/ImageCapture/ImgSpan/"; // 持续保存图片文件夹
    QDir dirLVDSImgSingle(saveDirLVDSImgSingle);
    if(!dirLVDSImgSingle.exists()){
        if(!dirLVDSImgSingle.mkpath(".")){
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirLVDSImgSingle);
            return;
        }
    }
    QDir dirLVDSImgSpan(saveDirLVDSImgSpan);
    if(!dirLVDSImgSpan.exists()){
        if(!dirLVDSImgSpan.mkpath(".")){
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirLVDSImgSpan);
            return;
        }
    }
    connect(ui->pushButton_saveImage_2,&QPushButton::clicked,this,&MainWindow::saveImgSingle);
    ui->pushButton_beginSaveImages_2->setCheckable(true);
    ui->pushButton_beginSaveImages_2->setChecked(false);
    connect(ui->pushButton_beginSaveImages_2,&QPushButton::clicked,this,&MainWindow::saveImgSpan);


    ui->pushButton_cardOpenClose->setCheckable(true);
    ui->pushButton_cardOpenClose_2->setCheckable(true);
    ui->pushButton_cardOpenClose->setChecked(false);
    ui->pushButton_cardOpenClose_2->setChecked(false);

    cardWorker1 = new lvdsCardWorker();// 创建板卡worker对象
    t1= new QThread();
    cardWorker1->moveToThread(t1);
    cardWorker1->setRingBuffer(ringBufferRS422);
    dataAnalysisHelperRS422 = new DataAnalysisHelper();
    dataAnalysisHelperRS422->setRingBuffer(ringBufferRS422);
    t_dataAnalysisRS422 = new QThread();
    dataAnalysisHelperRS422->moveToThread(t_dataAnalysisRS422);
    connect(t_dataAnalysisRS422,&QThread::started,dataAnalysisHelperRS422,&DataAnalysisHelper::onDataAnalysisRS422);
    t_dataAnalysisRS422->start();
    dataAnalysisHelperRS422->startProcessing(); // 置标志位m_running为true

    cardWorker2 = new lvdsCardWorker();// 创建板卡worker对象
    t2= new QThread();
    cardWorker2->moveToThread(t2);
    cardWorker2->setRingBuffer(ringBufferLVDS);
    dataAnalysisHelperLVDS = new DataAnalysisHelper();
    dataAnalysisHelperLVDS->setRingBuffer(ringBufferLVDS);
    t_dataAnalysisLVDS = new QThread();
    dataAnalysisHelperLVDS->moveToThread(t_dataAnalysisLVDS);
    connect(t_dataAnalysisLVDS,&QThread::started,dataAnalysisHelperLVDS,&DataAnalysisHelper::onDataAnalysisLVDS);
    connect(dataAnalysisHelperLVDS, &DataAnalysisHelper::imageSignal, this, [=](QImage image) {
            qDebug() << "槽函数执行线程ID：" << QThread::currentThreadId();
            qDebug() << "到达main函数";
            showImage(image);
        });
    t_dataAnalysisLVDS->start();
    dataAnalysisHelperLVDS->startProcessing();

    /// 笔记：刚开始我将下面的代码放在上面串口那边，会发生崩溃，原因是，这段代码太靠前，都位于dataAnalysisHelperRS422 = new DataAnalysisHelper();的
    /// 前面了，在主函数处设置槽函数连接时一定要特别注意槽函数所在的位置，不能过于靠前，起码该槽函数中涉及到的线程、对象等初始化等操作需要提前准备好
    connect(dataAnalysisHelperRS422,&DataAnalysisHelper::upgradeCentroidUi,this,[=](const CentroidFrameAttributes &attrs){
        qDebug()<<"质心帧解析参数到达main函数";
        ui->lineEdit_centroidX->setText(QString::number(attrs.centroidX));
        ui->lineEdit_centroidY->setText(QString::number(attrs.centroidY));
        ui->lineEdit_minGray->setText(QString::number(attrs.minGrayscale));
        ui->lineEdit_maxGray->setText(QString::number(attrs.maxGrayscale));
        ui->lineEdit_overGrayscaleNum->setText(QString::number(attrs.overThreshold));
        ui->lineEdit_exposureTime->setText(QString::number(attrs.exposureTime));
        ui->lineEdit_grayscaleSum->setText(QString::number(attrs.grayscaleSum));
        ui->lineEdit_totalCentroidFameNum->setText(QString::number(attrs.totalFrameNum));
        ui->lineEdit_validCentroidFameNum->setText(QString::number(attrs.validFrameNum));
        ui->lineEdit_errorCentroidFameNum->setText(QString::number(attrs.errorFrameNum));
    });

    // 更新遥测帧计数
    connect(dataAnalysisHelperRS422,&DataAnalysisHelper::updateFrameCheckSignal,this,[=](bool isValid){
        qDebug()<<"开始更新遥测帧计数";
        if(isValid){
            qDebug()<<"正确帧加一";
            validFrameCount += 1;
            ui->le_frame_crc_success_count->setText(QString::number(validFrameCount));
            // 在UI中更新当前接收到遥测的数量
            paraFrameCount += 1;
            ui->le_frame_total_count->setText(QString::number(paraFrameCount)) ;
        }
        else{
             qDebug()<<"错误帧加一";
            invalidFrameCount += 1;
            ui->le_frame_crc_wrong_count->setText(QString::number(invalidFrameCount));
            // 在UI中更新当前接收到遥测的数量
            paraFrameCount += 1;
            ui->le_frame_total_count->setText(QString::number(paraFrameCount)) ;
        }
    });

    connect(dataAnalysisHelper2,&DataAnalysisHelper::updateFrameCheckSignal,this,[=](bool isValid){  // 临时测试用
        qDebug()<<"开始更新遥测帧计数";
        if(isValid){
            qDebug()<<"正确帧加一";
            validFrameCount += 1;
            ui->le_frame_crc_success_count->setText(QString::number(validFrameCount));
            // 在UI中更新当前接收到遥测的数量
            paraFrameCount += 1;
            ui->le_frame_total_count->setText(QString::number(paraFrameCount)) ;
        }
        else{
             qDebug()<<"错误帧加一";
            invalidFrameCount += 1;
            ui->le_frame_crc_wrong_count->setText(QString::number(invalidFrameCount));
            // 在UI中更新当前接收到遥测的数量
            paraFrameCount += 1;
            ui->le_frame_total_count->setText(QString::number(paraFrameCount)) ;
        }
    });

    /// 更新遥测显示列表内容
    connect(dataAnalysisHelperRS422,&DataAnalysisHelper::updateParaListUiSignal,this,[=](QVector<ParaListUi> paraListUiList){
        for (int i = 0 ; i < paraListUiList.size(); ++i) {
            modelMain->setItem(i,2,new QStandardItem(paraListUiList.at(i).value));
            modelMain->setItem(i,3,new QStandardItem(paraListUiList.at(i).code));
            if(paraListUiList.at(i).isValid == true){
                modelMain->setItem(i,4,new QStandardItem("是"));
            }
            else{
                modelMain->setItem(i,4,new QStandardItem("否"));
            }

            // ====== 同步更新兴趣表 ======
            QString paraNo = paraListUiList.at(i).paraNo;
            for (int r = 0; r < modelInterest->rowCount(); ++r) {
                if (modelInterest->item(r, 0)->text() == paraNo) {
                    // 按列逐个更新
                    modelInterest->setItem(r, 2, new QStandardItem(paraListUiList.at(i).value));
                    modelInterest->setItem(r, 3, new QStandardItem(paraListUiList.at(i).code));
                    modelInterest->setItem(r, 4, new QStandardItem(paraListUiList.at(i).isValid ? "是" : "否"));
                    break; // 找到后退出循环
                }
            }
        }

        qDebug()<<"读取前 isSaveTMFile"<<isSaveTMFile;
        // 检查保存遥测标志位是否开启，如果是的话，读取saveFileNameTm文件开始写入
        if(isSaveTMFile){
            QFile file(saveFileNameTm);
            QFile fileAuto(saveFileNameTmAuto);
            if(ui->rb_save_data_manual->isChecked()){
                if(file.open((QIODevice::Append | QIODevice::Text))){
                    QTextStream out(&file);
                    QStringList rowValues;
                    for (int i = 0; i < paraListUiList.size(); ++i) {
                         rowValues << paraListUiList.at(i).value;
                     }
                    out << rowValues.join(",") << "\n";  // 写入CSV一行
                    file.close();
                }
                else{
                    qmessageBoxSet("无法打开遥测手动保存CSV文件");
                }
            }
            else {
                if(fileAuto.open((QIODevice::Append | QIODevice::Text))){
                    QTextStream out(&fileAuto);
                    QStringList rowValues;
                    for (int i = 0; i < paraListUiList.size(); ++i) {
                         rowValues << paraListUiList.at(i).value;
                     }
                    out << rowValues.join(",") << "\n";  // 写入CSV一行
                    fileAuto.close();
                }
                else{
                    qmessageBoxSet("无法打开遥测自动保存CSV文件");
                }
            }
        }else {
            qWarning() << "当前未开启遥测数据保存" << saveFileNameTm;
        }
    });

    dataAnalysisHelperCAN = new DataAnalysisHelper();
    /// 更新遥测显示列表内容
    connect(dataAnalysisHelperCAN,&DataAnalysisHelper::updateParaListUiSignal,this,[=](QVector<ParaListUi> paraListUiList){
        for (int i = 0 ; i < paraListUiList.size(); ++i) {
            modelMain->setItem(i,2,new QStandardItem(paraListUiList.at(i).value));
            modelMain->setItem(i,3,new QStandardItem(paraListUiList.at(i).code));
            if(paraListUiList.at(i).isValid == true){
                modelMain->setItem(i,4,new QStandardItem("是"));
            }
            else{
                modelMain->setItem(i,4,new QStandardItem("否"));
            }

            // ====== 同步更新兴趣表 ======
            QString paraNo = paraListUiList.at(i).paraNo;
            for (int r = 0; r < modelInterest->rowCount(); ++r) {
                if (modelInterest->item(r, 0)->text() == paraNo) {
                    // 按列逐个更新
                    modelInterest->setItem(r, 2, new QStandardItem(paraListUiList.at(i).value));
                    modelInterest->setItem(r, 3, new QStandardItem(paraListUiList.at(i).code));
                    modelInterest->setItem(r, 4, new QStandardItem(paraListUiList.at(i).isValid ? "是" : "否"));
                    break; // 找到后退出循环
                }
            }
        }

        qDebug()<<"读取前 isSaveTMFile"<<isSaveTMFile;
        // 检查保存遥测标志位是否开启，如果是的话，读取saveFileNameTm文件开始写入
        if(isSaveTMFile){
            QFile file(saveFileNameTm);
            QFile fileAuto(saveFileNameTmAuto);
            if(ui->rb_save_data_manual->isChecked()){
                if(file.open((QIODevice::Append | QIODevice::Text))){
                    QTextStream out(&file);
                    QStringList rowValues;
                    for (int i = 0; i < paraListUiList.size(); ++i) {
                         rowValues << paraListUiList.at(i).value;
                     }
                    out << rowValues.join(",") << "\n";  // 写入CSV一行
                    file.close();
                }
                else{
                    qmessageBoxSet("无法打开遥测手动保存CSV文件");
                }
            }
            else {
                if(fileAuto.open((QIODevice::Append | QIODevice::Text))){
                    QTextStream out(&fileAuto);
                    QStringList rowValues;
                    for (int i = 0; i < paraListUiList.size(); ++i) {
                         rowValues << paraListUiList.at(i).value;
                     }
                    out << rowValues.join(",") << "\n";  // 写入CSV一行
                    fileAuto.close();
                }
                else{
                    qmessageBoxSet("无法打开遥测自动保存CSV文件");
                }
            }
        }else {
            qWarning() << "当前未开启遥测数据保存" << saveFileNameTm;
        }
    });

    connect(dataAnalysisHelperCAN,&DataAnalysisHelper::updateParaListUiSignal_slow,this,[=](QVector<ParaListUi> paraListUiList){
        for (int i = 0 ; i < paraListUiList.size(); ++i) {
            modelMain_slow->setItem(i,2,new QStandardItem(paraListUiList.at(i).value));
            modelMain_slow->setItem(i,3,new QStandardItem(paraListUiList.at(i).code));
            if(paraListUiList.at(i).isValid == true){
                modelMain_slow->setItem(i,4,new QStandardItem("是"));
            }
            else{
                modelMain_slow->setItem(i,4,new QStandardItem("否"));
            }

            // ====== 同步更新兴趣表 ======
            QString paraNo = paraListUiList.at(i).paraNo;
            for (int r = 0; r < modelInterest_slow->rowCount(); ++r) {
                if (modelInterest_slow->item(r, 0)->text() == paraNo) {
                    // 按列逐个更新
                    modelInterest_slow->setItem(r, 2, new QStandardItem(paraListUiList.at(i).value));
                    modelInterest_slow->setItem(r, 3, new QStandardItem(paraListUiList.at(i).code));
                    modelInterest_slow->setItem(r, 4, new QStandardItem(paraListUiList.at(i).isValid ? "是" : "否"));
                    break; // 找到后退出循环
                }
            }
        }

        qDebug()<<"读取前 isSaveTMFile"<<isSaveTMFile_slow;
        // 检查保存遥测标志位是否开启，如果是的话，读取saveFileNameTm文件开始写入
        if(isSaveTMFile_slow){
            QFile file(saveFileNameTm);
            QFile fileAuto(saveFileNameTmAuto);
            if(ui->rb_save_data_manual->isChecked()){
                if(file.open((QIODevice::Append | QIODevice::Text))){
                    QTextStream out(&file);
                    QStringList rowValues;
                    for (int i = 0; i < paraListUiList.size(); ++i) {
                         rowValues << paraListUiList.at(i).value;
                     }
                    out << rowValues.join(",") << "\n";  // 写入CSV一行
                    file.close();
                }
                else{
                    qmessageBoxSet("无法打开遥测手动保存CSV文件");
                }
            }
            else {
                if(fileAuto.open((QIODevice::Append | QIODevice::Text))){
                    QTextStream out(&fileAuto);
                    QStringList rowValues;
                    for (int i = 0; i < paraListUiList.size(); ++i) {
                         rowValues << paraListUiList.at(i).value;
                     }
                    out << rowValues.join(",") << "\n";  // 写入CSV一行
                    fileAuto.close();
                }
                else{
                    qmessageBoxSet("无法打开遥测自动保存CSV文件");
                }
            }
        }else {
            qWarning() << "当前未开启遥测数据保存" << saveFileNameTm;
        }
    });


    connect(dataAnalysisHelper2,&DataAnalysisHelper::updateParaListUiSignal,this,[=](QVector<ParaListUi> paraListUiList){ // 临时测试用
        for (int i = 0 ; i < paraListUiList.size(); ++i) {
            modelMain->setItem(i,2,new QStandardItem(paraListUiList.at(i).value));
            modelMain->setItem(i,3,new QStandardItem(paraListUiList.at(i).code));
            if(paraListUiList.at(i).isValid == true){
                modelMain->setItem(i,4,new QStandardItem("是"));
            }
            else{
                modelMain->setItem(i,4,new QStandardItem("否"));
            }
            // ====== 同步更新兴趣表 ======
            QString paraNo = paraListUiList.at(i).paraNo;
            for (int r = 0; r < modelInterest->rowCount(); ++r) {
                if (modelInterest->item(r, 0)->text() == paraNo) {
                    // 按列逐个更新
                    modelInterest->setItem(r, 2, new QStandardItem(paraListUiList.at(i).value));
                    modelInterest->setItem(r, 3, new QStandardItem(paraListUiList.at(i).code));
                    modelInterest->setItem(r, 4, new QStandardItem(paraListUiList.at(i).isValid ? "是" : "否"));
                    break; // 找到后退出循环
                }
            }
        }

        qDebug()<<"读取前 isSaveTMFile"<<isSaveTMFile;
        // 检查保存遥测标志位是否开启，如果是的话，读取saveFileNameTm文件开始写入
        if(isSaveTMFile){
            QFile file(saveFileNameTm);
            QFile fileAuto(saveFileNameTmAuto);
            if(ui->rb_save_data_manual->isChecked()){
                if(file.open((QIODevice::Append | QIODevice::Text))){
                    QTextStream out(&file);
                    QStringList rowValues;
                    for (int i = 0; i < paraListUiList.size(); ++i) {
                         rowValues << paraListUiList.at(i).value;
                     }
                    out << rowValues.join(",") << "\n";  // 写入CSV一行
                    file.close();
                }
                else{
                    qmessageBoxSet("无法打开遥测手动保存CSV文件");
                }
            }
            else {
                if(fileAuto.open((QIODevice::Append | QIODevice::Text))){
                    QTextStream out(&fileAuto);
                    QStringList rowValues;
                    for (int i = 0; i < paraListUiList.size(); ++i) {
                         rowValues << paraListUiList.at(i).value;
                     }
                    out << rowValues.join(",") << "\n";  // 写入CSV一行
                    fileAuto.close();
                }
                else{
                    qmessageBoxSet("无法打开遥测自动保存CSV文件");
                }
            }
        }else {
            qWarning() << "当前未开启遥测数据保存" << saveFileNameTm;
        }
    });

    // 清空遥测帧计数
    connect(ui->pb_clear_frame_errors,&QPushButton::clicked,this,[=](){
        paraFrameCount = 0;
        validFrameCount = 0;
        invalidFrameCount = 0;
        ui->le_frame_total_count->setText(QString::number(paraFrameCount)) ;
        ui->le_frame_crc_wrong_count->setText(QString::number(invalidFrameCount)) ;
        ui->le_frame_crc_success_count->setText(QString::number(validFrameCount)) ;
    });

    connect(serialPortWorker1,&SerialPortWorker::RunError,this,&MainWindow::runError);
    connect(serialPortWorker2,&SerialPortWorker::RunError,this,&MainWindow::runError);

    // 启动程序时就将一些需要用到的默认参数传递出去
    // 获取灰度阈值
    int grayThrehold = ui->lineEdit_grayThre_2->text().toInt();
    settingsUiAttrs.grayThrehold = grayThrehold;
    dataAnalysisHelperLVDS->updataUiSettingsRec(settingsUiAttrs);
//    dataAnalysisHelper1->updataUiSettingsRec(settingsUiAttrs);
    dataAnalysisHelper2->updataUiSettingsRec(settingsUiAttrs);
    connect(ui->lineEdit_grayThre_2,&QLineEdit::textChanged,this,[=](){
        // 获取当前的最新内容
        int grayThrehold = ui->lineEdit_grayThre_2->text().toInt();
        settingsUiAttrs.grayThrehold = grayThrehold;
        dataAnalysisHelperLVDS->updataUiSettingsRec(settingsUiAttrs);
//        dataAnalysisHelper1->updataUiSettingsRec(settingsUiAttrs);
        dataAnalysisHelper2->updataUiSettingsRec(settingsUiAttrs);
    });

    // 设置板卡号
    connect(this,&MainWindow::cardNoConfigSignal,this,[=](bool no1IsRs422){
            cardWorker1->GTChoise(no1IsRs422);
            cardWorker2->GTChoise(no1IsRs422);
            });
    // 对于A号机，0是rs422
    quint8 rss422No = 0;
    quint8 lvdsNo = 1;
    emit cardNoConfigSignal(true);
//    if(){
//        // 对于B号机，1是rs422
//        rss422No = 1;
//        lvdsNo = 0;
//        emit cardNoConfigSignal(false);
//    }

    qDebug()<<"RS422卡号是"<<rss422No;
    cardWorker1->setCardNum(rss422No); // 0号卡（RS422）
    cardWorker2->setCardNum(lvdsNo); // 1号卡（LVDS）
    connect(this,&MainWindow::updateWorkerCardNum1,cardWorker1,&lvdsCardWorker::updateCardNoInput);
    connect(this,&MainWindow::updateWorkerCardNum2,cardWorker2,&lvdsCardWorker::updateCardNoInput);
    emit updateWorkerCardNum1(rss422No,lvdsNo);
    emit updateWorkerCardNum2(rss422No,lvdsNo);



/*
    /// 下面代码是放进子线程中的槽函数，但是connect(t1, &QThread::started, cardWorker, ...) 中函数体过大
    /// 启动线程时 lambda 里包含了两块 if 判断及调用实际业务逻辑，这块逻辑建议提取到 worker 的成员函数中，保证线程的职责清晰。
//    connect(t1,&QThread::started,cardWorker,[=](){
//        // 获取当前的两个板卡的使能状态，只要是处于使能状态，就开始执行数据接收
//        bool isCard1Start = cardWorker->getLVDSCardStartStatus(true);
//        bool isCard2Start = cardWorker->getLVDSCardStartStatus(false);
//        if(isCard1Start){
//            cardWorker->startLVDS(true);
//        }
//        if(isCard2Start){
//            cardWorker->startLVDS(false);
//        }
//    }); */

    connect(t1,&QThread::started,cardWorker1,&lvdsCardWorker::threadRun);
    connect(t2,&QThread::started,cardWorker2,&lvdsCardWorker::threadRun);

    connect(cardWorker1,&lvdsCardWorker::RunError,this,&MainWindow::runError);
    connect(cardWorker2,&lvdsCardWorker::RunError,this,&MainWindow::runError);

    connect(cardWorker1,&lvdsCardWorker::updateBarStatusSignal,this,&MainWindow::updateStatusBar);
    connect(cardWorker2,&lvdsCardWorker::updateBarStatusSignal,this,&MainWindow::updateStatusBar);


    /*
    笔记：
    invokeMethod() 会把 lambda 函数发送到 cardWorker 所在线程的事件队列中执行；
    BlockingQueuedConnection 表示主线程会等待子线程执行完 lambda 后再继续往下执行；
    使用 [&] 捕获 cardOpenStatus 的引用，让 lambda 中的修改能反映到外部变量上；
    所以这是标准的 线程安全跨线程调用方式。

//    int cardCount = 0;
//    QMetaObject::invokeMethod(cardWorker, [&]() {
//        cardCount = cardWorker->getCardCount();
//    }, Qt::BlockingQueuedConnection); // 以线程安全的方式获取板卡数量
    /// 注意，这里不能使用安全锁，因为此时cardWorker还没有移动到子线程，但是使用BlockingQueuedConnection
    /// 的要求是：QThread::currentThread() ！= cardWorker->thread() ☆☆☆☆☆☆☆☆☆☆☆☆☆
    /// */

    int cardCount = cardWorker1->getCardCount(); //获取板卡数量
    ui->lineEdit_cardCount->setText(QString::number(cardCount));
    ui->lineEdit_cardCount_2->setText(QString::number(cardCount));

    ui->pushButton_cardOpenClose->setCheckable(true);

    ui->pushButton_send->setCheckable(false);
    ui->pushButton_send_2->setCheckable(false);

    /// RS422板卡界面初始化配置
    // 波特率
    ui->comboBox_baudRate_card->addItem("1200",LVDS_BAUD_1200);
    ui->comboBox_baudRate_card->addItem("2400",LVDS_BAUD_2400);
    ui->comboBox_baudRate_card->addItem("4800",LVDS_BAUD_4800);
    ui->comboBox_baudRate_card->addItem("9600",LVDS_BAUD_9600);
    ui->comboBox_baudRate_card->addItem("19200",LVDS_BAUD_19200);
    ui->comboBox_baudRate_card->addItem("38400",LVDS_BAUD_38400);
    ui->comboBox_baudRate_card->addItem("57600",LVDS_BAUD_57600);
    ui->comboBox_baudRate_card->addItem("115200",LVDS_BAUD_115200);
    ui->comboBox_baudRate_card->addItem("128000",LVDS_BAUD_128000);
    ui->comboBox_baudRate_card->addItem("256000",LVDS_BAUD_256000);
    ui->comboBox_baudRate_card->addItem("3000000",3000000);
    ui->comboBox_baudRate_card->addItem("48000000",48000000);
    ui->comboBox_baudRate_card->setCurrentIndex(10); // 默认波特率

    // 数据位
    ui->comboBox_dataBit_card->addItem("1", LVDS_DATABITS_1); // 枚举类型的底层默认是整数（通常是 int 类型）
    ui->comboBox_dataBit_card->addItem("2", LVDS_DATABITS_2); // 枚举值的默认行为是从 0 开始，按顺序自动递增
    ui->comboBox_dataBit_card->addItem("3", LVDS_DATABITS_3); // 如果没有显式指定每个枚举项的值，C++ 编译器会按顺序为每个枚举项分配一个整数值。
    ui->comboBox_dataBit_card->addItem("4", LVDS_DATABITS_4);
    ui->comboBox_dataBit_card->addItem("5", LVDS_DATABITS_5);
    ui->comboBox_dataBit_card->addItem("6", LVDS_DATABITS_6);
    ui->comboBox_dataBit_card->addItem("7", LVDS_DATABITS_7);
    ui->comboBox_dataBit_card->addItem("8", LVDS_DATABITS_8);
    ui->comboBox_dataBit_card->setCurrentIndex(7); // 默认8位

    // 校验位
    ui->comboBox_parity_card->addItem("None", LVDS_PARITY_NO);
    ui->comboBox_parity_card->addItem("Odd", LVDS_PARITY_ODD);
    ui->comboBox_parity_card->addItem("Even", LVDS_PARITY_EVEN);
    ui->comboBox_parity_card->setCurrentIndex(1); // RS422本次项目需要奇校验

    // 停止位
    ui->comboBox_stopBits_card->addItem("1", LVDS_STOPBITS_10);
    ui->comboBox_stopBits_card->addItem("1.5", LVDS_STOPBITS_15);
    ui->comboBox_stopBits_card->addItem("2", LVDS_STOPBITS_20);
    ui->comboBox_stopBits_card->setCurrentIndex(2); // 默认1位停止位

    /// LVDS板卡界面初始化配置
    // 波特率
    ui->comboBox_baudRate_card_2->addItem("1200",LVDS_BAUD_1200);
    ui->comboBox_baudRate_card_2->addItem("2400",LVDS_BAUD_2400);
    ui->comboBox_baudRate_card_2->addItem("4800",LVDS_BAUD_4800);
    ui->comboBox_baudRate_card_2->addItem("9600",LVDS_BAUD_9600);
    ui->comboBox_baudRate_card_2->addItem("19200",LVDS_BAUD_19200);
    ui->comboBox_baudRate_card_2->addItem("38400",LVDS_BAUD_38400);
    ui->comboBox_baudRate_card_2->addItem("57600",LVDS_BAUD_57600);
    ui->comboBox_baudRate_card_2->addItem("115200",LVDS_BAUD_115200);
    ui->comboBox_baudRate_card_2->addItem("128000",LVDS_BAUD_128000);
    ui->comboBox_baudRate_card_2->addItem("256000",LVDS_BAUD_256000);
    ui->comboBox_baudRate_card_2->addItem("3000000",3000000);
    ui->comboBox_baudRate_card_2->addItem("48000000",48000000);
    ui->comboBox_baudRate_card_2->setCurrentIndex(11); // 默认波特率

    // 数据位
    ui->comboBox_dataBit_card_2->addItem("1", LVDS_DATABITS_1); // 枚举类型的底层默认是整数（通常是 int 类型）
    ui->comboBox_dataBit_card_2->addItem("2", LVDS_DATABITS_2); // 枚举值的默认行为是从 0 开始，按顺序自动递增
    ui->comboBox_dataBit_card_2->addItem("3", LVDS_DATABITS_3); // 如果没有显式指定每个枚举项的值，C++ 编译器会按顺序为每个枚举项分配一个整数值。
    ui->comboBox_dataBit_card_2->addItem("4", LVDS_DATABITS_4);
    ui->comboBox_dataBit_card_2->addItem("5", LVDS_DATABITS_5);
    ui->comboBox_dataBit_card_2->addItem("6", LVDS_DATABITS_6);
    ui->comboBox_dataBit_card_2->addItem("7", LVDS_DATABITS_7);
    ui->comboBox_dataBit_card_2->addItem("8", LVDS_DATABITS_8);
    ui->comboBox_dataBit_card_2->setCurrentIndex(7); // 默认8位

    // 校验位
    ui->comboBox_parity_card_2->addItem("None", LVDS_PARITY_NO);
    ui->comboBox_parity_card_2->addItem("Odd", LVDS_PARITY_ODD);
    ui->comboBox_parity_card_2->addItem("Even", LVDS_PARITY_EVEN);
    ui->comboBox_parity_card_2->setCurrentIndex(1); // RS422本次项目需要奇校验

    // 停止位
    ui->comboBox_stopBits_card_2->addItem("1", LVDS_STOPBITS_10);
    ui->comboBox_stopBits_card_2->addItem("1.5", LVDS_STOPBITS_15);
    ui->comboBox_stopBits_card_2->addItem("2", LVDS_STOPBITS_20);
    ui->comboBox_stopBits_card_2->setCurrentIndex(2); // 默认1位停止位

    /// 板卡通道编号
    // RS422
    ui->comboBox_channelNumRS422->addItem("通道0",LVDS_DATA_CH1);
    ui->comboBox_channelNumRS422->addItem("通道1",LVDS_DATA_CH2);
    const int maxChannels = 32;  // 最多 32 个通道
    for (int i = 2; i < maxChannels; ++i) {
        ui->comboBox_channelNumRS422->addItem(QString("通道%1").arg(i), LVDS_DATA_CH1+i);
    }
    ui->comboBox_channelNumRS422->setCurrentIndex(0);

    // LVDS
    ui->comboBox_channelNumLVDS->addItem("通道0",LVDS_DATA_CH1);
    ui->comboBox_channelNumLVDS->addItem("通道1",LVDS_DATA_CH2);
    const int maxChannelsLVDS = 32;  // 最多 32 个通道
    for (int i = 2; i < maxChannelsLVDS; ++i) {
        ui->comboBox_channelNumLVDS->addItem(QString("通道%1").arg(i), LVDS_DATA_CH1+i);
    }
    ui->comboBox_channelNumLVDS->setCurrentIndex(0);

    /// 板卡通道切换
    connect(ui->comboBox_channelNumRS422, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](){
        quint8 channelNumRS422  = static_cast<quint8>(ui->comboBox_channelNumRS422->currentData().toInt());
        cardWorker1->setChannelNum(true, channelNumRS422);
        // emit setChannelNumSignal(true,channelNumRS422);
        qDebug()<<"当前RS422通道号:"<<channelNumRS422;
    }); /// QOverload<int>::of    显式地选择了 currentIndexChanged(int) 信号重载。

    connect(ui->comboBox_channelNumLVDS, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](){
        quint8 channelNumLVDS  = static_cast<quint8>(ui->comboBox_channelNumLVDS->currentData().toInt());
        cardWorker2->setChannelNum(false, channelNumLVDS);
        // emit setChannelNumSignal(false,channelNumLVDS);
        qDebug()<<"当前LVDS通道号:"<<channelNumLVDS;
    }); /// QOverload<int>::of    显式地选择了 currentIndexChanged(int) 信号重载。
//    connect(this,&MainWindow::setChannelNumSignal,cardWorker1,[=](bool chooseCard, quint8 channelNum){
//        cardWorker1->setChannelNum(chooseCard, channelNum);
//    });
//    connect(this,&MainWindow::setChannelNumSignal,cardWorker2,[=](bool chooseCard, quint8 channelNum){
//        cardWorker2->setChannelNum(chooseCard, channelNum);
//    });
    ui->pushButton_saveDataRS422->setCheckable(true);
    ui->pushButton_saveDataLVDS->setCheckable(true);

    /// 开始保存数据到txt
    connect(ui->pushButton_saveDataRS422,&QPushButton::clicked,this,[=](){
        if(ui->pushButton_saveDataRS422->isChecked()){
            // 检查板卡使能状态
            bool cardIsStart = cardWorker1->getLVDSCardStartStatus(true);
            if(cardIsStart){
                ui->pushButton_saveDataRS422->setChecked(true);
                ui->pushButton_saveDataRS422->setText("等待");
                // 获取当前时间作为文件名
                QDateTime currentTime = QDateTime::currentDateTime();
                fileNameRS422 = currentTime.toString("yyyyMMdd_hhmmss") +"RS422"+ ".txt";
                updateStatusBar(true,"准备保存数据");
                ui->pushButton_saveDataRS422->setChecked(true);
                // 查询保存时间
                double saveTime = ui->lineEdit_saveTimeRS422->text().toDouble();
                isSaveDataRs422 = true; // 开始保存
                // 启动定时器，在指定时间后停止保存
                QTimer::singleShot(static_cast<int>(saveTime * 1000), this, [this](){
                    isSaveDataRs422 = false; // 停止保存
                    ui->pushButton_saveDataRS422->setChecked(false);
                    ui->pushButton_saveDataRS422->setText("开始");
                    QMessageBox::information(this, "保存完成", "数据已保存到文件");
                });
            }
            else{
                qmessageBoxSet("RS422板卡未使能！");
                updateStatusBar(true,"RS422板卡未使能！");
                ui->pushButton_saveDataRS422->setChecked(false);
            }
        }
        else{
            qmessageBoxSet("请等待当前数据保存完毕！");
            ui->pushButton_saveDataRS422->setChecked(true);
            ui->pushButton_saveDataRS422->setText("等待");
        }
    });

    /// 开始保存数据到txt
    connect(ui->pushButton_saveDataLVDS,&QPushButton::clicked,this,[=](){
        if(ui->pushButton_saveDataLVDS->isChecked()){
            // 检查板卡使能状态
            bool cardIsStart = cardWorker2->getLVDSCardStartStatus(false);
            if(cardIsStart){
                ui->pushButton_saveDataLVDS->setChecked(true);
                ui->pushButton_saveDataLVDS->setText("等待");
                // 获取当前时间作为文件名
                QDateTime currentTime = QDateTime::currentDateTime();
                fileNameLVDS = currentTime.toString("yyyyMMdd_hhmmss") +"LVDS"+ ".txt";
                updateStatusBar(false,"准备保存数据");
                ui->pushButton_saveDataLVDS->setChecked(true);
                // 查询保存时间
                double saveTime = ui->lineEdit_saveTimeLVDS->text().toDouble();
                isSaveDataLVDS = true; // 开始保存
                // 启动定时器，在指定时间后停止保存
                QTimer::singleShot(static_cast<int>(saveTime * 1000), this, [this](){
                    isSaveDataLVDS = false; // 停止保存
                    ui->pushButton_saveDataLVDS->setChecked(false);
                    ui->pushButton_saveDataLVDS->setText("开始");
                    QMessageBox::information(this, "保存完成", "数据已保存到文件");
                });
            }
            else{
                qmessageBoxSet("LVDS板卡未使能！");
                updateStatusBar(false,"LVDS板卡未使能！");
                ui->pushButton_saveDataLVDS->setChecked(false);
            }
        }
        else{
            qmessageBoxSet("请等待当前数据保存完毕！");
            ui->pushButton_saveDataLVDS->setChecked(true);
            ui->pushButton_saveDataLVDS->setText("等待");
        }
    });

    // 保存超阈值点元素坐标
    connect(ui->checkBox_overGrayScaleCoordinates,&QCheckBox::toggled,this, [this](bool checked){
       emit updateSaveCoordinateSignal(checked);
    });
    connect(this,&MainWindow::updateSaveCoordinateSignal,dataAnalysisHelperLVDS,&DataAnalysisHelper::updateSaveCoordinate,Qt::DirectConnection); // 要排队
//    connect(this,&MainWindow::updateSaveCoordinateSignal,dataAnalysisHelper2,&DataAnalysisHelper::updateSaveCoordinate,Qt::DirectConnection); //  临时测试


    saveDirTM = QCoreApplication::applicationDirPath() + "/TMData/Fragment/";
    saveDirTMAuto = QCoreApplication::applicationDirPath() + "/TMData/Auto/";
    QDir SaveDirTM(saveDirTM);
    if(!SaveDirTM.exists()){
        if(!SaveDirTM.mkpath(".")){
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirTM);
            return;
        }
    }
    QDir SaveDirAuto(saveDirTMAuto);
    if(!SaveDirAuto.exists()){
        if(!SaveDirAuto.mkpath(".")){
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirTMAuto);
            return;
        }
    }

    m_saveTimer = new QTimer(this);
    m_saveTimer->setSingleShot(true);

    /// 保存遥测数据到excel文件
    connect(ui->pb_begin_save_data,&QPushButton::clicked,this,[=](){
        if(ui->pb_begin_save_data->text() == "停止"){
            ui->pb_begin_save_data->setChecked(false);
            ui->pb_begin_save_data->setText("开始");
            isSaveTMFile = false;
            on_stopSaveButton_clicked(); // 提前终止定时器
            if(ui->rb_save_data_auto->isChecked()){
                qmessageBoxSet("自动保存遥测数据终止");
            }
            return;
        }
        else {
            ui->pb_begin_save_data->setChecked(true);
            ui->pb_begin_save_data->setText("停止");
            isSaveTMFile = true;
        }

        // 手动保存
        if(ui->rb_save_data_manual->isChecked()){
            // 获取保存时长
            QString  saveTimeString = ui->sb_time->text();
            saveTimeString.remove(QRegExp("[^0-9]"));  // 删除所有非数字字符
            int saveTime = saveTimeString.toInt();
            // 设置文件名 // 获取当前系统时间
            QDateTime currentTime = QDateTime::currentDateTime();
            saveFileNameTm = saveDirTM + currentTime.toString("yyyyMMdd_hhmmss") +saveTimeString+ ".csv";

            // 创建csv文件并设置标题行
            QFile file(saveFileNameTm);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);

                // 你自己定义的标题行，例如三列
                QStringList headers;
                for (int i = 0; i<modelMain->rowCount();++i) {
                    QStandardItem *it = modelMain->item(i, 1);  // 第二列
                    headers << it->text();
                }
                // 写入标题行（用逗号分隔）
                out << headers.join(",") << "\n";
                file.close();
            }
            else {
                qDebug() << "无法创建文件:" << saveFileNameTm;
                qmessageBoxSet("无法创建遥测CSV文件");
            }

            // 打开定时器,定时器开始期间，isSaveTMFile一直保持为true
            // 启动定时器，在指定时间后停止保存
            connect(m_saveTimer, &QTimer::timeout, this, [this]() {
                isSaveTMFile = false; // 停止保存
                ui->pb_begin_save_data->setChecked(false);
                ui->pb_begin_save_data->setText("开始");
                QMessageBox::information(this, "保存完成", "数据已保存到文件");
                m_saveTimer->deleteLater(); // 定时器完成任务后清理
                m_saveTimer = nullptr;
            });
            if(m_saveTimer){
                m_saveTimer->start(static_cast<int>(saveTime * 1000));
            }
        }

        // 自动保存
        if(ui->rb_save_data_auto->isChecked()){
            // 设置文件名 // 获取当前系统时间
            QDateTime currentTimeAuto = QDateTime::currentDateTime();
            saveFileNameTmAuto = saveDirTMAuto + currentTimeAuto.toString("yyyyMMdd_hhmmss") +"Auto"+ ".csv";

            // 创建csv文件并设置标题行
            QFile file(saveFileNameTmAuto);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);

                // 你自己定义的标题行，例如三列
                QStringList headers;
                for (int i = 0; i<modelMain->rowCount();++i) {
                    QStandardItem *it = modelMain->item(i, 1);  // 第二列
                    headers << it->text();
                }
                // 写入标题行（用逗号分隔）
                out << headers.join(",") << "\n";
                file.close();
            }
            else {
                qDebug() << "无法创建文件:" << saveFileNameTmAuto;
                qmessageBoxSet("无法创建遥测CSV文件");
            }
        }

    });

    /// 打开板卡
    connect(ui->pushButton_cardOpenClose,&QPushButton::clicked,this,[=](){
        cardOpenBtn(true); // 打开rs422板卡
    });
    connect(ui->pushButton_cardOpenClose_2,&QPushButton::clicked,this,[=](){
        cardOpenBtn(false); // 打开LVDS板卡
    });

    /// 复位板卡
    connect(ui->pushButton_reset,&QPushButton::clicked,this,[=](){
        bool resetSign = cardWorker1->resetCard(true);
        if(resetSign){
            updateStatusBar(true,"RS422板卡复位成功！");
        }
        else{
            qmessageBoxSet("RS422板卡复位失败！"); //复位失败，弹出警告窗口
        }
    });
    connect(ui->pushButton_reset_2,&QPushButton::clicked,this,[=](){
        bool resetSign = cardWorker2->resetCard(true);
        if(resetSign){
            updateStatusBar(false,"LVDS板卡复位成功！");
        }
        else{
            qmessageBoxSet("LVDS板卡复位失败！");
        }
    });

    /// 清空缓存
    connect(ui->pushButton_clearFIFO,&QPushButton::clicked,this,[=](){
        bool clearSign = cardWorker1->clearFIFO(true);
        if(clearSign){
            updateStatusBar(true,"RS422板卡清空缓存成功！");
        }
        else{
            qmessageBoxSet("RS422板卡清空缓存失败！");
        }
    });
    connect(ui->pushButton_clearFIFO_2,&QPushButton::clicked,this,[=](){
        bool clearSign = cardWorker2->clearFIFO(false);
        if(clearSign){
            updateStatusBar(false,"LVDS板卡清空缓存成功！");
        }
        else{
            qmessageBoxSet("LVDS板卡清空缓存失败！");
        }
    });

    /// 清空文本框
    // 板卡RS422
    connect(ui->pushButton_clearSend,&QPushButton::clicked,ui->plainTextEdit_send,&QPlainTextEdit::clear);
    connect(ui->pushButton_clearReceive,&QPushButton::clicked,ui->plainTextEdit_receive,&QPlainTextEdit::clear);
    // 板卡LVDS
    connect(ui->pushButton_clearSend_2,&QPushButton::clicked,ui->plainTextEdit_send_2,&QPlainTextEdit::clear);
    connect(ui->pushButton_clearReceive_2,&QPushButton::clicked,ui->plainTextEdit_receive_2,&QPlainTextEdit::clear);
    // CAN
    connect(ui->pushButton_clearSend_3,&QPushButton::clicked,ui->plainTextEdit_send_3,&QPlainTextEdit::clear);
    connect(ui->pushButton_clearReceive_3,&QPushButton::clicked,ui->plainTextEdit_receive_3,&QPlainTextEdit::clear);

    /// 板卡数据发送
    // RS422
    connect(ui->pushButton_send,&QPushButton::clicked,this,[=](){
        // 准备数据
        QString tempStringData = ui->plainTextEdit_send->toPlainText();
        QByteArray byteDataSend = dataTransfer->string2Bytearray(tempStringData); // byteDataSend： "\x00\x01\x02\x03\x04\x05\x06\x07"
        qDebug()<<"RS422 byteDataSend："<<byteDataSend;
        // 检查板卡是否使能
        bool cardOpenStatus = cardWorker1->getLVDSCardOpenStatus(true);
        if(!cardOpenStatus){
            qmessageBoxSet("发送失败，RS422板卡未打开！");
        }
        else {
            // 获取通道数
            quint8 channel = cardWorker1->getChannelNum(true);
            DWORD dwSendRet = cardWorker1->sendData(true,channel, reinterpret_cast<quint8*>(byteDataSend.data()), static_cast<size_t>(byteDataSend.length())); // 新的写法
            qDebug()<<"channel"<<channel;
            qDebug()<<"dwSendRet"<<dwSendRet;
            ///QByteArray::data() 返回指向数据缓冲区的 char* 指针（char*）,这个指针指向 byteDataSend 内部的原始数据。
            /// (quint8*)byteDataSend.data() 只是一个类型转换，不会改变数据本身，只是告诉编译器 把这个指针当作 unsigned char* 来使用
            /// reinterpret_cast<quint8*>(byteDataSend.data())  // 指针类型转换,它不会修改数据，而是告诉编译器“把这个指针当成另一种类型的指针来用”。
            /// static_cast<size_t>(byteDataSend.length())      // 数值类型转换,适用于安全的数值转换
        }
    });

    // LVDS
    connect(ui->pushButton_send_2,&QPushButton::clicked,this,[=](){
        // 准备数据
        QString tempStringData = ui->plainTextEdit_send_2->toPlainText();
        QByteArray byteDataSend = dataTransfer->string2Bytearray(tempStringData); // byteDataSend： "\x00\x01\x02\x03\x04\x05\x06\x07"
        qDebug()<<"LVDS byteDataSend："<<byteDataSend;
        // 检查板卡是否使能
        bool cardOpenStatus = cardWorker2->getLVDSCardOpenStatus(false);
        if(!cardOpenStatus){
            qmessageBoxSet("发送失败，LVDS板卡未打开！");
        }
        else {
            // 获取通道数
            quint8 channel = cardWorker2->getChannelNum(false);
            DWORD dwSendRet = cardWorker2->sendData(false,channel, reinterpret_cast<quint8*>(byteDataSend.data()), static_cast<size_t>(byteDataSend.length())); // 新的写法
            qDebug()<<"channel"<<channel;
            qDebug()<<"dwSendRet"<<dwSendRet;
        }
    });


    // 软件打开时就要检查当前两个板卡的打开和使能状态，如果板卡已经处于使能状态，直接将板卡使能按钮设置成点击后的状态
    bool cardOpenStatusRS422 = cardWorker1->getLVDSCardOpenStatus(true);
    statusBarShow(true, "RS422板卡打开状态：" + QString::number(cardOpenStatusRS422));
    if(cardOpenStatusRS422){ // 如果板卡处于打开状态
        ui->pushButton_cardOpenClose->setChecked(true);
        ui->pushButton_cardOpenClose->setText("关闭");
    }
    // 检查板卡使能状态
    bool startLVDSStatusRS422 = cardWorker1->getLVDSCardStartStatus(true);
    statusBarShow(true, "RS422板卡使能状态：" + QString::number(startLVDSStatusRS422));
    if(startLVDSStatusRS422){ // 如果板卡处于使能状态
        ui->pushButton_startLVDS->setChecked(true);
        ui->pushButton_startLVDS->setText("停止");
    }

    bool cardOpenStatusLVDS = cardWorker2->getLVDSCardOpenStatus(false);
    statusBarShow(false, "LVDS板卡打开状态：" + QString::number(cardOpenStatusLVDS));
    if(cardOpenStatusLVDS){ // 如果板卡处于打开状态
        ui->pushButton_cardOpenClose_2->setChecked(true);
        ui->pushButton_cardOpenClose_2->setText("关闭");
    }
    // 检查板卡使能状态
    bool startLVDSStatusLVDS = cardWorker2->getLVDSCardStartStatus(false);
    statusBarShow(false, "LVDS板卡使能状态：" + QString::number(startLVDSStatusLVDS));
    if(startLVDSStatusLVDS){ // 如果板卡处于使能状态
        ui->pushButton_startLVDS_2->setChecked(true);
        ui->pushButton_startLVDS_2->setText("停止");
    }

    /// 板卡使能
    /// rs422板卡使能
    connect(ui->pushButton_startLVDS,&QPushButton::clicked,this,[=](){
        // 检查板卡打开状态
        bool cardOpenStatus = cardWorker1->getLVDSCardOpenStatus(true);
        statusBarShow(true, "RS422板卡打开状态：" + QString::number(cardOpenStatus));
        if(cardOpenStatus){ // 如果板卡处于打开状态
            // 检查板卡使能状态
            bool startLVDSStatus = cardWorker1->getLVDSCardStartStatus(true);
            if(startLVDSStatus){ // 如果板卡处于使能状态
                // 执行板卡去使能
                bool isStartCheck = cardWorker1->StopCard(true);
                if(isStartCheck) { // 如果去使能成功
                    statusBarShow(true,"RS422板卡去使能成功！");
                    ui->pushButton_startLVDS->setChecked(false);
                    ui->pushButton_startLVDS->setText("使能");
                    // 停止t1
                    qDebug() << "Thread t1 state before quit:" << t1->isRunning();
                    t1->quit();      // 请求线程退出
                    t1->wait();
                    qDebug() << "Thread t1 state after quit:" << t1->isRunning();
                }
                else{ // 如果去使能失败
                    statusBarShow(true,"RS422板卡去使能失败！");
                    ui->pushButton_startLVDS->setChecked(true);
                    ui->pushButton_startLVDS->setText("停止");
                }
            }
            else{
                // 执行板卡使能
                bool isStartCheck = cardWorker1->StartCard(true);
                if(isStartCheck){ // 如果使能成功
                    statusBarShow(true,"RS422板卡使能成功！");
                    ui->pushButton_startLVDS->setChecked(true);
                    ui->pushButton_startLVDS->setText("停止");
                    // 启动t1
                    t1->start();
                }
                else{ // 如果使能失败
                    ui->pushButton_startLVDS->setChecked(false);
                    ui->pushButton_startLVDS->setText("使能");
                    statusBarShow(true,"RS422板卡使能失败！");
                }
            }
        }
        else{ // 如果板卡处于关闭状态
            qmessageBoxSet("RS422板卡未启动！请检查");
        }
    });

    /// LVDS板卡使能
    connect(ui->pushButton_startLVDS_2,&QPushButton::clicked,this,[=](){
        // 检查板卡打开状态
        bool cardOpenStatus = cardWorker2->getLVDSCardOpenStatus(false);
        statusBarShow(false, "LVDS板卡打开状态：" + QString::number(cardOpenStatus));
        if(cardOpenStatus){ // 如果板卡处于打开状态
            // 检查板卡使能状态
            bool startLVDSStatus = cardWorker2->getLVDSCardStartStatus(false);
            if(startLVDSStatus){ // 如果板卡处于使能状态
                // 执行板卡去使能
                bool isStartCheck = cardWorker2->StopCard(false);
                if(isStartCheck) { // 如果去使能成功
                    statusBarShow(false,"LVDS板卡去使能成功！");
                    ui->pushButton_startLVDS_2->setChecked(false);
                    ui->pushButton_startLVDS_2->setText("使能");
                    // 停止t2
                    qDebug() << "Thread t2 state before quit:" << t2->isRunning();
                    t2->quit();      // 请求线程退出
                    t2->wait();
                    qDebug() << "Thread t2 state after quit:" << t2->isRunning();
                }
                else{ // 如果去使能失败
                    statusBarShow(false,"LVDS板卡去使能失败！");
                    ui->pushButton_startLVDS_2->setChecked(false);
                    ui->pushButton_startLVDS_2->setText("停止");
                }
            }
            else{
                // 执行板卡使能
                bool isStartCheck = cardWorker2->StartCard(false);
                if(isStartCheck){ // 如果使能成功
                    statusBarShow(false,"LVDS板卡使能成功！");
                    ui->pushButton_startLVDS_2->setChecked(false);
                    ui->pushButton_startLVDS_2->setText("停止");
                    // 启动t2
                    t2->start();
                }
                else{ // 如果使能失败
                    ui->pushButton_startLVDS_2->setChecked(false);
                    ui->pushButton_startLVDS_2->setText("使能");
                    statusBarShow(false,"LVDS板卡使能失败！");
                }
            }
        }
        else{ // 如果板卡处于关闭状态
            qmessageBoxSet("LVDS板卡未启动！请检查");
        }
    });

    // 加载xml指令序列
    connect(xmlcommandparser,&XMLCommandParser::updateParaListSignal,this,[=](QVector<CommandQueue> commandQueueVector){
        QTableWidget* targetTable = ui->tableWidget_cmd_immediately;
        targetTable->setRowCount(commandQueueVector.count());
        for(int i=0;i<commandQueueVector.count();++i){
            // 创建列表项目
            QTableWidgetItem* timeItem = new QTableWidgetItem(commandQueueVector.at(i).commandTime);
            targetTable->setItem(i, 0, timeItem);
            QTableWidgetItem* nameItem = new QTableWidgetItem(commandQueueVector.at(i).commandName);
            targetTable->setItem(i, 1, nameItem);
            QTableWidgetItem* codeItem = new QTableWidgetItem(commandQueueVector.at(i).commandCode);
            targetTable->setItem(i, 2, codeItem);
            QTableWidgetItem* remarksItem = new QTableWidgetItem(commandQueueVector.at(i).commandRemarks);
            targetTable->setItem(i, 3, remarksItem);
        }
    });

    modelMain = new QStandardItemModel(this);
    modelInterest = new QStandardItemModel(this);
    ui->tableView->setModel(modelMain);
    ui->tableView_2->setModel(modelInterest);

    modelMain_slow = new QStandardItemModel(this);
    modelInterest_slow = new QStandardItemModel(this);
    ui->tableView_3_slow->setModel(modelMain_slow);
    ui->tableView_4_slow->setModel(modelInterest_slow);

    modelMain->setHorizontalHeaderLabels({"参数编号", "参数名称", "数值", "源码", "有效性", "是否关注"});
    modelInterest->setHorizontalHeaderLabels({"参数编号", "参数名称", "数值", "源码", "有效性"});
    modelMain_slow->setHorizontalHeaderLabels({"参数编号", "参数名称", "数值", "源码", "有效性", "是否关注"});
    modelInterest_slow->setHorizontalHeaderLabels({"参数编号", "参数名称", "数值", "源码", "有效性"});

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // 每列宽度会按内容调整
//    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
//    ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
//    ui->tableView->setColumnWidth(2, 120);
//    ui->tableView->setColumnWidth(3, 120);
    ui->tableView->horizontalHeader()->setStretchLastSection(true); // 最后一列会拉伸填满剩余空间

    ui->tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // 每列宽度会按内容调整
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
//    ui->tableView_2->setColumnWidth(2, 120);
//    ui->tableView_2->setColumnWidth(3, 120);
    ui->tableView_2->horizontalHeader()->setStretchLastSection(true); // 最后一列会拉伸填满剩余空间

    ui->tableView_3_slow->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // 每列宽度会按内容调整
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
//    ui->tableView_2->setColumnWidth(2, 120);
//    ui->tableView_2->setColumnWidth(3, 120);
    ui->tableView_3_slow->horizontalHeader()->setStretchLastSection(true); // 最后一列会拉伸填满剩余空间

    ui->tableView_4_slow->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // 每列宽度会按内容调整
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
//    ui->tableView_2->setColumnWidth(2, 120);
//    ui->tableView_2->setColumnWidth(3, 120);
    ui->tableView_4_slow->horizontalHeader()->setStretchLastSection(true); // 最后一列会拉伸填满剩余空间

    ui->tableView->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: rgba(255,255,255,40); }");
    ui->tableView->verticalHeader()->setStyleSheet("QHeaderView::section { background-color: rgba(255,255,255,40); }");

    ui->tableView_2->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: rgba(255,255,255,40); }");
    ui->tableView_2->verticalHeader()->setStyleSheet("QHeaderView::section { background-color: rgba(255,255,255,40); }");

    ui->tableView_3_slow->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: rgba(255,255,255,40); }");
    ui->tableView_3_slow->verticalHeader()->setStyleSheet("QHeaderView::section { background-color: rgba(255,255,255,40); }");

    ui->tableView_4_slow->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: rgba(255,255,255,40); }");
    ui->tableView_4_slow->verticalHeader()->setStyleSheet("QHeaderView::section { background-color: rgba(255,255,255,0); }");



    connect(xmlcommandparser,&XMLCommandParser::updateTMParaListSignal,this,[=](QVector<TMitemQueue> paraQueueVector){

        dataAnalysisHelperRS422->getXmlParser(paraQueueVector);
        dataAnalysisHelper2->getXmlParser(paraQueueVector); // 临时测试用
    });

    connect(xmlcommandparser,&XMLCommandParser::updateTMParaListSignal,this,[=](QVector<TMitemQueue> paraQueueVector){
        dataAnalysisHelperCAN->getXmlParser(paraQueueVector);
    });
    connect(xmlcommandparser,&XMLCommandParser::updateTMParaListSignal_slow,this,[=](QVector<TMitemQueue> paraQueueVector_slow){
        dataAnalysisHelperCAN->getXmlParser_slow(paraQueueVector_slow);
    });

    connect(xmlcommandparser,&XMLCommandParser::updateFrameTMSignal,this,[=](FrameTM &frameTmXML){

        //获取帧信息
        // ①帧长度
//        qDebug()<<"frameTm.frameLength"<<frameTmXML.frameLength;
//        int frameLength = frameTmXML.frameLength;
        // ②帧头
    //    QString frameHead = frameTm.frameHead;
        // 转成 QByteArray
//        dataAnalysisHelperCAN->headFrame = QByteArray::fromHex(frameTmXML.frameHead.toUtf8());
//        dataAnalysisHelperCAN->tailFrame = QByteArray::fromHex(frameTmXML.frameTail.toUtf8());

        QByteArray ba_frameHead = QByteArray::fromHex(frameTmXML.frameHead.toUtf8());
        QByteArray ba_frameHeadAll = QByteArray::fromHex(frameTmXML.frameHeadAll.toUtf8());

        quint8 h0_xml = static_cast<quint8>(ba_frameHead[0]);
        quint8 h1_xml = static_cast<quint8>(ba_frameHead[1]);
        qDebug()<<"h0_xml"<<h0_xml;
        qDebug()<<"h1_xml"<<h1_xml;

        quint8 h0_xml_all = static_cast<quint8>(ba_frameHeadAll[0]);
        quint8 h1_xml_all = static_cast<quint8>(ba_frameHeadAll[1]);
        qDebug()<<"h0_xml_all&&&&&&&&&&&&&"<<h0_xml_all;
        qDebug()<<"h1_xml_all&&&&&&"<<h1_xml_all;

        QByteArray ba_frameTail = QByteArray::fromHex(frameTmXML.frameTail.toUtf8());
        quint8 t0_xml = static_cast<quint8>(ba_frameTail[0]);
        quint8 t1_xml = static_cast<quint8>(ba_frameTail[1]);
        qDebug()<<"t0_xml"<<t0_xml;
        qDebug()<<"t1_xml"<<t1_xml;

        dataAnalysisHelper2->h0_xml = h0_xml; // 临时测试用
        dataAnalysisHelper2->h1_xml = h1_xml; // 临时测试用
        dataAnalysisHelper2->t0_xml = t0_xml; // 临时测试用
        dataAnalysisHelper2->t1_xml = t1_xml; // 临时测试用
        dataAnalysisHelper2->lengthHead = ba_frameHead.size(); // 临时测试用
        dataAnalysisHelper2->lengthTail = ba_frameTail.size(); // 临时测试用

        dataAnalysisHelperRS422->h0_xml = h0_xml;
        dataAnalysisHelperRS422->h1_xml = h1_xml;
        dataAnalysisHelperRS422->t0_xml = t0_xml;
        dataAnalysisHelperRS422->t1_xml = t1_xml;
        dataAnalysisHelperRS422->lengthHead = ba_frameHead.size() +2;
        dataAnalysisHelperRS422->lengthTail = ba_frameTail.size();

        dataAnalysisHelperCAN->h0_xml_fast = h0_xml;
        dataAnalysisHelperCAN->h1_xml_fast = h1_xml; 
        dataAnalysisHelperCAN->h0_xml_all = h0_xml_all;
        dataAnalysisHelperCAN->h1_xml_all = h1_xml_all;
        dataAnalysisHelperCAN->t0_xml_fast = t0_xml;
        dataAnalysisHelperCAN->t1_xml_fast = t1_xml;
        dataAnalysisHelperCAN->t0_xml_all = t0_xml;
        dataAnalysisHelperCAN->t1_xml_all = t1_xml;
        dataAnalysisHelperCAN->lengthHead_fast = ba_frameHead.size()+2;
        dataAnalysisHelperCAN->lengthTail_fast = ba_frameTail.size();
        dataAnalysisHelperCAN->lengthHead_all = ba_frameHeadAll.size()+2;
        dataAnalysisHelperCAN->lengthTail_all = ba_frameTail.size();


        dataAnalysisHelperCAN->frameTm.CutFront = frameTmXML.CutFront;
        // ④校验和位数
    //    int checkSumNum = frameTm.checkSumNum;

        qDebug()<<"开始更新frameTm";
        qDebug()<<"frameTm.frameHead"<<frameTmXML.frameHead;
        qDebug()<<"frameTm.frameTail"<<frameTmXML.frameTail;
        qDebug()<<"frameTm.frameHeadAll"<<frameTmXML.frameHeadAll;

        dataAnalysisHelper2->frameTm = frameTmXML; // 临时测试用
        dataAnalysisHelperRS422->frameTm = frameTmXML;
        dataAnalysisHelperCAN->frameTm = frameTmXML;

//        qDebug()<<"frameTm.frameHead"<<dataAnalysisHelper2->frameTm.frameHead;
//        qDebug()<<"frameTm.frameTail"<<dataAnalysisHelper2->frameTm.frameTail;

    });

    connect(xmlcommandparser,&XMLCommandParser::updateFrameTMSignal_slow,this,[=](FrameTM &frameTmXML){


        QByteArray ba_frameHead = QByteArray::fromHex(frameTmXML.frameHead.toUtf8());

        quint8 h0_xml = static_cast<quint8>(ba_frameHead[0]);
        quint8 h1_xml = static_cast<quint8>(ba_frameHead[1]);
        qDebug()<<"h0_xml"<<h0_xml;
        qDebug()<<"h1_xml"<<h1_xml;

        QByteArray ba_frameTail = QByteArray::fromHex(frameTmXML.frameTail.toUtf8());
        quint8 t0_xml = static_cast<quint8>(ba_frameTail[0]);
        quint8 t1_xml = static_cast<quint8>(ba_frameTail[1]);
        qDebug()<<"t0_xml"<<t0_xml;
        qDebug()<<"t1_xml"<<t1_xml;


        dataAnalysisHelperCAN->h0_xml_slow = h0_xml;
        dataAnalysisHelperCAN->h1_xml_slow = h1_xml;
        dataAnalysisHelperCAN->t0_xml_slow = t0_xml;
        dataAnalysisHelperCAN->t1_xml_slow = t1_xml;
        dataAnalysisHelperCAN->lengthHead_slow = ba_frameHead.size()+2;
        dataAnalysisHelperCAN->lengthTail_slow = ba_frameTail.size();


        // ④校验和位数
    //    int checkSumNum = frameTm.checkSumNum;

        qDebug()<<"开始更新frameTm";
        qDebug()<<"frameTm.frameHead"<<frameTmXML.frameHead;
        qDebug()<<"frameTm.frameTail"<<frameTmXML.frameTail;

        dataAnalysisHelperCAN->frameTm_slow = frameTmXML;

//        qDebug()<<"frameTm.frameHead"<<dataAnalysisHelper2->frameTm.frameHead;
//        qDebug()<<"frameTm.frameTail"<<dataAnalysisHelper2->frameTm.frameTail;

    });


    // 加载xml遥测序列
    connect(xmlcommandparser,&XMLCommandParser::updateTMParaListSignal,this,[=](QVector<TMitemQueue> paraQueueVector){
        qDebug() << "connect(xmlcommandparser,&XMLCommandParser::updateTMParaListSignal,this,[" ;
        int paraNum = paraQueueVector.size();

        // 第一列
        for(int i = 0; i <paraNum ; ++i){
            modelMain->setItem(i, 0, new QStandardItem(paraQueueVector.at(i).TMitemNo));
        }
        // 第二列
        for(int i = 0; i <paraNum ; ++i){
            modelMain->setItem(i, 1, new QStandardItem(paraQueueVector.at(i).TMitemName));
        }
        // 第三列
        for(int i = 0; i <paraNum ; ++i){
            modelMain->setItem(i, 2, new QStandardItem("暂无数据"));
        }
        // 第四列
        for(int i = 0; i <paraNum ; ++i){
            modelMain->setItem(i, 3, new QStandardItem("暂无数据"));
        }
        // 第五列
        for(int i = 0; i <paraNum ; ++i){
            modelMain->setItem(i, 4, new QStandardItem("是"));
        }
        // 第六列
        for(int i = 0; i <paraNum ; ++i){
            QStandardItem *item = new QStandardItem();
            item->setCheckable(true);            // 设置成复选框
            item->setCheckState(Qt::Unchecked);  // 默认不勾选
            modelMain->setItem(i, 5, item);
        }
    });

    // 加载xml遥测序列
    connect(xmlcommandparser,&XMLCommandParser::updateTMParaListSignal_slow,this,[=](QVector<TMitemQueue> paraQueueVector){
        qDebug() << "connect(xmlcommandparser,&XMLCommandParser::updateTMParaListSignal,this,[" ;
        int paraNum = paraQueueVector.size();

        // 第一列
        for(int i = 0; i <paraNum ; ++i){
            modelMain_slow->setItem(i, 0, new QStandardItem(paraQueueVector.at(i).TMitemNo));
        }
        // 第二列
        for(int i = 0; i <paraNum ; ++i){
            modelMain_slow->setItem(i, 1, new QStandardItem(paraQueueVector.at(i).TMitemName));
        }
        // 第三列
        for(int i = 0; i <paraNum ; ++i){
            modelMain_slow->setItem(i, 2, new QStandardItem("暂无数据"));
        }
        // 第四列
        for(int i = 0; i <paraNum ; ++i){
            modelMain_slow->setItem(i, 3, new QStandardItem("暂无数据"));
        }
        // 第五列
        for(int i = 0; i <paraNum ; ++i){
            modelMain_slow->setItem(i, 4, new QStandardItem("是"));
        }
        // 第六列
        for(int i = 0; i <paraNum ; ++i){
            QStandardItem *item = new QStandardItem();
            item->setCheckable(true);            // 设置成复选框
            item->setCheckState(Qt::Unchecked);  // 默认不勾选
            modelMain_slow->setItem(i, 5, item);
        }
    });

//    /// xml：注意，readTmXML()内部会发送很多信号，如果槽函数是在readTmXML(xmlTmPath);之后创建的，那么这些信号发送时，槽函数都不会有反应，因为此时还没有创建连接
//    readTmXML(xmlTmPath);

    connect(modelMain, &QStandardItemModel::itemChanged, this, [=](QStandardItem *item){
        if (item->column() == 5) {  // 只处理第6列（复选框列）
            int row = item->row();
            if (item->checkState() == Qt::Checked) {
                addRowToInterestTable(row);
            } else {
                removeRowFromInterestTable(row);
            }
        }
    });

    connect(modelMain_slow, &QStandardItemModel::itemChanged, this, [=](QStandardItem *item){
        if (item->column() == 5) {  // 只处理第6列（复选框列）
            int row = item->row();
            if (item->checkState() == Qt::Checked) {
                addRowToInterestTable_slow(row);
            } else {
                removeRowFromInterestTable_slow(row);
            }
        }
    });



    // 加载xml文件
    // 循环查找软件路径中的XmlFiles/CMD中的xml文件

    QDir dir(CMDXmlFilesPath);
    QStringList xmlFiles = dir.entryList(QStringList() << "*.xml", QDir::Files);
    qDebug()<<"xmlFiles.size"<<xmlFiles.size(); // 1

    for (int i = 0; i < xmlFiles.size(); ++i) {
//        qDebug() <<"xmlFiles.at(i).size()"<<xmlFiles.at(i).size(); // 字符串长度
//        qDebug() <<"xmlFiles.size()"<<xmlFiles.size(); // xml文件数量
//        qDebug() <<"xmlFiles.at(i)"<<xmlFiles.at(i); //xml文件名
        readXML(CMDXmlFilesPath + xmlFiles.at(i),true);
    }

//    readXML(xmlPath,true);
//    readXML(xmlPathVisibleLight,true);

    // 更新ui界面指令组显示
    ui->lb_current_cmd_name->setText(commandList.at(0)); // 默认显示第一个指令组的名称

    /// 发送立即令列表
    connect(ui->pb_add_cmd,&QPushButton::clicked,this,[=](){
        addCMDList(false);
    }); // 添加指令序列
    connect(ui->pb_insert_cmd,&QPushButton::clicked,this,[=](){
        addCMDList(true);
    }); // 插入指令序列
    connect(ui->pb_send_cmd_immediately,&QPushButton::clicked,this,[=](){
        sendCMD(0,true);
    }); // 发送立即令
    connect(ui->pb_del_cmd,&QPushButton::clicked,this,&MainWindow::delCMDList); // 删除指令序列
    connect(ui->pb_clear_cmd,&QPushButton::clicked,this,&MainWindow::clearCMDList); // 清空指令序列
//    tableWidget_cmd_immediately

    // 按顺序和时间戳发送指令序列
    connect(ui->pb_send_cmd,&QPushButton::clicked,this,[=](){

        // 清除可能存在的进行中任务
         if (m_SendRow  >= 0) {
             qWarning() << "Command sequence is already running";
             return;
         }

        // 获取表格tableWidget_cmd_immediately，按照顺序和时间戳发送指令
        // 实际上我只需要按照时间戳改变当前选取的索引号，然后紧接着运行sendCMD()即可
        // ①获取列表行数
         const int rowCount = ui->tableWidget_cmd_immediately->rowCount();
         if (rowCount == 0) return;
         qDebug()<<"rowCount"<<rowCount;

        // ②获取第二列时间戳并存进数组
         m_timeDelays.clear();
         for(int i = 0 ; i < rowCount ; i++){
             int timeDelay = ui->tableWidget_cmd_immediately->item(i,0)->text().toInt();
             qDebug()<<"timeDelay"<<timeDelay;
             m_timeDelays.append(timeDelay);
         }

         for(int i = 0 ; i < rowCount ; i++){
             int delay = m_timeDelays[i];  // 拷贝当前的 i 对应的延迟
             int delayIndex = i;
             QTimer::singleShot(delay * 1000, this, [this, delayIndex]() {
                 sendCMD(delayIndex,false);
             });
             /// 笔记：① 上述lambda 捕获列表里不能写成员变量，所以使用int delayIndex = i;也就是局部变量，如果想访问成员函数或成员变量，可以捕获 [this]
             /// ②绝对不能在for循环中使用sleep（）来实现延迟，这会阻塞主线程，正确的方式是使用QTimer定时器，原理是在定时器被打开时，并不会阻塞for循环，而是这个任务
             /// 被for循环放在一边计时了，for循环继续去完成后续任务。换句话说：避免在循环中使用 sleep() 实现延迟，这会阻塞主线程，应该使用 QTimer::singleShot 等事件驱动方式
             /// ③问题来了，定时任务可能还没有到时间执行，for循环新的一轮中就会重置delayIndex这个局部变量， 即使使用成员变量，也没用，因为上一个定时任务还没有用上这个变量，
             /// 这个变量就在新的一轮中被修改了，也就是所有的迭代中共享了同一个delayIndex，这就会导致最终可能所有定时器都完成的是同一个任务。
             /// 解决方案：使用lambda函数捕获局部变量，为每一个迭代的中的定时任务赋予独一无二的delayIndex
         }

//        m_SendRow = -1; // 每当指令序列发送完毕后m_SendRow重置为-1
    });

    // 启动程序时就设置checkBox的默认选项，也就是指令默认发送通道
    if (ui->listWidget_2->count() > 0) {
        ui->listWidget_2->setCurrentRow(0); // 默认选中第一个
        onTabItemClicked_2(ui->listWidget_2->item(0)); // 直接调用槽
    }

    /*
//    // 更新标志位槽函数
//    connect(this,&MainWindow::updateIsT1StartSignal,cardWorker,[=](bool chooseCard, bool isT1Start){
//        cardWorker->updateIsT1Start(chooseCard,isT1Start);
//    });

//    connect(cardWorker,&lvdsCardWorker::stopThreadRec,this,[=](){
//        qDebug() << "Thread t1 state before quit:" << t1->isRunning();
//        t1->quit();      // 请求线程退出
//        t1->wait();
//        qDebug() << "Thread t1 state after quit:" << t1->isRunning();
//        if(t1->isRunning()){
//            statusBarShow(true, "线程停止失败！");
//            statusBarShow(false, "线程停止失败！");
//        }
//        else
//        {
//            statusBarShow(true, "线程停止成功！");
//            statusBarShow(false, "线程停止失败！");
//        }
//    });
    */


    /// ********************************2025.7.7***********************************************
    ///板卡接收数据临时
//    FrameAssemble *showHex2Ui = new FrameAssemble();
//    connect(showHex2Ui,&FrameAssemble::stringRecHexDataSend,this,[=](QString stringRecHexData){
//        qDebug()<<"wossssss";
//        ui->plainTextEdit_receive->appendPlainText(stringRecHexData);
//    });

    ///板卡接收数据
    /// RS422
    connect(cardWorker1, &lvdsCardWorker::onRS422DataToUi, this, &MainWindow::handleRS422Data);
    /// LVDS
    connect(cardWorker2, &lvdsCardWorker::onLVDSDataToUi, this, &MainWindow::handleLVDSData);

//    connect(serialPortWorker1,&SerialPortWorker::imageToShowUiSignal,this,[=](QImage image){
//        qDebug()<<"到达main函数";
//        showImage(image);
//    });
//    connect(serialPortWorker2,&SerialPortWorker::imageToShowUiSignal,this,[=](QImage image){
//        showImage(image);
//    });

    connect(dataAnalysisHelper2,&DataAnalysisHelper::imageSignal,this,[=](QImage image){
        qDebug()<<"串口2接收图像数据到达main函数"; // 临时测试
        showImage(image);
    });

    connect(dataAnalysisHelperLVDS,&DataAnalysisHelper::imageSignal,this,[=](QImage image){
        qDebug()<<"LVDS接收图像数据到达main函数";
        showImage(image);
    });

    connect(dataAnalysisHelperLVDS,&DataAnalysisHelper::upgradeErrorImgNumUi,this,[=](const ImageAttributes &attrs){
        qDebug()<<"LVDS接收帧计数到达main函数";
        ui->lineEdit_frameNum_2->setText(QString::number(attrs.totalImgNum));
        ui->lineEdit_timeCode_2->setText(attrs.timeCode);
        ui->lineEdit_falseFrameNum_2->setText(QString::number(attrs.errorImgNum));
        ui->lineEdit_rightFrameNum_2->setText(QString::number(attrs.validImgNum));
        ui->lineEdit_oversizeNum_2->setText(QString::number(attrs.overThreshold));
//        ui->lineEdit_imageFrameNum_2->setText(QString::number(attrs.imgFrameNo));
        ui->lineEdit_imageFrameNum_2->setText(attrs.imgFrameNoString);
        ui->lineEdit_imageWindowSize_2->setText(QString::number(attrs.imgWinSize));
    });

    connect(dataAnalysisHelper2,&DataAnalysisHelper::upgradeErrorImgNumUi,this,[=](const ImageAttributes &attrs){
        qDebug()<<"到达main函数";
        qDebug()<<"attrs.totalImgNum"<<attrs.totalImgNum;
        ui->lineEdit_frameNum_2->setText(QString::number(attrs.totalImgNum));
        ui->lineEdit_timeCode_2->setText(attrs.timeCode);
        ui->lineEdit_falseFrameNum_2->setText(QString::number(attrs.errorImgNum));
        ui->lineEdit_rightFrameNum_2->setText(QString::number(attrs.validImgNum));
        ui->lineEdit_oversizeNum_2->setText(QString::number(attrs.overThreshold));
//        ui->lineEdit_imageFrameNum_2->setText(QString::number(attrs.imgFrameNo));
        ui->lineEdit_imageFrameNum_2->setText(attrs.imgFrameNoString);
        ui->lineEdit_imageWindowSize_2->setText(QString::number(attrs.imgWinSize));
        overThresholdCoords = attrs.overThresholdCo;
        qDebug()<<"overThresholdCoords"<<overThresholdCoords.count();
    });


    connect(cardWorker1,&lvdsCardWorker::imageSignal,this,[=](QImage image){
        qDebug()<<"到达main函数";
        showImage(image);
    });
    connect(cardWorker2,&lvdsCardWorker::imageSignal,this,[=](QImage image){
        showImage(image);
    });


    // 设置定时器（每50ms触发一次更新）rs422
    updateTimer_RS422.setInterval(50);
    connect(&updateTimer_RS422, &QTimer::timeout, this, [this]() {
        if (!pendingData_RS422.isEmpty()) {
            ui->plainTextEdit_receive->setPlainText(pendingData_RS422.join("\n"));
            pendingData_RS422.clear(); // 清空缓冲区
        }
    });
    updateTimer_RS422.start();

    // 设置定时器（每200ms触发一次更新）lvds
    updateTimer_LVDS.setInterval(200);
    connect(&updateTimer_LVDS, &QTimer::timeout, this, [this]() {
        if (!pendingData_LVDS.isEmpty()) {
            ui->plainTextEdit_receive_2->setPlainText(pendingData_LVDS.join("\n"));
            pendingData_LVDS.clear(); // 清空缓冲区
        }
    });
    updateTimer_LVDS.start();

    // 设置定时器（每50ms触发一次更新）串口1
    updateTimer_Serial1.setInterval(50);
    connect(&updateTimer_Serial1, &QTimer::timeout, this, [this]() {
        if (!pendingData_Serial1.isEmpty()) {
            ui->plainTextEdit_receive_serial->setPlainText(pendingData_Serial1.join("\n"));
            pendingData_Serial1.clear(); // 清空缓冲区
        }
    });
    updateTimer_Serial1.start();

    // 设置定时器（每50ms触发一次更新）串口2
    updateTimer_Serial2.setInterval(200);
    connect(&updateTimer_Serial2, &QTimer::timeout, this, [this]() {
        if (!pendingData_Serial2.isEmpty()) {
            ui->plainTextEdit_receive_serial_2->setPlainText(pendingData_Serial2.join("\n"));
            pendingData_Serial2.clear(); // 清空缓冲区
        }
    });
    updateTimer_Serial2.start();


    connect(this,&MainWindow::imgToPeakingSignal,dataAnalysisHelperLVDS,&DataAnalysisHelper::imgToPeaking,Qt::DirectConnection);
    connect(this,&MainWindow::imgToPeakingSignal,dataAnalysisHelper2,&DataAnalysisHelper::imgToPeaking,Qt::DirectConnection);
    connect(ui->checkBox_peaking, &QCheckBox::toggled, this, [=](bool checked){
        // 获取上位机图像锐化阈值
        int peakingThreshold = ui->lineEdit_peakingThreshold->text().toInt();
        emit imgToPeakingSignal(checked, peakingThreshold);
        qDebug()<<"checked"<<checked;
        qDebug()<<"peakingThreshold"<<peakingThreshold;
    });
    connect(ui->lineEdit_peakingThreshold,&QLineEdit::textChanged,this,[=](){
        int peakingThreshold = ui->lineEdit_peakingThreshold->text().toInt();
        if(ui->checkBox_peaking->isChecked()){
            emit imgToPeakingSignal(true, peakingThreshold);
        }
        else {
            emit imgToPeakingSignal(false, peakingThreshold);
        }
    });
    // 图像是否需要锐化操作
    int peakingThreshold = ui->lineEdit_peakingThreshold->text().toInt();
    if(ui->checkBox_peaking->isChecked()){
        // 获取上位机图像锐化阈值
        emit imgToPeakingSignal(true, peakingThreshold);
    }
    else {
        emit imgToPeakingSignal(false, peakingThreshold);
    }


    // ui界面上手动加载xml文件——加载指令xml文件到listWidget_2
    connect(ui->pushButton_import_com_xml,&QPushButton::clicked,this,[=](){
        // 打开文件选择窗口，选取xml文件
        QString addXmlPath = QFileDialog::getOpenFileName(
                this,
                tr("选择 XML 文件"),
                "",
                tr("XML 文件 (*.xml)")
            );
            if (!addXmlPath.isEmpty()) {
                readXML(addXmlPath,false);
            }
    });

    // ui界面上手动加载xml文件——加载指令序列xml文件到tableWidget_cmd_immediately
    connect(ui->pb_import_xml,&QPushButton::clicked,this,[=](){
        // 打开文件选择窗口，选取xml文件
        QString addXmlPath = QFileDialog::getOpenFileName(
                this,
                tr("选择 XML 文件"),
                "",
                tr("XML 文件 (*.xml)")
            );
            if (!addXmlPath.isEmpty()) {
                readXML2(addXmlPath);
            }
    });

    dialogPara = new DialogPara(this);
    connect(dialogPara, &DialogPara::getDocNodeListTempSignal, this, [=](){
        qDebug()<<"开始执行docNodeList = xmlcommandparser->getDocNodeListTemp();";
        docNodeList = xmlcommandparser->getDocNodeListTemp();
    });
    // 编辑指令序列行
    connect(ui->pushButton_editPara,&QPushButton::clicked,this,[=](){
        // 根据当前tableWidget_cmd_immediately中选定指令的索引从docNodeList中取出元素
        // 根据取出的元素在dialogpara.ui中的tableWidget_paraList中填充指令细节
        // 编辑指令序列时，同样需要按照EDIT和COMBOX进行槽函数连接，即时修改docNodeList中的内容
//        DialogPara dialog(this);     // 创建对话框，父对象设为 MainWindow

        /// ⭐️获取当前选中行（指令项列表）
        int rowCom = -1;
        rowCom = ui->listWidget_2->currentRow();
        if (rowCom == -1) {
            QMessageBox::warning(this, "提示", "请选择指令项");
            return;
        }

        /// ⭐️获取当前选中行（指令序列）
        int row = -1;
        row = ui->tableWidget_cmd_immediately->currentRow();
        if (row == -1) {
            QMessageBox::warning(this, "提示", "请选需要编辑的指令序列");
            return;
        }

        xmlcommandparser->setDocNodeList(docNodeList);

        dialogPara->loadCommand(xmlcommandparser,docNodeList,rowCom,row);


        // ✅ 弹出窗口（模态）
        dialogPara->exec();
        // 或者非模态
        // dialog.show();
    });
    // 连接信号
    connect(dialogPara, &DialogPara::updateParaSignal, this, [=](const QString &commandSequence){
        int row = ui->tableWidget_cmd_immediately->currentRow(); // 获取当前选中行
        if(row < 0 || row >= docNodeList.size()) return;
        // ⚡ 这里可以直接操作 MainWindow UI
        qDebug() << "收到 DialogPara 信号"<<commandSequence;
        // 比如刷新 tableWidget 或 docNodeList
        QTableWidgetItem *cellItem = ui->tableWidget_cmd_immediately->item(row, 2);
        if(cellItem) cellItem->setText(commandSequence);
        else {
            qDebug()<<"cellItem不存在";
        }

        // 更新 docNodeList 中的 Command 节点
        QDomElement &commandElement = docNodeList[row];

    },
    Qt::UniqueConnection); //  ⭐️这里使用Qt::UniqueConnection以后就解决了点击弹出窗口中的下拉框修改值时程序崩溃的问题
    // 将指令序列写入xml文件
    connect(ui->pb_export_xml,&QPushButton::clicked,this,[=](){
        QString filePath = QFileDialog::getSaveFileName(
                this,
                "另存为 XML 文件",
                QDir::currentPath(),        // 默认目录
                "XML Files (*.xml);;All Files (*.*)"
            );
            if (!filePath.isEmpty()) {
                saveDocNodeListToXML(filePath, docNodeList);
            }
    });

    /// 在图像窗口中显示像素灰度值和坐标
    pixmapItem = new QGraphicsPixmapItem(); // 初始化这个pixmapItem
    view = findChild<ImageGraphicsView*>("graphicsView"); // ☆☆☆☆从现有的 UI 中找到 QGraphicsView☆☆☆☆

    // 注意：这里的view并不一定能指向某个目标，如果findChild失败，view将是一个空指针
    if (view) {
        view->setPixmapItem(pixmapItem); // 假设你在 NewGraphicsView 中有这个方法
    }
    else{
        qDebug() << "view not found";
        return;  // 如果没有找到，退出函数（注意：是所在的整个函数）以防止空指针调用
    }

    // 创建 QLabel 来显示鼠标坐标
    QLabel *viewStatusLabel = new QLabel(view);  // 把 QLabel 放到 QGraphicsView 内
    viewStatusLabel->setStyleSheet("QLabel { color: white; background-color: rgba(0, 0, 0, 128); }");
    viewStatusLabel->setFixedSize(200, 20);  // 设置标签大小
    viewStatusLabel->move(10, view->height()-30);  // 将 QLabel 放在视图的左下角

    // 创建 QLabel 来显示鼠标点击处的灰度值
    QLabel *viewStatusLabelPixel = new QLabel(view);  // 把 QLabel 放到 QGraphicsView 内
    viewStatusLabelPixel->setStyleSheet("QLabel { color: white; background-color: rgba(0, 0, 0, 128); }");
    viewStatusLabelPixel->setFixedSize(200, 20);  // 设置标签大小
    viewStatusLabelPixel->move(240, view->height()-30);  // 将 QLabel 放在视图的左下角

    connect(view,&ImageGraphicsView::mousePositionChanged,this,[=](int x,int y,QPointF scenePos){
        QPointF itemPos = pixmapItem->mapFromScene(scenePos); // 转换到图像坐标
        if(pixmapItem->contains(itemPos)){
            viewStatusLabel->setText(
                QString("Mouse Position: [%1, %2]").arg(int(x)).arg(int(y)));
        }
    });

    connect(view,&ImageGraphicsView::mouseClicked,this,[=](int pixelValue,QPointF scenePos){
        if(pixmapItem->contains(pixmapItem->mapFromScene(scenePos))){
            viewStatusLabelPixel->setText(QString("Pixel Value: [%1]").arg(pixelValue));
        }
    });


    /*
//    connect(cardWorker,&lvdsCardWorker::onRS422DataToUi,this,[=](quint8 channel,QByteArray data){
//        qDebug()<<"RS422DataReady信号："<<data;
//        Q_UNUSED(channel); // 显示声明 channel 没有被使用
//        dataReceivedRS422(data);
//    });
//    /// LVDS板卡
//    connect(cardWorker,&lvdsCardWorker::onLVDSDataToUi,this,[=](quint8 channel,QByteArray data){
//        qDebug()<<"LVDSDataReady信号："<<data;
//        Q_UNUSED(channel); // 显示声明 channel 没有被使用
//        dataReceivedLVDS(data);
//    });

    // 使能、去使能、复位、清空缓存、打开板卡、关闭板卡、通道帧配置槽函数
    connect(this,&MainWindow::startLVDSSignal,cardWorker,[=](bool chooseCard){
        if(chooseCard) cardWorker->startLVDS(true);
        else cardWorker->startLVDS(false);
    });
    connect(this,&MainWindow::stopLVDSSignal,cardWorker,[=](bool chooseCard){
        if(chooseCard) cardWorker->stopLVDS(true);
        else cardWorker->stopLVDS(false);
    });
    connect(this,&MainWindow::resetCardSignal,cardWorker,[=](bool chooseCard){
        if(chooseCard) cardWorker->resetLvdsCard(true);
        else cardWorker->resetLvdsCard(false);
    });
    connect(this,&MainWindow::clearFIFOSignal,cardWorker,[=](bool chooseCard){
        if(chooseCard) cardWorker->clearFIFO(true);
        else cardWorker->clearFIFO(false);
    });
    connect(this,&MainWindow::openCardSignal,cardWorker,[=](bool chooseCard){
        if(chooseCard) cardWorker->openLvdsCard(true);
        else cardWorker->openLvdsCard(false);
    });
    connect(this,&MainWindow::closeCardSignal,cardWorker,[=](bool chooseCard){
        if(chooseCard) cardWorker->CloseLVDSCard(true);
        else cardWorker->CloseLVDSCard(false);
    });
    connect(this,&MainWindow::setChannelFrame,cardWorker,[=](bool chooseCard){
        if(chooseCard) cardWorker->configLVDSCardFrame(true);
        else cardWorker->configLVDSCardFrame(false);
    });
    connect(this,&MainWindow::setChannelPara,cardWorker,[=](bool chooseCard, bool isRecv,quint8 channel,ST_LVDS_CFG stCfg){
        if(chooseCard) cardWorker->configChannelPara(chooseCard,isRecv,channel,stCfg);
        else cardWorker->configChannelPara(chooseCard,isRecv,channel,stCfg);
    }); */


    ///********************************CAN卡*****************************************
//    ui->com
    /// CAN接收数据保存
    saveDirCANTxt = QCoreApplication::applicationDirPath() + "/DataCapture/CAN/txt/";
    saveDirCANBin = QCoreApplication::applicationDirPath() + "/DataCapture/CAN/bin/";
    QDir dirCANTxt(saveDirCANTxt);
    if (!dirCANTxt.exists()) {
        if (!dirCANTxt.mkpath(".")) {  // 创建多级目录
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirCANTxt);
            return;
        }
    }
    QDir dirCANBin(saveDirCANBin);
    if (!dirCANBin.exists()) {
        if (!dirCANBin.mkpath(".")) {  // 创建多级目录
            QMessageBox::critical(this, "错误", "无法创建目录: " + saveDirCANBin);
            return;
        }
    }

    canWorker1 = new canWorker();
    t5 = new QThread();

    canWorker1->moveToThread(t5);
    canWorker1->setRingBuffer(ringBufferCAN);
    canWorker1->setRingBuffer_slow(ringBufferCAN_slow);
    connect(t5, &QThread::started, canWorker1, &canWorker::threadRun);
    connect(t5, &QThread::finished, canWorker1, &QObject::deleteLater);

    // 按钮点击事件修改
    connect(ui->pushButton_startCAN, &QPushButton::clicked, this, [=](){
        if (ui->pushButton_startCAN->text()=="使能" ) {
            if(ui->pushButton_openCAN ->text() == "打开"){
                QMessageBox::warning(this,"错误","请先打开CAN设备！");
            }
            else {
                // 首次启动创建线程
                canWorker1->startReceive();
                // 先清空缓存再启动接收
                canWorker1->clearBuffer(nDeviceType,nDeviceInd,nCANInd);
                t5->start();
                ui->pushButton_startCAN->setText("停止");
            }

        } else{
            // 停止线程
            canWorker1->stopReceive();
            ui->pushButton_startCAN->setText("使能");
            double delayTimeUi = ui->lineEdit_cycleTime->text().toDouble();
            emit  openCloseBroadcastSignal(false,delayTimeUi);
//            t5->requestInterruption();
//            t5->quit();                  // 退出事件循环
//            t5->wait();                  // 等待线程真正退出
            // 发送停止遥测命令，停止下位机向缓存传输数据（未完成）

            // 停止遥测请求2026.2.25 解决仇男豪反馈的循环弹出CAN错误的问题
            tmRequestTimer->stop();
            ui->pb_begin_request->setText("开始请求");
            ui->pb_begin_request_slow->setText("开始请求");
            ui->sb_request_all->setText("快慢遥同时请求");
            qDebug() << "停止循环发送遥测请求";
        }
    });



    t_dataAnalysisCAN = new QThread();
    dataAnalysisHelperCAN->moveToThread(t_dataAnalysisCAN);
    dataAnalysisHelperCAN->setRingBuffer(ringBufferCAN);
    dataAnalysisHelperCAN->setRingBuffer_slow(ringBufferCAN_slow);

    connect(t_dataAnalysisCAN,&QThread::started,dataAnalysisHelperCAN,&DataAnalysisHelper::onDataAnalysisCAN);
    t_dataAnalysisCAN->start(); // 开始对数据进行处理
    dataAnalysisHelperCAN->startProcessing();

    connect(dataAnalysisHelperCAN,&DataAnalysisHelper::dataToUiDirect,this,[=](QString textToShowDirect){
//        ui->plainTextEdit_receive_3->appendPlainText(textToShowDirect);

        dataReceivedCAN(textToShowDirect);

    });
    connect(canWorker1,&canWorker::dataToUiDirectCanWorker,this,[=](QString textToShowDirect){
        qDebug()<<textToShowDirect;
        qDebug()<<"dataToUiDirectCanWorker";
        ui->plainTextEdit_receive_3->setPlainText(textToShowDirect);

    },Qt::QueuedConnection);   // 明确写更安全

    //    DWORD  nDeviceType = 4; /* USBCAN-2A或USBCAN-2C或CANalyst-II */
    //    DWORD  nDeviceInd = 0; /* 第1个设备 */
    //    DWORD  nCANInd = 1;          /* 第1个通道 */
        connect(this,&MainWindow::updateConfigCANSignal,canWorker1,[=](DWORD nDeviceTypeIn, DWORD nDeviceIndIn, DWORD nCANIndIn){
            canWorker1->updateConfig(nDeviceTypeIn,nDeviceIndIn,nCANIndIn);
        },Qt::DirectConnection);

        ui->comboBox_deviceType->addItem("CANalyst-II",4);
        ui->comboBox_deviceType->setCurrentIndex(0);
        ui->comboBox_deviceNo->addItem("设备号0",0);
        ui->comboBox_deviceNo->addItem("设备号1",1);
        ui->comboBox_deviceNo->setCurrentIndex(0);
        ui->comboBox_deviceChannel->addItem("通道A",0);
        ui->comboBox_deviceChannel->addItem("通道B",1);
        ui->comboBox_deviceChannel->setCurrentIndex(0);
        nDeviceType = ui->comboBox_deviceType->currentData().toUInt();
        nDeviceInd = ui->comboBox_deviceNo->currentData().toUInt();
        nCANInd = ui->comboBox_deviceChannel->currentData().toUInt();
        qDebug()<<"nDeviceType"<<nDeviceType;
        qDebug()<<"nDeviceInd"<<nDeviceInd;
        qDebug()<<"nCANInd"<<nCANInd;
        emit updateConfigCANSignal(nDeviceType,nDeviceInd,nCANInd);

        connect(ui->comboBox_deviceType,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
            nDeviceType = ui->comboBox_deviceType->currentData().toUInt();
            emit updateConfigCANSignal(nDeviceType,nDeviceInd,nCANInd);
            qDebug()<<"new nDeviceType"<<nDeviceType;
        });
        connect(ui->comboBox_deviceNo,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
            nDeviceInd = ui->comboBox_deviceNo->currentData().toUInt();
            emit updateConfigCANSignal(nDeviceType,nDeviceInd,nCANInd);
            qDebug()<<"new nDeviceInd"<<nDeviceInd;
        });
        connect(ui->comboBox_deviceChannel,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
            nCANInd = ui->comboBox_deviceChannel->currentData().toUInt();
            emit updateConfigCANSignal(nDeviceType,nDeviceInd,nCANInd);
            qDebug()<<"new nCANInd"<<nCANInd;
        });


    // 设置can参数
    // 这两个变量用于配置哪种 CAN ID 会通过硬件过滤器
//    vic.AccCode=0x80000000; // 验收码
//    vic.AccMask=0xFFFFFFFF;  // 屏蔽码
//    vic.AccCode = 0x00A00000;  // 源节点地址 = 0b001010
//    vic.AccMask = 0x03F00000;  // 匹配 ID26~ID21，共 6bit
//    vic.AccCode = 0x05000000;  // 源节点地址 = 0b001010
//    vic.AccMask = 0x1F800000;  // 匹配 ID26~ID21，共 6bit
    vic.AccCode = 0x80000008;  // 源节点地址 = 0b001010
    vic.AccMask = 0xFFFFFFFF;  // 匹配 ID26~ID21，共 6bit
    vic.Filter=1; // 1接收所有类型、2只接收标准帧、3只接收扩展帧
    vic.Timing0=0x00; //波特率T0设置 500Kbps
    vic.Timing1=0x1C; //波特率T1设置
    vic.Mode=0; // 0正常模式、1监听模式、2回环模式

//    vic.Filter=vic.Filter=ui->comboBox_recType->currentData().toInt(); //  1接收所有类型、2只接收标准帧、3只接收扩展帧; // 1接收所有类型、2只接收标准帧、3只接收扩展帧
//    if(ui->comboBox_baudRate_can->currentIndex() == 0){
//        vic.Timing0=0x00; //波特率T0设置 500Kbps
//        vic.Timing1=0x14; //波特率T1设置
//    }
//    vic.Mode=ui->comboBox_workMode->currentData().toInt(); // 0正常模式、1监听模式、2回环模式; // 0正常模式、1监听模式、2回环模式

    ui->comboBox_baudRate_can->addItem("500Kbps");
    ui->comboBox_baudRate_can->setCurrentIndex(0);
    ui->comboBox_workMode->addItem("正常模式",0);
    ui->comboBox_workMode->addItem("监听模式",1);
    ui->comboBox_workMode->addItem("回环模式",2);
    ui->comboBox_workMode->setCurrentIndex(0);
    ui->comboBox_recType->addItem("所有类型",1);
    ui->comboBox_recType->addItem("标准帧",2);
    ui->comboBox_recType->addItem("扩展帧",3);
    ui->comboBox_recType->setCurrentIndex(0);

//    bool dwRel = canWorker1->initCAN(nDeviceType, nDeviceInd, nCANInd, &vic);

    connect(ui->comboBox_baudRate_can,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](int index){
        if(index == 0){
            vic.Timing0=0x00; //波特率T0设置 500Kbps
            vic.Timing1=0x14; //波特率T1设置
        }
        qDebug()<<"vic.Timing0"<<vic.Timing0 <<"vic.Mode"<<vic.Timing1;
//        bool dwRel = canWorker1->initCAN(nDeviceType, nDeviceInd, nCANInd, &vic);
//        if(dwRel !=1)
//        {
//            VCI_CloseDevice(nDeviceType, nDeviceInd);
//            ui->plainTextEdit_statusBar_3->appendPlainText("初始化CAN失败！");
//            ui->pushButton_openCAN->setText("打开");
//        }

    });

    connect(ui->comboBox_workMode,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](int index){
        vic.Mode=ui->comboBox_workMode->currentData().toInt(); // 0正常模式、1监听模式、2回环模式
        qDebug()<<"vic.Mode"<<vic.Mode;
//        bool dwRel = canWorker1->initCAN(nDeviceType, nDeviceInd, nCANInd, &vic);

    });
    connect(ui->comboBox_recType,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](int index){
        vic.Filter=ui->comboBox_recType->currentData().toInt(); //  1接收所有类型、2只接收标准帧、3只接收扩展帧
        qDebug()<<"vic.Filter"<<vic.Filter;
//        bool dwRel = canWorker1->initCAN(nDeviceType, nDeviceInd, nCANInd, &vic);
//        if(dwRel !=1)
//        {
//            VCI_CloseDevice(nDeviceType, nDeviceInd);
//            ui->plainTextEdit_statusBar_3->appendPlainText("初始化CAN失败！");
//            ui->pushButton_openCAN->setText("打开");
//        }

    });

    /// 打开、关闭、初始化、启动CAN卡
    connect(ui->pushButton_openCAN, &QPushButton::clicked, this,[=](){
        if(ui->pushButton_openCAN->text() == "关闭"){
            // 关闭板卡
            bool isTrue  = canWorker1->closeDevice(nDeviceType,nDeviceInd);
            if(!isTrue){
                ui->plainTextEdit_statusBar_3->appendPlainText("关闭CAN设备失败！");
            }
            else{
                ui->plainTextEdit_statusBar_3->appendPlainText("关闭CAN设备成功！");
                ui->pushButton_openCAN->setText("打开");
            }
        }
        else{
            // 打开板卡
            // 暂时不从主UI加载can卡配置
            // 固定can卡配置
            qDebug()<<"nDeviceType"<<nDeviceType;
            qDebug()<<"nDeviceInd"<<nDeviceInd;
            qDebug()<<"nCANInd"<<nCANInd;
            bool isTrue  = canWorker1->openDevice(nDeviceType,nDeviceInd,0);
            if(!isTrue){
                ui->plainTextEdit_statusBar_3->appendPlainText("打开CAN设备失败！");
            }
            else{
                ui->plainTextEdit_statusBar_3->appendPlainText(QStringLiteral("打开CAN设备成功！ CAN索引：") + QString::number(nDeviceInd) + "  CAN通道：" + QString::number(nCANInd));
                ui->pushButton_openCAN->setText("关闭");
            }
            if(isTrue){
                // 初始化CAN
                bool dwRel = canWorker1->initCAN(nDeviceType, nDeviceInd, nCANInd, &vic);
                if(dwRel !=1)
                {
                    VCI_CloseDevice(nDeviceType, nDeviceInd);
                    ui->plainTextEdit_statusBar_3->appendPlainText("初始化CAN失败！");
                    ui->pushButton_openCAN->setText("打开");
                    return false;
                }
                else{
                    ui->plainTextEdit_statusBar_3->appendPlainText("初始化CAN成功！");
                    // 启动CAN
                    bool startSucc = canWorker1->startCAN(nDeviceType, nDeviceInd, nCANInd);
                    if(!startSucc){
                        ui->plainTextEdit_statusBar_3->appendPlainText("启动CAN失败！");
                        ui->pushButton_openCAN->setText("打开");
                        return false;
                    }
                    else{
                        ui->plainTextEdit_statusBar_3->appendPlainText("启动CAN成功！");
                        return true;
                    }
                    return true;
                }
            }
        }

    });

    /// 复位
    connect(ui->pushButton_resetCAN,&QPushButton::clicked,this,[=](){
        bool isSuc = canWorker1->resetCAN(nDeviceType, nDeviceInd, nCANInd);
        if(!isSuc){
            ui->plainTextEdit_statusBar_3->appendPlainText("复位CAN失败！");
        }else{
            ui->plainTextEdit_statusBar_3->appendPlainText("复位CAN成功！");
        }
    });

    canFrameHelper = new CANFramer();
    /// 发送数据
    connect(ui->pushButton_send_3,&QPushButton::clicked,this,[=](){
        if (ui->pushButton_openCAN->text()=="打开") {
            ui->plainTextEdit_statusBar_3->appendPlainText("请先打开 CAN 通道！");
            return;
        }
        sendDataDirect();
    });

    connect(dialogCanFrame,&DialogCanFrame::updateCanFrameConfigUiSignal,this,[=](canFrameConfig canFrameConfigDia){
        canFrameConfigUi.priority = canFrameConfigDia.priority;
        canFrameConfigUi.srcAddress = canFrameConfigDia.srcAddress;
        canFrameConfigUi.multicast = canFrameConfigDia.multicast;
        canFrameConfigUi.destAdrr = canFrameConfigDia.destAdrr;
        canFrameConfigUi.funCode = canFrameConfigDia.funCode;
        qDebug()<<"优先级："<<canFrameConfigDia.priority;

    });
    connect(dialogCanFrame2,&DialogCanFrame2::updateCanFrameConfigUiSignal,this,[=](canFrameConfig canFrameConfigDia2){
        canFrameConfigUi2.priority = canFrameConfigDia2.priority;
        canFrameConfigUi2.srcAddress = canFrameConfigDia2.srcAddress;
        canFrameConfigUi2.multicast = canFrameConfigDia2.multicast;
        canFrameConfigUi2.destAdrr = canFrameConfigDia2.destAdrr;
        canFrameConfigUi2.funCode = canFrameConfigDia2.funCode;
    });

    /// ※注意：遥测请求的帧ID配置是写死的，区别于调试助手页面和指令发送页面的可编辑选项
    ///  但是根据用户在上位机勾选正X和负X来分别进行不同的配置
    if(ui->radioButton_A->isChecked()){
        canFrameConfig_tmRequest.priority = 0;
        canFrameConfig_tmRequest.srcAddress = 1;
        canFrameConfig_tmRequest.multicast = 0;
        canFrameConfig_tmRequest.destAdrr = 10; // A机
        canFrameConfig_tmRequest.funCode = 1;
    }
    else{
        canFrameConfig_tmRequest.priority = 0;
        canFrameConfig_tmRequest.srcAddress = 1;
        canFrameConfig_tmRequest.multicast = 0;
        canFrameConfig_tmRequest.destAdrr = 11; // B机
        canFrameConfig_tmRequest.funCode = 1;
    }
    connect(ui->radioButton_A, &QRadioButton::toggled,this, [=](){
        canFrameConfig_tmRequest.priority = 0;
        canFrameConfig_tmRequest.srcAddress = 1;
        canFrameConfig_tmRequest.multicast = 0;
        canFrameConfig_tmRequest.destAdrr = 10; // A机
        canFrameConfig_tmRequest.funCode = 1;
    });

    connect(ui->radioButton_B, &QRadioButton::toggled,this,[=](){
        canFrameConfig_tmRequest.priority = 0;
        canFrameConfig_tmRequest.srcAddress = 1;
        canFrameConfig_tmRequest.multicast = 0;
        canFrameConfig_tmRequest.destAdrr = 11; // B机
        canFrameConfig_tmRequest.funCode = 1;
    });


    connect(this,&MainWindow::canTmIsOpeUpdateSignal,canWorker1,[=](bool canTmOpenClose){
          canWorker1->updateCanTmIsOpen(canTmOpenClose);
    },Qt::DirectConnection);
    connect(this,&MainWindow::canTmIsOpeUpdateSignal_slow,canWorker1,[=](bool canTmOpenClose){
          canWorker1->updateCanTmIsOpen_slow(canTmOpenClose);
    },Qt::DirectConnection);

    /// 请求遥测
    emit canTmIsOpeUpdateSignal(true);
    tmRequestTimer = new QTimer(this);
    connect(tmRequestTimer, &QTimer::timeout,this, [=](){
        if(fastSend ){
            transmitDirectCan(canParaQStringFast);
        }
        else if(slowSend){
            transmitDirectCan(canParaQStringSlow);
        }
        else if (allSend ) {
            transmitDirectCan(canParaQStringAll);
        }
        else {
            return ;
        }
    });

    connect(ui->pb_begin_request,&QPushButton::clicked,this,[=](){
        if(ui->pb_begin_request->text() == "开始请求"){
                   tmRequestTimer->stop();
        }
        ui->pb_begin_request_slow->setText("开始请求");
        fastSend = true;
        slowSend = false;
        allSend = false;
        // 发送遥测请求指令
        // 发送周期，单位 ms（比如从 UI 读）
        int intervalMs = 1000;  // 1 秒，可改成 ui->lineEdit_xxx->text().toInt()

        if (!tmRequestTimer->isActive()&& ui->pushButton_startCAN->text() == "停止") {
            // 开始循环发送
            tmRequestTimer->start(intervalMs);
            ui->pb_begin_request->setText("停止请求");
            qDebug() << "开始循环发送遥测请求";
        }
        else if (!tmRequestTimer->isActive()&& ui->pushButton_startCAN->text() == "使能") {
            QMessageBox::warning(this,"error","请先使能CAN");
        }
        else {
            // 停止
            tmRequestTimer->stop();
            ui->pb_begin_request->setText("开始请求");
            qDebug() << "停止循环发送遥测请求";
        }

    });
    emit canTmIsOpeUpdateSignal_slow(true);
    connect(ui->pb_begin_request_slow,&QPushButton::clicked,this,[=](){
//        // 检查遥测请求can参数配置是否正常切换，针对A机和B机
//        qDebug()<<canFrameConfig_tmRequest.priority;
//        qDebug()<<canFrameConfig_tmRequest.srcAddress;
//        qDebug()<<canFrameConfig_tmRequest.multicast;
//        qDebug()<<canFrameConfig_tmRequest.destAdrr;
//        qDebug()<<canFrameConfig_tmRequest.funCode;

        if(ui->pb_begin_request_slow->text() == "开始请求"){
                   tmRequestTimer->stop();
        }
        ui->pb_begin_request->setText("开始请求");
        fastSend = false;
        slowSend = true;
        allSend = false;
        // 发送遥测请求指令
        // 发送周期，单位 ms（比如从 UI 读）
        int intervalMs = 4000;  // 1 秒，可改成 ui->lineEdit_xxx->text().toInt()

        if (!tmRequestTimer->isActive()&& ui->pushButton_startCAN->text() == "停止") {
            // 开始循环发送
            tmRequestTimer->start(intervalMs);
            ui->pb_begin_request_slow->setText("停止请求");
            qDebug() << "开始循环发送遥测请求";
        }
        else if (!tmRequestTimer->isActive()&& ui->pushButton_startCAN->text() == "使能") {
            QMessageBox::warning(this,"error","请先使能CAN");
        }
        else {
            // 停止
            tmRequestTimer->stop();
            ui->pb_begin_request_slow->setText("开始请求");
            qDebug() << "停止循环发送遥测请求";
        }
    });

    connect(ui->sb_request_all,&QPushButton::clicked,this,[=](){
//        tmRequestTimer->stop();
        if(ui->sb_request_all->text() == "开始循环发送全部遥测请求"){
                   tmRequestTimer->stop();
        }
        ui->pb_begin_request->setText("开始请求");
        ui->pb_begin_request_slow->setText("开始请求");
        fastSend = false;
        slowSend = false;
        allSend = true; // 循环发送全部遥测
        // 发送遥测请求指令
        // 发送周期，单位 ms（比如从 UI 读）
        int intervalMs = 1000;  // 1 秒，可改成 ui->lineEdit_xxx->text().toInt()

        if (!tmRequestTimer->isActive()&& ui->pushButton_startCAN->text() == "停止") {
            // 开始循环发送
            tmRequestTimer->start(intervalMs);
            ui->sb_request_all->setText("停止请求");
            qDebug() << "开始循环发送全部遥测请求";
        }
        else if (!tmRequestTimer->isActive()&& ui->pushButton_startCAN->text() == "使能") {
            QMessageBox::warning(this,"error","请先使能CAN");
        }
        else {
            // 停止
            tmRequestTimer->stop();
            ui->sb_request_all->setText("快慢遥同时请求");
            qDebug() << "停止循环发送遥测请求";
        }

        // 单次发送
//        if(ui->pushButton_startCAN->text() == "使能"){
//             QMessageBox::warning(this,"error","请先使能CAN");
//        }
//        else {
//            transmitDirectCan(canParaQStringAll);
//        }

    });

    dialogCanFrame->onOkButtonClicked();
    dialogCanFrame2->onOkButtonClicked();

    // 设置定时器（每200ms触发一次更新）CAN
    updateTimer_CAN.setInterval(200);
    connect(&updateTimer_CAN, &QTimer::timeout, this, [this]() {
        if (!pendingData_CAN.isEmpty()) {
            ui->plainTextEdit_receive_3->setPlainText(pendingData_CAN.join("\n"));
            pendingData_CAN.clear(); // 清空缓冲区
        }
    });
    updateTimer_CAN.start();

    ui->pushButton_saveDataCAN->setCheckable(true); // 允许按下
    /// 开始保存数据到txt
    connect(ui->pushButton_saveDataCAN,&QPushButton::clicked,this,[=](){
        if(ui->pushButton_saveDataCAN->isChecked()){
            // 检查CAN使能状态
            bool canIsStart = false;
            if(ui->pushButton_openCAN->text() == "关闭" && ui->pushButton_startCAN->text() == "停止"){
                canIsStart = true;
            }
            if(canIsStart){
                ui->pushButton_saveDataCAN->setChecked(true);
                ui->pushButton_saveDataCAN->setText("等待");
                // 获取当前时间作为文件名
                QDateTime currentTime = QDateTime::currentDateTime();
                fileNameCAN = currentTime.toString("yyyyMMdd_hhmmss") +"CAN"+ ".txt";
                updateStatusBar(false,"准备保存数据");
                ui->pushButton_saveDataCAN->setChecked(true);
                // 查询保存时间
                double saveTime = ui->lineEdit_saveTimeCAN->text().toDouble();
                isSaveDataCAN = true; // 开始保存
                // 启动定时器，在指定时间后停止保存
                QTimer::singleShot(static_cast<int>(saveTime * 1000), this, [this](){
                    isSaveDataCAN = false; // 停止保存
                    ui->pushButton_saveDataCAN->setChecked(false);
                    ui->pushButton_saveDataCAN->setText("开始");
                    QMessageBox::information(this, "保存完成", "数据已保存到文件");
                });
            }
            else{
                qmessageBoxSet("CAN未使能！");
                updateStatusBar(false,"CAN板卡未使能！");
                ui->pushButton_saveDataCAN->setChecked(false);
            }
        }
        else{
            qmessageBoxSet("请等待当前数据保存完毕！");
            ui->pushButton_saveDataCAN->setChecked(true);
            ui->pushButton_saveDataCAN->setText("等待");
        }
    });


    ///********************************广播P1临时测试*****************************************
    // t6暂时没用上，暂时使用QTimer实现循环发送
    t6 = new QThread();
    broadcastWorker = new BroadcastWorker();
    broadcastWorker->moveToThread(t6);
    connect(t6, &QThread::started, broadcastWorker, &BroadcastWorker::threadRun);
    t6->start(); // 循环发送数据，暂时不用子线程
    connect(t6, &QThread::finished, canWorker1, &QObject::deleteLater);

    // 循环发送
    connect(this,&MainWindow::openCloseBroadcastSignal,broadcastWorker,&BroadcastWorker::updateOpenCloseSign,Qt::DirectConnection);
//    connect(ui->pushButton_reTransmit,&QPushButton::clicked,this,&MainWindow::repeatSend);
    connect(ui->pushButton_reTransmit,&QPushButton::clicked,this,&MainWindow::repeatTransmit);
    connect(this, &MainWindow::updateBroadcastDataSignal,
            broadcastWorker, &BroadcastWorker::updateBroadcastData,Qt::DirectConnection);

    broadcastTimer = new QTimer(this);

    connect(broadcastTimer, &QTimer::timeout,this, [=](){
        if(broadcastSend){
//            QString canDataQStringBroadcast = ui->plainTextEdit_send_3->toPlainText();
//            transmitDirectCan(canDataQStringBroadcast);
            sendDataDirect();
        }
    });

    ///********************************广播正式*****************************************
    frmPagesDataBroadcast *broadcastPage = new frmPagesDataBroadcast(this);
    ui->verticalLayout_50->addWidget(broadcastPage);

}

MainWindow::~MainWindow()
{

    if (m_saveTimer) {
        m_saveTimer->stop();
        delete m_saveTimer;
    }

    delete ui;
    /// 1. 去使能
    cardWorker1->StopCard(true); // 板卡RS422去使能
    cardWorker2->StopCard(false); // 板卡LVDS去使能

    canWorker1->stopReceive();

    /// 2. 释放资源，关闭板卡
    cardWorker1->resetCard(true); // 板卡复位
    cardWorker2->resetCard(false);

    cardWorker1->clearFIFO(true); // 清空缓存
    cardWorker2->clearFIFO(false);

    cardWorker1->closeCard(true); // 关闭板卡
    cardWorker2->closeCard(false);

    /// 3. 关闭线程
    qDebug() << "Thread t1 state before quit:" << t1->isRunning();
    t1->quit();      // 请求线程退出
    t1->wait();
    qDebug() << "Thread t1 state after quit:" << t1->isRunning();


    qDebug() << "Thread t2 state before quit:" << t2->isRunning();
    t2->quit();      // 请求线程退出
    t2->wait();
    qDebug() << "Thread t2 state after quit:" << t2->isRunning();

    qDebug() << "Thread t3 state before quit:" << t3->isRunning();
    t3->quit();      // 请求线程退出
    t3->wait();
    qDebug() << "Thread t3 state after quit:" << t3->isRunning();

    qDebug() << "Thread t4 state before quit:" << t4->isRunning();
    t4->quit();      // 请求线程退出
    t4->wait();
    qDebug() << "Thread t4 state after quit:" << t4->isRunning();


    // 在停止按钮逻辑中
    canWorker1->stopReceive();
    t5->requestInterruption();  // 请求中断
    qDebug() << "Thread t5 state before quit:" << t5->isRunning();
    t5->quit();      // 请求线程退出
    t5->wait();
    qDebug() << "Thread t5 state after quit:" << t5->isRunning();

//    emit openCloseBroadcastSignal(false,0);
//    t6->requestInterruption();  // 请求中断
//    qDebug() << "Thread t6 state before quit:" << t6->isRunning();
//    t6->quit();      // 请求线程退出
//    t6->wait();
//    qDebug() << "Thread t6 state after quit:" << t6->isRunning();

    dataAnalysisHelper1->stopProcessing(); // 先停止while循环
    qDebug() << "Thread t_dataAnalysis1 state before quit:" << t_dataAnalysis1->isRunning();
    t_dataAnalysis1->quit();      // 请求线程退出
    t_dataAnalysis1->wait();
    qDebug() << "Thread t_dataAnalysis1 state after quit:" << t_dataAnalysis1->isRunning();

    dataAnalysisHelper2->stopProcessing(); // 先停止while循环
    qDebug() << "Thread t_dataAnalysis2 state before quit:" << t_dataAnalysis2->isRunning();
    t_dataAnalysis2->quit();      // 请求线程退出
    t_dataAnalysis2->wait();
    qDebug() << "Thread t_dataAnalysis2 state after quit:" << t_dataAnalysis2->isRunning();

    dataAnalysisHelperRS422->stopProcessing();
    qDebug() << "Thread t_dataAnalysisRS422 state before quit:" << t_dataAnalysisRS422->isRunning();
    t_dataAnalysisRS422->quit();      // 请求线程退出
    t_dataAnalysisRS422->wait();
    qDebug() << "Thread t_dataAnalysisRS422 state after quit:" << t_dataAnalysisRS422->isRunning();

    dataAnalysisHelperLVDS->stopProcessing();
    qDebug() << "Thread t_dataAnalysisLVDS state before quit:" << t_dataAnalysisLVDS->isRunning();
    t_dataAnalysisLVDS->quit();      // 请求线程退出
    t_dataAnalysisLVDS->wait();
    qDebug() << "Thread t_dataAnalysisLVDS state after quit:" << t_dataAnalysisLVDS->isRunning();

    dataAnalysisHelperCAN->stopProcessing();
    qDebug() << "Thread dataAnalysisHelperCAN state before quit:" << t_dataAnalysisCAN->isRunning();
    t_dataAnalysisCAN->quit();      // 请求线程退出
    t_dataAnalysisCAN->wait();
    qDebug() << "Thread dataAnalysisHelperCAN state after quit:" << t_dataAnalysisCAN->isRunning();


//    cardWorker1->clearBuffer(); // 手动维护清空缓存区
//    cardWorker2->clearBuffer();    // 手动维护清空缓存区
//    serialPortWorker1->clearFIFO();
//    serialPortWorker2->clearFIFO();


}


// ---------------------- 工具函数：添加自定义标签项 ----------------------
void MainWindow::addTabItem(QListWidget* listWidget, const QIcon& icon, const QString& text) {
    // 1. 创建QListWidgetItem（作为容器）
    QListWidgetItem* item = new QListWidgetItem(listWidget);
    item->setSizeHint(QSize(listWidget->width(), 100)); // 项大小（宽度自适应listWidget，高度100px）

    // 2. 创建自定义TabItem（图标+文字）
    TabItem* tabWidget = new TabItem(icon, text, listWidget);

    // 3. 将TabItem设置为item的 widget
    listWidget->setItemWidget(item, tabWidget);
}

/// ---------------------- 工具函数：为stackedWidget添加页面 ----------------------
void MainWindow::addStackedPage(QStackedWidget* stackedWidget, const QString& pageName) {
    // 1. 创建页面 widget（可替换为实际业务控件，比如串口设置界面）
    QWidget* page = new QWidget(stackedWidget);
//    page->setStyleSheet("background-color: blue;"); // 页面背景色（可选）

    // 2. 添加测试内容（替换为实际控件，比如你的串口名下拉、打开串口按钮）
//    QLabel* label = new QLabel(pageName, page);
//    label->setStyleSheet("font-size: 20px; color: #666;");
//    QVBoxLayout* layout = new QVBoxLayout(page);
//    layout->addWidget(label, 0, Qt::AlignCenter); // 测试内容居中

    // 3. 将页面添加到stackedWidget
    stackedWidget->addWidget(page);
}

// ---------------------- 槽函数：点击QListWidget项切换页面 ----------------------
void MainWindow::onTabItemClicked(QListWidgetItem* item) {
    // 1. 获取点击项的索引（与stackedWidget页面索引对应）
    int index = ui->listWidget->row(item);

    // 2. 切换stackedWidget到对应页面
    ui->stackedWidget->setCurrentIndex(index);

    // 3. 更新选中项的样式（触发TabItem的paintEvent）
    updateTabSelection(index);
}

void MainWindow::onTabItemClicked_2(QListWidgetItem *item) {
    if(!item) return;
    // 1. 获取点击项的索引（与stackedWidget页面索引对应）
    int index = ui->listWidget_2->row(item);

    // 2. 切换stackedWidget到对应页面
    ui->stackedWidget_com->setCurrentIndex(index);

    // 3. 更新选中项的样式（触发TabItem的paintEvent）
    updateTabSelection_2(index);

    // 4. 确定当前指令发送通道
    QString tabChoose = item->data(Qt::UserRole).toString();
    qDebug()<<"tabChoose"<<tabChoose;
    updateCheckboxesForTab(tabChoose);

//    // 更新ui界面指令组显示
//    ui->lb_current_cmd_name->setText(commandList.at(index));
    // 更安全的访问方式：
    if (!commandList.isEmpty() && index >= 0 && index < commandList.size()) {
        ui->lb_current_cmd_name->setText(commandList.at(index)); // 或 commandList[index]（无越界检查）
    } else {
        ui->lb_current_cmd_name->setText(""); // 索引无效时显示空字符串
        qWarning() << "索引越界：index=" << index << "，列表大小=" << commandList.size();
    }
}

// ---------------------- 槽函数：更新选中项的样式 ----------------------
void MainWindow::updateTabSelection(int selectedIndex) {
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem* item = ui->listWidget->item(i);
        TabItem* tabWidget = qobject_cast<TabItem*>(ui->listWidget->itemWidget(item));
        if (tabWidget) {
            // 设置选中状态（触发paintEvent）
            tabWidget->setProperty("isSelected", (i == selectedIndex));
            tabWidget->update(); // 强制重绘
        }
    }
}

void MainWindow::updateTabSelection_2(int selectedIndex) {
    for (int i = 0; i < ui->listWidget_2->count(); ++i) {
        QListWidgetItem* item = ui->listWidget_2->item(i);
        TabItemSub* tabWidget = qobject_cast<TabItemSub*>(ui->listWidget_2->itemWidget(item));
        if (tabWidget) {
            // 设置选中状态（触发paintEvent）
            tabWidget->setProperty("isSelected", (i == selectedIndex));
            tabWidget->update(); // 强制重绘
        }
    }
}

/// ***************************串口函数************************

void MainWindow::updatePortState(bool serialChoose, bool isOpen){
    if(serialChoose){
        ui->pushButton_serialOpenClose->setText(isOpen ? "关闭串口" : "打开串口");
    }
    else{
        ui->pushButton_serialOpenClose_2->setText(isOpen ? "关闭串口" : "打开串口");
    }
}

// 程序启动时运行
void MainWindow::refreshSerialPorts()
{
    ui->comboBox_serial->clear();  // 清空原有内容
    ui->comboBox_serial_2->clear();

    // 获取所有可用串口
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    if(ports.isEmpty()) {
        ui->comboBox_serial->addItem("Null");
        ui->comboBox_serial_2->addItem("Null");
        return;
    }

    // 将串口信息添加到下拉框
    foreach(const QSerialPortInfo &port, ports) {
        // 显示格式示例："COM3 (USB-SERIAL CH340)"
        ui->comboBox_serial->addItem(port.portName() + " (" + port.description() + ")",
                                  port.portName()); // 实际存储的用户数据（这里存的是纯端口名）
        ui->comboBox_serial_2->addItem(port.portName() + " (" + port.description() + ")",
                                  port.portName()); // 实际存储的用户数据（这里存的是纯端口名）
    }
    // 其他参数
}

void MainWindow::serialOpenClose(bool serialChoose){
    // COMI
    if(serialChoose){
        // 获取当前选择的端口名
        QString portName = ui->comboBox_serial->currentData().toString();
        if(portName.isEmpty()){
            qmessageBoxSet("请选择有效的串口");
            return;
        }
        // 获取波特率等参数
        int baudRate = ui->comboBox_baudRate->currentData().toInt();
        int dataBit = ui->comboBox_dataBit->currentData().toInt();
        int parity = ui->comboBox_parity->currentData().toInt();
        int stopBit = ui->comboBox_stopBits->currentData().toInt();

        emit initSerialSignal(portName,baudRate,dataBit,parity, stopBit);
        emit openCloseSignal();
    }
    // COMII
    else{
        // 获取当前选择的端口名
        QString portName = ui->comboBox_serial_2->currentData().toString();
        if(portName.isEmpty()){
            qmessageBoxSet("请选择有效的串口");
            return;
        }
        // 获取波特率等参数
        int baudRate = ui->comboBox_baudRate_2->currentData().toInt();
        int dataBit = ui->comboBox_dataBit_2->currentData().toInt();
        int parity = ui->comboBox_parity_2->currentData().toInt();
        int stopBit = ui->comboBox_stopBits_2->currentData().toInt();

        emit initSerialSignal_2(portName,baudRate,dataBit,parity, stopBit);
        emit openCloseSignal_2();
    }
}

void MainWindow::updatePlainText(bool serialChoose, QString textToShowDirect){
    if(serialChoose){
//        ui->plainTextEdit_receive_serial->appendPlainText(textToShowDirect);
        pendingData_Serial1.append(textToShowDirect); // 数据存入缓冲区

        /**
          如果直接使用上述写入代码而不加限制的话会出现如下问题：
            文本内容不断累积，内存占用持续增长
            界面需要渲染的文本量越来越大
            每一次追加文本都会触发重绘，且随着文档变大，重绘开销增加
            撤销栈（Undo Stack）也会记录每一个操作，占用大量内存
            这些因素综合起来，会导致界面越来越卡，尤其是当文本量很大时，缩放操作（需要重绘）也会变得非常缓慢
            解决办法：
            ui->plainTextEdit_receive_serial->setMaximumBlockCount(MAX_LOG_LINES); 限制最大行数
            ui->plainTextEdit_receive_serial->setUndoRedoEnabled(false); 禁用撤销/重做功能，避免了撤销栈的内存占用
        */

//        appendWithLimit(ui->plainTextEdit_receive_serial, textToShowDirect);

        // 检查保存数据标志位，询问是否需要保存数据
        if(isSaveDataSerial1){
            qDebug()<<"正在写入txt文本和bin文本中";
            // 1. 保存为 TXT 文件（文本格式）
            QFile file(saveDirSerial1Txt + fileNameSerial1);
            if(file.open(QIODevice::Append | QIODevice::Text)){
                QTextStream stream(&file);
                stream << textToShowDirect + ' ';
    //            stream << textToShowDirect << "\n";  // 加换行符分隔不同数据块
                file.close();

            } else {
                QMessageBox::warning(this, "错误", "无法打开文件");
                isSaveDataSerial1 = false;
                return;
            }

            // 2. 保存为 BIN 文件（二进制格式）
            QString binFileName = fileNameSerial1; // 创建副本
            binFileName.replace(".txt", ".bin");
            QFile binFile(saveDirSerial1Bin + binFileName); // 替换扩展名
            if (binFile.open(QIODevice::Append)) { // 二进制模式无需 QIODevice::Text
                // 将十六进制字符串转为二进制数据
                QByteArray binaryData = QByteArray::fromHex(textToShowDirect.toLatin1());
                binFile.write(binaryData); // 直接写入二进制数据
                binFile.close();
            } else {
                QMessageBox::warning(this, "错误", "无法打开 BIN 文件");
                isSaveDataSerial1 = false;
            }
        }
        else{
//            qDebug()<<"停止写入文本";
        }
    }
    else{
//        ui->plainTextEdit_receive_serial_2->appendPlainText(textToShowDirect);
        pendingData_Serial2.append(textToShowDirect); // 数据存入缓冲区
        // 检查保存数据标志位，询问是否需要保存数据
        if(isSaveDataSerial2){
            qDebug()<<"正在写入txt文本和bin文本中";
            // 1. 保存为 TXT 文件（文本格式）
            QFile file(saveDirSerial2Txt + fileNameSerial2);
            if(file.open(QIODevice::Append | QIODevice::Text)){
                QTextStream stream(&file);
                stream << textToShowDirect;
    //            stream << textToShowDirect << "\n";  // 加换行符分隔不同数据块
                file.close();

            } else {
                QMessageBox::warning(this, "错误", "无法打开文件");
                isSaveDataSerial2 = false;
                return;
            }

            // 2. 保存为 BIN 文件（二进制格式）
            QString binFileName = fileNameSerial2; // 创建副本
            binFileName.replace(".txt", ".bin");
            QFile binFile(saveDirSerial2Bin + binFileName); // 替换扩展名
            if (binFile.open(QIODevice::Append)) { // 二进制模式无需 QIODevice::Text
                // 将十六进制字符串转为二进制数据
                QByteArray binaryData = QByteArray::fromHex(textToShowDirect.toLatin1());
                binFile.write(binaryData); // 直接写入二进制数据
                binFile.close();
            } else {
                QMessageBox::warning(this, "错误", "无法打开 BIN 文件");
                isSaveDataSerial2 = false;
            }
        }
        else{
//            qDebug()<<"停止写入文本";
        }
    }
}

void MainWindow::updateCheckboxesForTab(const QString &tabName)
{
    // 先全部取消勾选，避免上一次的状态干扰
    ui->checkBox_COMII->setChecked(false);
    ui->checkBox_COMI->setChecked(false);
    ui->checkBox_CARDRS422->setChecked(false);

    if (tabName == "红外相机") {
        ui->checkBox_COMII->setChecked(true);
    }
    else if (tabName == "可见光相机") {
        ui->checkBox_COMI->setChecked(true);
        ui->checkBox_CARDRS422->setChecked(true);
    }
}

void MainWindow::transmitDirectCan(QString canDataToTransmit)
{
        QByteArray commandCanFrame = dataTransfer->string2Bytearray(canDataToTransmit);
        qDebug()<<"*********canDataToTransmit*********"<<canDataToTransmit;
        auto frames = canFrameHelper->frameCAN(commandCanFrame,canFrameConfig_tmRequest); /// ⭐️注意，这里发送遥测请求的按钮所使用的ID配置并不同于指令页面ID配置，单独设置比较好（0、1、0、10、1）
        if(ui->pushButton_startCAN->text() == "停止"){
            auto frames = canFrameHelper->frameCAN(commandCanFrame,canFrameConfig_tmRequest);
            for(auto &obj : frames)
            {
                int ret = canWorker1->transmit(nDeviceType, nDeviceInd, nCANInd, &obj,1);  // 发送数据，其实就已经写进缓存区了·
                if(ret > 0){
                    QString byteDataSendSizeString = QString::number(obj.DataLen);
                    ui->plainTextEdit_statusBar_3->appendPlainText("数据发送成功，数据长度：" + byteDataSendSizeString + "字节");
                    // qDebug() << "CAN 发送成功，字节数：" << byteDataSend.size();
                } else {
                    // qDebug() << "CAN 发送失败！错误码：" << VCI_ReadErrInfo(DevType, DevIndex, 0);
                    ui->plainTextEdit_statusBar_3->appendPlainText("数据发送失败");
                }
                // 这里暂时没有使用msleep，因为发送的指令是单帧，不涉及到多帧发送时丢帧的情况。
            }
        }
        else{
            qmessageBoxSet("请先使能CAN通道！");
        }
}

bool MainWindow::sendData1(UINT channel, UINT ID, BYTE remoteFlag, BYTE externFlag, const unsigned char *data, BYTE len)
{
    if(ui->pushButton_startCAN->text()=="使能"){
        return 0;
    }
    unsigned int dwRel;
    VCI_CAN_OBJ vco;
    vco.ID = ID ;
    vco.RemoteFlag = remoteFlag;
    vco.ExternFlag = externFlag;
    vco.SendType   = 1;
    vco.DataLen = len;
    for(UINT j = 0;j < len;j++)
        vco.Data[j] = data[j];
    dwRel = VCI_Transmit(4, 0, 0,&vco,1);
    if(dwRel>0)
        return true;
    else
        return false;
}

void MainWindow::statusBarShowSerial(bool chooseSerial,QString showString){
    if(chooseSerial) ui->plainTextEdit_statusBar_serial->appendPlainText(showString);
    else ui->plainTextEdit_statusBar_serial_2->appendPlainText(showString);
}

/// ***************************板卡函数************************
/// lambda函数的qt5.13.0支持**************
void MainWindow::runError(QString errorString){
    qmessageBoxSet(errorString);
}

void MainWindow::updateStatusBar(bool chooseCard,QString errorText){
    statusBarShow(chooseCard, errorText);
}

/*
// 启动指令序列（处理首行延迟）
void MainWindow::startCommandSequence()
{
    if (m_nextSendRow < 0 || m_nextSendRow >= m_timeDelays.size()) {
           return;
       }

       const int currentRow = m_nextSendRow;
       const int delayMs = m_timeDelays[currentRow] * 1000;

       // 处理首行延迟
       if (delayMs > 0) {
           QTimer::singleShot(delayMs, this, [this, currentRow]() {
               sendCommandWithDelay();
           });
       } else {
           // 无延迟直接发送
           sendCommandWithDelay();
       }
}
// 发送指令并处理下一行
void MainWindow::sendCommandWithDelay()
{
    if (m_nextSendRow < 0 || m_nextSendRow >= m_timeDelays.size()) {
           m_nextSendRow = -1;  // 重置状态
           return;
       }

       const int currentRow = m_nextSendRow;

       // 设置当前行并发送命令
       ui->tableWidget_cmd_immediately->setCurrentCell(currentRow, 0);
       sendCMD();

       // 准备下一行
       m_nextSendRow++;

       // 如果还有后续指令
       if (m_nextSendRow < m_timeDelays.size()) {
           const int nextDelayMs = m_timeDelays[m_nextSendRow] * 1000;

           // 设置下一行的延迟
           if (nextDelayMs > 0) {
               QTimer::singleShot(nextDelayMs, this, &MainWindow::sendCommandWithDelay);
           } else {
               // 无延迟直接发送下一行
               QTimer::singleShot(0, this, &MainWindow::sendCommandWithDelay);
           }
       } else {
           // 序列完成
           m_nextSendRow = -1;
           qDebug() << "Command sequence completed";
       }
}
*/

/// lambda函数的qt5.13.0支持**************

void MainWindow::handleRS422Data(quint8 channel, QString dataString)
{
    qDebug() << "RS422DataReady信号：" << dataString;
    qDebug() << "RS422DataReady信号size：" << dataString.size();
    Q_UNUSED(channel);
    dataReceivedRS422(dataString);
}

void MainWindow::handleLVDSData(quint8 channel, QString dataString)
{
    qDebug() << "LVDSDataReady信号：" << dataString;
    qDebug() << "LVDSDataReady信号size：" << dataString.size();
    Q_UNUSED(channel);
    dataReceivedLVDS(dataString);
}

// RS422板卡接收数据直接显示
void MainWindow::dataReceivedRS422(QString hexString){
//    ui->plainTextEdit_receive->clear(); // 先清空再填数据，这是临时防止ui界面卡顿的策略
//    ui->plainTextEdit_receive->appendPlainText(hexString);
    pendingData_RS422.append(hexString); // 数据存入缓冲区

    // 检查保存数据标志位，询问是否需要保存数据
    if(isSaveDataRs422){
        qDebug()<<"正在写入txt文本和bin文本中";
        // 1. 保存为 TXT 文件（文本格式）
        QFile file(saveDirRS422Txt + fileNameRS422);
        if(file.open(QIODevice::Append | QIODevice::Text)){
            QTextStream stream(&file);
            stream << hexString;
//            stream << hexString << "\n";  // 加换行符分隔不同数据块
            file.close();

        } else {
            QMessageBox::warning(this, "错误", "无法打开文件");
            isSaveDataRs422 = false;
            return;
        }

        // 2. 保存为 BIN 文件（二进制格式）
        QString binFileName = fileNameRS422; // 创建副本
        binFileName.replace(".txt", ".bin");
        QFile binFile(saveDirRS422Bin + binFileName); // 替换扩展名
        if (binFile.open(QIODevice::Append)) { // 二进制模式无需 QIODevice::TSext
            // 将十六进制字符串转为二进制数据
            QByteArray binaryData = QByteArray::fromHex(hexString.toLatin1());
            binFile.write(binaryData); // 直接写入二进制数据
            binFile.close();
        } else {
            QMessageBox::warning(this, "错误", "无法打开 BIN 文件");
            isSaveDataRs422 = false;
        }
    }
    else{
//        qDebug()<<"停止写入文本";
    }
}

// LVDS板卡接收数据直接显示
void MainWindow::dataReceivedLVDS(QString hexString){
//    ui->plainTextEdit_receive_2->clear(); // 先清空再填数据，这是临时防止ui界面卡顿的策略
//    ui->plainTextEdit_receive_2->appendPlainText(hexString); // 这里还是会占据线程，导致图像框卡顿
    pendingData_LVDS.append(hexString); // 数据存入缓冲区

    // 检查保存数据标志位，询问是否需要保存数据
    if(isSaveDataLVDS){
        qDebug()<<"正在写入txt文本和bin文本中";
        // 1. 保存为 TXT 文件（文本格式）
        QFile file(saveDirLVDSTxt + fileNameLVDS);
        if(file.open(QIODevice::Append | QIODevice::Text)){
            QTextStream stream(&file);
            stream << hexString;
//            stream << hexString << "\n";  // 加换行符分隔不同数据块
            file.close();

        } else {
            QMessageBox::warning(this, "错误", "无法打开文件");
            isSaveDataLVDS = false;
            return;
        }

        // 2. 保存为 BIN 文件（二进制格式）
        QString binFileName = fileNameLVDS; // 创建副本
        binFileName.replace(".txt", ".bin");
        QFile binFile(saveDirLVDSBin + binFileName); // 替换扩展名
        if (binFile.open(QIODevice::Append)) { // 二进制模式无需 QIODevice::Text
            // 将十六进制字符串转为二进制数据
            QByteArray binaryData = QByteArray::fromHex(hexString.toLatin1());
            binFile.write(binaryData); // 直接写入二进制数据
            binFile.close();
        } else {
            QMessageBox::warning(this, "错误", "无法打开 BIN 文件");
            isSaveDataLVDS = false;
        }
    }
    else{
//        qDebug()<<"停止写入文本";
    }
}

void MainWindow::dataReceivedCAN(QString hexString)
{
    pendingData_CAN.append(hexString); // 数据存入缓冲区

    // 检查保存数据标志位，询问是否需要保存数据
    if(isSaveDataCAN){
        qDebug()<<"正在写入txt文本和bin文本中";
        // 1. 保存为 TXT 文件（文本格式）
        QFile file(saveDirCANTxt + fileNameCAN);
        if(file.open(QIODevice::Append | QIODevice::Text)){
            QTextStream stream(&file);
            stream << hexString;
//            stream << hexString << "\n";  // 加换行符分隔不同数据块
            file.close();

        } else {
            QMessageBox::warning(this, "错误", "无法打开文件");
            isSaveDataCAN = false;
            return;
        }

        // 2. 保存为 BIN 文件（二进制格式）
        QString binFileName = fileNameCAN; // 创建副本
        binFileName.replace(".txt", ".bin");
        QFile binFile(saveDirCANBin + binFileName); // 替换扩展名
        if (binFile.open(QIODevice::Append)) { // 二进制模式无需 QIODevice::Text
            // 将十六进制字符串转为二进制数据
            QByteArray binaryData = QByteArray::fromHex(hexString.toLatin1());
            binFile.write(binaryData); // 直接写入二进制数据
            binFile.close();
        } else {
            QMessageBox::warning(this, "错误", "无法打开 BIN 文件");
            isSaveDataCAN = false;
        }
    }
    else{
//        qDebug()<<"停止写入文本";
    }
}



void MainWindow::cardOpenBtn(bool chooseCard){
     qDebug()<<"当前选择的板卡是："<<chooseCard;
     QString chooseCardString;
     if(chooseCard) chooseCardString = "RS422";
     else chooseCardString = "LVDS";
     statusBarShow(false, "当前选择的板卡是：" + chooseCardString);

    // 创建一个函数，这个函数会获取ui界面关闭板卡设置的所有参数，然后在lvdscardhelper中对板卡参数进行修改
    int cardCount = 2;
    if(cardCount==0){
        ui->pushButton_cardOpenClose->setChecked(false);
        qmessageBoxSet("未检测到板卡！");
        return;
    }
    else{ // 检测到当前运行的主机中存在板卡
        // 判断当前是RSS422板卡还是LVDS板卡
        if(chooseCard){ // RS422板卡
            // 判断按钮是否按下
            if(ui->pushButton_cardOpenClose->isChecked()){ // 按钮被按下
                quint8 currentChannelNo422 = static_cast<quint8>(ui->comboBox_channelNumRS422->currentData().toInt()); // BYTE 获取当前通道数
                qDebug()<<"currentChannelNo422"<<currentChannelNo422; // 0
                statusBarShow(false, "当前选择通道号是：" + QString::number(currentChannelNo422));
                cardWorker1->openCard(true); //打开板卡
                cardWorker1->resetCard(true); // 板卡复位
                cardWorker1->clearFIFO(true); //清理缓存


                /// 设置通道参数
                ST_LVDS_CFG stCfgRS422;
                    memset(&stCfgRS422,0,sizeof(stCfgRS422));
                    // 下面的这些参数都来自于ui界面的combox
                    stCfgRS422.dwBaudRate   = ui->comboBox_baudRate_card->currentData().toUInt();//波特率 DWORD
                    stCfgRS422.ucDataBits   = static_cast<UCHAR>(ui->comboBox_dataBit_card->currentData().toUInt());//数据位; UCHAR
                    stCfgRS422.ucParity     = static_cast<UCHAR>(ui->comboBox_parity_card->currentData().toUInt());//校验位 UCHAR
                    stCfgRS422.ucStopBits   = static_cast<UCHAR>(ui->comboBox_stopBits_card->currentData().toUInt());//停止位 UCHAR
                qDebug()<<stCfgRS422.dwBaudRate;
                qDebug()<<stCfgRS422.ucDataBits;
                qDebug()<<stCfgRS422.ucParity;
                qDebug()<<stCfgRS422.ucStopBits;

                cardWorker1->setChannelNum(true, currentChannelNo422);

                ///板卡通道帧配置
                cardWorker1->configLVDSCardFrame(true);

                /// 板卡通道参数配置
                bool channelSet = cardWorker1->configChannelPara(true,false,currentChannelNo422,stCfgRS422);
                if(channelSet){
                    statusBarShow(true, "成功初始化发送通道");
                }
                else statusBarShow(true, "初始化发送通道失败");
                bool channelSetRec = cardWorker1->configChannelPara(true,true,currentChannelNo422,stCfgRS422);
                if(channelSetRec){
                    statusBarShow(true, "成功初始化接收通道");
                }
                else statusBarShow(true, "初始化接收通道失败");

                bool cardStatus = cardStatus = cardWorker1->getLVDSCardOpenStatus(true);
                if(cardStatus){
                    ui->pushButton_cardOpenClose->setText("关闭");
                }
                else{
                    qmessageBoxSet("RS422板卡打开失败！");
                    ui->pushButton_cardOpenClose->setChecked(false);
                }
            }
            // 按钮关闭
            else{
                // 板卡去使能
                bool isStartCheck = cardWorker1->StopCard(true);
                if(isStartCheck){
                    statusBarShow(true, "RS422板卡去使能成功！");
                    ui->pushButton_startLVDS->setChecked(false);
                    ui->pushButton_startLVDS->setText("使能");
                    // 停止t1
                    qDebug() << "Thread t1 state before quit:" << t1->isRunning();
                    t1->quit();      // 请求线程退出
                    t1->wait();
                    qDebug() << "Thread t1 state after quit:" << t1->isRunning();
                }
                else{
                    statusBarShow(true, "RS422板卡去使能失败！");
                    ui->pushButton_startLVDS->setChecked(true);
                    ui->pushButton_startLVDS->setText("停止");
                }

                // 板卡关闭
                cardWorker1->closeCard(true);
                // 检查板卡状态是否是关闭的，如果是，切换按钮的显示状态
                bool cardIsClosed = cardWorker1->getLVDSCardOpenStatus(true);
                if(!cardIsClosed){ // 422板卡成功关闭
                    ui->pushButton_cardOpenClose->setText("打开");
                    ui->pushButton_cardOpenClose->setChecked(false);
                }
                else{ // 422板卡关闭失败
                    qmessageBoxSet("RS422板卡关闭失败！");
                    ui->pushButton_cardOpenClose->setText("关闭");
                    ui->pushButton_cardOpenClose->setChecked(true);
                }
            }
        }

        // LVDS板卡
        else {
            // 判断按钮是否按下
            if(ui->pushButton_cardOpenClose_2->isChecked()){ // 按钮被按下
                qDebug()<<"按钮被按下";
                quint8 currentChannelNoLVDS = static_cast<quint8>(ui->comboBox_channelNumLVDS->currentData().toInt()); // BYTE 获取当前通道数
                cardWorker2->openCard(false); //打开板卡
                cardWorker2->resetCard(false); // 板卡复位
                cardWorker2->clearFIFO(false); //清理缓存
                qDebug()<<"板卡初始化成功";

                /// 设置通道参数
                ST_LVDS_CFG stCfgLVDS;
                    memset(&stCfgLVDS,0,sizeof(stCfgLVDS));
                    // 下面的这些参数都来自于ui界面的combox
                    stCfgLVDS.dwBaudRate   = ui->comboBox_baudRate_card->currentData().toUInt();//波特率 DWORD
                    stCfgLVDS.ucDataBits   = static_cast<UCHAR>(ui->comboBox_dataBit_card->currentData().toUInt());//数据位; UCHAR
                    stCfgLVDS.ucParity     = static_cast<UCHAR>(ui->comboBox_parity_card->currentData().toUInt());//校验位 UCHAR
                    stCfgLVDS.ucStopBits   = static_cast<UCHAR>(ui->comboBox_stopBits_card->currentData().toUInt());//停止位 UCHAR
                qDebug()<<stCfgLVDS.dwBaudRate;
                qDebug()<<stCfgLVDS.ucDataBits;
                qDebug()<<stCfgLVDS.ucParity;
                qDebug()<<stCfgLVDS.ucStopBits;

                cardWorker2->setChannelNum(false, currentChannelNoLVDS);
                ///板卡通道帧配置
                cardWorker2->configLVDSCardFrame(false);

                /// 板卡通道参数配置
                bool channelSet = cardWorker2->configChannelPara(false,false,currentChannelNoLVDS,stCfgLVDS);
                if(channelSet){
                    statusBarShow(false, "成功初始化发送通道");
                }
                else statusBarShow(false, "初始化发送通道失败");
                bool channelSetRec = cardWorker2->configChannelPara(false,true,currentChannelNoLVDS,stCfgLVDS);
                if(channelSetRec){
                    statusBarShow(false, "成功初始化接收通道");
                }
                else statusBarShow(false, "初始化接收通道失败");

                bool cardStatus = cardStatus = cardWorker2->getLVDSCardOpenStatus(false);
                if(cardStatus){
                    ui->pushButton_cardOpenClose_2->setText("关闭");
                }
                else{
                    qmessageBoxSet("LVDS板卡打开失败！");
                    ui->pushButton_cardOpenClose_2->setChecked(false);
                }
            }
            // 按钮关闭
            else{
                // 板卡去使能
                bool isStartCheck = cardWorker2->StopCard(false);
                if(isStartCheck){
                    statusBarShow(false, "LVDS板卡去使能成功！");
                    ui->pushButton_startLVDS_2->setChecked(false);
                    ui->pushButton_startLVDS_2->setText("使能");
                    // 停止t2
                    qDebug() << "Thread t2 state before quit:" << t2->isRunning();
                    t2->quit();      // 请求线程退出
                    t2->wait();
                    qDebug() << "Thread t2 state after quit:" << t2->isRunning();
                }
                else{
                    statusBarShow(false, "lvds板卡去使能失败！");
                    ui->pushButton_startLVDS_2->setChecked(true);
                    ui->pushButton_startLVDS_2->setText("停止");
                }

                // 板卡关闭
                cardWorker2->closeCard(false);
                // 检查板卡状态是否是关闭的，如果是，切换按钮的显示状态
                bool cardIsClosed = cardWorker2->getLVDSCardOpenStatus(false);
                if(!cardIsClosed){ // lvds板卡成功关闭
                    ui->pushButton_cardOpenClose_2->setText("打开");
                    ui->pushButton_cardOpenClose_2->setChecked(false);
                }
                else{ // lvds板卡关闭失败
                    qmessageBoxSet("lvds板卡关闭失败！");
                    ui->pushButton_cardOpenClose_2->setText("关闭");
                    ui->pushButton_cardOpenClose_2->setChecked(true);
                }
            }
        }
    }
}

void MainWindow::qmessageBoxSet(QString errorString){
    QMessageBox::warning(this,"错误",errorString);
}


//void MainWindow::addCMDList(bool insertOrAdd)
//{
//    /// 我希望在点击添加按钮时，就直接将当前的配置写进结构体然后保存下来，方便后续保存进xml文件
//    // 修改docNodeInitList中当前选中项的配置
//    // docNodeInitList.at(tempIndexListWidget);

//    // 检查当前的StackWidget中是否有tableWidget
//    QWidget* currentPage = ui->stackedWidget_com->currentWidget(); // 假设StackedWidget名称为`stackedWidget`
//    QTableWidget* currentTable = qobject_cast<QTableWidget*>(currentPage);
//    if (!currentTable) {
//       QMessageBox::warning(this, "错误", "当前未显示任何指令表格！");
//       return;
//    }

//    // 获取当前指令配置表格tableWidget的组装指令
//    int sourceColumnIdx = -1;
//    for (int i = 0; i < currentTable->columnCount(); ++i) {
//        QTableWidgetItem* header = currentTable->horizontalHeaderItem(i);
//        if (header && header->text().trimmed() == "源码") { // 匹配列标题“源码”
//            sourceColumnIdx = i;
//            break;
//        }
//    }

//    // 边界处理：未找到“源码”列
//    if (sourceColumnIdx == -1) {
//        QMessageBox::warning(this, "错误", "当前表格缺少\"源码\"列！");
//        return;
//    }

//    // 将指令添加到列表中的一行
//    QStringList sourceCodes; // 存储有效的源码（大写十六进制）
//    int rowCount = currentTable->rowCount();
//    qDebug()<<"rowCount"<<rowCount;

//    for (int row = 0; row < rowCount; ++row) {
//        QTableWidgetItem* sourceItem = currentTable->item(row, sourceColumnIdx);
//        if (!sourceItem) continue; // 跳过无源码的行

//        QString sourceText = sourceItem->text().trimmed();
//        if (sourceText.isEmpty()) continue; // 跳过空值

//        // 验证是否为有效的十六进制（可选，增强鲁棒性）
//        bool isHexValid = true;
//        for (QChar c : sourceText) {
//            if (!c.isDigit() && !((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
//                isHexValid = false;
//                break;
//            }
//        }
//        if (!isHexValid) {
//            QMessageBox::warning(this, "警告", QString("第%1行的源码\"%2\"无效（非十六进制），已跳过！").arg(row+1).arg(sourceText));
//            continue;
//        }

//        sourceCodes << sourceText.toUpper(); // 统一转为大写（可选，规范格式）
//    }
//    qDebug()<<"sourceCodes:"<<sourceCodes;
//    // 边界处理：无有效源码
//    if (sourceCodes.isEmpty()) {
//        QMessageBox::critical(this, "错误", "未收集到任何有效源码，请检查表格！");
//        return;
//    }

//    // ==================== 步骤4：组装指令序列（按行顺序拼接） ====================
//    QString commandSequence = sourceCodes.join(""); // 例如：“2C15400140010020012C28”
//    qDebug()<<"commandSequence:"<<commandSequence;
//    // ==================== 步骤5：获取时间戳（来自用户输入） ====================
//    // 假设“时间戳”输入框是`QSpinBox`，名称为`spinBox_timeStamp`（单位：秒）
//    int timeStamp = ui->spinBox->value(); // 获取当前时间戳数值
//    qDebug()<<"timeStamp:"<<timeStamp;

//    // ==================== 步骤6：添加到目标表格（tableWidget_cmd_immediately） ====================
//    QTableWidget* targetTable = ui->tableWidget_cmd_immediately;
//    int addNewRow = targetTable->rowCount(); // 获取指令序列表格当前行数，同时也是最后一行索引值
//    int tempIndexListWidget = ui->listWidget_2->currentRow(); // 获取当前选中指令序列行索引值
//    qDebug()<<"当前选中的指令索引是"<<tempIndexListWidget;

//    if(!insertOrAdd){
//        // 添加新行
//        targetTable->insertRow(addNewRow);
//        // 设置“时间戳”列（列0为“时间戳（秒）”）
//        QTableWidgetItem* timeItem = new QTableWidgetItem(QString::number(timeStamp));
//        targetTable->setItem(addNewRow, 0, timeItem);

//        // 设置“名称”列（列1为“名称”）
//        QTableWidgetItem* seqItem = new QTableWidgetItem(ui->lb_current_cmd_name->text());
//        targetTable->setItem(addNewRow, 1, seqItem);

//        // 设置“指令序列”列（列2为“源码”）
//        QTableWidgetItem* commandName = new QTableWidgetItem(commandSequence);
//        targetTable->setItem(addNewRow, 2, commandName);

//        // 设置“备注”列（列3为“备注”）
//        QTableWidgetItem* noteItem = new QTableWidgetItem("手动添加"); // 可自定义备注（如“来自相机设置页面”）
//        targetTable->setItem(addNewRow, 3, noteItem);

//        // 获取当前指令组索引，根据索引从docNodeInitList中取出对应的element放在docNodeList的末尾

////          docNodeList.append(docNodeInitList.at(tempIndexListWidget)); // ❌️浅拷贝
//        QDomElement original = docNodeInitList.at(tempIndexListWidget); // ✅️深拷贝:解决指令序列错乱的问题
//        QDomNode clonedNode = original.cloneNode(true);
//        QDomElement clonedElement = clonedNode.toElement();
//        docNodeList.append(clonedElement);

//        qDebug()<<"docNodeList.newcount()"<<docNodeList.count();
//    }
//    else {
//        // 插入新行
//        // 获取当前选中的行索引，如果没有选中，则弹窗警告，提示需要选中一行
//        int insertNewRow = ui->tableWidget_cmd_immediately->currentRow() + 1; // 获取当前选中行后一行索引
//        if (insertNewRow == -1) {
//            QMessageBox::warning(this, "提示", "请先选中一行指令再执行该操作");
//            return;
//        }
//        targetTable->insertRow(insertNewRow);
//        // 设置“时间戳”列（列0为“时间戳（秒）”）
//        QTableWidgetItem* timeItem = new QTableWidgetItem(QString::number(timeStamp));
//        targetTable->setItem(insertNewRow, 0, timeItem);

//        // 设置“名称”列（列1为“名称”）
//        QTableWidgetItem* seqItem = new QTableWidgetItem(ui->lb_current_cmd_name->text());
//        targetTable->setItem(insertNewRow, 1, seqItem);

//        // 设置“指令序列”列（列2为“源码”）
//        QTableWidgetItem* commandName = new QTableWidgetItem(commandSequence);
//        targetTable->setItem(insertNewRow, 2, commandName);

//        // 设置“备注”列（列3为“备注”）
//        QTableWidgetItem* noteItem = new QTableWidgetItem("手动添加"); // 可自定义备注（如“来自相机设置页面”）
//        targetTable->setItem(insertNewRow, 3, noteItem);

//        // 获取当前指令组索引，同时还需要获取当前选中行的索引，根据索引从docNodeInitList中取出对应的element放在docNodeList的对应索引处

////            docNodeList.insert(insertNewRow,docNodeInitList.at(tempIndexListWidget)); ❌️浅拷贝
//        QDomElement original = docNodeInitList.at(tempIndexListWidget); // ✅️深拷贝
//        QDomElement clonedElement = original.cloneNode(true).toElement();
//        docNodeList.insert(insertNewRow, clonedElement);

//        qDebug()<<"docNodeList.newcount()insert"<<docNodeList.count();
//    }

//    // ==================== 步骤7：优化显示（提升用户体验） ====================
////        targetTable->resizeColumnToContents(0); // 自动调整“指令序列”列宽
////        targetTable->resizeColumnToContents(1); // 自动调整“时间戳”列宽
//    targetTable->scrollToBottom(); // 滚动到目标表格底部，显示新添加的行
//}

void MainWindow::addCMDList(bool insertOrAdd)
{
    QWidget* currentPage = ui->stackedWidget_com->currentWidget();
    QTableWidget* currentTable = qobject_cast<QTableWidget*>(currentPage);
    if (!currentTable) {
        QMessageBox::warning(this, "错误", "当前未显示任何指令表格！");
        return;
    }

    int tempIndexListWidget = ui->listWidget_2->currentRow();
    if(tempIndexListWidget < 0){
        QMessageBox::warning(this,"错误","未选择指令模板！");
        return;
    }

    /// ========= 第一步：UI → XML 同步 =========
    QDomElement elem = docNodeInitList[tempIndexListWidget];
    QDomNodeList commandParaList = elem.elementsByTagName("CommandPara");

    for(int row = 0; row < currentTable->rowCount(); ++row)
    {
        QDomElement elemCommandPara = commandParaList.at(row).toElement();

        // ---------- COMBOX ----------
        QWidget* w = currentTable->cellWidget(row,2);
        if(QComboBox* combo = qobject_cast<QComboBox*>(w))
        {
            int comboIndex = combo->currentIndex();
            elemCommandPara.setAttribute("CommandParaInitSelectD", comboIndex);
        }
        // ---------- EDIT ----------
        else if(QTableWidgetItem* editItem = currentTable->item(row,2))
        {
            QVariant data = editItem->data(Qt::UserRole);
            if(data.isValid())   // 说明是 EDIT
            {
                QString value = editItem->text();
                QString hex   = currentTable->item(row,3)->text();

                QDomNodeList paraList = elemCommandPara.elementsByTagName("Para");
                QDomElement elemPara = paraList.at(0).toElement();

                elemPara.setAttribute("ParaInitCode", value);
                elemPara.setAttribute("ParaRawCodeH", hex);
            }
        }
    }

    /// ========= 第二步：收集源码 =========
    int sourceColumnIdx = 3; // 源码列固定第3列
    QStringList sourceCodes;

    for (int row = 0; row < currentTable->rowCount(); ++row)
    {
        QTableWidgetItem* sourceItem = currentTable->item(row, sourceColumnIdx);
        if (!sourceItem) continue;

        QString sourceText = sourceItem->text().trimmed();
        if (sourceText.isEmpty()) continue;

        sourceCodes << sourceText.toUpper();
    }

    if (sourceCodes.isEmpty()) {
        QMessageBox::critical(this, "错误", "未收集到任何有效源码！");
        return;
    }

    QString commandSequence = sourceCodes.join("");
    int timeStamp = ui->spinBox->value();

    QTableWidget* targetTable = ui->tableWidget_cmd_immediately;

    /// ========= 第三步：深拷贝最新XML =========
    QDomElement original = docNodeInitList[tempIndexListWidget];
    QDomElement clonedElement = original.cloneNode(true).toElement();

    if(!insertOrAdd)
    {
        int addNewRow = targetTable->rowCount();
        targetTable->insertRow(addNewRow);

        targetTable->setItem(addNewRow, 0, new QTableWidgetItem(QString::number(timeStamp)));
        targetTable->setItem(addNewRow, 1, new QTableWidgetItem(ui->lb_current_cmd_name->text()));
        targetTable->setItem(addNewRow, 2, new QTableWidgetItem(commandSequence));
        targetTable->setItem(addNewRow, 3, new QTableWidgetItem("手动添加"));

        docNodeList.append(clonedElement);
    }
    else
    {
        int insertNewRow = targetTable->currentRow() + 1;
        if (insertNewRow <= 0) {
            QMessageBox::warning(this, "提示", "请先选中一行指令再执行该操作");
            return;
        }

        targetTable->insertRow(insertNewRow);

        targetTable->setItem(insertNewRow, 0, new QTableWidgetItem(QString::number(timeStamp)));
        targetTable->setItem(insertNewRow, 1, new QTableWidgetItem(ui->lb_current_cmd_name->text()));
        targetTable->setItem(insertNewRow, 2, new QTableWidgetItem(commandSequence));
        targetTable->setItem(insertNewRow, 3, new QTableWidgetItem("手动添加"));

        docNodeList.insert(insertNewRow, clonedElement);
    }

    targetTable->scrollToBottom();
}

void MainWindow::saveDocNodeListToXML(const QString &filePath, QList<QDomElement> &docNodeList)
{
    QDomDocument doc;

    // 创建根节点
    QDomElement root = doc.createElement("SYSTEM");
    doc.appendChild(root);

    // 遍历 docNodeList，把每个 QDomElement 加到新文档中
    for (int i = 0; i < docNodeList.size(); ++i) {
//        // 测试
//        qDebug() << "第" << i << "行 CommandParaInitSelectD = "
//                     << docNodeList[i].attribute("CommandParaInitSelectD");

        QDomElement &cmd = docNodeList[i];  // 引用，直接修改

        // 从表格第 0 列取 CommandTimeD
        if (QTableWidgetItem *item = ui->tableWidget_cmd_immediately->item(i, 0)) {
            updateCommandAttribute(cmd, "CommandTimeD", item->text());
        }

        // 从表格第 4 列取备注（假设 CommandSpecificationT 就是备注）
        if (QTableWidgetItem *item = ui->tableWidget_cmd_immediately->item(i, 3)) {
            updateCommandAttribute(cmd, "RemarksD", item->text());
        }

        // 再拷进目标文档
        QDomNode imported = doc.importNode(cmd, true);
        root.appendChild(imported);
    }

//    for (int i = 0; i < docNodeList.size(); ++i)
//    {
//        QDomElement cmd = docNodeList[i];

//        QDomElement commandPara = cmd.firstChildElement("CommandPara");

//        QComboBox* combo = qobject_cast<QComboBox*>(
//            ui->tableWidget_cmd_immediately->cellWidget(i, 2));

//        if(combo)
//        {
//            commandPara.setAttribute("CommandParaInitSelectD",
//                                     QString::number(combo->currentIndex()));
//        }

//        QDomNode imported = doc.importNode(cmd, true);
//        root.appendChild(imported);
//    }

//    for (int i = 0; i < ui->tableWidget_cmd_immediately->rowCount(); ++i)
//    {
//        QComboBox* combo =
//            qobject_cast<QComboBox*>(ui->tableWidget_cmd_immediately->cellWidget(i, 2));
//        if(combo)
//            qDebug() << "UI 第" << i << "行 index =" << combo->currentIndex();
//    }

//    for (int i = 0; i < docNodeList.size(); ++i)
//    {
//        QDomElement cmd = docNodeList[i];
//        QDomElement para = cmd.firstChildElement("CommandPara");

//        qDebug() << "XML 第" << i << "行 index ="
//                 << para.attribute("CommandParaInitSelectD");
//    }
//    for (int i = 0; i < ui->tableWidget_cmd_immediately->rowCount(); ++i)
//    {
//        QWidget* w = ui->tableWidget_cmd_immediately->cellWidget(i, 2);
//        qDebug() << "第" << i << "行 widget =" << w;
//    }

    // 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, "保存失败", "无法保存XML文件: " + filePath);
        return;
    }

    QTextStream out(&file);
    doc.save(out, 4); // 4 表示缩进
    file.close();

    QMessageBox::information(this, "保存成功", "文件已保存到:\n" + filePath);
}

// 工具函数：修改 parent 下 tagName 节点的文本
// 修改 Command 节点的属性
void MainWindow::updateCommandAttribute(QDomElement &commandElem,
                            const QString &attrName,
                            const QString &newValue)
{
    if (commandElem.isNull()) return;
    commandElem.setAttribute(attrName, newValue);
}


// 右键单击ui->listWidget_2菜单删除指令组
void MainWindow::onListWidgetContextMenuRequested(const QPoint &pos)
{
    // 获取点击位置的 item
    QListWidgetItem *item = ui->listWidget_2->itemAt(pos);
    if (!item) return;  // 如果没点在 item 上，不弹菜单

    QMenu menu(this);
    QAction *deleteAction = menu.addAction("删除该项");

    QAction *selectedAction = menu.exec(ui->listWidget_2->mapToGlobal(pos));
    if (selectedAction == deleteAction) {
        // 弹出确认对话框
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "确认删除",
                                      QString("确定要删除“%1”吗？").arg(item->text()),
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            int row = ui->listWidget_2->row(item); // 先取索引

            // 1. 先删除 UI
            delete ui->listWidget_2->takeItem(row);

            // 2. 删除后台数据（使用之前保存的索引）
            xmlcommandparser->deleteComChoose(row);

            // 3. 删除docNodeInitList中对应内容
            docNodeInitList.removeAt(row);

            // 4. 重要：删除commandList中对应的项目
            if (row < commandList.size()) {
                commandList.removeAt(row);
            }

            // 5. 同时删除stackWidget中对应的页面
            if (row < ui->stackedWidget_com->count()) {
                QWidget *widget = ui->stackedWidget_com->widget(row);
                ui->stackedWidget_com->removeWidget(widget);
                delete widget;
            }

            qDebug()<<"2docNodeInitList.count() "<<docNodeInitList.count();
        }
    }
}

void MainWindow::sendCMD(int sendIndex, bool directOrList)
{
    // 检查当前指令应该从哪个接口发送
    // ①可见光相机从串口1发送，波特率3M②红外相机从串口2发送，波特率115200

    // 调用相应的接口数据发送函数
    // 获取当前源码所在的列
    // 获取表头列数
    int columnCount = ui->tableWidget_cmd_immediately->columnCount();
    if(columnCount<1){
        QMessageBox::warning(this, "提示", "请配置并载入指令序列");
        return;
    }
    int row = -1;
//    int row = ui->tableWidget_cmd_immediately->currentRow(); // 获取当前选中行
    if(!directOrList){
       row = sendIndex;
    }
    else {
        row = ui->tableWidget_cmd_immediately->currentRow(); // 获取当前选中行
    }

    if (row == -1) {
        QMessageBox::warning(this, "提示", "请选需要发送的立即令序列");
        return;
    }

    int sourceColumnIndex = -1;
    // 遍历表头，查找“源码”所在列索引
    for (int col = 0; col < columnCount; ++col) {
        QString headerText = ui->tableWidget_cmd_immediately->horizontalHeaderItem(col)->text();
        if (headerText == "源码") {
            sourceColumnIndex = col;
            break;
        }
    }
    if(sourceColumnIndex == -1){
        QMessageBox::warning(this, "提示", "当前指令序列中没有“源码”");
        return;
    }

    // 获取纯指令数据
    QString tempStringData = ui->tableWidget_cmd_immediately->item(row,sourceColumnIndex)->text();

    // 获取帧头、帧尾、校验和长度
     QString frameHead;
     QString frameTail;
     int checkSumLength;
     QDomElement  tempElem  = docNodeList.at(row).toElement();
     frameHead  = tempElem.attribute("FrameHeader");
     frameTail = tempElem.attribute("FrameTail");
     checkSumLength = tempElem.attribute("CheckSumLength").toInt();
     qDebug()<<"listHeader"<<frameHead;
     qDebug()<<"listTail"<<frameTail;
     qDebug()<<"listCheckSumLength"<<checkSumLength;

    // 添加校验和
    QByteArray immediateCommand = dataTransfer->string2Bytearray(tempStringData);
    QByteArray checkSum =  frameHelper->frameChecksum(immediateCommand,checkSumLength);
    QString checkSumString = QString(checkSum.toHex().toUpper());

    // 添加帧头帧尾
    QString frameHead0 = "B9E0"; // 是否所有指令都是这个帧头
    QString byteDataSendString = frameHead + tempStringData + checkSumString + frameTail;
//    QString byteDataSendString = frameHead0 + tempStringData + checkSumString;
    qDebug()<<"byteDataSendString"<<byteDataSendString;
    QByteArray byteDataSend = dataTransfer->string2Bytearray(byteDataSendString);

    // 通过串口1发送
    if(ui->checkBox_COMI->isChecked()) emit writeDataSignal(byteDataSend);
    // 通过串口2发送
    if(ui->checkBox_COMII->isChecked()) emit writeDataSignal_2(byteDataSend);
    // 通过板卡RS422发送
    if (ui->checkBox_CARDRS422->isChecked()){
        // 通过板卡发送
        // 检查板卡是否使能
        bool cardOpenStatus = cardWorker1->getLVDSCardOpenStatus(false);
        if(!cardOpenStatus){
            qmessageBoxSet("发送失败，RS422板卡未打开！");
            return;
        }
        else {
            // 获取通道数
            quint8 channel = cardWorker1->getChannelNum(true);
            DWORD dwSendRet = cardWorker1->sendData(true,channel, reinterpret_cast<quint8*>(byteDataSend.data()), static_cast<size_t>(byteDataSend.length())); // 新的写法
        }
    }

    // 通过CAN总线发送
    if(ui->checkBox_CAN->isChecked() ){
        // tempStringData是没有添加任何外壳的纯指令数据（QString），immediateCommand是没有添加任何外壳的纯指令数据（QByteArray）
        // 创建一个专用的CAN指令组装函数，这个函数用于给指令数据套上外壳，如校验、指令码等
        // ⭐️指令码（2字节）
       QString paraCode  = tempElem.attribute("paraCode");
       // 长度字节
       QString commandLenth = "";
       //   ⭐️字节长度(0字节或2字节)
       if(immediateCommand.size()<=5){
           // 单帧足够
            commandLenth = ""; // 0字节
       }
       else{
           // 需要多帧才够
           commandLenth = QString("%1").arg(immediateCommand.size() + 2, 4, 16, QChar('0')).toUpper(); // 2字节
       }

        // 先组一帧
        QString canParaQSring =  commandLenth + paraCode + tempStringData;
        QByteArray canParaQByteArray = dataTransfer->string2Bytearray(canParaQSring);
        //⭐️校验和（1字节）
        // 判断单帧和多帧，要匹配不同的校验和
        QByteArray checkSumCan;
        if(commandLenth.size() >0){
            QByteArray tempArray = canParaQByteArray.mid(2,-1);
            checkSumCan =  frameHelper->frameChecksum(tempArray,checkSumLength);
        }
        else{
            checkSumCan =  frameHelper->frameChecksum(canParaQByteArray,checkSumLength);
        }
        QString checkSumCanString = QString(checkSumCan.toHex().toUpper());
        // 检查指令长度，包括指令码、校验和，如果超过8字节就添加额外的长度，占据第一第二字节
        canParaQSring += checkSumCanString;
        QByteArray commandCanFrame = dataTransfer->string2Bytearray(canParaQSring);
        qDebug()<<"*********canParaQSring*********"<<canParaQSring;
        auto frames = canFrameHelper->frameCAN(commandCanFrame,canFrameConfigUi2); // 指令页面配置
        if(ui->pushButton_startCAN->text() == "停止"){
            auto frames = canFrameHelper->frameCAN(commandCanFrame,canFrameConfigUi2);
            for(auto &obj : frames)
            {
                int ret = canWorker1->transmit(nDeviceType, nDeviceInd, nCANInd, &obj,1);  // 发送数据，其实就已经写进缓存区了

                if(ret > 0){
                    QString byteDataSendSizeString = QString::number(obj.DataLen);
                    ui->plainTextEdit_statusBar_3->appendPlainText("数据发送成功，数据长度：" + byteDataSendSizeString + "字节");
                    // qDebug() << "CAN 发送成功，字节数：" << byteDataSend.size();
                } else {
                    // qDebug() << "CAN 发送失败！错误码：" << VCI_ReadErrInfo(DevType, DevIndex, 0);
                    ui->plainTextEdit_statusBar_3->appendPlainText("数据发送失败");
                }
                QThread::msleep(35);
            }
        }
        else{
            qmessageBoxSet("请先使能CAN通道！");
        }

    }
}

void MainWindow::delCMDList()
{
    int row = ui->tableWidget_cmd_immediately->currentRow(); // 获取当前选中行
    if (row >= 0) {
        ui->tableWidget_cmd_immediately->removeRow(row); // 删除该行
        // 同时还要删除docNodeList中对应的元素
        docNodeList.removeAt(row);
    } else {
        QMessageBox::warning(this, "提示", "请先选中要删除的行");
    }
    qDebug()<<"docNodeList.count()"<<docNodeList.count();

}

void MainWindow::clearCMDList()
{
//    ui->tableWidget_cmd_immediately->clear(); // 只能清理表格内容
//    ui->tableWidget_cmd_immediately->setRowCount(0);  // 删除所有
    /**
    上述代码会导致程序崩溃，原因是clear() 这个函数 不仅清空内容，还清空了表头（Header Labels）和列结构，所以之后调用 setItem(newRow, 0, item) 就会崩溃
    因为 tableWidget_cmd_immediately 根本就没有列了，设置第 0 列就等于访问非法内存。
    在本程序的最开始，就已经对tableWidget_cmd_immediately进行表头和行结构的设置了：
    ui->tableWidget_cmd_immediately->setColumnCount(4); // 设置3列（指令序列、时间戳、备注）
    ui->tableWidget_cmd_immediately->setHorizontalHeaderLabels({
        "时间戳（秒）",    // 列0
        "名称",// 列1
        "源码"  ,       // 列2
        "备注"         // 列3
    });
    这里使用clear（）会把这些初始化信息删除，导致后面尝试从表格中查询源码时找不到这些结构信息，发生崩溃，解决方式就是在clear后及时补充表格初始化信息，如下
    */

    ui->tableWidget_cmd_immediately->setRowCount(0); // 删除所有行

    docNodeList.clear();             // ⭐ 必须加

    // 重设列数并重新设置表头，防止崩溃
    ui->tableWidget_cmd_immediately->setColumnCount(4); // 设置3列（指令序列、时间戳、备注）
    ui->tableWidget_cmd_immediately->setHorizontalHeaderLabels({
        "时间戳（秒）",    // 列0
        "名称",// 列1
        "源码"  ,       // 列2
        "备注"         // 列3
    });

}

// 加在开头或构造函数外部
QSize MainWindow::lastSize = QSize();  // ✅ 静态成员变量定义：在类中声明了静态变量，所有静态成员变量都必须在类外再定义一次，否则链接器（linker）会找不到它的地址


void MainWindow::saveImgSingle()
{
    // 检查图像框中是否有图像item，如果有就保存，以系统时间命名，如果没有就报错
    QGraphicsScene *currentScene = ui->graphicsView->scene();
    if(!currentScene){
       qmessageBoxSet("保存图像失败，图像框中没有关联scene!");
       return;
    }

    // 查找第一个 QGraphicsPixmapItem
    QGraphicsPixmapItem *pixmapItem = nullptr;
    for (QGraphicsItem *item : currentScene->items()) {
        pixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem *>(item);
        if (pixmapItem) {
            break;
        }
    }

    if (!pixmapItem) {
        QMessageBox::information(this, "提示", "图像框中没有图像！");
        return;
    }

    // 获取图像并保存
    QPixmap pixmap = pixmapItem->pixmap();

    // 获取当前系统时间作为文件名
    QDateTime currentTime = QDateTime::currentDateTime();
    QString fileNameSingle = saveDirLVDSImgSingle + currentTime.toString("yyyyMMdd_hhmmss") + ".bmp"; /// 这里不能使用jpg，使用jpg会在光斑周围产生很多杂乱像素
    if (!pixmap.isNull()) {
        if (!fileNameSingle.isEmpty()) {
            if (!pixmap.save(fileNameSingle)) {
                QMessageBox::warning(this, "错误", "保存图像失败！");
            } else {
                // 同时输出超阈值点像素坐标
                if(ui->checkBox_overGrayScaleCoordinates->isChecked()){
                    // 将overThresholdCoords中的每一个QPoint打印到excel中
                    // 创建名为fileNameExcel的excel文件
                    qDebug()<<"overThresholdCoords"<<overThresholdCoords.count();
                    QString fileNameExcel = saveDirLVDSImgSingle + currentTime.toString("yyyyMMdd_hhmmss") + ".csv"; /// 这里不能使用jpg，使用jpg会在光斑周围产生很多杂乱像素
                    QFile file(fileNameExcel);
                        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
                            qWarning() << "Cannot open file for writing:" << fileNameExcel;
                            return;
                        }

                        QTextStream out(&file);
                        out << "X,Y\n";  // 可选：写表头
                        for(const QPoint &pt : overThresholdCoords){
                            out << pt.x() << "," << pt.y() << "\n";  // 每个坐标一行
                        }
                        qDebug()<<"222222222";
                        file.close();
                        qDebug() << "Saved coordinates to" << fileNameExcel;
                }
                QMessageBox::information(this, "成功", "图像保存成功！");
            }
        }
    } else {
        QMessageBox::warning(this, "错误", "图像数据为空！");
    }
}

void MainWindow::saveImgSpan()
{
    if(!ui->pushButton_beginSaveImages_2->isChecked()){
        ui->pushButton_beginSaveImages_2->setChecked(true);
        qmessageBoxSet("请等待当前保存任务结束");
        return;
    }

    // 获取持续保存图片的时间
    QString saveImgTime = ui->lineEdit_saveImagesTime_2->text();
    double saveImgTimeDouble = saveImgTime.toDouble();
    QDateTime currentTime = QDateTime::currentDateTime();
    LVDSImgSpanName = saveDirLVDSImgSpan + currentTime.toString("yyyyMMdd_hhmmss") + "-" + saveImgTime + "s/";
    // 创建文件夹
    QDir imgFolderSpan(LVDSImgSpanName);
    if(!imgFolderSpan.exists()){
        if(!imgFolderSpan.mkpath(".")){
            QMessageBox::critical(this, "错误", "无法创建目录: " + LVDSImgSpanName);
            return;
        }
    }
    isSaveImgLVDS = true;
    ui->pushButton_beginSaveImages_2->setChecked(true);
    ui->pushButton_beginSaveImages_2->setText("保存中");
    // 设计一个定时器，按照用户设定的时间来计时，当时间到时，isSaveImgLVDS重置为false
    // 启动定时器，在指定时间后停止保存
    QTimer::singleShot(static_cast<int>(saveImgTimeDouble * 1000), this, [this](){
        isSaveImgLVDS = false; // 停止保存
        ui->pushButton_beginSaveImages_2->setChecked(false);
        ui->pushButton_beginSaveImages_2->setText("开始保存");
        QMessageBox::information(this, "保存完成", "图像已保存完毕");
    });
}

void MainWindow::showImage(QImage image)
{

    // 清除 scene 中已有的内容
    ui->graphicsView->scene()->clear();

    // 创建新的 QGraphicsPixmapItem 来显示图像
    pixmapItem->setPixmap(QPixmap::fromImage(image));
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    ui->graphicsView->scene()->addItem(item);

//    // 设置视图自动适应图像大小
//    ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);  // 这里不推荐使用自适应，因为会阻碍图像的自由缩放和拖动等行为
//    // ✅ 设置 scene 的边界范围（让它比图像大一点，才能拖）
//    QRectF imgRect = item->boundingRect();
//    qreal margin = std::max(imgRect.width(), imgRect.height()) * 0.5; // 50% 扩展
//    ui->graphicsView->setSceneRect(imgRect.adjusted(-margin, -margin, margin, margin));

    if (image.size() != lastSize){
       lastSize  =  image.size();
       ui->graphicsView->setSceneRect(item->boundingRect().adjusted(-1000, -1000, 1000, 1000));  // adjusted(left, top, right, bottom)
    }

    // 保存图像数据：这里需要注意的是，线程算力是否支持同时显示图像和保存图像，是否会出现线程拥挤的情况，如果实际测试时出现异常，考虑
    // 设计成在保存图片时暂停显示图像，等图片保存任务结束后继续显示图像。
    if(isSaveImgLVDS){
        QDateTime currentTime = QDateTime::currentDateTime();
        QString imgNameSpan = LVDSImgSpanName + currentTime.toString("yyyyMMdd_hhmmss_zzz") + ".bmp";
        // 检查图像框中是否有图像item，如果有就保存，以系统时间命名，如果没有就报错
        QGraphicsScene *currentScene = ui->graphicsView->scene();
        if(!currentScene){
           qmessageBoxSet("保存图像失败，图像框中没有关联scene!");
           return;
        }

        // 查找第一个 QGraphicsPixmapItem
        QGraphicsPixmapItem *pixmapItem = nullptr;
        for (QGraphicsItem *item : currentScene->items()) {
            pixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem *>(item);
            if (pixmapItem) {
                break;
            }
        }

        if (!pixmapItem) {
            QMessageBox::information(this, "提示", "图像框中没有图像！");
            return;
        }

        // 获取图像并保存
        QPixmap pixmap = pixmapItem->pixmap();
        if (!pixmap.isNull()) {
            if (!imgNameSpan.isEmpty()) {
                if (!pixmap.save(imgNameSpan)) {
                    QMessageBox::warning(this, "错误", "保存图像失败！");
                }
            }
        } else {
            QMessageBox::warning(this, "错误", "图像数据为空！");
        }
    }
}

void MainWindow::readTmXML(QString xmlFiles)
{
    xmlcommandparser->loadTmXML(xmlFiles);
}
void MainWindow::readTmXML_slow(QString xmlFiles)
{
    xmlcommandparser->loadTmXML_slow(xmlFiles);
}

void MainWindow::repeatSend()
{
    ui->plainTextEdit_statusBar_3->appendPlainText("开启重复发送（广播模拟模式）");
    if(ui->pushButton_reTransmit->text() =="循环发送" && ui->pushButton_startCAN->text() == "停止"){
        ui->pushButton_reTransmit->setText("关闭");
//        connect(t6, &QThread::started, broadcastWorker, &BroadcastWorker::threadRun);
        double delayTimeUi = ui->lineEdit_cycleTime->text().toDouble();
        emit  openCloseBroadcastSignal(true,delayTimeUi);
        // ✅ UI 线程读取
        QString text = ui->plainTextEdit_send_3->toPlainText();

        // 假设你是十六进制字符串
        QByteArray data = QByteArray::fromHex(text.remove(' ').toLatin1());

        emit updateBroadcastDataSignal(data);
    }
    else{
        ui->pushButton_reTransmit->setText("循环发送");
        double delayTimeUi = ui->lineEdit_cycleTime->text().toDouble();
        emit  openCloseBroadcastSignal(false,delayTimeUi);
    }
}

void MainWindow::repeatTransmit()
{
//    broadcastTimer->stop();
//    ui->pushButton_reTransmit->setText("停止发送");
//    broadcastSend = true;
    // 发送遥测请求指令
    // 发送周期，单位 ms（比如从 UI 读）
    int intervalMs = ui->lineEdit_cycleTime->text().toInt();

    if (!broadcastTimer->isActive()&& ui->pushButton_startCAN->text() == "停止") {
        // 开始循环发送
        broadcastSend = true;
        broadcastTimer->start(intervalMs);
        ui->pushButton_reTransmit->setText("停止发送");
        qDebug() << "开始循环发送窗口数据";
    }
    else if (!broadcastTimer->isActive()&& ui->pushButton_startCAN->text() == "使能") {
        QMessageBox::warning(this,"error","请先使能CAN");
    }
    else {
        // 停止
        broadcastSend = false;
        broadcastTimer->stop();
        ui->pushButton_reTransmit->setText("循环发送");
        qDebug() << "停止循环发送窗口数据";
    }
}

void MainWindow::sendDataDirect()
{
    if (ui->pushButton_openCAN->text()=="打开") {
        ui->plainTextEdit_statusBar_3->appendPlainText("请先打开 CAN 通道！");
        return;
    }
    // 准备数据
    QString tempStringData = ui->plainTextEdit_send_3->toPlainText();
    QByteArray byteDataSend = dataTransfer->string2Bytearray(tempStringData);
    qDebug()<<"CAN byteDataSend.size："<<byteDataSend.size();


    auto frames = canFrameHelper->frameCAN(byteDataSend,canFrameConfigUi);
    for(auto &obj : frames)
    {
        int ret = canWorker1->transmit(nDeviceType, nDeviceInd, nCANInd, &obj,1);  // 发送数据，其实就已经写进缓存区了
        if(ret > 0){
            QString byteDataSendSizeString = QString::number(obj.DataLen);
//                obj.Data
            ui->plainTextEdit_statusBar_3->appendPlainText("数据发送成功，数据长度：" + byteDataSendSizeString + "字节");
            // qDebug() << "CAN 发送成功，字节数：" << byteDataSend.size();
        } else {
             qDebug() << "CAN 发送失败";
            ui->plainTextEdit_statusBar_3->appendPlainText("数据发送失败");
        }
//            int frameDelayTime = ui->lineEdit_frameDelayTime->text().toInt();
//            QThread::msleep(static_cast<unsigned long>(frameDelayTime)); // 这里需要延时一下，否则dsp那边芯片接收有问题
//            QThread::msleep(0);
    }
}

void MainWindow::readXML(QString xmlFiles, bool setCurrentToNew)
{
    if (xmlcommandparser->loadXML(xmlFiles,docNodeInitList)) {

        int rowCom = 0; // 初始时默认为0
        xmlcommandparser->populateStackedWidget(ui->stackedWidget_com, ui->listWidget_2,setCurrentToNew, docNodeInitList, rowCom); // 将 CommandList中的每一个Command模块（一般是一个xml） 填入 ui 中的 QTabWidget
    }
}

void MainWindow::readXML2(QString xmlFiles)
{
    xmlcommandparser->loadXML2(xmlFiles,docNodeList);
}

void MainWindow::addRowToInterestTable(int row)
{
    int newRow = rowSort.size(); // 默认插到最后
    for (int k = 0; k < rowSort.size();++k) {
        if(row<rowSort.at(k)){
            newRow = k;
            break;
        }
    }
    rowSort.insert(newRow,row);

    QList<QStandardItem*> items;
    for (int col = 0; col < modelMain->columnCount(); ++col) {
        // 复制主表格的内容（不包括复选框）
        if (col == 5) continue; // 不要复选框这一列
        QStandardItem *item = modelMain->item(row, col);
        if (item) {
            items.append(new QStandardItem(item->text()));
        }
    }
    modelInterest->insertRow(newRow,items);
    ui->tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // 每列宽度会按内容调整
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
//    ui->tableView_2->setColumnWidth(2, 120); // 生效且不会被覆盖
//    ui->tableView_2->setColumnWidth(3, 120); // 生效且不会被覆盖
    ui->tableView_2->horizontalHeader()->setStretchLastSection(true); // 最后一列会拉伸填满剩余空间
}

void MainWindow::addRowToInterestTable_slow(int row)
{
    int newRow = rowSort_slow.size(); // 默认插到最后
    for (int k = 0; k < rowSort_slow.size();++k) {
        if(row<rowSort_slow.at(k)){
            newRow = k;
            break;
        }
    }
    rowSort_slow.insert(newRow,row);

    QList<QStandardItem*> items;
    for (int col = 0; col < modelMain_slow->columnCount(); ++col) {
        // 复制主表格的内容（不包括复选框）
        if (col == 5) continue; // 不要复选框这一列
        QStandardItem *item = modelMain_slow->item(row, col);
        if (item) {
            items.append(new QStandardItem(item->text()));
        }
    }
    modelInterest_slow->insertRow(newRow,items);
    ui->tableView_4_slow->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // 每列宽度会按内容调整
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
//    ui->tableView_2->setColumnWidth(2, 120); // 生效且不会被覆盖
//    ui->tableView_2->setColumnWidth(3, 120); // 生效且不会被覆盖
    ui->tableView_4_slow->horizontalHeader()->setStretchLastSection(true); // 最后一列会拉伸填满剩余空间
}

void MainWindow::removeRowFromInterestTable(int row)
{
    QString id = modelMain->item(row, 0)->text(); // 取参数编号
    for (int r = 0; r < modelInterest->rowCount(); ++r) {
        if (modelInterest->item(r, 0)->text() == id) {
            modelInterest->removeRow(r);
            break;
        }
    }
    int index = rowSort.indexOf(row);
    if(index != -1) rowSort.removeAt(index);
//    ui->tableView_2->resizeColumnsToContents();
    // 每次更新兴趣表格时，为了显示全内容，重新设置单元格尺寸自适应
    ui->tableView_2->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // 每列宽度会按内容调整
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
//    ui->tableView_2->setColumnWidth(2, 120); // 生效且不会被覆盖
//    ui->tableView_2->setColumnWidth(3, 120); // 生效且不会被覆盖
    ui->tableView_2->horizontalHeader()->setStretchLastSection(true); // 最后一列会拉伸填满剩余空间
}

void MainWindow::removeRowFromInterestTable_slow(int row)
{
    QString id = modelMain_slow->item(row, 0)->text(); // 取参数编号
    for (int r = 0; r < modelInterest_slow->rowCount(); ++r) {
        if (modelInterest_slow->item(r, 0)->text() == id) {
            modelInterest_slow->removeRow(r);
            break;
        }
    }
    int index = rowSort_slow.indexOf(row);
    if(index != -1) rowSort_slow.removeAt(index);
//    ui->tableView_2->resizeColumnsToContents();
    // 每次更新兴趣表格时，为了显示全内容，重新设置单元格尺寸自适应
    ui->tableView_4_slow->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // 每列宽度会按内容调整
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
//    ui->tableView_2->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
//    ui->tableView_2->setColumnWidth(2, 120); // 生效且不会被覆盖
//    ui->tableView_2->setColumnWidth(3, 120); // 生效且不会被覆盖
    ui->tableView_4_slow->horizontalHeader()->setStretchLastSection(true); // 最后一列会拉伸填满剩余空间
}

void MainWindow::init()
{
    /// xml：注意，readTmXML()内部会发送很多信号，如果槽函数是在readTmXML(xmlTmPath);之后创建的，那么这些信号发送时，槽函数都不会有反应，因为此时还没有创建连接
    qDebug()<<"***************初始化**************";
    readTmXML(xmlTmPath);
    readTmXML_slow(xmlTmPath_2);
}


void MainWindow::statusBarShow(bool chooseCard,QString showString){
    if(chooseCard) ui->plainTextEdit_statusBar->appendPlainText(showString);
    else ui->plainTextEdit_statusBar_2->appendPlainText(showString);
}

//// 最大保留的行数（可以根据需要调整）
//const int MAX_LOG_LINES = 50; // 限制串口接收数据显示框中最多显示50行

//// 高性能追加并限制行数
//void MainWindow::appendWithLimit(QPlainTextEdit *edit, const QString &text)
//{
//    // 禁用撤销，防止内存占用累积
//    edit->setUndoRedoEnabled(false);

//    // 追加新内容
//    edit->appendPlainText(text);

//    // 检查并删除多余行
//    QTextDocument *doc = edit->document();


//    /// 方式2：使用if一次性删除多余的行，减少操作频率，但是如果数据更新过快，一样有风险，删除操作是在主线程执行的，如果日志更新非常频繁，仍可能导致卡顿
//    /// 每次删除多行时还是逐行操作(for循环)，不够高效
////    if (doc->blockCount() > MAX_LOG_LINES)
////    {
////        QTextCursor cursor(doc);
////        cursor.movePosition(QTextCursor::Start);
////        for (int i = 0; i < doc->blockCount() - MAX_LOG_LINES; ++i) {
////            cursor.select(QTextCursor::BlockUnderCursor);
////            cursor.removeSelectedText();
////            cursor.deleteChar();
////        }
////    }

//    /// 方式1：使用while一行一行删除多余的行，操作频率高
////    while (doc->blockCount() > MAX_LOG_LINES)
////    {
////        QTextCursor cursor(doc);
////        cursor.movePosition(QTextCursor::Start);
////        cursor.select(QTextCursor::BlockUnderCursor);
////        cursor.removeSelectedText();
////        cursor.deleteChar(); // 删除换行符
////    }
//}


// 添加停止定时器的槽函数
void MainWindow::on_stopSaveButton_clicked()
{
    if (m_saveTimer && m_saveTimer->isActive()) {
        m_saveTimer->stop();

        isSaveTMFile = false;
        ui->pb_begin_save_data->setChecked(false);
        ui->pb_begin_save_data->setText("开始");
        QMessageBox::information(this, "保存已取消", "数据保存已被中途停止");
    }
}
