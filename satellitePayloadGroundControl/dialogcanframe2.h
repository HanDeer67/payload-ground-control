#ifndef DIALOGCANFRAME2_H
#define DIALOGCANFRAME2_H

#include <QDialog>
#include "Structs.h"

namespace Ui {
class DialogCanFrame2;
}

class DialogCanFrame2 : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCanFrame2(QWidget *parent = nullptr);
    ~DialogCanFrame2();

public:
    canFrameConfig canFrameConfigDia2;

    void onOkButtonClicked();
private:
    Ui::DialogCanFrame2 *ui;

signals:
    void updateCanFrameConfigUiSignal(canFrameConfig canFrameConfigDia2);

};

#endif // DIALOGCANFRAME2_H
