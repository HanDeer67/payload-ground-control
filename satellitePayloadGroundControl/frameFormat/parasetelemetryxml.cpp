#include "parasetelemetryxml.h"
#include <QFile>
#include <QMessageBox>
#include <QDomDocument>
#include <QtDebug>
#include <QCheckBox>
#include <QHeaderView>
#include "luahelper.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include "comboxdelegate.h"
#include "checkbox.h"
#include "app.h"

#include <QMutex>
ParaseTelemetryXml *ParaseTelemetryXml::self = nullptr;
ParaseTelemetryXml *ParaseTelemetryXml::Instance()
{
    if (!self) {
        QMutex mutex;
        QMutexLocker locker(&mutex);
        if (!self) {
            self = new ParaseTelemetryXml;
        }
    }

    return self;
}


class FilterModel : public QSortFilterProxyModel
{
public:
    FilterModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override {
        QModelIndex index = sourceModel()->index(sourceRow, 6, sourceParent);
        return (sourceModel()->data(index).toString() == "是");
    }
};


ParaseTelemetryXml::ParaseTelemetryXml(QObject *parent) : QObject(parent)
{

}

void ParaseTelemetryXml::paraseXml(QString xmlName)
{
    // 打开文件
    QFile file(xmlName);
    if (!file.open(QFileDevice::ReadOnly)) {
        QMessageBox::information(nullptr, "提示", xmlName+"文件打开失败!"+file.errorString());
        return;
    }

    QDomDocument doc;

    // 将doc与file关联起来
    // 这个函数从IO设备dev中读取XML文档，如果内容被成功解析，则返回true;否则返回false。
    if (!doc.setContent(&file)) {
        QMessageBox::information(nullptr, "提示", "操作的文件不是XML文件!");
        file.close();
        return;
    }
    // 关联后可以关闭文件了
    file.close();

    //获取根节点
    QDomElement root = doc.documentElement();
    //获取SYSTEM名字
    systemName = readAttributeWithString(root,"SYSTEMSpecificationT");

    //获取XML文件类型
    xmlType = readAttributeWithString(root,"XMLtypeT");


    //获取第一个孩子节点
    //QDomElement ele = root.firstChildElement();
    //qDebug()<<ele.nodeName();

    //获取节点名字为TMitem的所有节点
    QDomNodeList cmdList = root.elementsByTagName("TMitem");
    //qDebug()<<"Command个数:"<<cmdList.count();

    //每个单独的一行 有可能是下拉框 数值 文本 等
    //编号  名称 值  源码  有效性

    //处理每个单独的命令
    for (int i = 0; i < cmdList.size(); i++) {
        // 获取节点
        QDomNode nodei = cmdList.at(i);
        if(nodei.isElement())
        {
            TM tm;
            QDomElement elementI = nodei.toElement();
            //获取命令的名称通过读取CommandSpecificationT属性

            //要解析TMitem中的占用几个字节 itemList 并且需要知道是什么类型的数据 int double float等 十六进制显示 QString  int char 等等

            //编号
            tm.TMItemName = readAttributeWithString(elementI,"TMitemNoName");
            //名称
            tm.TMItemSpecification = readAttributeWithString(elementI,"TMitemSpecificationT");
            //占几个字节
            tm.TmItemByteLength =  readAttributeWithString(elementI,"TMitemByteLengthD").toShort();

            allByteLength+=tm.TmItemByteLength;

            //数据类型 int EDIT
            tm.TmItemType = readAttributeWithString(elementI,"TMitemType");

            //转换公式
            tm.TmItemFun = readAttributeWithString(elementI,"TMitemEncodeTypeT");

            //处理命令下面的参数
            QDomNodeList cmdParaList = elementI.elementsByTagName("Para");
            for (int j =0;j<cmdParaList.length();j++) {
                 QDomNode nodej = cmdParaList.at(j);
                 if(nodej.isElement())
                 {
                     QDomElement elementJ = nodej.toElement();
                    //如果有下拉框 则需要知道显示的QString和对应的十六进制 也就是RawCodeH
                    QString value = readAttributeWithString(elementJ,"ParaSpecificationT");
                    QString key = readAttributeWithString(elementJ,"ParaRawCodeH");
                    tm.listMap.insert(key,value);
                 }
            }

            itemList.append(tm);
        }
    }

    isParaserInit = true;
}

void ParaseTelemetryXml::refreashXml(QString xmlName)
{
    //清空数组
    itemList.clear();
    allByteLength = 0;
    isParaserInit = false;
    //解析xml
    paraseXml(xmlName);
    //初始化widget
    //initTable()
}

