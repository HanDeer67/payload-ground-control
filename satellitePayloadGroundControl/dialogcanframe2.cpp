#include "dialogcanframe2.h"
#include "ui_dialogcanframe2.h"


DialogCanFrame2::DialogCanFrame2(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCanFrame2)
{
    ui->setupUi(this);

    ui->comboBox_srcAdrr_2->addItem("星务", 1);
    ui->comboBox_srcAdrr_2->addItem("正X星遥星间激光载荷", 10);
    ui->comboBox_srcAdrr_2->addItem("负X星遥星间激光载荷",11);
    ui->comboBox_srcAdrr_2->addItem("星载路由",8);
    ui->comboBox_srcAdrr_2->setCurrentIndex(0);
    ui->lineEdit_srcAdrr_2->setText((ui->comboBox_srcAdrr_2->currentData().toString()));

    ui->comboBox_destAdrr_2->addItem("星务", 1);
    ui->comboBox_destAdrr_2->addItem("正X星遥星间激光载荷", 10);
    ui->comboBox_destAdrr_2->addItem("负X星遥星间激光载荷",11);
    ui->comboBox_destAdrr_2->addItem("星载路由",8);
    ui->comboBox_destAdrr_2->setCurrentIndex(1);
    ui->lineEdit_destAdrr_2->setText((ui->comboBox_destAdrr_2->currentData().toString()));

    ui->comboBox_priority_2->addItem("遥控指令",0);
    ui->comboBox_priority_2->addItem("广播/组播指令",1);
    ui->comboBox_priority_2->addItem("遥测参数类数据",2);
    ui->comboBox_priority_2->addItem("其余数据",3);
    ui->comboBox_priority_2->setCurrentIndex(0);
    ui->lineEdit_priority_2->setText((ui->comboBox_priority_2->currentData().toString()));

    ui->comboBox_multicast_2->addItem("点对点传输",0);
    ui->comboBox_multicast_2->addItem("广播",3);
    ui->comboBox_multicast_2->addItem("组播",1);
    ui->comboBox_multicast_2->addItem("保留",2);
    ui->comboBox_multicast_2->setCurrentIndex(0);
    ui->lineEdit_multicast_2->setText((ui->comboBox_multicast_2->currentData().toString()));

    ui->comboBox_funCode_2->addItem("自主发送",0);
    ui->comboBox_funCode_2->addItem("轮询控制序列",1);
    ui->comboBox_funCode_2->addItem("轮询应答",2);
    ui->comboBox_funCode_2->addItem("遥控指令/数据",3);
    ui->comboBox_funCode_2->addItem("保留",4);
    ui->comboBox_funCode_2->setCurrentIndex(3);
    ui->lineEdit_funCode_2->setText(ui->comboBox_funCode_2->currentData().toString());


    connect(ui->comboBox_srcAdrr_2, QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
       ui->lineEdit_srcAdrr_2->setText((ui->comboBox_srcAdrr_2->currentData().toString()));
    });
    connect(ui->comboBox_destAdrr_2, QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
       ui->lineEdit_destAdrr_2->setText((ui->comboBox_destAdrr_2->currentData().toString()));
    });
    connect(ui->comboBox_priority_2, QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
       ui->lineEdit_priority_2->setText((ui->comboBox_priority_2->currentData().toString()));
    });
    connect(ui->comboBox_multicast_2, QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
       ui->lineEdit_multicast_2->setText((ui->comboBox_multicast_2->currentData().toString()));
    });
    connect(ui->comboBox_funCode_2, QOverload<int>::of(&QComboBox::currentIndexChanged),this,[=](){
       ui->lineEdit_funCode_2->setText((ui->comboBox_funCode_2->currentData().toString()));
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogCanFrame2::onOkButtonClicked);


}

DialogCanFrame2::~DialogCanFrame2()
{
    delete ui;
}


void DialogCanFrame2::onOkButtonClicked(){
    // 将can指令帧配置发送给主UI
    // 将can指令帧配置发送给主UI
    canFrameConfigDia2.priority=ui->comboBox_priority_2->currentData().toUInt();
    canFrameConfigDia2.srcAddress=ui->comboBox_srcAdrr_2->currentData().toUInt();
    canFrameConfigDia2.multicast=ui->comboBox_multicast_2->currentData().toUInt();
    canFrameConfigDia2.destAdrr=ui->comboBox_destAdrr_2->currentData().toUInt();
    canFrameConfigDia2.funCode=ui->comboBox_funCode_2->currentData().toUInt();

    emit updateCanFrameConfigUiSignal(canFrameConfigDia2);
}
