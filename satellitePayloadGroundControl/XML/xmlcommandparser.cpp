#include "xmlcommandparser.h"
#include <QDebug>
#include <QHeaderView>
#include <QComboBox>
#include <cmath>
#include <QJSEngine>


XMLCommandParser::XMLCommandParser(QObject *parent) : QObject(parent) {

    listWidgetHelper = new ListWidgetHelper(this);
//    connect(listWidgetHelper,&ListWidgetHelper::updateUiCurComShowSignal,this,[=](QString text){
//        emit updateUiCurComShowSignal(text);
//    });
}

bool XMLCommandParser::loadXML(const QString &filePath, QList<QDomElement> &docNodeInitList)
{
    /// 准备xml文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开XML文件:" << filePath;
        return false;
    }

    /// 准备xml文档对象（容器）
    QDomDocument doc;
    if (!doc.setContent(&file)) { //把整个 XML 文件一次性读入内存，构造成一棵“DOM 树”
        qWarning() << "XML 解析失败";
        file.close();
        return false;
    }
    file.close();

    /// 元素节点，最常用
    QDomElement root = doc.documentElement();// 根元素节点——SYSTEM，注意，这里虽然是获取根元素节点，但是通过DOM方法可以访问全部子元素
    if (root.tagName() != "SYSTEM") {
        qWarning() << "XML根标签不是<SYSTEM>";
        return false;
    }

    parseCommands(root, docNodeInitList);
    return true;
}

void XMLCommandParser::parseCommands(const QDomElement &root, QList<QDomElement> &docNodeInitList) /// 注意，这里必须使用引用传递，否则，只是对拷贝的对象进行操作
{
//    QList<Command> commandList; // 只要来一个xml就创建一个commandList
    /// 节点列表
    QDomNodeList commandNodes = root.elementsByTagName("Command"); // 在根下找所有Command

    /*
        会在当前元素的整棵子树里，找到所有名为 <Command> 的元素（不分层级深浅），只看
        直接子节点或同级兄弟，不会深入到孙子/重孙。
    */
//    qDebug()<<"commandNodes size"<<commandNodes.size();
//    qDebug()<<"commandNodes count"<<commandNodes.count();

    // 解析 Command 下面的属性，注意，第二个属性CommandSeqIdxD表示该指令包编号
    for (int i = 0; i < commandNodes.count(); ++i) {

        /// 任何节点的基类
        QDomNode node = commandNodes.at(i);
        // 如果是元素节点（Element），可以进一步访问其属性和子节点
        if (node.isElement()) {
            QDomElement element = node.toElement();

            docNodeInitList.append(element); // 将当前加载xml文件中的Command节点保存起来，方便后续修改配置并保存新的xml文件
            qDebug()<<"docNodeInitList.count()"<<docNodeInitList.count();
            // 打印该元素的所有属性
            QDomNamedNodeMap attributes = element.attributes();
            for (int j = 0; j < attributes.count(); ++j) {
                QDomNode attr = attributes.item(j);
                qDebug() << "   Attribute:" << attr.nodeName() << "=" << attr.nodeValue();
            }
        }
    }

    for (int i = 0; i < commandNodes.count(); ++i) {

        QDomElement commandElem = commandNodes.at(i).toElement();
        Command cmd;
        cmd.name = commandElem.attribute("CommandSpecificationT", QString("Command %1").arg(i)); // 当前指令模块的名称
        cmd.Index = commandElem.attribute("CommandSeqIdxD", QString::number(i)).toInt(); // 当前指令模块的索引
        cmd.header = commandElem.attribute("FrameHeader", QString::number(i));  // 当前指令模块的帧头
        cmd.tail = commandElem.attribute("FrameTail", QString::number(i));  // 当前指令模块的帧尾
        cmd.checkSumLength = commandElem.attribute("CheckSumLength", QString::number(i)).toInt();  // 当前指令模块校验和长度

        // 系统会首先去寻找CommandSeqIdxD属性的值，然后将其转为int类型，如果没有这个属性，就会将i转化为int
//        qDebug()<<"cmd.name"<<cmd.name; // 当前模块的名称 —— 用于给左侧指令选择按钮命名
//        qDebug()<<"cmd.Index"<<cmd.Index; // 当前模块的编号 —— 用于在StackWidget的相应Page中创建表格
        emit updateUiCurComShowSignal(cmd.name);

        QDomNodeList attrNodes = commandElem.elementsByTagName("CommandPara");
        for (int j = 0; j < attrNodes.count(); ++j) {
            QDomElement attrElem = attrNodes.at(j).toElement();
            CommandAttribute attr;
            attr.paraNo = attrElem.attribute("CommandParaSeqIdxD"); // 参数编号
            attr.paraName = attrElem.attribute("CommandParaSpecificationT"); // 参数名称
            attr.paraType = attrElem.attribute("CommandParamTypeT"); // 参数类别
            attr.byteNotes = attrElem.attribute("AttributeByteLenD");// 备注
            attr.initIndex = attrElem.attribute("CommandParaInitSelectD").toInt(); // 转为整数; // 默认索引

            // 查找 <Para> 子项
            QDomNodeList paraNodes = attrElem.elementsByTagName("Para");
            for (int k = 0; k < paraNodes.count(); ++k) {
                QDomElement paraElem = paraNodes.at(k).toElement();
                QString option = paraElem.attribute("ParaSpecificationT");
                QString code = paraElem.attribute("ParaRawCodeH");
                QString intCode = paraElem.attribute("ParaInitCode");

                if (!option.isEmpty())
                    attr.paraOptions << option; // combobox对应的选项
                if (!code.isEmpty())
                    attr.hexCodes << code; // combobox对应的源码

                if (k == 0) {
                    // 默认值来自第一个选项
                    attr.paraValue = option;
                    attr.byteHex = code;
                    attr.paraIntCode = intCode;
                }
            }

            // 新增字段解析
             attr.encodeRule = attrElem.attribute("CommandParaEncodeTypeT");
             attr.byteLength = attrElem.attribute("CommandParaByteLengthD").toInt(); // 转为整数
             attr.minValue = attrElem.attribute("CommandParaMinF");
             attr.maxValue = attrElem.attribute("CommandParaMaxF");

            cmd.attributes.append(attr);
        }
        commandList.append(cmd);
    }
    qDebug() << "解析到 Command 个数:" << commandList.size();
    for (const Command &cmd : commandList) {
        qDebug() << "Command 名称:" << cmd.name << "，参数数量:" << cmd.attributes.size();
        }
    qDebug()<<"1docNodeInitList.count() "<<docNodeInitList.count();
}

