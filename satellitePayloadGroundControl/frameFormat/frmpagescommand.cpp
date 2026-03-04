#include "frmpagescommand.h"
#include "ui_frmpagescommand.h"
#include <QComboBox>
#include <QTableWidgetItem>
#include <QVariant>
#include <QDropEvent>
#include <QMimeData>
#include <QEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>

#include "luahelper.h"
#include "delegate.h"
#include "frameassemble.h"
#include "frmshowmsg.h"
#include "app.h"

#define WORK_COLUMN  2

#define LocalRole Qt::UserRole+100



frmPagesCommand::frmPagesCommand(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmPagesCommand)
{
    ui->setupUi(this);
    this->initTabel();

    //绑定接收rs422响应数据
    connect(FrameAssemble::Instance(),&FrameAssemble::sendRS422Respond,this,&frmPagesCommand::processRS422Respond);
}

frmPagesCommand::~frmPagesCommand()
{
    delete ui;
}

void frmPagesCommand::initTabel()
{
    //设置table1
    QTableWidget *tableWidget = ui->tableWidget_cmd;

    QStringList headLabelList;
    //参数编号 参数名称 参数数值 源码 备注
    headLabelList << tr("参数编号")<< tr("参数名称") << tr("参数数值")<< tr("源码")<< tr("备注");
    tableWidget->setColumnCount(headLabelList.length());
    tableWidget->setHorizontalHeaderLabels(headLabelList);
    tableWidget->verticalHeader()->setVisible(false);//隐藏垂直表头
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);//选择一整行
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);//选择单个目标
    tableWidget->setAlternatingRowColors(true);
    tableWidget->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(tableWidget->horizontalHeader()->count()-2,QHeaderView::Stretch);

    //设置table2
    tableWidget = ui->tableWidget_cmd_immediately;
    headLabelList.clear();
    headLabelList << tr("时间") << tr("名称")<< tr("源码")<< tr("备注");//tr("序号")
    tableWidget->setColumnCount(headLabelList.length());
    tableWidget->setHorizontalHeaderLabels(headLabelList);
    tableWidget->verticalHeader()->setVisible(true);//显示垂直表头
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);//选择一整行
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);//选择单个目标
    tableWidget->setAlternatingRowColors(true);
    tableWidget->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(tableWidget->horizontalHeader()->count()-3,QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(tableWidget->horizontalHeader()->count()-2,QHeaderView::Stretch);

    //拖拽相关
    tableWidget->setDragDropMode(QAbstractItemView::DragDrop);
    tableWidget->viewport()->installEventFilter(this);
    tableWidget->setDropIndicatorShown(false);  // drop位置 提示

    //解析XML
    Command::Instance()->initXML(App::cmdConfigDirPath+"cmd.xml");
    Command::Instance()->registerTabelWidget(ui->tableWidget_cmd,ui->listWidget);

    QListWidgetItem * item = ui->listWidget->item(0);
    if(item)ui->lb_current_cmd_name->setText(item->text());

    //一个指令分为:指令名称 时间戳
    //参数编号 参数名称 参数数值 源码 备注


    //item变更后将变更后的内容进行处理,并计算十六进制数据并更新全局数据，处理后进入dataChanged中进行右侧数据进行更新
    //connect(ui->tableWidget_cmd,&QTableWidget::itemChanged,this,&frmPagesCommand::itemChanged);
    //connect(ui->tableWidget_cmd->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

    //listwidget设置
    QFont font(QStringLiteral("微软雅黑"), 12);
    ui->listWidget->setFont(font);
    //取消边框
    ui->listWidget->setFrameShape(QListWidget::NoFrame);

    //双击事件 获取当前命令的索引
    connect(ui->listWidget,&QListWidget::doubleClicked,this,[=](QModelIndex index){

        currentCommandIndex = index.row();
        Command::Instance()->changeCommand(currentCommandIndex);
        ui->lb_current_cmd_name->setText(ui->listWidget->item(index.row())->text());
        //Command::Instance()->sendCMD();
    });
}

