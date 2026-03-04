#include "dialogpara.h"
#include "ui_dialogpara.h"
#include "xmlcommandparser.h"
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>

DialogPara::DialogPara(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPara)
{
    ui->setupUi(this);

    // 当点击窗口下面的OK按钮时，将当前页面的配置组成新的指令替换原本的指令
    // 连接按钮信号
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogPara::onOkButtonClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DialogPara::on_buttonBox_rejected);
}

DialogPara::~DialogPara()
{
    delete ui;
}

void DialogPara::loadCommand(XMLCommandParser* parser,QList<QDomElement> &docNodeList , int indexCom, int index)
{
    // 清空所有旧页面
    while (ui->stackedWidget_paraList->count() > 0) {
        QWidget *widget = ui->stackedWidget_paraList->widget(0);
        ui->stackedWidget_paraList->removeWidget(widget);
        delete widget; // 更高效
    }
    if (!parser) return;
    parser->parseCommands3(docNodeList,index,ui->stackedWidget_paraList, indexCom);
}

void DialogPara::onOkButtonClicked()
{
//    emit getDocNodeListTempSignal(); // 请求更新docNodeList

    // 从表格中组装最新参数
    // 检查当前的StackWidget中是否有tableWidget
    QWidget* currentPage = ui->stackedWidget_paraList->currentWidget();
    QTableWidget* currentTable = qobject_cast<QTableWidget*>(currentPage);
    if (!currentTable) {
       QMessageBox::warning(this, "错误", "当前未显示任何指令表格！");
       return;
    }
    // 获取当前指令配置表格tableWidget的组装指令
    int sourceColumnIdx = -1;
    for (int i = 0; i < currentTable->columnCount(); ++i) {
        QTableWidgetItem* header = currentTable->horizontalHeaderItem(i);
        if (header && header->text().trimmed() == "源码") { // 匹配列标题“源码”
            sourceColumnIdx = i;
            break;
        }
    }
    // 边界处理：未找到“源码”列
    if (sourceColumnIdx == -1) {
        QMessageBox::warning(this, "错误", "当前表格缺少\"源码\"列！");
        return;
    }
    else {
//        qDebug()<<"源码所在列:"<<sourceColumnIdx;
    }

    // 将指令添加到列表中的一行
    QStringList sourceCodes; // 存储有效的源码（大写十六进制）
        int rowCount = currentTable->rowCount();
        qDebug()<<"rowCount"<<rowCount;

        for (int row = 0; row < rowCount; ++row) {
            QTableWidgetItem* sourceItem = currentTable->item(row, sourceColumnIdx);
            if (!sourceItem) continue; // 跳过无源码的行

            QString sourceText = sourceItem->text().trimmed();
            if (sourceText.isEmpty()) continue; // 跳过空值

            // 验证是否为有效的十六进制（可选，增强鲁棒性）
            bool isHexValid = true;
            for (QChar c : sourceText) {
                if (!c.isDigit() && !((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
                    isHexValid = false;
                    break;
                }
            }
            if (!isHexValid) {
                QMessageBox::warning(this, "警告", QString("第%1行的源码\"%2\"无效（非十六进制），已跳过！").arg(row+1).arg(sourceText));
                continue;
            }

            sourceCodes << sourceText.toUpper(); // 统一转为大写（可选，规范格式）
        }
        qDebug()<<"sourceCodes:"<<sourceCodes;
        // 边界处理：无有效源码
        if (sourceCodes.isEmpty()) {
            QMessageBox::critical(this, "错误", "未收集到任何有效源码，请检查表格！");
            return;
        }

        // ==================== 步骤4：组装指令序列（按行顺序拼接） ====================
        QString commandSequence = sourceCodes.join(""); // 例如：“2C15400140010020012C28”
        qDebug()<<"commandSequence:"<<commandSequence;

        emit updateParaSignal(commandSequence);

        emit getDocNodeListTempSignal(); // 同步刷新docNodeList，将docNodeListTemp传递给docNodeList
}

void DialogPara::on_buttonBox_rejected() { // 连接Cancel按钮
        reject(); // 不发射信号
    }


