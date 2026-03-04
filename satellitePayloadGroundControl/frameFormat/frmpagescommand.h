#ifndef FRMPAGESCOMMAND_H
#define FRMPAGESCOMMAND_H

#include <QWidget>
#include "command.h"

class QTableWidget;
class QDomElement;
namespace Ui {
class frmPagesCommand;
}

class frmPagesCommand : public QWidget
{
    Q_OBJECT

public:
    explicit frmPagesCommand(QWidget *parent = nullptr);
    ~frmPagesCommand();
    void initTabel();
    void showResponseResult(QString msg);
protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void showEvent(QShowEvent *event);
public slots:
    void itemChanged(QTableWidgetItem *item);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());

    //处理响应帧处理
    void processRS422Respond(QByteArray &rs422Data);

private slots:
    void on_pb_send_cmd_immediately_clicked();

    void on_pb_add_cmd_clicked();

    void on_pb_clear_cmd_clicked();

    void on_pb_del_cmd_clicked();

    void on_pb_insert_cmd_clicked();

    void on_pb_export_xml_clicked();

    void on_pb_import_xml_clicked();

    void on_pb_send_cmd_clicked();

    void on_pb_t_1_clicked();

    void on_pb_t_2_clicked();

    void on_pb_t_3_clicked();

private:
    void refreshTable();
private:
    Ui::frmPagesCommand *ui;
    //当前命令的第几条
    int currentCommandIndex = 0;
    //本地接收响应统计
    quint32 responseCount = 0;
    //立即令列表
    QVector<CommandInfo> immdCmdInfo;
};

#endif // FRMPAGESCOMMAND_H
