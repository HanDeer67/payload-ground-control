#ifndef DOUBLESPINBOX_H
#define DOUBLESPINBOX_H

#include <QDoubleSpinBox>

class DoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit DoubleSpinBox(QWidget *parent = nullptr);
    int row = -1;
    int col = -1;
signals:

public slots:
};

#endif // DOUBLESPINBOX_H
