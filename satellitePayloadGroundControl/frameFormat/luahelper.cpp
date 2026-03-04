#include "luahelper.h"
#include <QDebug>




#include <QMutex>
LuaHelper *LuaHelper::self = nullptr;
LuaHelper *LuaHelper::Instance()
{
    if (!self) {
        QMutex mutex;
        QMutexLocker locker(&mutex);
        if (!self) {
            self = new LuaHelper;
        }
    }

    return self;
}

//int LuaHelper::execIntString(QString str, int x)
//{
//    if(L)
//    {
//        lua_pushinteger(L, x);
//        lua_setglobal(L, "x");
//        QString ret = "y=";
//        ret+=str;
//        int r = luaL_dostring(L,ret.toLatin1().data());
//        if(r == LUA_OK)
//        {
//            lua_getglobal(L,"y");
//            if(lua_isnumber(L,-1))
//            {
//                int result = static_cast<int>(lua_tonumber(L,-1));
//                //qDebug()<<"result:"<<result;
//                return result;
//            }
//        }
//        else
//        {
//            QString errorMsg = lua_tostring(L,-1);
//            qDebug()<<"lua clc error:"<<errorMsg;
//        }
//    }

//    return 0;
//}

double LuaHelper::execString(QString str,double x)
{
    if(L)
    {
        lua_pushnumber(L, x);
        lua_setglobal(L, "x");
        QString ret = "y=";
        ret+=str;
        int r = luaL_dostring(L,ret.toLatin1().data());
        if(r == LUA_OK)
        {
            lua_getglobal(L,"y");
            if(lua_isnumber(L,-1))
            {
                double result = static_cast<double>(lua_tonumber(L,-1));
                //qDebug()<<"result:"<<result;
                return result;
            }
        }
        else
        {
            QString errorMsg = lua_tostring(L,-1);
            qDebug()<<"lua clc error:"<<errorMsg;
        }
    }

    return 0.0;
}

LuaHelper::LuaHelper(QObject *parent) : QObject(parent)
{
    L = luaL_newstate();
    if(L)
    {
      luaL_openlibs(L);
    }
}

LuaHelper::~LuaHelper()
{
    //4.关闭state
    lua_close(L);
}