void ParaseTelemetryXml::initTable(QTableWidget *tableWidget)
{
    if(!tableWidget)
    {
        qDebug()<<"widget is null!";
        return;
    }
    //tableWidget->clearContents();
    QStringList headLabelList;
    //参数编号 参数名称 参数数值 源码 备注
    headLabelList <<"参数编号"<<"参数名称"<<"数值"<<"源码"<<"有效性"<<"是否感兴趣"<<"测试";
    tableWidget->setColumnCount(headLabelList.length());
    tableWidget->setHorizontalHeaderLabels(headLabelList);
    tableWidget->verticalHeader()->setVisible(false);//隐藏垂直表头
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);//选择一整行
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);//选择单个目标
    tableWidget->setAlternatingRowColors(true);
    tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    //设置table的行数
    tableWidget->setRowCount(itemList.length());

    for(int i = 0;i<itemList.length();i++)
    {

        QTableWidgetItem* item0 = new QTableWidgetItem(itemList.at(i).TMItemName);
        item0->setFlags(item0->flags() ^ Qt::ItemIsEditable);
        tableWidget->setItem(i,0,item0);

        QTableWidgetItem* item1 = new QTableWidgetItem(itemList.at(i).TMItemSpecification);
        item1->setFlags(item1->flags() ^ Qt::ItemIsEditable);
        tableWidget->setItem(i,1,item1);

        QTableWidgetItem* item2 = new QTableWidgetItem("暂无数据");
        item2->setFlags(item2->flags() ^ Qt::ItemIsEditable);
        tableWidget->setItem(i,2,item2);

        QTableWidgetItem* item3 = new QTableWidgetItem("暂无数据");
        item3->setFlags(item3->flags() ^ Qt::ItemIsEditable);
        tableWidget->setItem(i,3,item3);

        QTableWidgetItem* item4 = new QTableWidgetItem("是");
        item4->setFlags(item4->flags() ^ Qt::ItemIsEditable);
        tableWidget->setItem(i,4,item4);

        QCheckBox * checkBox = new QCheckBox("感兴趣");

        tableWidget->setCellWidget(i,5,checkBox);
    }

    QAbstractItemModel *model = tableWidget->model();



    //tableView_3
    //tableWidget-

}

