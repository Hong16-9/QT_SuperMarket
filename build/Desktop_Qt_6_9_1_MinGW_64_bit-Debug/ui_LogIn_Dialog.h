/********************************************************************************
** Form generated from reading UI file 'LogIn_Dialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGIN_DIALOG_H
#define UI_LOGIN_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class Ui_LogIn_Dialog
{
public:

    void setupUi(QDialog *LogIn_Dialog)
    {
        if (LogIn_Dialog->objectName().isEmpty())
            LogIn_Dialog->setObjectName("LogIn_Dialog");
        LogIn_Dialog->resize(800, 600);

        retranslateUi(LogIn_Dialog);

        QMetaObject::connectSlotsByName(LogIn_Dialog);
    } // setupUi

    void retranslateUi(QDialog *LogIn_Dialog)
    {
        LogIn_Dialog->setWindowTitle(QCoreApplication::translate("LogIn_Dialog", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LogIn_Dialog: public Ui_LogIn_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGIN_DIALOG_H
