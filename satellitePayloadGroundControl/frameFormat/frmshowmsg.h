#ifndef FRMSHOWMSG_H
#define FRMSHOWMSG_H

#include <QWidget>

namespace Ui {
class frmShowMsg;
}

class frmShowMsg : public QWidget
{
    Q_OBJECT

public:
    explicit frmShowMsg(QWidget *parent = nullptr);
    ~frmShowMsg();
    static frmShowMsg *Instance();
    //显示接收响应数据
    void showResponseMsg(QByteArray data);
    //显示接收响应结果
    void showResponseResultMsg(QString msg);

private:
    static frmShowMsg *self;
private:
    Ui::frmShowMsg *ui;
    QString currentData();
};

#endif // FRMSHOWMSG_H
