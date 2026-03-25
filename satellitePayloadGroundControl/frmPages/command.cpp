#include "command.h"
#include <QFile>
#include <QMessageBox>
#include "frameassemble.h"
#include "combobox.h"
#include "doublespinbox.h"
#include "luahelper.h"
#include <QHeaderView>
#include <QHBoxLayout>

#include <QMutex>
Command *Command::self = nullptr;
Command *Command::Instance()
{
    if (!self) {
        QMutex mutex;
        QMutexLocker locker(&mutex);
        if (!self) {
            self = new Command;
        }
    }

    return self;
}

Command::Command(QObject *parent) : QObject(parent)
{

}

QVector<CommandInfo> Command::readXML(QString xmlName)
{
    // 打开文件
    QFile file(xmlName);
    if (!file.open(QFileDevice::ReadOnly)) {
        QMessageBox::information(nullptr, "提示", xmlName+"文件打开失败!"+file.errorString());
        return QVector<CommandInfo>();
    }

    QDomDocument doc;

    // 将doc与file关联起来
    // 这个函数从IO设备dev中读取XML文档，如果内容被成功解析，则返回true;否则返回false。
    if (!doc.setContent(&file)) {
        QMessageBox::information(nullptr, "提示", "操作的文件不是XML文件!");
        file.close();
        return QVector<CommandInfo>();
    }
    // 关联后可以关闭文件了
    file.close();

    QVector<CommandInfo> CommandList;

    //获取根节点
    QDomElement root = doc.documentElement();
    //获取SYSTEM名字
    systemName = readAttributeWithString(root,"SYSTEMSpecificationT");
    //获取XML文件类型
    xmlType = readAttributeWithString(root,"XMLtypeT");

    //获取第一个孩子节点
    //QDomElement ele = root.firstChildElement();
    //qDebug()<<ele.nodeName();

    //获取节点名字为Command的所有节点
    QDomNodeList cmdList = root.elementsByTagName("Command");
    //qDebug()<<"Command个数:"<<cmdList.count();
    //处理每个单独的命令
    for (int i = 0; i < cmdList.size(); i++) {
        // 获取节点
        QDomNode nodei = cmdList.at(i);
        if(nodei.isElement())
        {
            QDomElement elementI = nodei.toElement();
            //获取命令的名称通过读取CommandSpecificationT属性
            CommandInfo commInfo;

            //命令备注
            commInfo.CommandRemark = "暂无备注";
            //获取名称
            commInfo.CommandName = readAttributeWithString(elementI,"CommandSpecificationT");
            //获取命令编码
            commInfo.CommandCode = readAttributeWithString(elementI,"CommandCodeH");
            //获取时间
            commInfo.CommandTime = readAttributeWithString(elementI,"CommandTimeD");

            //处理命令下面的参数
            QDomNodeList cmdParaList = elementI.elementsByTagName("CommandPara");
            for (int j =0;j<cmdParaList.length();j++) {
                QDomNode nodej = cmdParaList.at(j);
                if(nodej.isElement())
                {
                    QDomElement elementJ = nodej.toElement();

                    ParamInfo pInfo;

                    //获取呈现类型
                    QString paramShowType = readAttributeWithString(elementJ,"CommandParamTypeT");
                    if(paramShowType.isEmpty())paramShowType = readAttributeWithString(elementJ,"CommandParamType");
                    if(paramShowType.isEmpty())paramShowType = readAttributeWithString(elementJ,"CommandParaType");


                    if(!paramShowType.isEmpty())
                    {
                        //下拉框
                        if(paramShowType == "COMBOX")
                        {
                            pInfo.type = PARAM_TYPE::COMBOBOX;
                        }
                        //编辑器
                        else if(paramShowType == "EDIT")
                        {
                            pInfo.type = PARAM_TYPE::EDIT;
                        }
                        //其他
                        else
                        {
                            pInfo.type = PARAM_TYPE::NORMAL;
                        }
                    }
                    else
                    {
                        pInfo.type = PARAM_TYPE::EDIT;
                    }

                    //获取参数名称
                    pInfo.paraName = readAttributeWithString(elementJ,"CommandParaSpecificationT");
                    if(pInfo.paraName.isEmpty())pInfo.paraName = readAttributeWithString(elementJ,"CommandParaName");
                    pInfo.currentShowIndex = readAttributeWithString(elementJ,"CommandParaInitSelectD").toInt();

                    //占几个字节
                    pInfo.paramByteLength = readAttributeWithString(elementJ,"CommandParaByteLengthD").toUInt();
                    //获取该参数解析和设置的计算函数
                    pInfo.paramEncodeFun = readAttributeWithString(elementJ,"CommandParaEncodeTypeT");
                    pInfo.paramDecodeFun = readAttributeWithString(elementJ,"CommandParaDecodeTypeT");

                    //最大值和最小值进行解析
                    pInfo.paramMinValue = readAttributeWithString(elementJ,"CommandParaMinF");
                    pInfo.paramMaxValue = readAttributeWithString(elementJ,"CommandParaMaxF");

                    QDomNodeList paraList = elementJ.elementsByTagName("Para");
                    for (int k =0;k<paraList.length();k++) {
                        QDomNode nodek = paraList.at(k);
                        if(nodek.isElement())
                        {
                            QDomElement elementK = nodek.toElement();
                            //获取参数名称
                            QString paramName = readAttributeWithString(elementK,"ParaSpecificationT");
                            pInfo.paraNameList<<paramName;
                            //获取初始值ParaInitCode
                            QString paramInitCode = readAttributeWithString(elementK,"ParaInitCode");
                            pInfo.paraInitCodeList<<paramInitCode;
                            //获取初始值对应的ParaRawCodeH
                            QString paramRowCode = readAttributeWithString(elementK,"ParaRawCodeH");
                            pInfo.paraRawCodeList<<paramRowCode;
                        }
                    }

                    //正常记录
                    pInfo.paraRemark ="暂无备注";
                    commInfo.paraInfo.append(pInfo);
                }
            }
            //查看每个节点下有几个节点
            //qDebug()<<"CommandPara:"<<cmdParaList.length();
            CommandList.append(commInfo);
        }

        // 删除子节点
        //root.removeChild(element);
    }

    return CommandList;
}

