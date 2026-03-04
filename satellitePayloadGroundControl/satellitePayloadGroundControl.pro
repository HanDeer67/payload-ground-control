
QT       += core gui widgets qml xml

# 强制使用 DirectX 11
# QMAKE_CXXFLAGS += -DQT_USE_DIRECT3D11
# 强制使用 OpenGL
#QMAKE_CXXFLAGS += -DQT_USE_OPENGL
QT += opengl



greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#CONFIG += c++17
 CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogcanframe.cpp \
    dialogcanframe2.cpp \
    dialogpara.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Structs.h \
    dialogcanframe.h \
    dialogcanframe2.h \
    dialogpara.h \
    mainwindow.h

FORMS += \
    dialogcanframe.ui \
    dialogcanframe2.ui \
    dialogpara.ui \
#    frmpagesdatabroadcast.ui \
    mainwindow.ui

# 添加所需的 Qt 模块
QT += core gui widgets serialport


# 添加库文件路径配置
win32 {
    contains(QMAKE_TARGET.arch, x86_64) {
        # 64位系统配置
        LIBS += -L$$PWD/Driver/DLL/Win64 -lLVDS_DLL
        INCLUDEPATH += $$PWD/Driver/DLL/Win64
    } else {
        # 32位系统配置
        LIBS += -L$$PWD/Driver/DLL/Win32 -lLVDS_DLL
        INCLUDEPATH += $$PWD/Driver/DLL/Win32
    }
}

# CAN
# 添加库文件路径配置
win32 {
    contains(QMAKE_TARGET.arch, x86_64) {
        # 64位系统配置
        LIBS += -L$$PWD/Driver/CANalyst/x64(64bit) -lControlCAN
        INCLUDEPATH += $$PWD/Driver/CANalyst/Include/inc
        LIBS += -lversion
    } else {
        # 32位系统配置
        LIBS += -L$$PWD/Driver/CANalyst/win32(32bit) -lControlCAN
        INCLUDEPATH += $$PWD/Driver/CANalyst/Include/inc
        LIBS += -lversion
    }
}


# 地检小白盒
#LIBS += -L$$PWD/Driver/zlgCan/Win32/Debug -lControlCAN
#INCLUDEPATH += $$PWD/Driver/zlgCan/Include/inc
#LIBS += -lversion


CODECFORSRC = UTF-8

QMAKE_CXXFLAGS += -Wno-unused-parameter  #mingW版本的
#QMAKE_CXXFLAGS += /wd4828

RESOURCES += \
    resources.qrc

#INCLUDEPATH += $$PWD/frmPages
#include($$PWD/frmPages/frmPages.pri)

INCLUDEPATH += $$PWD/cardModule
include($$PWD/cardModule/cardModule.pri)

INCLUDEPATH += $$PWD/serialPortModule
include($$PWD/serialPortModule/serialPortModule.pri)

INCLUDEPATH += $$PWD/frameFormat
include($$PWD/frameFormat/frameFormat.pri)

INCLUDEPATH += $$PWD/frameFormatCamera
include($$PWD/frameFormatCamera/frameFormatCamera.pri)

INCLUDEPATH += $$PWD/XML
include($$PWD/XML/XML.pri)

INCLUDEPATH += $$PWD/GeneralTools
include($$PWD/GeneralTools/GeneralTools.pri)

INCLUDEPATH += $$PWD/UICustom
include($$PWD/UICustom/UICustom.pri)

INCLUDEPATH += $$PWD/ZLGCAN
include($$PWD/ZLGCAN/ZLGCAN.pri)

#INCLUDEPATH += $$PWD/CANalyst
#include($$PWD/CANalyst/CANalyst.pri)

INCLUDEPATH += $$PWD/CANFrame
include($$PWD/CANFrame/CANFrame.pri)

# 复制 XmlFiles 到构建输出目录
#XML_DIR = $$PWD/XmlFiles
#TARGET_XML_DIR = $$OUT_PWD/XmlFiles
#QMAKE_POST_LINK += $$quote($(COPY)) $$quote($$XML_DIR) $$quote($$TARGET_XML_DIR)


# 拷贝 XML 文件夹到构建输出目录
XML_DIR = $$PWD/XmlFiles
TARGET_XML_DIR = $$OUT_PWD/XmlFiles
QMAKE_POST_LINK += xcopy /s /e /y /i $$quote($$XML_DIR) $$quote($$TARGET_XML_DIR)

# 添加上述几行代码的目的是不想把xml文件放在资源文件中，因为如果把xml文件放入资源文件中，那么在发布可执行程序时
# xml文件将无法找出来修改，所以我们使用外部资源文件夹的形式调用其中的xml文件，这样，用户也可以随时找到xml文件并
# 修改其中的内容。
# 但是遇到一个问题：当第一次构建时，OUT_PWD（输出目录）可能还没有完全创建好，或者文件系统还没有完全准备好，导致xcopy命令失败。第二次运行时，目录已经存在，所以能成功。


