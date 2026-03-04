#ifndef DIALOGPARA_H
#define DIALOGPARA_H

#include <QDialog>
#include "xmlcommandparser.h"

namespace Ui {
class DialogPara;
}

class DialogPara : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPara(QWidget *parent = nullptr);
    ~DialogPara();

    void loadCommand(XMLCommandParser *parser, QList<QDomElement> &docNodeList, int indexCom, int index);
    void onOkButtonClicked();
    void on_buttonBox_rejected();
private:
    Ui::DialogPara *ui;

signals:
    void updateParaSignal(QString newCodeHex);
    void getDocNodeListTempSignal();
};

#endif // DIALOGPARA_H