void Command::writeXML(QString xmlName, QVector<CommandInfo> cmdList)
{
    //保存成xml文件方便下次读取
    QDomDocument doc;

    // 创建XML处理类，通常用于处理第一行描述信息
    QDomProcessingInstruction instruction;

    // 创建XML头部格式
    instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");

    // 添加到XML文件中
    doc.appendChild(instruction);


    // 创建根元素<SYSTEM>
    QDomElement root = doc.createElement("SYSTEM");
    root.setAttribute("SYSTEMSpecificationT", systemName);
    root.setAttribute("XMLtypeT", xmlType);

    //根据cmdlist进行cmd保存
    for (int i =0; i < cmdList.length();i++) {

        CommandInfo commInfo = cmdList.at(i);
        // 创建<Command>命令元素
        QDomElement command = doc.createElement("Command");
        command.setAttribute("CommandSpecificationT", commInfo.CommandName);
        command.setAttribute("CommandCodeH", commInfo.CommandCode);
        command.setAttribute("CommandTimeD", commInfo.CommandTime);

        for (int j = 0;j < cmdList.at(i).paraInfo.length();j++) {
            // 创建<CommandPara>命令参数元素
            QDomElement cmdPara = doc.createElement("CommandPara");

            ParamInfo pInfo = commInfo.paraInfo.at(j);

            if(pInfo.type == PARAM_TYPE::COMBOBOX)
            {
                cmdPara.setAttribute("CommandParamTypeT", "COMBOX");
            }
            else if(pInfo.type == PARAM_TYPE::EDIT)
            {
                cmdPara.setAttribute("CommandParamTypeT", "EDIT");
            }
            else
            {
                cmdPara.setAttribute("CommandParamTypeT", "EDIT");
            }

            cmdPara.setAttribute("CommandParaSpecificationT", pInfo.paraName);
            cmdPara.setAttribute("CommandParaInitSelectD", QString::number(pInfo.currentShowIndex));
            cmdPara.setAttribute("CommandParaByteLengthD", QString::number(pInfo.paramByteLength));
            cmdPara.setAttribute("CommandParaEncodeTypeT", pInfo.paramEncodeFun);
            cmdPara.setAttribute("CommandParaDecodeTypeT", pInfo.paramDecodeFun);
            cmdPara.setAttribute("CommandParaMinF", pInfo.paramMinValue);
            cmdPara.setAttribute("CommandParaMaxF", pInfo.paramMaxValue);

            for (int k = 0;k < cmdList.at(i).paraInfo.at(j).paraNameList.length();k++) {

                // 创建<Para>参数元素
                QDomElement para = doc.createElement("Para");
                para.setAttribute("ParaSpecificationT", pInfo.paraNameList.at(k));
                para.setAttribute("ParaInitCode", pInfo.paraInitCodeList.at(k));
                para.setAttribute("ParaRawCodeH", pInfo.paraRawCodeList.at(k));

                cmdPara.appendChild(para);
            }

            command.appendChild(cmdPara);
        }

        root.appendChild(command);

    }

    doc.appendChild(root);
    // 将DOM树写入XML文件
    QFile file(xmlName);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        //stream << doc.toString();
        doc.save(stream,4);
        file.close();
    }
}

