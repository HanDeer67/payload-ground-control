#include "comboxdelegate.h"
#include <QComboBox>

ComBoxDelegate::ComBoxDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    items<<"是"<<"否";
}

QWidget *ComBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    if (index.isValid()) {
        QComboBox* editor = new QComboBox(parent);
        // 在ComboBox中添加选项
        editor->addItems(items);
        return editor;
    }
    else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

}

void ComBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (QComboBox* comboBox = qobject_cast<QComboBox*>(editor)) {
        comboBox->setCurrentText(index.model()->data(index, Qt::EditRole).toString());
    }
    else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void ComBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (QComboBox* comboBox = qobject_cast<QComboBox*>(editor)) {
        model->setData(index, comboBox->currentIndex(), Qt::EditRole);
        model->setData(index, comboBox->currentText(), Qt::DisplayRole); // 显示ComboBox的当前文本
    }
    else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

void ComBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}
