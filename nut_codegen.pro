QT -= gui
QT += sql

OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
RCC_DIR=build

DESTDIR = bin

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        fieldqttypelookup.cpp \
        inflector.cpp \
        libkode/class.cpp \
        libkode/code.cpp \
        libkode/enum.cpp \
        libkode/file.cpp \
        libkode/function.cpp \
        libkode/license.cpp \
        libkode/membervariable.cpp \
        libkode/namer.cpp \
        libkode/printer.cpp \
        libkode/style.cpp \
        libkode/typedef.cpp \
        libkode/variable.cpp \
        main.cpp \
        nutcodegen.cpp \
        table.cpp \
        tablefield.cpp


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
  common/kode_export.h \
  fieldqttypelookup.h \
  inflector.h \
  libkode/class.h \
  libkode/code.h \
  libkode/enum.h \
  libkode/file.h \
  libkode/function.h \
  libkode/license.h \
  libkode/membervariable.h \
  libkode/namer.h \
  libkode/printer.h \
  libkode/style.h \
  libkode/typedef.h \
  libkode/variable.h \
  nutcodegen.h \
  table.h \
  tablefield.h \
  tablerelation.h