void frmPagesCommand::showResponseResult(QString msg)
{
    QString text = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+":"+msg;
    static int lineCount = 0;

    if(lineCount>10000)
    {
        ui->te_response_result->clear();
        lineCount = 0;
    }
    lineCount++;
    ui->te_response_result->append(text);

    //定位到最后
    QTextCursor cursor = ui->te_response_result->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->te_response_result->setTextCursor(cursor);
}

bool frmPagesCommand::eventFilter(QObject *watched, QEvent *event)
{
    QTableWidget *tableWidget = ui->tableWidget_cmd_immediately;
    if(watched == tableWidget->viewport())
    {
        if(event->type() == QEvent::Drop)
        {
            const QMimeData *mime = ((QDropEvent*)event)->mimeData();
            QByteArray encodedata = mime->data("application/x-qabstractitemmodeldatalist");
            if (encodedata.isEmpty())
            {
                return false;
            }
            QDataStream stream(&encodedata, QIODevice::ReadOnly);
            while (!stream.atEnd())
            {
                int srcRow, srcCol;
                QMap<int,  QVariant> roleDataMap;
                ///拖的row和col
                stream >> srcRow >> srcCol >> roleDataMap;
                QTableWidgetItem* pDropItem = tableWidget->itemAt(((QDropEvent*)event)->pos());
                if(!pDropItem)
                {
                    return true;
                }

                //放的row
                int destRow = pDropItem->row();



                if(destRow == srcRow)
                {
                    return true;
                }

                 //再此处调整 immdCmdInfo 的列表顺序
                if(srcRow>=immdCmdInfo.length() || destRow>=immdCmdInfo.length()) return true;

                CommandInfo Info = immdCmdInfo.at(srcRow);
                immdCmdInfo.removeAt(srcRow);
                immdCmdInfo.insert(destRow,Info);

                int rowMax = 0;
                int rowMin = 0;

                QVector<QTableWidgetItem*> lineItemList;
                //先取出源数据
                for(int i = 0;i < tableWidget->columnCount();i++){            // 遍历列
                    lineItemList<<tableWidget->takeItem(srcRow,i);
                }

                if(destRow < srcRow)
                {
                    rowMax = srcRow;
                    rowMin = destRow;
                    for (int row=rowMax;row>rowMin;row--) {
                        for(int i = 0;i < tableWidget->columnCount();i++){            // 遍历列
                            tableWidget->setItem(row,i,tableWidget->takeItem(row-1,i));// 每一列item的移动
                        }
                    }
                }
                else {
                    rowMax = destRow;
                    rowMin = srcRow;
                    for (int row=rowMin;row<rowMax;row++) {
                        for(int i = 0;i < tableWidget->columnCount();i++){            // 遍历列
                            tableWidget->setItem(row,i,tableWidget->takeItem(row+1,i));// 每一列item的移动
                        }
                    }

                }
                //将源数据赋值到目标数据
                for(int i = 0;i < tableWidget->columnCount();i++){            // 遍历列
                    tableWidget->setItem(destRow,i,lineItemList[i]);           // 每一列item的移动
                }

                for (int i = 0;i<immdCmdInfo.length();i++) {
                    qDebug()<<"time:"<<immdCmdInfo.at(i).CommandTime;
                }

                //不要交给系统处理，否则他会给你新增一行
                return true;
            }
        }
        else
        {
            return QWidget::eventFilter(watched,event);
        }
    }
    return QWidget::eventFilter(watched,event);
}

void frmPagesCommand::showEvent(QShowEvent *event)
{
//    currentCommandIndex = 0;
//    Command::Instance()->changeCommand(currentCommandIndex);
//    ui->lb_current_cmd_name->setText(ui->listWidget->item(0)->text());
}

