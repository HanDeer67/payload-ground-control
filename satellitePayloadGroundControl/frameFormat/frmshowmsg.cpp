#include "frmshowmsg.h"
#include "ui_frmshowmsg.h"
#include <QDateTime>
#include "apptools.h"

#include <QMutex>
frmShowMsg *frmShowMsg::self = nullptr;
frmShowMsg *frmShowMsg::Instance()
{
    if (!self) {
        QMutex mutex;
        QMutexLocker locker(&mutex);
        if (!self) {
            self = new frmShowMsg;
        }
    }

    return self;
}

frmShowMsg::frmShowMsg(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmShowMsg)
{
    ui->setupUi(this);
    AppTools::setFormInCenter(this);
}

frmShowMsg::~frmShowMsg()
{
    delete ui;
}


void frmShowMsg::showResponseMsg(QByteArray data)
{
    QString text = currentData()+":"+data.toHex(' ');
    static int lineCount = 0;

    if(lineCount>10000)
    {
        ui->te_response_data->clear();
        lineCount = 0;
    }
    lineCount++;
    ui->te_response_data->append(text);

    //定位到最后
    QTextCursor cursor = ui->te_response_data->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->te_response_data->setTextCursor(cursor);

}

void frmShowMsg::showResponseResultMsg(QString msg)
{
    QString text = currentData()+":"+msg;
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

QString frmShowMsg::currentData()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}
