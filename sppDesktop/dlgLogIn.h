#ifndef dlgLogIn_H
#define dlgLogIn_H

#include "dataModel/Doc.h"

#pragma warning( push, 0 )
#include <QDialog>
#include <QDir>
#pragma warning(pop)

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
class QListWidget;
class QListWidgetItem;
QT_END_NAMESPACE

class dlgLogIn : public QDialog
{
    Q_OBJECT

public:
    dlgLogIn(QWidget *parent = 0);

    bool connected();
    
private slots:
    void onOk();
    void onCancel();
    void slotOnFolderBtnPressed();
    
private:
    bool m_connected;
    
    QPushButton *createButton(const QString &text, const char *member);

    QLineEdit *editDataFolder, *editPort;
    
    QPushButton *btnOk, *btnCancel;
};

#endif