void Command::initXML(QString xmlName)
{
    this->CommandList = readXML(xmlName);
}

void Command::initTable()
{

    if(!listWidget)return;
    if(!tableWidget)return;

    //delegate = new Delegate(this);
    tableWidget->clearContents();
    //可以全局设置也可以只设置这一列或者这一行,未设置的部分不会触发delegate的createEditor函数
    //tableWidget->setItemDelegate(delegate);
    //tableWidget->setItemDelegateForColumn(2,delegate);

    //初始显示的命令
    if(this->CommandList.length()>0)
    {
        //初始化listWidget
        for (int i = 0;i<CommandList.length();i++) {

            if(listWidget)
            {
                listWidget->addItem(new QListWidgetItem(QIcon(":/Resources/images/command.png"),CommandList.at(i).CommandName));
            }
        }
        changeCommand(0);
    }
}

void Command::changeCommand(int cmdIndex)
{
    if(commandIndex == cmdIndex) return;

    int commandCount = this->CommandList.length();
    if(cmdIndex>=commandCount)
    {
        qDebug()<<cmdIndex<<"error!";
        return;
    }

    //获取当前命令索引
    commandIndex = cmdIndex;

    CommandInfo cmdInfo = this->CommandList.at(cmdIndex);


    int paramCount = cmdInfo.paraInfo.length();


    //设置行数
    tableWidget->setRowCount(paramCount);
    tableWidget->horizontalHeader()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
    //    if(delegate)
    //    {
    //        delegate->setCommand(cmdInfo);
    //    }


    for(int i = 0;i<paramCount;i++)
    {

        ParamInfo parm = cmdInfo.paraInfo[i];

        //参数编号  //参数名称  // 输入数值  源码  备注
        for (int j = 0;j<tableWidget->columnCount();j++)
        {

            if(j == 0)
            {
                QTableWidgetItem* item = tableWidget->item(i,j);
                QString text = QString("参数编号%1").arg(i);
                if(!item)
                {
                    item = new QTableWidgetItem();
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    tableWidget->setItem(i,j,item);
                }
                item->setText(text);
            }

            else if(j == 1)
            {
                QTableWidgetItem* item = tableWidget->item(i,j);
                QString text = parm.paraName;
                if(!item)
                {
                    item = new QTableWidgetItem();
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    tableWidget->setItem(i,j,item);
                }
                item->setText(text);
            }

            //判断类型
            if(parm.type == PARAM_TYPE::COMBOBOX)
            {


                int index = parm.currentShowIndex;

                if(j == 2)
                {


                    QWidget* widget =  tableWidget->cellWidget(i,j);

                    if(widget)
                    {
                        tableWidget->removeCellWidget(i,j);
                        widget->deleteLater();
                    }
                    ComboBox* combobox = new ComboBox;
                    QFont font = combobox->font();
                    font.setPixelSize(14);
                    font.setWeight(QFont::Bold);
                    combobox->setFont(font);
                    //                        QWidget *newWidget = new QWidget;
                    //                        QHBoxLayout *layout = new QHBoxLayout;
                    //                        layout->setSpacing(0);
                    //                        layout->setMargin(1);
                    //                        layout->addWidget(combobox);
                    //                        newWidget->setLayout(layout);

                    tableWidget->setCellWidget(i,j,combobox);
                    connect(combobox,&ComboBox::currentTextChanged,this,[=]{
                        //tableWidget->item(1,1);
                        //qDebug()<<combobox->currentText();

                        int row = combobox->row;
                        int col = combobox->col;

                        //根据当前命令行获取类型
                        CommandInfo cmdInfo = Command::Instance()->CommandList.at(commandIndex);

                        if(cmdInfo.paraInfo.length()<=row)return;

                        ParamInfo paramInfo = cmdInfo.paraInfo.at(row);

                        int index = paramInfo.paraNameList.indexOf(combobox->currentText());
                        if(index<0)return;
                        Command::Instance()->CommandList[commandIndex].paraInfo[row].currentShowIndex = index;

                        QString  result = paramInfo.paraRawCodeList.at(index);

                        QTableWidgetItem *item = tableWidget->item(row,col+1);
                        if(item)
                        {
                            item->setText(result);
                        }

                    });


                    combobox->row = i;
                    combobox->col = j;
                    combobox->addItems(parm.paraNameList);
                    combobox->setCurrentIndex(parm.currentShowIndex);



                    //                     QTableWidgetItem* item = tableWidget->item(i,j);
                    //                     QString text = parm.paraNameList.at(index);
                    //                     if(!item)
                    //                     {
                    //                         item = new QTableWidgetItem();
                    //                         tableWidget->setItem(i,j,item);
                    //                     }
                    //                     item->setText(text);
                }

                if(j == 3)
                {

                    QTableWidgetItem* item = tableWidget->item(i,j);
                    QString text = parm.paraRawCodeList.at(index);
                    if(!item)
                    {
                        item = new QTableWidgetItem();
                        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                        tableWidget->setItem(i,j,item);
                    }
                    item->setText(text);
                }
            }
            else
            {
                if(j == 2)
                {

                    QWidget* widget =  tableWidget->cellWidget(i,j);

                    if(widget)
                    {
                        tableWidget->removeCellWidget(i,j);
                        widget->deleteLater();
                    }


                    DoubleSpinBox* dSpinBox = new DoubleSpinBox;
                    QFont font = dSpinBox->font();
                    font.setPixelSize(14);
                    font.setWeight(QFont::Bold);
                    dSpinBox->setFont(font);

                    tableWidget->setCellWidget(i,j,dSpinBox);
                    connect(dSpinBox,QOverload<double>::of(&DoubleSpinBox::valueChanged),this,[=]{
                        //tableWidget->item(1,1);
                        //qDebug()<<dSpinBox->value();


                        int row = dSpinBox->row;
                        int col = dSpinBox->col;
                        //根据当前命令行获取类型
                        CommandInfo cmdInfo = Command::Instance()->CommandList.at(commandIndex);

                        if(cmdInfo.paraInfo.length()<=row)return;

                        ParamInfo paramInfo = cmdInfo.paraInfo.at(row);


                        //根据当前值进行计算
                        Command::Instance()->CommandList[commandIndex].paraInfo[row].paraInitCodeList[0] = dSpinBox->text();
                        double x = dSpinBox->value();
                        //lua测试
                        double ret = LuaHelper::Instance()->execString(paramInfo.paramEncodeFun,x);
                        int showLen = Command::Instance()->CommandList[commandIndex].paraInfo[row].paramByteLength*2;
                        QString  result = QString("%1").arg((quint16)ret,showLen,16,QLatin1Char('0')).toUpper();
                        Command::Instance()->CommandList[commandIndex].paraInfo[row].paraRawCodeList[0] = result;

                        QTableWidgetItem *item = tableWidget->item(row,col+1);
                        if(item)
                        {
                            item->setText(result);
                        }

                    });

                    dSpinBox->row = i;
                    dSpinBox->col = j;
                    bool isOk1 = false;
                    bool isOk2 = false;

                    double dMax = parm.paramMaxValue.toInt(&isOk1);
                    double dMin = parm.paramMinValue.toInt(&isOk2);

                    if(isOk1 && isOk2)
                    {
                        //显示0位小数
                        dSpinBox->setDecimals(0);
                        dSpinBox->setSingleStep(1);
                    }
                    else
                    {
                        //显示几位小数
                        dSpinBox->setSingleStep(1);
                        dSpinBox->setDecimals(6);
                        dMax = parm.paramMaxValue.toDouble(&isOk1);
                        dMin = parm.paramMinValue.toDouble(&isOk2);
                    }


                    //qDebug()<<value.paramMinValue<<value.paramMaxValue<<dMax<<dMin;
                    dSpinBox->setMaximum(dMax);
                    dSpinBox->setMinimum(dMin);

                    dSpinBox->setValue(parm.paraInitCodeList.first().toDouble());

                    //                    QTableWidgetItem* item = tableWidget->item(i,j);
                    //                    QString text = parm.paraInitCodeList.first();
                    //                    if(!item)
                    //                    {
                    //                        item = new QTableWidgetItem();
                    //                        tableWidget->setItem(i,j,item);
                    //                    }
                    //                    item->setText(text);

                }
                else if(j == 3)
                {
                    QTableWidgetItem* item = tableWidget->item(i,j);
                    QString text = parm.paraRawCodeList.first();
                    if(!item)
                    {
                        item = new QTableWidgetItem();
                        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                        tableWidget->setItem(i,j,item);
                    }
                    item->setText(text);
                }
            }

            if(j == 4)
            {
                QTableWidgetItem* item = tableWidget->item(i,j);
                QString text = parm.paraRemark;
                if(!item)
                {
                    item = new QTableWidgetItem();
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    tableWidget->setItem(i,j,item);
                }
                item->setText(text);
            }
        }
    }

    //此处是解决widget会覆盖table的分割线的问题
    static bool isFirst = true;
    int headerWidth = tableWidget->horizontalHeader()->sectionSize(2);
    //qDebug()<<"headerWidth:"<<headerWidth;
    tableWidget->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Fixed);
    //    qDebug()<<"maxWidth:"<<maxWidth;
    if(isFirst)
    {
        tableWidget->setColumnWidth(2,headerWidth+60);
        isFirst = false;
    }
    else
    {
        tableWidget->setColumnWidth(2,headerWidth+20);
    }

}

