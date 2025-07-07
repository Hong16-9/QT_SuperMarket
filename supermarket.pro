QT       += core gui
QT += core gui sql widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/include

SOURCES += \
    src/Check/Check_Mainwindow.cpp \
    src/main.cpp \
    src/LogIn/LogIn_Dialog.cpp \
    src/LogIn/dbmanager.cpp \
    src/Product/Product.cpp \

HEADERS += \
    include/Check/Check_Mainwindow.h \
    include/LogIn/LogIn_Dialog.h \
    include/LogIn/dbmanager.h \
    include/Product/Product.h \

FORMS += \
    src/Check/Check_Mainwindow.ui \
    src/LogIn/LogIn_Dialog.ui \
    src/Product/Product.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    src/LogIn/LogIn.pro.user \
    src/Product/Product.pro.user \
    supermarket.pro.user