/**
 笔记：
 QList<QDomElement> docNodeInitListUi    传入的是一个拷贝,操作的是一份副本，不会影响外部的原始数据函数返回后，内部修改会丢失。安全，不会意外改坏外部数据。但如果 QList 很大，拷贝会有额外的性能开销。
 QList<QDomElement> *docNodeInitListUi   传入的是一个指向 QList<QDomElement> 的指针,在函数内部通过 docNodeInitListUi->xxx 修改，直接作用在外部对象上。
 QList<QDomElement> &docNodeInitListUi   传入的就是外部对象的一个别名，不产生拷贝。在函数内部操作时，修改的就是外部的对象。不需要 ->，用法和按值传递一模一样，但没有拷贝开销。
                                         不存在“空引用”，所以比指针更安全。一旦进入函数内部，你就确定是在操作原始对象。
 指针和引用的根本区别
 1. 是否允许“空”
     指针：可以是空的（nullptr），表示“可能没有对象”。
     引用：必须绑定到一个有效对象（C++ 语义上不存在“空引用”）。
 2.是否能重新指向
     指针：可以在函数内改变它的指向。
     引用：一旦初始化，就不能再绑定到别的对象。
 3. 语义清晰度
     引用：强调“我要直接操作你传进来的对象”。
     指针：强调“我操作的是地址，可能为空，可能会换目标”。
*/

