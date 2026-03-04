#ifndef HEAD_H
#define HEAD_H
#include <QtCore>
enum PARAM_TYPE{NORMAL,EDIT,COMBOBOX,CHECKBOX};

//typedef struct DELEGATE_KEY
//{
//    int row;
//    int col;
//    DELEGATE_KEY()
//    {
//        this->row = 0;
//        this->col = 0;
//    }
//    DELEGATE_KEY(int row,int col)
//    {
//        this->row = row;
//        this->col = col;
//    }

//    bool operator< (const DELEGATE_KEY& other) const   //注: const 不能省
//    {
//        //有一个数值不相等则就是不同的key
//        //qDebug()<<"DELEGATE_KEY <:"<<this->row<<this->col<<other.row<<other.col<<!(row == other.row && col == other.col);
//        //逻辑不对，此处实现的是小于
//        //return !(row == other.row && col == other.col);

//        if(this->row<other.row)return true;
//        if(this->col<other.col)return true;
//        return false;
//    }

//}DelegateKey;

//typedef struct DELEGATE_VALUE
//{
//    //参数类型
//    //PARAM_TYPE type;
//    //QVariant data;
//    QStringList paraNameList;           //参数名称列表
//    QStringList paraInitCodeList;       //原始数据 十进制显示
//    QStringList paraRawCodeList;        //原始数据 十六进制显示
//    QString paramMinValue;              //最小值
//    QString paramMaxValue;              //最大值

//}DelegateValue;
//typedef struct DELEGATE_INFO
//{
//    //每条命令
//    QList<int> cols;
//    QList<int> rows;
//    QMap<DelegateKey,DelegateValue> map;
//    QMap<DelegateKey,ParamInfo> map2;
//}DelegateInfo;




//对应CommandPara 每个数据域所具有的属性
typedef struct PARAM_INFO
{
    QString paraName;                           //参数名称列表
    PARAM_TYPE type;                            //参数类型
    int currentShowIndex;                       //当前显示的索引
    QString currentShowText;                    //显示当前索引对应的文字
    QString paramEncodeFun;                     //编码函数
    QString paramDecodeFun;                     //解码函数
    quint16 paramByteLength;                    //占几个字节
    QString paraRemark;                         //每个命令域的备注信息
    QStringList paraNameList;                   //参数名称列表
    QStringList paraInitCodeList;               //原始数据 十进制显示
    QStringList paraRawCodeList;                //原始数据 十六进制显示
    QString paramMinValue;                      //最小值
    QString paramMaxValue;                      //最大值

}ParamInfo;

//对应Command
typedef struct COMMAND_INFO_S
{
    QString CommandName;            //参数名称
    QString CommandCode;            //命令码 发送命令使用
    QString CommandTime;            //命令发送的时间戳
    QString CommandRemark;          //这条命令的备注信息
    QList<ParamInfo> paraInfo;      //参数信息 一个命令有多个数据域
}CommandInfo;


//全局Command::Instance()->CommandList 动态更改后还需要将更改后的内容更新到这个列表中

#endif // HEAD_H
