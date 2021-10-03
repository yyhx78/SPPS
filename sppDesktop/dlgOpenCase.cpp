#pragma warning( push, 0 )
#include <QtGui>
#include <QtWidgets>
#include <QLineEdit.h>
#pragma warning(pop)

#include "dlgOpenCase.h"
#include "dataModel/Connect.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

class MainWnd;
extern MainWnd* mainWnd();

dlgOpenCase::dlgOpenCase(QWidget* parent)
    : QDialog(parent)
{
    m_selectedCaseFile = "";//nothing is selected

    auto& cn = SPP::Connect::cn();

    folderLineEdit = new QLineEdit;
    folderLineEdit->setText(cn.mountedFolder());
    
    auto folderBtn = createButton("Folder", SLOT(slotOnFolderBtnPressed()));
    
    btnOk = createButton(tr("&Ok"), SLOT(onOk()));
    btnCancel = createButton(tr("&Cancel"), SLOT(onCancel()));

    nameLabel = new QLabel(tr("Case List:"));
   
    listWidget = new QListWidget(this);

    auto& lSrvDoc = SPP::Doc::getDoc();
    
    auto& lCaseList = lSrvDoc.GetCaseAddressList();
    for (auto c : lCaseList)
    {
        std::string s = c.GetName();
        new QListWidgetItem(tr(s.c_str()), listWidget);
    }

    listWidget->setMinimumWidth(listWidget->sizeHintForColumn(0));

    QObject::connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotItemDoubleClicked(QListWidgetItem*)));

    auto btnBox = new QDialogButtonBox;
    btnBox->addButton(btnOk, QDialogButtonBox::AcceptRole);
    btnBox->addButton(btnCancel, QDialogButtonBox::RejectRole);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
    mainLayout->addWidget(folderLineEdit, 0, 0);
    mainLayout->addWidget(folderBtn, 0, 1);
    mainLayout->addWidget(nameLabel, 1, 0);
    mainLayout->addWidget(listWidget, 2, 0);
    mainLayout->addWidget(btnBox, 3, 0);

    setLayout(mainLayout);

    setWindowTitle(tr("Open Case"));
}

void dlgOpenCase::slotItemDoubleClicked(QListWidgetItem*)
{
    onOk();
}

//search in the folder for OF cases, return the found cases in aCaseFolders
void searchResultsFolder(const char* aFolder, std::vector<SPP::CaseAddress> &aCaseFolders)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    //if a service exists, kill and remove it to build a new one with the new data folder
    auto& cn = SPP::Connect::cn();
    std::string lId, lVersion;
    std::string lIP("http://localhost");
    int iPort = -1;
    bool lState = false;
    if (cn.findDockerContainer(lId, lVersion, lIP, iPort, lState))
    {
        if (lState || iPort < 0 || lVersion.empty())
        {//it is running, kill it
            cn.killService(lId);
        }
        
        cn.removeService(lId);
    }
    
    {//open the folder
        iPort = 9597;
        std::string pt("9597");
        std::string dataFolder = aFolder;
        cn.createService(pt, dataFolder);

        std::string url = lIP + ":" + std::to_string(iPort);
        cn.urlRoot(url.c_str());
        cn.mountedFolder(dataFolder.c_str());
        
        auto& lSrvDoc = SPP::Doc::getDoc();
        
        int N = 10; //the data transfer may take time, need to wait. About N*2 seconds, should be enough
        while (N > 0)
        {
            auto& lCaseList = lSrvDoc.GetCaseAddressList(true);
            if (!lCaseList.empty())
            {
                aCaseFolders = lCaseList;
                break;
            }
            
#ifdef _WIN32
            Sleep(2000);
#else
            sleep(2);//wait for 2s
#endif
            --N;
        }
    }
    
    QApplication::restoreOverrideCursor();
}

void dlgOpenCase::slotOnFolderBtnPressed()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    
    folderLineEdit->setText(dir);
    
    if (dir.isEmpty())
        return;
 
    std::vector<SPP::CaseAddress> lCaseFolders;
    searchResultsFolder(dir.toStdString().c_str(), lCaseFolders);
    
    if (listWidget)
    {
        listWidget->clear();
        for (auto &s : lCaseFolders)
        {
            listWidget->addItem(s.GetName());
        }
    }

}

void dlgOpenCase::onOk()
{
    auto iSelected = listWidget->currentRow();
    if (iSelected < 0)
        return;
    
    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCaseList = lSrvDoc.GetCaseAddressList();
    
    auto address = lCaseList[iSelected];
    m_selectedCaseFile = address.GetFile();
 
    accept();
}

void dlgOpenCase::onCancel()
{
    m_selectedCaseFile = "";//cancel the selection

    accept();
}

QPushButton *dlgOpenCase::createButton(const QString &text, const char *member)
{
    QPushButton *button = new QPushButton(text);
    connect(button, SIGNAL(clicked()), this, member);
    return button;
}