void ParaseTelemetryXml::initTable(QTableView *widget,QTableView *widget2)
{

    model = new QStandardItemModel;

    QStringList headLabelList;
    headLabelList <<"参数编号"<<"参数名称"<<"数值"<<"源码"<<"有效性"<<"是否关注"<<"是否";
    model->setHorizontalHeaderLabels(headLabelList);

    model->setColumnCount(headLabelList.length());

    //设置table的行数
    model->setRowCount(itemList.length());

    FilterModel *filterModel = new FilterModel();
    filterModel->setSourceModel(model);
    widget->setModel(model);
    widget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    //widget->setColumnWidth(0,65);
    //widget->setColumnWidth(4,60);
    //widget->setColumnWidth(5,65);

    widget2->setModel(filterModel);
    widget2->hideColumn(5);
    widget2->hideColumn(6);
    widget->hideColumn(6);

    widget->verticalHeader()->setVisible(false);//隐藏垂直表头
    widget->setSelectionBehavior(QAbstractItemView::SelectRows);//选择一整行
    widget->setSelectionMode(QAbstractItemView::SingleSelection);//选择单个目标
    widget->setAlternatingRowColors(true);
    widget->setColumnWidth(0,70);
    widget->setColumnWidth(4,50);
    widget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    widget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    widget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    widget2->verticalHeader()->setVisible(false);//隐藏垂直表头
    widget2->setSelectionBehavior(QAbstractItemView::SelectRows);//选择一整行
    widget2->setSelectionMode(QAbstractItemView::SingleSelection);//选择单个目标
    widget2->setAlternatingRowColors(true);

    //widget2->setColumnWidth(0,65);
    //widget2->setColumnWidth(4,60);
    //widget2->setColumnWidth(5,65);
    widget2->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    widget2->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    widget2->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    for(int i = 0;i<itemList.length();i++)
    {

        QStandardItem* item0 = new QStandardItem(itemList.at(i).TMItemName);
        item0->setFlags(item0->flags() ^ Qt::ItemIsEditable);
        model->setItem(i,0,item0);

        QStandardItem* item1 = new QStandardItem(itemList.at(i).TMItemSpecification);
        item1->setFlags(item1->flags() ^ Qt::ItemIsEditable);
        model->setItem(i,1,item1);

        QStandardItem* item2 = new QStandardItem("暂无数据");
        item2->setFlags(item2->flags() ^ Qt::ItemIsEditable);
        model->setItem(i,2,item2);

        QStandardItem* item3 = new QStandardItem("暂无数据");
        item3->setFlags(item3->flags() ^ Qt::ItemIsEditable);
        model->setItem(i,3,item3);

        QStandardItem* item4 = new QStandardItem("是");
        item4->setFlags(item4->flags() ^ Qt::ItemIsEditable);
        model->setItem(i,4,item4);

        QStandardItem* item6 = new QStandardItem("否");
        //item5->setTextAlignment(Qt::AlignCenter);
        model->setItem(i,6,item6);
        if(item6->text() == "是")
        {
            item6->setForeground(selectColor);
        }
        else
        {
            item6->setForeground(unSelectColor);
        }

        CheckBox * checkBox = new CheckBox("关注");
        checkBox->row = i;
        checkBox->col = 5;

        connect(checkBox,&CheckBox::clicked,this,[=](){
            //qDebug()<<"checkBox"<<checkBox<<"Clicked!"<<checkBox->row<<checkBox->col;
            QStandardItem* item6 = model->item(checkBox->row,checkBox->col+1);
            //qDebug()<<item6<<checkBox->isChecked();

            if(item6)
            {
                if(checkBox->isChecked())
                {
                    item6->setText("是");
                }
                else
                {
                    item6->setText("否");
                }
            }

            widget2->resizeColumnToContents(1);
            widget2->resizeColumnToContents(2);
            widget2->resizeColumnToContents(3);

        });
        //model.setCellWidget(i,5,checkBox);
        widget->setIndexWidget(model->index(i,5),checkBox);
    }

    //widget->setSelectionModel();

//    qDebug()<<"inti table!";
//    //不行就添加一个代理 ComBox 选中 添加到这个里面
//    connect(widget, &QTableView::clicked, this,[=](const QModelIndex &index){
//        QCheckBox *box = qobject_cast<QCheckBox*>(sender());
//        qDebug()<<box;
//        qDebug()<<index.row();
//    });


    //ComBoxDelegate * delegate = new ComBoxDelegate;
    //widget->setItemDelegateForColumn(6,delegate);

    //发送信号调整感兴趣区域的列宽
//    connect(delegate,&ComBoxDelegate::closeEditor,this,[=]()
//    {
//        widget2->resizeColumnToContents(1);
//        widget2->resizeColumnToContents(2);
//        widget2->resizeColumnToContents(3);
//    });
    //
    connect(model,&QStandardItemModel::dataChanged,this,&ParaseTelemetryXml::dataChanged);

}
//遥测解析部分
void ParaseTelemetryXml::paraseData(QByteArray data)
{
    //遥测数据预留512个字节 解析收到的数据并更新到tabel上
    if(!isParaserInit)
    {
        qDebug()<<"未初始化!";
        return;
    }
    if(allByteLength>data.length())
    {
        qDebug()<<"数据长度与模板长度不匹配!";
        //return;
    }

    for (int i =0; i< itemList.length();i++) {
        //首先判断该域需要几个字节然后截取对应的个数
        QByteArray itemData = data.mid(0,itemList.at(i).TmItemByteLength);
        data = data.mid(itemList.at(i).TmItemByteLength);
        //qDebug()<<"itemData:"<<itemData.toHex('|')<<itemList.length()<<itemList.at(i).TmItemByteLength<<data.length();
        //如果模板中字节个数过多，数据没有这么多则不进行后续的解析。
        if(itemData.length()<=0 || itemData.length() != itemList.at(i).TmItemByteLength)return;

        //更新源码部分
        if(model)
        {
            QStandardItem* item3 = model->item(i,3);
            item3->setText("0x"+itemData.toHex().toLower());

            //qDebug()<<"index:"<<i<<itemList.at(i).listMap.count();

            QStandardItem* item2 = model->item(i,2);
            QStandardItem* item4 = model->item(i,4);
            //说明是combox
            if(itemList.at(i).listMap.count()>0)
            {
                //判断当前的十六进制
                if(itemData.length()==1)
                {
                    //匹配大写
                    QString show = itemList.at(i).listMap.value(itemData.toHex().toUpper(),"");
                    //匹配小写
                    if(show.isEmpty())show = itemList.at(i).listMap.value(itemData.toHex().toLower(),"");

                    if(!show.isEmpty())
                    {
                        if(item2)item2->setText(show);
                    }
                    else
                    {
                        if(item2)item2->setText("无效");
                        if(item4)item4->setText("无效");
                    }
                }
            }
            //说明是edit
            else
            {
                //根据字节数进行调整 是小数还是整数还是什么数据
                qint32 value = 0;
                for (int i =0;i<itemData.length();i++) {
                    value =(value<<8)+(quint8)itemData.at(i);
                }

                //根据 数学公式计算出对应的值
                double ret = LuaHelper::Instance()->execString(itemList.at(i).TmItemFun,value);
                //qDebug()<<"显示的结果:"<<value<<ret;
                if(item2)item2->setText(QString::number(ret,'f',6));
            }

        }
    }
}