void frmPagesCommand::itemChanged(QTableWidgetItem *item)
{
    //根据
    QTableWidget* tableWidget = ui->tableWidget_cmd;

    int col = tableWidget->column(item);
    int row = tableWidget->row(item);
    //qDebug()<<"itemChanged" << row <<col<<item->text();
    if(col != WORK_COLUMN)return;


    if(item->text().isEmpty())return;

    //根据当前的指令类型

    //根据当前命令行获取类型
    CommandInfo cmdInfo = Command::Instance()->CommandList.at(currentCommandIndex);

    if(cmdInfo.paraInfo.length()<=row)return;

    ParamInfo paramInfo = cmdInfo.paraInfo.at(row);

    if(paramInfo.type == PARAM_TYPE::COMBOBOX)
    {
        int index = paramInfo.paraNameList.indexOf(item->text());
        if(index<0)return;
        Command::Instance()->CommandList[currentCommandIndex].paraInfo[row].currentShowIndex = index;
        item->setData(LocalRole, paramInfo.paraRawCodeList.at(index));
    }
    else if(paramInfo.type == PARAM_TYPE::EDIT)
    {

        //根据当前值进行计算
        Command::Instance()->CommandList[currentCommandIndex].paraInfo[row].paraInitCodeList[0] = item->text();
        double x = item->text().toDouble();
        //lua测试
        double ret = LuaHelper::Instance()->execString(paramInfo.paramEncodeFun,x);
        int showLen = Command::Instance()->CommandList[currentCommandIndex].paraInfo[row].paramByteLength*2;
        QString  result = QString("%1").arg((quint16)ret,showLen,16,QLatin1Char('0')).toUpper();
        //qDebug()<<paramInfo.paramDecodeFun<<x<<item->text()<<ret<<result;
        Command::Instance()->CommandList[currentCommandIndex].paraInfo[row].paraRawCodeList[0] = result;
        item->setData(LocalRole,result);
    }
}

void frmPagesCommand::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    //qDebug()<<"dataChanged:"<<topLeft.row()<<topLeft.column();
    //return;
    int row = topLeft.row();
    int col = topLeft.column();

    if(col != WORK_COLUMN)return;

    QTableWidget *tableWidget = ui->tableWidget_cmd;

    QTableWidgetItem *item = tableWidget->item(row,col);
    if(item)
    {
        QVariant data1 = item->data(LocalRole);
        if(topLeft.column()+1< tableWidget->columnCount() && topLeft.row()<tableWidget->rowCount())
        {
            QTableWidgetItem *item2 = tableWidget->item(topLeft.row(),topLeft.column()+1);
            if(item2)
            {
                item2->setText(data1.toString());
            }
        }
    }
}

