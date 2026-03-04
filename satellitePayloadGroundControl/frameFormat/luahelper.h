#ifndef LUAHELPER_H
#define LUAHELPER_H

#include <QObject>
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
class LuaHelper : public QObject
{
    Q_OBJECT
public:
    static LuaHelper *Instance();
    //处理字符串
    double execString(QString str,double x);
    //int execIntString(QString str,int x);
private:
    static LuaHelper *self;
    explicit LuaHelper(QObject *parent = nullptr);
    ~LuaHelper();
signals:

public slots:

private:
    lua_State * L = nullptr;
};

#endif // LUAHELPER_H
