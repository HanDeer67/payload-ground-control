#include "dialogcanframe.h"
#include "ui_dialogcanframe.h"


DialogCanFrame::DialogCanFrame(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCanFrame)
{
    ui->setupUi(this);

    ui->comboBox_srcAdrr->addItem("星务", 1);
    ui->comboBox_srcAdrr->addItem("正X星遥星间激光载荷", 10);
    ui->comboBox_srcAdrr->addItem("负X星遥星间激光载荷",11);
    ui->comboBox_srcAdrr->addItem("星载路由",8);
    ui->comboBox_srcAdrr->setCurrentIndex(0);
    ui->lineEdit_srcAdrr->setText((ui->comboBox_srcAdrr->currentData().toString()));

    ui->comboBox_destAdrr->addItem("星务", 1);
    ui->comboBox_destAdrr->addItem("正X星遥星间激光载荷", 10);
    ui->comboBox_destAdrr->addItem("负X星遥星间激光载荷",11);
    ui->comboBox_destAdrr->addItem("星载路由",8);
    ui->comboBox_destAdrr->addItem("姿态组播测试（临时）",63);
    ui->comboBox_destAdrr->setCurrentIndex(4);
    ui->lineEdit_destAdrr->setText((ui->comboBox_destAdrr->currentData().toString()));

    ui->comboBox_priority->addItem("遥控指令",0);
    ui->comboBox_priority->addItem("广播/组播指令",1);
    ui->comboBox_priority->addItem("遥测参数类数据",2);
    ui->comboBox_priority->addItem("其余数据",3);
    ui->comboBox_priority->setCurrentIndex(0);
    ui->lineEdit_priority->setText((ui->comboBox_priority->currentData().toString()));

    ui->comboBox_multicast->addItem("点对点传输",0);
    ui->comboBox_multicast->addItem("广播",3);
    ui->comboBox_multicast->addItem("组播",1);
    ui->comboBox_multicast->addItem("保留",2);
    ui->comboBox_multicast->setCurrentIndex(0);
    ui->lineEdit_multicast->setText((ui->comboBox_multicast->currentData().toString()));

    ui->comboBox_funCode->addItem("自主发送",0);
    ui->comboBox_funCode->addItem("轮询控制序列",1);
    ui->comboBox_funCode->addItem("轮询应答",2);
    ui->comboBox_funCode->addItem("遥控指令/数据",3);
    ui->comboBox_funCode->addItem("保留",4);
    ui->comboBox_funCode->setCurrentIndex(1);
    ui->lineEdit_funCode->setText(ui->comboBox_funCode->currentData().toString());


    connect(ui->comboBox_srcAdrr, QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
       ui->lineEdit_srcAdrr->setText((ui->comboBox_srcAdrr->currentData().toString()));
    });
    connect(ui->comboBox_destAdrr, QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
       ui->lineEdit_destAdrr->setText((ui->comboBox_destAdrr->currentData().toString()));
    });
    connect(ui->comboBox_priority, QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
       ui->lineEdit_priority->setText((ui->comboBox_priority->currentData().toString()));
    });
    connect(ui->comboBox_multicast, QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
       ui->lineEdit_multicast->setText((ui->comboBox_multicast->currentData().toString()));
    });
    connect(ui->comboBox_funCode, QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
       ui->lineEdit_funCode->setText((ui->comboBox_funCode->currentData().toString()));
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogCanFrame::onOkButtonClicked);



}

DialogCanFrame::~DialogCanFrame()
{
    delete ui;
}

void DialogCanFrame::onOkButtonClicked(){
    // 将can指令帧配置发送给主UI
    canFrameConfigDia.priority=ui->comboBox_priority->currentData().toUInt();
    canFrameConfigDia.srcAddress=ui->comboBox_srcAdrr->currentData().toUInt();
    canFrameConfigDia.multicast=ui->comboBox_multicast->currentData().toUInt();
    canFrameConfigDia.destAdrr=ui->comboBox_destAdrr->currentData().toUInt();
    canFrameConfigDia.funCode=ui->comboBox_funCode->currentData().toUInt();

    emit updateCanFrameConfigUiSignal(canFrameConfigDia);
}