void frmPagesCommand::processRS422Respond(QByteArray &rs422Data)
{

    //发送过来的是整个传输层数据 因此需要截取 13个字节以后点数据以及提出最后点校验2个字节
    QByteArray data = rs422Data.mid(13,rs422Data.length()-13-2);
    frmShowMsg::Instance()->showResponseMsg(data);

    //获取数据长度
    int length = (quint8)data.at(1)*256*256+(quint8)data.at(2)*256+(quint8)data.at(3);
    //进行校验
    //进行校验对比
    quint16 frameCrcValue = (quint8)data.at(data.length()-2)*256+(quint8)data.at(data.length()-1);
    quint16 clcCrcValue = FrameAssemble::rs422CheckCRC(data);
    if(frameCrcValue != clcCrcValue)
    {
        qDebug()<<"processRS422Respond crc error!"<<"帧中校验数据:"<<frameCrcValue<<"计算校验数据:"<<clcCrcValue;
        return;
    }

    QByteArray respondData = data.mid(4,length);
    responseCount++;
    ui->lb_local_count->setText(QString::number(responseCount));

    //响应帧第一个字节
    if((quint8)respondData.at(0) == 0x80)
    {

        //获取时间戳
        quint32 time = ((quint8)respondData.at(1)<<24) + ((quint8)respondData.at(2)<<16)+((quint8)respondData.at(3)<<8)+(quint8)respondData.at(4);
        ui->lb_time->setText(QString::number(time));
        //参数长度
        quint8 paramLength = (quint8)respondData.at(5);
        //指令类型
        quint8 cmdType = (quint8)respondData.at(6);
        ui->lb_cmd_id->setText("0x"+QString::number(cmdType,16));

        //是否正确接收
        quint8 isSuccess = (quint8)respondData.at(7);

        //如果错误 错误类型
        if(isSuccess == 0x23)
        {
            ui->lb_result->setText("正确");
            frmShowMsg::Instance()->showResponseResultMsg("指令接收正确!");
            showResponseResult("指令接收正确!");
        }
        //如果错误 错误类型
        else if(isSuccess == 0x26)
        {
            ui->lb_result->setText("错误");
            QString msg = "指令接收错误:";
            quint32 errorNo = (quint8)data.at(8)<<24 + (quint8)data.at(9)<<16+(quint8)data.at(10)<<8+(quint8)data.at(11);
            if(errorNo == 0x30)
            {
                msg+="校验和错误";
                ui->lb_error_type->setText("校验和错误");

            }
            else if(errorNo == 0x33)
            {
                msg+="接收超时";
                ui->lb_error_type->setText("接收超时");
            }
            else if(errorNo == 0x36)
            {
                msg+="复合帧序号错误";
                 ui->lb_error_type->setText("复合帧序号错误");
            }
            else
            {
                msg+="无效";
                 ui->lb_error_type->setText("无效");
            }
            msg+="!";
            frmShowMsg::Instance()->showResponseResultMsg(msg);
            showResponseResult(msg);
        }
        else
        {
            //无效
            frmShowMsg::Instance()->showResponseResultMsg("指令接收错误:无效!");
            showResponseResult("指令接收错误:无效!");
            ui->lb_result->setText("无效");
            ui->lb_error_type->setText("无效");
        }
    }
}

void frmPagesCommand::on_pb_send_cmd_immediately_clicked()
{
    //立即发送 当前选中的cmd
    Command::Instance()->sendCMD();
}

void frmPagesCommand::on_pb_add_cmd_clicked()
{
    //获取当前时间
    int time = ui->spinBox->value();
    //根据当前currentCommandIndex 获取
    CommandInfo cmdInfo = Command::Instance()->CommandList.at(currentCommandIndex);
    cmdInfo.CommandTime = QString::number(time);
    //将当前的信息插入
    QTableWidget *tableWidget = ui->tableWidget_cmd_immediately;
    int lineCount = tableWidget->rowCount();
    tableWidget->setRowCount(lineCount+1);
    tableWidget->setItem(lineCount,0,new QTableWidgetItem(cmdInfo.CommandTime));
    tableWidget->setItem(lineCount,1,new QTableWidgetItem(cmdInfo.CommandName));
    tableWidget->setItem(lineCount,2,new QTableWidgetItem(Command::Instance()->getCMDString(cmdInfo)));
    tableWidget->setItem(lineCount,3,new QTableWidgetItem(cmdInfo.CommandRemark));
    immdCmdInfo.append(cmdInfo);
}

void frmPagesCommand::on_pb_clear_cmd_clicked()
{
    immdCmdInfo.clear();
    QTableWidget *tableWidget = ui->tableWidget_cmd_immediately;
    tableWidget->clearContents();
    tableWidget->setRowCount(immdCmdInfo.length());
}

void frmPagesCommand::on_pb_del_cmd_clicked()
{
    //先选中行
     QTableWidget *tableWidget = ui->tableWidget_cmd_immediately;
     QList<QTableWidgetItem*> items  = tableWidget->selectedItems();
    //如果没有选中行则不能进行删除
    if(!items.empty())
    {
        int row = items.first()->row();
        immdCmdInfo.removeAt(row);
        tableWidget->removeRow(row);
    }
    else
    {
       QMessageBox::warning(this,"警告","请先选择一行再进行操作");
    }
}