void ParaseTelemetryXml::saveData()
{
    //根据当前的mode数据进行文本保存
    //保存为csv格式
    if(saveFileName.isEmpty())return;
    //如果目录不存在则创建
    QDir dir;
    if(!dir.exists(App::saveTMDataPath))
    {
        dir.mkpath(App::saveTMDataPath);
    }
    QString fileName = App::saveTMDataPath+"/"+saveFileName;

    QFile file(fileName);

    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    if(file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
        QTextStream stream(&file);
        stream.setCodec("GB2312");
        //如果是第一次写文件则需要先写头
        if(file.size() == 0)
        {
            //stream<<"时间,"+"\n";
            QString title = "时间";
            for (int i = 0;i<model->rowCount();i++)
            {
                for (int i = 0;i<5;i++) {
                    QStringList headLabelList;
                    title+=","+model->horizontalHeaderItem(i)->text();
                }
            }
            stream<< title;
            stream<<"\n";
        }

        QString text = timeStamp;

        for (int i = 0;i<model->rowCount();i++) {

             text+=","+model->item(i,0)->text();
             text+=","+model->item(i,1)->text();
             text+=","+model->item(i,2)->text();
             text+=","+model->item(i,3)->text();
             text+=","+model->item(i,4)->text();
           // qDebug()<<model->item(i,0)->text()<<model->item(i,1)->text()<<model->item(i,2)->text()<<model->item(i,3)->text()<<model->item(i,4)->text();
        }

        stream<< text;
        stream<<"\n";
        file.close();
    }
}


void ParaseTelemetryXml::readAttribute(QDomElement element, QString attNodeName)
{
    //每个节点的属性处理
    QDomNamedNodeMap att = element.attributes();
    for (int l=0;l<att.count();l++)
    {
        QDomNode attNode = att.item(l);
        //寻找属性节点
        if(QDomNode::AttributeNode == attNode.nodeType())
        {
            if(attNode.nodeName() == attNodeName)
            {
                qDebug()<<attNode.nodeName()<<attNode.nodeType()<<attNode.nodeValue();
            }

        }

    }
}

QString ParaseTelemetryXml::readAttributeWithString(QDomElement element, QString attNodeName)
{
    //每个节点的属性处理
    QDomNamedNodeMap att = element.attributes();
    for (int l=0;l<att.count();l++)
    {
        QDomNode attNode = att.item(l);
        //寻找属性节点
        if(QDomNode::AttributeNode == attNode.nodeType())
        {
            if(attNode.nodeName() == attNodeName)
            {
                //qDebug()<<attNode.nodeName()<<attNode.nodeType()<<attNode.nodeValue();
                return attNode.nodeValue();
            }
        }
    }
    return QString();
}

void ParaseTelemetryXml::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    //qDebug()<<"dataChanged:"<<topLeft.row()<<topLeft.column();
    int row = topLeft.row();
    int col = topLeft.column();
    QStandardItemModel* model = static_cast<QStandardItemModel*>(sender());

    if(model)
    {
        QStandardItem * item = model->item(row,col);
        if(col == 5)
        {
            if(item->text() == "是")
            {
                item->setForeground(selectColor);
            }
            else
            {
                item->setForeground(unSelectColor);
            }
        }
        // 数值和有效性
        else if(col == 2 || col == 4)
        {
            if(item->text() == "无效")
            {
                item->setForeground(Qt::red);
            }
            else
            {
                item->setForeground(QColor("#20B2AA"));
            }
        }
        //源码
        else if(col == 3)
        {
            if(item->text() == "无效")
            {
                item->setForeground(Qt::red);
            }
            else
            {
                item->setForeground(Qt::blue);
            }
        }
    }

}