QTableWidget* XMLCommandParser::createTableForCommand(const Command &cmd, QList<QDomElement> &docNodeInitListUi, int index)
{
    QTableWidget *table = new QTableWidget(cmd.attributes.size(), 5);
    table->setHorizontalHeaderLabels({"参数编号", "参数名称", "参数数值", "源码", "备注"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);

    for (int i = 0; i < cmd.attributes.size(); ++i) {
        const CommandAttribute &attr = cmd.attributes.at(i);

        table->setItem(i, 0, new QTableWidgetItem(attr.paraNo));
        table->setItem(i, 1, new QTableWidgetItem(attr.paraName));

        if (attr.paraType == "COMBOX") {
            QComboBox *combo = new QComboBox(table);
            combo->addItems(attr.paraOptions);
            combo->setCurrentText(attr.paraValue);
            combo->setCurrentIndex(attr.initIndex);
            qDebug()<<"attr.initIndex"<<attr.initIndex;
            // 设置初始源码
            QTableWidgetItem* item = new QTableWidgetItem(attr.hexCodes[attr.initIndex]); // ✅ 按选项索引取源码
            table->setItem(i, 3, item); // ✅ 更新当前行第4列

            table->setCellWidget(i, 2, combo);
            // 设置与单元格一样宽
            combo->setMinimumWidth(table->columnWidth(2));
            // 使用 lambda 或槽函数绑定变化信号
//            connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
            connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this,
                    [this, &docNodeInitListUi, seqIndex=index, table, i, cmd](int comboIndex)
            {
                const CommandAttribute &attr = cmd.attributes.at(i);
                // 确保索引有效
                if (comboIndex >= 0 && comboIndex < attr.hexCodes.size()) {
                    QTableWidgetItem* item = new QTableWidgetItem(attr.hexCodes[comboIndex]); // ✅ 按选项索引取源码
                    table->setItem(i, 3, item); // ✅ 更新当前行第4列

                    /// 修改docNodeInitList中的COMBOX值
                    // ⬅️ 同步 XML 节点
                    // 获取当前command列表索引
                    int curIndexCom = seqIndex;
                    qDebug() << "curIndexCom" <<curIndexCom;

                    QDomElement elem = docNodeInitListUi.at(curIndexCom);
                    if (seqIndex < 0 || seqIndex >= docNodeInitListUi.size())
                        return;
                    QDomNodeList commandParaList = elem.elementsByTagName("CommandPara");
                    QDomElement elemCommandPara = commandParaList.at(i).toElement();
                    elemCommandPara.setAttribute("CommandParaInitSelectD", comboIndex);

//                    // ✅ 直接打印这个节点看看
//                    qDebug() << "attr after set =" << elemCommandPara.attribute("CommandParaInitSelectD");

//                    // ✅ 再把整个 doc 打印出来看看
//                    QDomDocument doc = elem.ownerDocument();
//                    QString xml;
//                    QTextStream out(&xml);
//                    doc.save(out, 4);   // 第二个参数是缩进宽度
//                    qDebug().noquote() << xml;

                }
            });

        }
//        else if (attr.paraType == "EDIT") {
//            QTableWidgetItem *item = new QTableWidgetItem(attr.paraIntCode);
//            item->setFlags(item->flags() | Qt::ItemIsEditable);
//            table->setItem(i, 2, item);

//        else if (attr.paraType == "EDIT") {

//            QString initValue = attr.paraIntCode; // 默认值

//            // ⭐ 从 XML 读取真实当前值
//            if (index >= 0 && index < docNodeInitListUi.size()) {

//                QDomElement elem = docNodeInitListUi.at(index);
//                QDomNodeList commandParaList = elem.elementsByTagName("CommandPara");

//                if (i < commandParaList.size()) {

//                    QDomElement elemCommandPara = commandParaList.at(i).toElement();
//                    QDomNodeList paraList = elemCommandPara.elementsByTagName("Para");

//                    if (!paraList.isEmpty()) {

//                        QDomElement elemPara = paraList.at(0).toElement();

//                        QString xmlInit = elemPara.attribute("ParaInitCode");

//                        if (!xmlInit.isEmpty())
//                            initValue = xmlInit;   // ⭐ 用XML覆盖
//                    }
//                }
//            }

//            QTableWidgetItem *item = new QTableWidgetItem(initValue);
//            item->setFlags(item->flags() | Qt::ItemIsEditable);
//            table->setItem(i, 2, item);

//            // 存储规则到UserRole（用于后续计算）
//            QVariantMap editData;
//            editData["encodeRule"] = attr.encodeRule;
//            editData["byteLength"] = attr.byteLength;
//            item->setData(Qt::UserRole, editData);

//            // ⭐ 新增：初始化源码
//                QString initHex = calculateHex(attr.paraIntCode,
//                                               attr.encodeRule,
//                                               attr.byteLength);
//                table->setItem(i, 3, new QTableWidgetItem(initHex));

//        }
//        else {
//            // 未识别类型，显示为普通不可编辑项
//            table->setItem(i, 2, new QTableWidgetItem(attr.paraValue));
//        }

////        table->setItem(i, 3, new QTableWidgetItem(attr.hexCodes[attr.initIndex]));
//        table->setItem(i, 4, new QTableWidgetItem(attr.byteNotes));
//    }

        /// 初始化EDIT类型指令
        else if (attr.paraType == "EDIT") {
            QTableWidgetItem *item = new QTableWidgetItem(attr.paraIntCode);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            table->setItem(i, 2, item);

            // 存储规则到UserRole（用于后续计算）
            QVariantMap editData;
            editData["encodeRule"] = attr.encodeRule;
            editData["byteLength"] = attr.byteLength;
            item->setData(Qt::UserRole, editData);
        }
        else {
            // 未识别类型，显示为普通不可编辑项
            table->setItem(i, 2, new QTableWidgetItem(attr.paraValue));
        }

        table->setItem(i, 3, new QTableWidgetItem(attr.hexCodes[attr.initIndex]));
        table->setItem(i, 4, new QTableWidgetItem(attr.byteNotes));
    }
    // 监听所有EDIT单元格的变化
    // 新的需求设想：在监听表格中单元格信息的变化时，可不可以实时修改位于docNodeInitList中的QDomElement元素
    connect(table, &QTableWidget::itemChanged, [=](QTableWidgetItem *item) {
        if (item->column() != 2) return; // 只处理参数数值列（第2列）

        QVariant data = item->data(Qt::UserRole);
        if (!data.isValid()) return; // 非EDIT类型跳过

        QVariantMap editData = data.toMap();
        QString encodeRule = editData["encodeRule"].toString(); // 使用编码规则编码
        int byteLength = editData["byteLength"].toInt();

        // 计算十六进制源码
        QString hexCode = calculateHex(item->text(), encodeRule, byteLength);

        // 更新源码列（第3列）
        int row = item->row();
        table->blockSignals(true); // 避免递归触发
        table->item(row, 3)->setText(hexCode);
        table->blockSignals(false);

        /// 修改docNodeInitList中的EDIT值
        int curIndexCom = index;
//        qDebug() << "curIndexCom" <<curIndexCom;
//        qDebug() << "item->row()" <<item->row();
//        qDebug() << "item->text()" <<item->text();
        QDomElement elem = docNodeInitListUi.at(curIndexCom);
        QDomNodeList commandParaList = elem.elementsByTagName("CommandPara");
        QDomElement elemCommandPara = commandParaList.at(item->row()).toElement();
        QDomNodeList paraList = elemCommandPara.elementsByTagName("Para");
        QDomElement elemPara = paraList.at(0).toElement();
        elemPara.setAttribute("ParaInitCode", item->text());
        elemPara.setAttribute("ParaRawCodeH", hexCode);
    });

    table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    return table;
}