void frmPagesCommand::on_pb_insert_cmd_clicked()
{
    //先选中行
     QTableWidget *tableWidget = ui->tableWidget_cmd_immediately;
     QList<QTableWidgetItem*> items  = tableWidget->selectedItems();
    //如果没有选中行则不能进行删除
    if(!items.empty())
    {
        int row = items.first()->row();
        //获取当前时间
        int time = ui->spinBox->value();
        //根据当前currentCommandIndex 获取
        CommandInfo cmdInfo = Command::Instance()->CommandList.at(currentCommandIndex);
        cmdInfo.CommandTime = QString::number(time);
        //将当前的信息插入
        tableWidget->insertRow(row);
        tableWidget->setItem(row,0,new QTableWidgetItem(cmdInfo.CommandTime));
        tableWidget->setItem(row,1,new QTableWidgetItem(cmdInfo.CommandName));
        tableWidget->setItem(row,2,new QTableWidgetItem(Command::Instance()->getCMDString(cmdInfo)));
        tableWidget->setItem(row,3,new QTableWidgetItem(cmdInfo.CommandRemark));
        immdCmdInfo.insert(row,cmdInfo);
    }
    else
    {
       QMessageBox::warning(this,"警告","请先选择一行再进行操作");
    }
}

void frmPagesCommand::on_pb_export_xml_clicked()
{
    //打开目录保存对应的文件名称
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("保存文件"),
            "",
            tr("XML Files (*.xml)"));

        if (!fileName.isNull())
        {
            //fileName是文件名
            qDebug()<<fileName<<immdCmdInfo.length();
            //le_save_xml_name
            if(immdCmdInfo.length()>0)Command::Instance()->writeXML(fileName,immdCmdInfo);
        }
        else
        {
            //点的是取消
        }


}

void frmPagesCommand::on_pb_import_xml_clicked()
{
    //1.打开文件对话框 选择对应的xml文件

    QString fileName = QFileDialog::getOpenFileName(this,
            tr("打开文件"),
            "",
            tr("XML Files (*.xml)"),
            0);
        if (!fileName.isNull())
        {
            //fileName是文件名
            qDebug()<<fileName;

            //判断是否能解析正确
            immdCmdInfo = Command::Instance()->readXML(fileName);
            //2.遍历结果 输出到界面
            if(immdCmdInfo.length()>0)refreshTable();
        }
        else{
            //点的是取消
        }




}

void frmPagesCommand::refreshTable()
{
    //先选中行
     QTableWidget *tableWidget = ui->tableWidget_cmd_immediately;
     tableWidget->clearContents();
     tableWidget->setRowCount(immdCmdInfo.length());

     for (int i = 0; i < immdCmdInfo.length();i++) {
         CommandInfo cmdInfo = immdCmdInfo.at(i);
         tableWidget->setItem(i,0,new QTableWidgetItem(cmdInfo.CommandTime));
         tableWidget->setItem(i,1,new QTableWidgetItem(cmdInfo.CommandName));
         tableWidget->setItem(i,2,new QTableWidgetItem(Command::Instance()->getCMDString(cmdInfo)));
         tableWidget->setItem(i,3,new QTableWidgetItem(cmdInfo.CommandRemark));
     }
}


void frmPagesCommand::on_pb_send_cmd_clicked()
{
    Command::Instance()->sendCMDList(immdCmdInfo);
}

void frmPagesCommand::on_pb_t_1_clicked()
{
    QString hexData = "550000000001CC";
    QByteArray cmd = QByteArray::fromHex(hexData.toLocal8Bit());
    FrameAssemble::Instance()->RS422FixedFrameAssemble(cmd);
}

void frmPagesCommand::on_pb_t_2_clicked()
{
    QString hexData = "550000000001CB";
    QByteArray cmd = QByteArray::fromHex(hexData.toLocal8Bit());
    FrameAssemble::Instance()->RS422FixedFrameAssemble(cmd);
}

void frmPagesCommand::on_pb_t_3_clicked()
{
    QString hexData = "550000000001CA";
    QByteArray cmd = QByteArray::fromHex(hexData.toLocal8Bit());
    FrameAssemble::Instance()->RS422FixedFrameAssemble(cmd);
}