void Command::sendCMD()
{
    CommandInfo cmdInfo = this->CommandList.at(commandIndex);
    sendCMD(cmdInfo);
}

void Command::sendCMD(CommandInfo cmdInfo)
{
    int paramCount = cmdInfo.paraInfo.length();
    QByteArray cmd;
    cmd.append(QByteArray::fromHex(QByteArray(cmdInfo.CommandCode.toLatin1().data())));
    int timestamp = cmdInfo.CommandTime.toInt();
    //低字节序转高字节序
    timestamp = qbswap<int>(timestamp);
    cmd.append((char*)&timestamp,sizeof (timestamp));

    QByteArray data;
    for (int i = 0 ;i < paramCount;i++) {

        ParamInfo para = cmdInfo.paraInfo[i];
        //判断类型
        if(para.type == PARAM_TYPE::COMBOBOX)
        {
            int index = para.currentShowIndex;
            QString rawCode = para.paraRawCodeList.at(index);
            data.append(QByteArray::fromHex(QByteArray(rawCode.toLatin1().data())));
        }
        else
        {
            QString rawCode = para.paraRawCodeList.first();
            data.append(QByteArray::fromHex(QByteArray(rawCode.toLatin1().data())));
        }
    }

    cmd.append(data.length());
    cmd.append(data);

    //qDebug()<<"real:"<<cmd.toHex(' ');
    realSendCMD(cmd);

}
//发送数据函数
void Command::realSendCMD(QByteArray cmd)
{
    FrameAssemble::Instance()->RS422FixedFrameAssemble(cmd);
}