void XMLCommandParser::populateStackedWidget(QStackedWidget *stackWidget, QListWidget *listWidget, bool setCurrentToNew, QList<QDomElement> &docNodeInitListUi, int rowCom)
{
    // 根据XML文件中Command下的CommandSeqIdxD号来确定当前指令应该放在stackWidget的第几个页面，首先，0和1是默认占据的，分别代表可见光相机指令和红外相机指令
//    QWidget *widget = stackWidget->widget(0);

    listWidget->clear(); /// 这一句很重要，因为每读取一个xml文件就会运行一次readXML()和populateStackedWidget()，readXML()会将新读到的指令模块加入到
                         /// commandList，所以，两次读取xml后，commandList中会有两个Command， 但是，addTabItem也会运行两次，第一次将读取到的第一个xml指令
                         /// 加入到listWidget，第二次读取时，由于此时commandList中累加了两个Command，所以这次又有两个xml指令加入到listWidget，也就是第一次：1
                         /// 第二次：1和2，这样，左侧listWidget显示的效果就是1,1,2。但是我们在建立listWidget和stackWidget之间的链接时是按照索引链接的，所以左侧
                         /// 的三个item中只有前两个会生效，也就出现了我遇到了的一个效果，两个1可以分别链接两个stackWidget，但是第三个2是无效的，因为我们全程的CommandList
                         /// 只有两个，所以它找不到索引为2的页面。

    // 清空所有旧页面
    while (stackWidget->count() > 0) {
        QWidget *widget = stackWidget->widget(0);
        stackWidget->removeWidget(widget);
//        widget->deleteLater(); // 可能导致内存延迟释放
        delete widget; // 更高效
    }

    // 添加新页面
    for (const Command &cmd : commandList) {
        qDebug() << "cmd.name = " << cmd.name;

        QTableWidget *table = createTableForCommand(cmd, docNodeInitListUi, rowCom);
        stackWidget->addWidget(table);  // 不再使用 addTab，而是 addWidget
        QIcon iconTabSub(":/Icon/3592866-attach-attachment-clip-clipping-general-office-paperclip_107732.png");
        listWidgetHelper->addTabItem(listWidget,iconTabSub,cmd.name,setCurrentToNew);
    }
    qDebug()<<"stackWidget->count()"<<stackWidget->count();
}

QString XMLCommandParser::calculateHex(const QString& input, const QString& rule, int byteLength) {
    // 1. 输入值转数字
    bool ok;
    double value = input.toDouble(&ok);
    if (!ok) return "Invalid";

    // 2. 应用编码规则
//    if (rule == "x") {
//        // 直接使用原值
//    } else if (rule.startsWith("x*")) {
//        double factor = rule.mid(2).toDouble(&ok);
//        if (ok) value *= factor;
//    } else if (rule.startsWith("x/")) {
//        double divisor = rule.mid(2).toDouble(&ok);
//        if (ok && divisor != 0) value /= divisor;
//    } else {
//        return "UnknownRule";
//    }

    // 上面的自定义编码规则太局限，使用QJSEngine支持更多格式rule
    QJSEngine engine;
    // 给表达式中的 x 赋值
    engine.globalObject().setProperty("x", value);
    // 计算表达式
    QJSValue result = engine.evaluate(rule);
    // 出错时返回错误信息
    if (result.isError()) {
        return QString("EvalError: ") + result.toString();
    }
//    // 成功时返回结果
//    return QString::number(result.toNumber(), 'f', 6);

    // 3. 取整并转十六进制  注意这里的取整策略，是直接去掉小数点后数据还是四舍五入
//    int intValue = static_cast<int>(std::round(result.toNumber()));
//    QString hex = QString::number(intValue, 16).toUpper();

    // 3. 取整 → 转为无符号整数 → 转十六进制
    quint64 rawValue = static_cast<quint64>(std::llround(result.toNumber()));
    // 按字节长度截断（非常关键，防止高位污染）
    if (byteLength > 0) {
        quint64 mask = (byteLength >= 8)
            ? 0xFFFFFFFFFFFFFFFFULL
            : ((1ULL << (byteLength * 8)) - 1);
        rawValue &= mask;
    }

    // 转十六进制
    QString hex = QString::number(rawValue, 16).toUpper();

    // 4. 按字节长度补零（如长度1 → 2字符，长度2 → 4字符）
    if (byteLength > 0) {
        int totalChars = byteLength * 2;
        hex = hex.rightJustified(totalChars, '0'); // 右侧补零
    }
    return hex;
}

void XMLCommandParser::deleteComChoose(int delIndex)
{
    commandList.removeAt(delIndex);
}

///*********************************加载指令序列xml文件*************************************

bool XMLCommandParser::loadXML2(const QString &filePath,QList<QDomElement> &docNodeList)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开XML文件:" << filePath;
        return false;
    }

    /// 整个XML文档对象
    //    QDomDocument doc;
    doc.clear(); // 如上一行，这里的doc刚开始设计成局部变量，随着函数的结束就销毁了，但是后续parseCommands2中的很多地方都依赖它，会导致程序崩溃

    if (!doc.setContent(&file)) { //把整个 XML 文件一次性读入内存，构造成一棵“DOM 树”
        qWarning() << "XML 解析失败";
        file.close();
        return false;
    }
    file.close();

    /// 元素节点，最常用
    QDomElement root = doc.documentElement();// 拿到根元素 System
    if (root.tagName() != "SYSTEM") {
        qWarning() << "XML根标签不是<SYSTEM>";
        return false;
    }

    parseCommands2(root,docNodeList);
    return true;
}

