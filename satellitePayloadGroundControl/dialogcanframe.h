#ifndef DIALOGCANFRAME_H
#define DIALOGCANFRAME_H

#include <QDialog>
#include "Structs.h"

namespace Ui {
class DialogCanFrame;
}

class DialogCanFrame : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCanFrame(QWidget *parent = nullptr);
    ~DialogCanFrame();

    void onOkButtonClicked();

public:
    canFrameConfig canFrameConfigDia;
private:
    Ui::DialogCanFrame *ui;

signals:
    void updateCanFrameConfigUiSignal(canFrameConfig canFrameConfigDia);

};

#endif // DIALOGCANFRAME_H
