#ifndef dlgOpenCase_H
#define dlgOpenCase_H

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

class dlgOpenCase : public QDialog
{
    Q_OBJECT

public:
    dlgOpenCase(QWidget *parent = 0);

    std::string m_selectedCaseFile;
private slots:
    void slotItemDoubleClicked(QListWidgetItem*);
    void slotOnFolderBtnPressed();
    void onOk();
    void onCancel();

private:
    QPushButton *createButton(const QString &text, const char *member);

    QListWidget* listWidget;
    QLabel *nameLabel;
    QLineEdit *nameEdit, *folderLineEdit;
    QPushButton *btnOk, *btnCancel;
};

#endif