void XMLCommandParser::parseCommands2(const QDomElement &root, QList<QDomElement> &docNodeList)
{
    docNodeList.clear();
    commandQueueVector.clear();
    /// 节点列表
    QDomNodeList nodeList_Command = root.elementsByTagName("Command"); /// 1_Command：element——>list

    qDebug()<<"Command 数量"<<nodeList_Command.count();
    for (int i = 0; i < nodeList_Command.count(); ++i) {

        /// 任何节点的基类
        QDomElement element_Command = nodeList_Command.at(i).toElement(); /// 1_Command：list——>element

//        docNodeList.append(element_Command); // 将当前加载xml文件中的Command节点保存起来，方便后续修改配置并保存新的xml文件
        docNodeList.append(element_Command.cloneNode(true).toElement()); // ⭐️加 true 才会深拷贝子节点。
        // 开始读取我所需要的信息填入指令序列
        QString commandTime = element_Command.attribute("CommandTimeD");
        QString commandName = element_Command.attribute("CommandSpecificationT");
        QString commandCodeH = element_Command.attribute("CommandCodeH");
        QString commandRemarksH = element_Command.attribute("RemarksD");

        CommandQueue queue;
        queue.commandTime = commandTime;
        queue.commandName = commandName;
        queue.commandCodeH = commandCodeH;
        queue.commandRemarks = commandRemarksH;

        /*// 打印该元素的所有属性
        QDomNamedNodeMap commandAttributes = commandElement.attributes();
        qDebug()<<"commandAttributes 数量"<<commandAttributes.count();
        for (int j = 0; j < commandAttributes.count(); ++j) {
            QDomNode attr = commandAttributes.item(j);
            qDebug() << "   Attribute:" << attr.nodeName() << "=" << attr.nodeValue();
        }*/

        QDomNodeList nodeList_CommandPara = element_Command.elementsByTagName("CommandPara"); /// 2_CommandPara：element——>list
        QString commandCode;
        qDebug()<<"CommandPara 数量"<<nodeList_CommandPara.count();
        for (int j = 0; j < nodeList_CommandPara.count();++j) {
            QDomElement element_CommandPara = nodeList_CommandPara.at(j).toElement(); /// 2_CommandPara：list——>element
            int initIndex = element_CommandPara.attribute("CommandParaInitSelectD").toInt();
//            qDebug()<<"initIndex:"<<initIndex;

            QDomNodeList nodeList_Para = element_CommandPara.elementsByTagName("Para"); /// 3_Para：element——>list
//            QDomElement element_Para = nodeList_Para.at(i).toElement(); /// 3_Para：list——>element
            QVector<QString> paraCodeList;
            qDebug()<<"nodeList_Para 数量"<<nodeList_Para.count();
            for (int k = 0; k < nodeList_Para.count();++k) {
                QDomElement element_Para = nodeList_Para.at(k).toElement(); /// 3_Para：list——>element
                QString ParaRawCodeH = element_Para.attribute("ParaRawCodeH");
                qDebug()<<"ParaRawCodeH :"<<ParaRawCodeH;
                paraCodeList.append(ParaRawCodeH);
            }
            commandCode+=paraCodeList.at(initIndex);
        }
        queue.commandCode = commandCode;
        commandQueueVector.append(queue);
    }
    for (int i = 0; i < commandQueueVector.size(); ++i) {
        const CommandQueue &cmd = commandQueueVector.at(i);
        qDebug()  << "commandTime=" << cmd.commandTime
                  << "commandName=" << cmd.commandName
                  << "commandCodeH=" << cmd.commandCodeH
                  << "commandCode=" << cmd.commandCode
                  << "commandRemarks=" << cmd.commandRemarks
                 ;
    }

    // 将加载的指令序列添加到列表中
    emit updateParaListSignal(commandQueueVector);
    qDebug()<<"信号已发送";
    qDebug()<<"docNodeList.count()"<<docNodeList.count();

}

