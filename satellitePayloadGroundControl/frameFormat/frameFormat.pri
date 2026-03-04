QT       += xml


SDKPATH =
DRIVER_SDK_PATH =

#判断编译环境是32位还是64位
contains(QT_ARCH, i386) {
    message("32-bit")
    win32:mingw{
        message("mingw!")
        SDKPATH = $$PWD/lua/lua-5.4.2_Win32_mingw6_lib
        message(PWD: $$PWD)
        INCLUDEPATH += $$SDKPATH/include
        message(INCLUDEPATH: $$INCLUDEPATH)
        LIBS+=$$SDKPATH/liblua54.a
    }
    win32:msvc{
        message("msvc!")
        SDKPATH = $$PWD/lua/lua-5.4.2_Win32_dllw6_lib
        INCLUDEPATH += $$SDKPATH/include
        LIBS+=$$SDKPATH/liblua54.a
    }
} else {
    win32:mingw{
        message("mingw!")
        SDKPATH = $$PWD/lua/lua-5.4.2_Win64_mingw6_lib
        INCLUDEPATH += $$SDKPATH/include
        LIBS+=$$SDKPATH/liblua54.a
    }
    win32:msvc{
        message("msvc!")
        SDKPATH = $$PWD/lua/lua-5.4.2_Win64_dllw6_lib
        INCLUDEPATH += $$SDKPATH/include
        LIBS+=$$SDKPATH/liblua54.a
    }
}



HEADERS += \
    $$PWD/app.h \
    $$PWD/apptools.h \
    $$PWD/cardmanager.h \
    $$PWD/checkbox.h \
    $$PWD/combobox.h \
    $$PWD/comboxdelegate.h \
    $$PWD/command.h \
    $$PWD/delegate.h \
    $$PWD/doublespinbox.h \
    $$PWD/frameassemble.h \
    $$PWD/frmpagescommand.h \
    $$PWD/frmshowmsg.h \
    $$PWD/head.h \
    $$PWD/luahelper.h \
    $$PWD/parasetelemetryxml.h \
    $$PWD/savedatahelper.h

SOURCES += \
    $$PWD/app.cpp \
    $$PWD/apptools.cpp \
    $$PWD/cardmanager.cpp \
    $$PWD/checkbox.cpp \
    $$PWD/combobox.cpp \
    $$PWD/comboxdelegate.cpp \
    $$PWD/command.cpp \
    $$PWD/delegate.cpp \
    $$PWD/doublespinbox.cpp \
    $$PWD/frameassemble.cpp \
    $$PWD/frmpagescommand.cpp \
    $$PWD/frmshowmsg.cpp \
    $$PWD/luahelper.cpp \
    $$PWD/parasetelemetryxml.cpp \
    $$PWD/savedatahelper.cpp

FORMS += \
    $$PWD/frmpagescommand.ui \
    $$PWD/frmshowmsg.ui
