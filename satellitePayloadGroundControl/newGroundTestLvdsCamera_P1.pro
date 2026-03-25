
QT       += core gui widgets qml

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
#XML_DIR = $$PWD/XmlFiles
#TARGET_XML_DIR = $$OUT_PWD/XmlFiles
#QMAKE_POST_LINK += xcopy /s /e /y /i $$quote($$XML_DIR) $$quote($$TARGET_XML_DIR)

# 添加上述几行代码的目的是不想把xml文件放在资源文件中，因为如果把xml文件放入资源文件中，那么在发布可执行程序时
# xml文件将无法找出来修改，所以我们使用外部资源文件夹的形式调用其中的xml文件，这样，用户也可以随时找到xml文件并
# 修改其中的内容。
# 但是遇到一个问题：当第一次构建时，OUT_PWD（输出目录）可能还没有完全创建好，或者文件系统还没有完全准备好，导致xcopy命令失败。第二次运行时，目录已经存在，所以能成功。

# 2026.3.25更新：上述代码的最初目的是，将xml文件夹自动生成到debug文件夹中，但是实际开发中发现并不如意，且还会导致上述问题，就是每次点击debug或者release按钮时，第一次会先出错，第二次才运行成功
## 将ini文件夹自动copy到构建文件夹中
#CONFIG_INI_SRC = $$PWD/ConfigFiles/*
## 定义目标路径（构建目录，和可执行文件同目录）
#CONFIG_INI_DEST_DEBUG = $$OUT_PWD/debug/ConfigFiles
#CONFIG_INI_DEST_RELEASE = $$OUT_PWD/release/ConfigFiles
## 构建前自动复制config.ini（如果源文件有修改才会重新复制）
#copy_config_debug.files = $$CONFIG_INI_SRC
#copy_config_debug.path = $$CONFIG_INI_DEST_DEBUG
#copy_config_release.files = $$CONFIG_INI_SRC
#copy_config_release.path = $$CONFIG_INI_DEST_RELEASE
## 把复制操作添加到构建步骤
#COPIES += copy_config_debug
#COPIES += copy_config_release

## 将xml文件夹自动copy到构建文件夹中
#XML_FILES_SRC = $$PWD/XmlFiles/*

#XML_FILES_DEST_DEBUG = $$OUT_PWD/debug/XmlFiles
#XML_FILES_DEST_RELEASE = $$OUT_PWD/release/XmlFiles

#copy_xml_debug.files = $$XML_FILES_SRC
#copy_xml_debug.path = $$XML_FILES_DEST_DEBUG
#copy_xml_release.files = $$XML_FILES_SRC
#copy_xml_release.path = $$XML_FILES_DEST_RELEASE

#COPIES += copy_xml_debug
#COPIES += copy_xml_release


# 2026.3.25 更新，下面两行代码的目的是直接在build-newGroundTestLvdsCamera_P1-Desktop_Qt_5_13_0_MinGW_32_bit-Debug/Release
# 文件夹中构建文件而不是在文件夹中再次创建Debug文件夹和Release文件夹
CONFIG += debug release
CONFIG -= debug_and_release

# 2026.3.25 更新，下面些代码的目的是优化上述代码，上述代码要分别对debug构建时以及release构建时进行配置，下面的代码进行简化，因为PWD本身就会对debug和release进行区分
CONFIG_INI_SRC = $$PWD/ConfigFiles/*
CONFIG_INI_DEST = $$OUT_PWD/ConfigFiles

copy_config.files = $$CONFIG_INI_SRC
copy_config.path = $$CONFIG_INI_DEST

COPIES += copy_config


XML_FILES_SRC = $$PWD/XmlFiles/*
XML_FILES_DEST = $$OUT_PWD/XmlFiles

copy_xml.files = $$XML_FILES_SRC
copy_xml.path = $$XML_FILES_DEST

COPIES += copy_xml

## 发布目录
#DEPLOY_PATH = $$PWD/../deploy # 代码所在文件夹
##DEPLOY_PATH = $$OUT_PWD/deploy # 构建目录

## ConfigFiles
#deploy_config.files = $$PWD/ConfigFiles/*  # 将要被拷贝的文件们
#deploy_config.path = $$DEPLOY_PATH/ConfigFiles # 文件们将要被拷贝到的地址
#COPIES += deploy_config

## XmlFiles
#deploy_xml.files = $$PWD/XmlFiles/*
#deploy_xml.path = $$DEPLOY_PATH/XmlFiles
#COPIES += deploy_xml

## LVDS_DLL.dll
#deploy_lvds_dll.files = $$PWD/Driver/DLL/Win32/LVDS_DLL.dll
#deploy_lvds_dll.path = $$DEPLOY_PATH
#COPIES += deploy_lvds_dll

## ControlCAN.dll
#deploy_control_can_dll.files = $$PWD/Driver/CANalyst/win32(32bit)/ControlCAN.dll
#deploy_control_can_dll.path = $$DEPLOY_PATH
#COPIES += deploy_control_can_dll

# 2. 处理可执行程序 (.exe) 的拷贝
# TARGET 变量代表程序名，加上 .exe 后缀
# 如果你在 pro 中设置了 DESTDIR，请将 $$OUT_PWD 替换为 $$DESTDIR
#deploy_exe.files = $$OUT_PWD/$${TARGET}.exe
#deploy_exe.path = $$DEPLOY_PATH
#COPIES += deploy_exe
# 上面三行代码虽然可以实现自动拷贝xml文件夹和config文件夹，但是会报一个“循环依赖”的错误