void XMLCommandParser::parseCommands3(QList<QDomElement> &docNodeList, int index, QStackedWidget *stackedWidget, int indexCom) // index指的当前选中的指令序列中的指令行
{
    qDebug() << "PARSE";
    QDomElement commandElem = docNodeList.at(index).toElement();
    Command cmd;
    cmd.name = commandElem.attribute("CommandSpecificationT", QString("Command %1").arg(index)); // 当前指令模块的名称
    cmd.Index = commandElem.attribute("CommandSeqIdxD", QString::number(index)).toInt(); // 当前指令模块的名称
    emit updateSubUiCurComShowSignal(cmd.name);
    QDomNodeList attrNodes = commandElem.elementsByTagName("CommandPara");
    for (int j = 0; j < attrNodes.count(); ++j) {
        QDomElement attrElem = attrNodes.at(j).toElement();
        CommandAttribute attr;
        attr.paraNo = attrElem.attribute("CommandParaSeqIdxD"); // 参数编号
        attr.paraName = attrElem.attribute("CommandParaSpecificationT"); // 参数名称
        attr.paraType = attrElem.attribute("CommandParamTypeT"); // 参数类别
        attr.byteNotes = attrElem.attribute("AttributeByteLenD");// 备注
        attr.initIndex = attrElem.attribute("CommandParaInitSelectD").toInt(); // 转为整数; // 默认索引

        // 查找 <Para> 子项
        QDomNodeList paraNodes = attrElem.elementsByTagName("Para");
        for (int k = 0; k < paraNodes.count(); ++k) {
            QDomElement paraElem = paraNodes.at(k).toElement();
            QString option = paraElem.attribute("ParaSpecificationT");
            QString code = paraElem.attribute("ParaRawCodeH");
            QString intCode = paraElem.attribute("ParaInitCode");

            if (!option.isEmpty())
                attr.paraOptions << option; // combobox对应的选项
            if (!code.isEmpty())
                attr.hexCodes << code; // combobox对应的源码

            if (k == 0) {
                // 默认值来自第一个选项
                attr.paraValue = option;
                attr.byteHex = code;
                attr.paraIntCode = intCode;
            }
        }

        // 新增字段解析
         attr.encodeRule = attrElem.attribute("CommandParaEncodeTypeT");
         attr.byteLength = attrElem.attribute("CommandParaByteLengthD").toInt(); // 转为整数
         attr.minValue = attrElem.attribute("CommandParaMinF");
         attr.maxValue = attrElem.attribute("CommandParaMaxF");

        cmd.attributes.append(attr);
    }
//    docNodeListTemp = docNodeList; // 为了让子窗口编辑指令时不要立即修改docNodeList中的元素，只有当用户点击OK按钮时
    /**
        docNodeListTemp = docNodeList;
        这是浅拷贝：docNodeListTemp 中的 QDomElement 还是指向原始 DOM 树里的节点
        docNodeList 和 docNodeListTemp 都是 QList<QDomElement> 类型。
        QDomElement 是一个轻量级对象，内部持有指向 DOM 节点的指针（类似智能指针）
        docNodeListTemp = docNodeList;
        这只是把 QList 容器里的元素复制了一份，但每个 QDomElement 内部仍然指向同一个 DOM 节点。
        也就是说，修改 docNodeListTemp 里的元素，实际上修改的还是原始 XML 树里的节点。这就是“浅拷贝”的含义。
        所以需要替换为下面的深拷贝

        浅拷贝：拷贝对象的“指针”，原对象和副本共用同一份数据。
        深拷贝：拷贝对象本身及它的内容，原对象和副本互不影响。
        对于 QDomElement，默认的赋值操作（=）是浅拷贝；如果要在子窗口独立修改，就必须用 cloneNode() 做深拷贝。
    */

//    docNodeListTemp.clear();
//    for (const auto &elem : docNodeList) {
//        docNodeListTemp.append(elem.cloneNode().toElement()); // 深拷贝
//    }
//    qDebug() << "重新生成 docNodeListTemp";

    QTableWidget *table = createTableForCommand(cmd, docNodeListTemp, index);
    stackedWidget->addWidget(table);  // 不再使用 addTab，而是 addWidget
    stackedWidget->setCurrentWidget(table);
}

QList<QDomElement> XMLCommandParser::getDocNodeListTemp()
{
    return docNodeListTemp;
}


/// 遥测XML文件读取及表格创建
bool XMLCommandParser::loadTmXML(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开XML文件:" << filePath;
        return false;
    }

    /// 整个XML文档对象
    QDomDocument doc;
    if (!doc.setContent(&file)) { //把整个 XML 文件一次性读入内存，构造成一棵“DOM 树”
        qWarning() << "XML 解析失败";
        file.close();
        return false;
    }
    file.close();

    /// 元素节点，最常用
    QDomElement root = doc.documentElement();// 拿到根元素 System
    if (root.tagName() != "SYSTEM") {
        qWarning() << "XML根标签不是<SYSTEM>";
        return false;
    }

    parseTMs(root);
    return true;
}

/// 遥测XML文件读取及表格创建
bool XMLCommandParser::loadTmXML_slow(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开XML文件:" << filePath;
        return false;
    }

    /// 整个XML文档对象
    QDomDocument doc;
    if (!doc.setContent(&file)) { //把整个 XML 文件一次性读入内存，构造成一棵“DOM 树”
        qWarning() << "XML 解析失败";
        file.close();
        return false;
    }
    file.close();

    /// 元素节点，最常用
    QDomElement root = doc.documentElement();// 拿到根元素 System
    if (root.tagName() != "SYSTEM") {
        qWarning() << "XML根标签不是<SYSTEM>";
        return false;
    }

    parseTMs_slow(root);
    return true;
}

