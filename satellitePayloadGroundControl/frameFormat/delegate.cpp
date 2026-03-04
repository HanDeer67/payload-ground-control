#include "delegate.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDebug>
#include <QDateTime>

//createEditor->setEditorData->setModelData->setEditorData->...->closeEditor(信号)->destroyEditor;
Delegate::Delegate(QObject *parent) : QItemDelegate(parent)
{
//    connect(this,&QItemDelegate::closeEditor,this,[=](){
//        qDebug()<<"closeEditor!";
//    });
}
void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    QItemDelegate::paint(painter, option, index);
}

QSize Delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QItemDelegate::sizeHint(option, index);
}

QWidget *Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                                   const QModelIndex &index) const
{
    //qDebug()<<QDateTime::currentDateTime()<<"createEditor!";
    //if (index.isValid() && delegateInfo.cols.contains(index.column()) && delegateInfo.rows.contains(index.row()))
    if (index.isValid())
    {
        //DelegateKey key = DelegateKey(index.row(),index.column());
        int row = index.row();

        if(cmdInfo.paraInfo.length()>row )//&& delegateInfo.map.contains(key))
        {
            ParamInfo parm = cmdInfo.paraInfo.at(row);

            //DelegateValue value = delegateInfo.map.value(key);

            if(parm.type == PARAM_TYPE::COMBOBOX)
            {
                QComboBox *editor = new QComboBox(parent);
                editor->addItems(parm.paraNameList);
                editor->setEditable(false);
                editor->installEventFilter(const_cast<Delegate *>(this));
                return editor;
            }
            else if(parm.type == PARAM_TYPE::EDIT)
            {
                QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
                editor->installEventFilter(const_cast<Delegate *>(this));
                bool isOk1 = false;
                bool isOk2 = false;

                double dMax = parm.paramMaxValue.toInt(&isOk1);
                double dMin = parm.paramMinValue.toInt(&isOk2);

                if(isOk1 && isOk2)
                {
                    //显示0位小数
                    editor->setDecimals(0);
                    editor->setSingleStep(1);
                }
                else
                {
                    //显示几位小数
                    //editor->setDecimals(4);
                    editor->setSingleStep(1);

                    dMax = parm.paramMaxValue.toDouble(&isOk1);
                    dMin = parm.paramMinValue.toDouble(&isOk2);
                }


                //qDebug()<<value.paramMinValue<<value.paramMaxValue<<dMax<<dMin;
                editor->setMaximum(dMax);
                editor->setMinimum(dMin);
                return editor;
            }
            else
            {
               return QItemDelegate::createEditor(parent, option, index);
            }
        }
        else
        {
           return QItemDelegate::createEditor(parent, option, index);
        }
    }
    else
    {
        return QItemDelegate::createEditor(parent, option, index);
    }
}

void Delegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    //qDebug()<<QDateTime::currentDateTime()<<"setEditorData!"<<index<<index.model()->data(index, Qt::DisplayRole).toString();
    if (index.isValid())
    {
        int row = index.row();
        if(cmdInfo.paraInfo.length()>row )
        {
            ParamInfo parm = cmdInfo.paraInfo.at(row);

            if(parm.type == PARAM_TYPE::COMBOBOX)
            {
                QString curValue = index.model()->data(index, Qt::DisplayRole).toString();
                QComboBox *combox = static_cast<QComboBox *>(editor);
                combox->setCurrentText(curValue);
            }
            else if(parm.type == PARAM_TYPE::EDIT)
            {
                int curValue = index.model()->data(index, Qt::DisplayRole).toInt();
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox *>(editor);
                spinBox->setValue(curValue);
            }
            else
            {
                QItemDelegate::setEditorData(editor, index);
            }
        }
        else
        {
           QItemDelegate::setEditorData(editor, index);
        }
    }
    else
    {
        QItemDelegate::setEditorData(editor, index);
    }
}

void Delegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                             const QModelIndex &index) const
{
    //qDebug()<<QDateTime::currentDateTime()<<"setModelData!"<<index<<index.model()->data(index, Qt::DisplayRole).toString();
    if (index.isValid())
    {
        //DelegateKey key = DelegateKey(index.row(),index.column());
        int row = index.row();

        if(cmdInfo.paraInfo.length()>row)
        {
            ParamInfo parm = cmdInfo.paraInfo.at(row);

            if(parm.type == PARAM_TYPE::COMBOBOX)
            {
                QComboBox *combox = static_cast<QComboBox *>(editor);
                model->setData(index, combox->currentText());
            }
            else if(parm.type == PARAM_TYPE::EDIT)
            {
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox *>(editor);
                model->setData(index, spinBox->value());
            }
            else
            {
                QItemDelegate::setModelData(editor, model, index);
            }

        }
        else
        {
            QItemDelegate::setModelData(editor, model, index);
        }

    }
    else
    {
        QItemDelegate::setModelData(editor, model, index);
    }
}

void Delegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
//    qDebug()<<"destroyEditor"<<index<<index.model()->data(index, Qt::DisplayRole).toString();
    QItemDelegate::destroyEditor(editor,index);
}

//void Delegate::setDelegateInfo(DelegateInfo info)
//{
//    //this->delegateInfo = info;
//}

void Delegate::setCommand(CommandInfo cmd)
{
    this->cmdInfo = cmd;
}
