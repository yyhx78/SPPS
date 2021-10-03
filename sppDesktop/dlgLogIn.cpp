#pragma warning( push, 0 )
#include <QtGui>
#include <QtWidgets>
#pragma warning(pop)

#include "dlgLogIn.h"
#include "dataModel/Connect.h"

class MainWnd;
extern MainWnd* mainWnd();

dlgLogIn::dlgLogIn(QWidget* parent)
    : QDialog(parent)
{
    m_connected = false;
    
    auto& cn = SPP::Connect::cn();
    std::string lVersion, lId;
    std::string lIP("http://localhost");
    int iPort = -1;
    bool lState = false;
    if (cn.findDockerContainer(lId, lVersion, lIP, iPort, lState))
    {
        if (iPort <= 0 || lVersion.empty())
        {
            if (lState)
                cn.killService(lId);
            //invalid port number, delete the container to create a new one
            cn.removeService(lId);
        } else
        {
            auto url = lIP + ":" + std::to_string(iPort);
            cn.urlRoot(url.c_str());
            if (!lState)
            {//it is not running
                if (cn.startService())
                {
                    m_connected = true;
                }
            } else
            {
                m_connected = true;
            }
        }
        if (m_connected)
        {
            std::string lFolder;
            if (cn.findContainerMountSource(lId, lFolder))
            {
                cn.mountedFolder(lFolder.c_str());
            }
        }
    }

    //create and connect with a container
    editDataFolder = new QLineEdit;
    editDataFolder->setText(cn.mountedFolder()); //default value
    auto folderBtn = createButton("...", SLOT(slotOnFolderBtnPressed()));
    
    auto folderBox = new QHBoxLayout;
    folderBox->addWidget(editDataFolder);
    folderBox->addWidget(folderBtn);


    editPort = new QLineEdit;
    editPort->setText("7777"); //default value

    btnOk = createButton(tr("&Ok"), SLOT(onOk()));
    btnCancel = createButton(tr("&Cancel"), SLOT(onCancel()));

    auto btnBox = new QDialogButtonBox;
    btnBox->addButton(btnOk, QDialogButtonBox::AcceptRole);
    btnBox->addButton(btnCancel, QDialogButtonBox::RejectRole);

    auto *mainLayout = new QFormLayout;
    mainLayout->addRow("Data Folder", folderBox);//lFolderWidget);
    mainLayout->addRow("Port", editPort);
    mainLayout->addRow("", btnBox);

    setLayout(mainLayout);

    setWindowTitle(tr("Connection"));
}

bool dlgLogIn::connected()
{
    return m_connected;
}

void dlgLogIn::slotOnFolderBtnPressed()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    
    if (!dir.isEmpty())
        editDataFolder->setText(dir);
}

void dlgLogIn::onOk()
{
    auto folder = editDataFolder->text();
    if (folder.isEmpty())
        return;
    
    auto port = editPort->text();
    if (port.isEmpty())
        return;
    
    auto& cn = SPP::Connect::cn();
    
    auto sPort = port.toStdString();
    auto sFolder = folder.toStdString();
    if (!cn.createService(sPort, sFolder))
        return;
    
    //check whether the creation is successful
    std::string lId, lVersion;
    std::string lIP("http://localhost");
    int iPort = -1;
    bool lState = false;
    if (!cn.findDockerContainer(lId, lVersion, lIP, iPort, lState))
        return;
    
    if (iPort < 0 || lVersion.empty())
        return;
    
    if (!cn.startService())
        return;
    
    cn.mountedFolder(sFolder.c_str());
    
    std::string url = "http://localhost:" + sPort;
    cn.urlRoot(url.c_str());

    accept();
}

void dlgLogIn::onCancel()
{
    reject();
}

QPushButton *dlgLogIn::createButton(const QString &text, const char *member)
{
    QPushButton *button = new QPushButton(text);
    connect(button, SIGNAL(clicked()), this, member);
    return button;
}