bool XMLCommandParser::parseTMs(QDomElement root){

//    int frameLength = 0;
    QDomNodeList nodeList_TM = root.elementsByTagName("TM");
    for (int i = 0; i < nodeList_TM.size(); ++i) {
        QDomElement elemTM = nodeList_TM.at(i).toElement();
//        frameLength = elemTM.attribute("FrameLength").toInt();
        frameTmXML.frameLength = elemTM.attribute("FrameLength").toInt();
        frameTmXML.frameHead = elemTM.attribute("FrameHeader");
        frameTmXML.frameTail = elemTM.attribute("FrameTail");
        frameTmXML.checkSumNum = elemTM.attribute("CheckSumLength").toInt();

        frameTmXML.frameLengthAll = elemTM.attribute("FrameLengthAll").toInt();
        frameTmXML.frameHeadAll = elemTM.attribute("FrameHeaderAll");
        frameTmXML.CutFront = elemTM.attribute("CutFront");

    }
    qDebug()<<"FrameLength"<<frameTmXML.frameLength; // 127
    emit updateFrameTMSignal(frameTmXML); // 更新dataAnalysishelper中的帧头帧尾等设置

    /// 节点列表
    QDomNodeList nodeList_TMitem = root.elementsByTagName("TMitem"); /// 1_TMitem：element——>list
    int TMitemCount = nodeList_TMitem.count(); // 遥测参数数量
    qDebug()<<"TMitem 数量"<<TMitemCount; // 测试59/实际114

    for (int i = 0; i < nodeList_TMitem.count(); ++i) {

        /// 任何节点的基类
        QDomElement element_TMitem = nodeList_TMitem.at(i).toElement(); /// 1_TMitem：list——>element

        // 开始读取我所需要的信息填入指令序列
        QString TMitemLength = element_TMitem.attribute("TMitemByteLengthD");
        QString TMitemNo = element_TMitem.attribute("TMitemNoName");
        QString TMitemName = element_TMitem.attribute("TMitemSpecificationT");
        QString ParaType = element_TMitem.attribute("TMitemType");
        QString EncodeType = element_TMitem.attribute("TMitemEncodeTypeT");
        QString Para = element_TMitem.attribute("TMitemNoName");
        QString TMitemRemarks = element_TMitem.attribute("TMitemNote");
        QString TMitemMinF = element_TMitem.attribute("TMitemMinF");
        QString TMitemMaxF = element_TMitem.attribute("TMitemMaxF");
        QString ByteInherit = element_TMitem.attribute("ByteInherit");
        QString BitOffset = element_TMitem.attribute("BitOffset");
        QString BitLength = element_TMitem.attribute("BitLength");
        QString TMsignedType = element_TMitem.attribute("TMsignedType");

        TMitemQueue queue;
        queue. TMitemLength = TMitemLength; // 遥测参数源码长度
        queue. TMitemNo = TMitemNo; // 遥测参数编号
        queue. TMitemName = TMitemName ; // 遥测参数名称
        queue. ParaType = ParaType; // 遥测参数类型
        queue. EncodeType = EncodeType; // 遥测参数解码规则（从十六位源码到十进制数值）
//        queue. Para = Para; // 遥测参数数值
        queue. TMitemRemarks  = TMitemRemarks; // 备注
        queue. TMitemMinF  = TMitemMinF; // 备注// 最小值
        queue. TMitemMaxF  = TMitemMaxF; // 备注// 最大值
        queue. ByteInherit  = ByteInherit; // 继承性
        queue. BitOffset  = BitOffset; // 继承性
        queue. BitLength  = BitLength; // 继承性
        queue. TMsignedType = TMsignedType; // 符号数类型
//        queue.commandCodeH = commandCodeH;
//        queue.commandRemarks = commandRemarksH;

        /*// 打印该元素的所有属性
        QDomNamedNodeMap commandAttributes = commandElement.attributes();
        qDebug()<<"commandAttributes 数量"<<commandAttributes.count();
        for (int j = 0; j < commandAttributes.count(); ++j) {
            QDomNode attr = commandAttributes.item(j);
            qDebug() << "   Attribute:" << attr.nodeName() << "=" << attr.nodeValue();
        }*/

        QDomNodeList nodeList_Para = element_TMitem.elementsByTagName("Para"); /// 2_Para：element——>list
        qDebug()<<"Para 数量"<<nodeList_Para.count();
        for (int j = 0; j < nodeList_Para.count();++j) {
            QDomElement element_Para = nodeList_Para.at(j).toElement(); /// 2_Para：list——>element
            QString paraName = element_Para.attribute("ParaSpecificationT");
            QString paraCode = element_Para.attribute("ParaRawCodeH");
            TMparaQueue TMqueue;
            TMqueue.paraName = paraName;
            TMqueue.initCode = paraCode;
            queue.TMparaList.append(TMqueue);
        }
        paraQueueVector.append(queue);
    }
    for (int i = 0; i < paraQueueVector.size(); ++i) {
        const TMitemQueue &cmd = paraQueueVector.at(i);
        qDebug()  << "源码长度" << cmd.TMitemLength
                  << "参数编号" << cmd.TMitemNo
                  << "参数名称" << cmd.TMitemName
                  << "参数类型" << cmd.ParaType
                  << "编码方式" << cmd.EncodeType
                  << "备注" << cmd.TMitemRemarks
                  << "最小值" <<cmd.TMitemMinF
                  << "最大值" <<cmd.TMitemMaxF
                    << "继承性" <<cmd.ByteInherit
                    <<"字位移"<<cmd.BitOffset
                   <<"字长度"<<cmd.BitLength
                    <<"符号类型"<<cmd.TMsignedType;
         for(int j = 0 ; j < cmd.TMparaList.size(); ++j){
            qDebug()<<j<<"paraName"<< cmd.TMparaList.at(j).paraName;
            qDebug()<<j<<"initCode"<< cmd.TMparaList.at(j).initCode;
        }
    }

    // 将加载的遥测序列添加到列表中
    // 将遥测加载格式发送给dataanalysis
    emit updateTMParaListSignal(paraQueueVector);
    qDebug()<<"遥测列表初始化信号已发送";
    qDebug()<<paraQueueVector.at(0).FrameLength.toInt();

}

