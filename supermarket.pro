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
    src/LogIn/LoginDialog.cpp \
    src/Product/AddProductDialog.cpp \
    src/Product/StockDialog.cpp \
    src/main.cpp \
    src/LogIn/dbmanager.cpp \
    src/Product/Product.cpp \
    src/LogIn/registerdialog.cpp \
    src/LogIn/memberregisterdialog.cpp

HEADERS += \
    include/Check/Check_Mainwindow.h \
    include/LogIn/LoginDialog.h \
    include/LogIn/dbmanager.h \
    include/Product/AddProductDialog.h \
    include/Product/StockDialog.h \
    include/Product/Product.h \
    include/LogIn/registerdialog.h \
    include/LogIn/memberregisterdialog.h

FORMS += \
    src/Check/Check_Mainwindow.ui \
    src/LogIn/LogInDialog.ui \
    src/Product/Product.ui \
    src/LogIn/registerdialog.ui \
    src/LogIn/memberregisterdialog.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    src/LogIn/LogIn.pro.user \
    src/Product/Product.pro.user \
    supermarket.pro.user

RESOURCES += \
    icon.qrc