QString Command::getCMDString(CommandInfo cmdInfo)
{
    int paramCount = cmdInfo.paraInfo.length();
    QByteArray cmd;
    cmd.append(QByteArray::fromHex(QByteArray(cmdInfo.CommandCode.toLatin1().data())));
    int timestamp = cmdInfo.CommandTime.toInt();
    //低字节序转高字节序
    timestamp = qbswap<int>(timestamp);
    cmd.append((char*)&timestamp,sizeof (timestamp));

    QByteArray data;
    for (int i = 0 ;i < paramCount;i++) {

        ParamInfo para = cmdInfo.paraInfo[i];
        //判断类型
        if(para.type == PARAM_TYPE::COMBOBOX)
        {
            int index = para.currentShowIndex;
            QString rawCode = para.paraRawCodeList.at(index);
            data.append(QByteArray::fromHex(QByteArray(rawCode.toLatin1().data())));
        }
        else
        {
            QString rawCode = para.paraRawCodeList.first();
            data.append(QByteArray::fromHex(QByteArray(rawCode.toLatin1().data())));
        }
    }

    cmd.append(data.length());
    cmd.append(data);

    return cmd.toHex(' ').toUpper().replace(' ',"");
}

QByteArray Command::getCMDByteArray(CommandInfo cmdInfo)
{
    int paramCount = cmdInfo.paraInfo.length();
    QByteArray cmd;
    cmd.append(QByteArray::fromHex(QByteArray(cmdInfo.CommandCode.toLatin1().data())));
    int timestamp = cmdInfo.CommandTime.toInt();
    //低字节序转高字节序
    timestamp = qbswap<int>(timestamp);
    cmd.append((char*)&timestamp,sizeof (timestamp));

    QByteArray data;
    for (int i = 0 ;i < paramCount;i++) {

        ParamInfo para = cmdInfo.paraInfo[i];
        //判断类型
        if(para.type == PARAM_TYPE::COMBOBOX)
        {
            int index = para.currentShowIndex;
            QString rawCode = para.paraRawCodeList.at(index);
            data.append(QByteArray::fromHex(QByteArray(rawCode.toLatin1().data())));
        }
        else
        {
            QString rawCode = para.paraRawCodeList.first();
            data.append(QByteArray::fromHex(QByteArray(rawCode.toLatin1().data())));
        }
    }

    cmd.append(data.length());
    cmd.append(data);

    return cmd;
}

void Command::sendCMDList(QVector<CommandInfo> cmdInfoList)
{
    QByteArray cmd;
    int byteCount = 0;
    for (int i = 0; i < cmdInfoList.length();i++) {
        CommandInfo cmdInfo = cmdInfoList.at(i);
        QByteArray data = getCMDByteArray(cmdInfo);
        byteCount+=data.length();
        cmd.append(data);
    }

    qDebug()<<byteCount;
    //低字节序转高字节序
    byteCount = qbswap<int>(byteCount);
    cmd.prepend((char*)&byteCount,sizeof (byteCount));

    //qDebug()<<cmd.toHex('|');

    //通过变长指令发送
    FrameAssemble::Instance()->RS422CommandDataSend(cmd);
}

void Command::registerTabelWidget(QTableWidget *tableWidget,QListWidget* listWidget)
{
    this->tableWidget = tableWidget;
    this->listWidget = listWidget;
    initTable();
}

void Command::readAttribute(QDomElement element, QString attNodeName)
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

QString Command::readAttributeWithString(QDomElement element, QString attNodeName)
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