bool XMLCommandParser::parseTMs_slow(QDomElement root){

//    int frameLength = 0;
    QDomNodeList nodeList_TM = root.elementsByTagName("TM");
    for (int i = 0; i < nodeList_TM.size(); ++i) {
        QDomElement elemTM = nodeList_TM.at(i).toElement();
//        frameLength = elemTM.attribute("FrameLength").toInt();
        frameTmXML_slow.frameLength = elemTM.attribute("FrameLength").toInt();
        frameTmXML_slow.frameHead = elemTM.attribute("FrameHeader");
        frameTmXML_slow.frameTail = elemTM.attribute("FrameTail");
        frameTmXML_slow.checkSumNum = elemTM.attribute("CheckSumLength").toInt();

    }
    qDebug()<<"FrameLength"<<frameTmXML_slow.frameLength; // 169
    emit updateFrameTMSignal_slow(frameTmXML_slow);



    /// 节点列表
    QDomNodeList nodeList_TMitem = root.elementsByTagName("TMitem"); /// 1_TMitem：element——>list
    int TMitemCount = nodeList_TMitem.count(); // 遥测参数数量
    qDebug()<<"TMitem 数量"<<TMitemCount; // 82

    for (int i = 0; i < nodeList_TMitem.count(); ++i) {

        /// 任何节点的基类
        QDomElement element_TMitem = nodeList_TMitem.at(i).toElement(); /// 1_TMitem：list——>element

        // 开始读取我所需要的信息填入指令序列
        QString TMitemLength = element_TMitem.attribute("TMitemByteLengthD");
        QString TMitemNo = element_TMitem.attribute("TMitemNoName");
        QString TMitemName = element_TMitem.attribute("TMitemSpecificationT");
        QString ParaType = element_TMitem.attribute("TMitemType");
        QString EncodeType = element_TMitem.attribute("TMitemEncodeTypeT");
        QString Para = element_TMitem.attribute("TMitemNoName");
        QString TMitemRemarks = element_TMitem.attribute("TMitemNote");
        QString TMitemMinF = element_TMitem.attribute("TMitemMinF");
        QString TMitemMaxF = element_TMitem.attribute("TMitemMaxF");
        QString ByteInherit = element_TMitem.attribute("ByteInherit");
        QString BitOffset = element_TMitem.attribute("BitOffset");
        QString BitLength = element_TMitem.attribute("BitLength");
        QString TMsignedType = element_TMitem.attribute("TMsignedType");


        TMitemQueue queue;
        queue. TMitemLength = TMitemLength; // 遥测参数源码长度
        queue. TMitemNo = TMitemNo; // 遥测参数编号
        queue. TMitemName = TMitemName ; // 遥测参数名称
        queue. ParaType = ParaType; // 遥测参数类型
        queue. EncodeType = EncodeType; // 遥测参数解码规则（从十六位源码到十进制数值）
//        queue. Para = Para; // 遥测参数数值
        queue. TMitemRemarks  = TMitemRemarks; // 备注
        queue. TMitemMinF  = TMitemMinF; // 备注// 最小值
        queue. TMitemMaxF  = TMitemMaxF; // 备注// 最大值
        queue. ByteInherit  = ByteInherit; // 继承性
        queue. BitOffset  = BitOffset; // 继承性
        queue. BitLength  = BitLength; // 继承性
        queue. TMsignedType = TMsignedType; // 符号数类型
//        queue.commandCodeH = commandCodeH;
//        queue.commandRemarks = commandRemarksH;

        /*// 打印该元素的所有属性
        QDomNamedNodeMap commandAttributes = commandElement.attributes();
        qDebug()<<"commandAttributes 数量"<<commandAttributes.count();
        for (int j = 0; j < commandAttributes.count(); ++j) {
            QDomNode attr = commandAttributes.item(j);
            qDebug() << "   Attribute:" << attr.nodeName() << "=" << attr.nodeValue();
        }*/

        QDomNodeList nodeList_Para = element_TMitem.elementsByTagName("Para"); /// 2_Para：element——>list
        qDebug()<<"Para 数量"<<nodeList_Para.count();
        for (int j = 0; j < nodeList_Para.count();++j) {
            QDomElement element_Para = nodeList_Para.at(j).toElement(); /// 2_Para：list——>element
            QString paraName = element_Para.attribute("ParaSpecificationT");
            QString paraCode = element_Para.attribute("ParaRawCodeH");
            TMparaQueue TMqueue;
            TMqueue.paraName = paraName;
            TMqueue.initCode = paraCode;
            queue.TMparaList.append(TMqueue);
        }
        paraQueueVector_slow.append(queue);
    }
    for (int i = 0; i < paraQueueVector_slow.size(); ++i) {
        const TMitemQueue &cmd = paraQueueVector_slow.at(i);
        qDebug()  << "源码长度" << cmd.TMitemLength
                  << "参数编号" << cmd.TMitemNo
                  << "参数名称" << cmd.TMitemName
                  << "参数类型" << cmd.ParaType
                  << "编码方式" << cmd.EncodeType
                  << "备注" << cmd.TMitemRemarks
                  << "最小值"<<cmd.TMitemMinF
                  <<"最大值"<<cmd.TMitemMaxF
                 <<"符号类型"<<cmd.TMsignedType;
         for(int j = 0 ; j < cmd.TMparaList.size(); ++j){
            qDebug()<<j<<"paraName"<< cmd.TMparaList.at(j).paraName;
            qDebug()<<j<<"initCode"<< cmd.TMparaList.at(j).initCode;
        }
    }

    // 将加载的遥测序列添加到列表中
    // 将遥测加载格式发送给dataanalysis
    emit updateTMParaListSignal_slow(paraQueueVector_slow);
    qDebug()<<"遥测列表初始化信号已发送";
    qDebug()<<paraQueueVector_slow.at(0).FrameLength.toInt();

}

void XMLCommandParser::setDocNodeList(const QList<QDomElement> &list)
{
    qDebug() << "CLONE TEMP";
    docNodeListTemp.clear();
    for (const auto &elem : list)
        docNodeListTemp.append(elem.cloneNode(true).toElement());
}
