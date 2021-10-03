#include "pageCuttingPlane.h"
#pragma warning( push, 0 )
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qmenu.h>
#include <qlistwidget.h>

#include <QMouseEvent>
#include <QDebug>
#include <QtWidgets>
#pragma warning(pop)

#include <sstream>

#include "editQueryResults.h"
#include "QtVtkView.h"
#include "MainWnd.h"

#include "dataModel/Doc.h"

extern MainWnd* mainWnd();

static pageCuttingPlane *localeP = nullptr;
void cbkCuttingPlaneModified::Execute(vtkObject *caller, unsigned long uEvent, void*d)
{
    if (uEvent == vtkCommand::ModifiedEvent)
    {
        if (localeP)
        {
            localeP->onCuttingPlaneModified(vtkPlane::SafeDownCast(caller));
        }
    }
}

// Constructor
pageCuttingPlane::pageCuttingPlane(QWidget* parent, Qt::WindowFlags f)
            : QWidget(parent, f)
{
    localeP = this;
                                   
    //left side
    auto leftPane = new QWidget(this);
    {
        leftPane->setMaximumWidth(200);
        
        QPushButton *btnNew = new QPushButton("&New");
        connect(btnNew, SIGNAL(pressed()), this, SLOT(slotOnNewCuttingPlane()));
        QPushButton *btnDelete = new QPushButton("&Delete");
        connect(btnDelete, SIGNAL(pressed()), this, SLOT(slotOnDeleteCuttingPlane()));

        listWidget = new QListWidget(leftPane);

        QObject::connect(listWidget, SIGNAL(itemChanged(QListWidgetItem*)),
                         this, SLOT(slotItemChecked(QListWidgetItem*)));
        QObject::connect(listWidget, SIGNAL(itemSelectionChanged()),
                         this, SLOT(slotItemSelectionChanged()));

        auto btnLayout = new QHBoxLayout(leftPane);
        btnLayout->addWidget(btnNew);
        btnLayout->addWidget(btnDelete);
        QGroupBox *btnBox = new QGroupBox(leftPane);
        btnBox->setLayout(btnLayout);

        auto vBox = new QVBoxLayout;
        vBox->addWidget(listWidget);
        vBox->addWidget(btnBox);

        leftPane->setLayout(vBox);
    }


    //right side
    auto rightPane = new QWidget(this);
    {
        auto labelOrigin = new QLabel(tr("Origin"));
        lineEditOrigin = new QLineEdit;
        lineEditOrigin->setMinimumWidth(300);
        connect(lineEditOrigin, SIGNAL(editingFinished()), this, SLOT(slotOriginEditingFinished()));

        auto labelNormal = new QLabel(tr("Normal"));
        lineEditNormal = new QLineEdit;
        connect(lineEditNormal, SIGNAL(editingFinished()), this, SLOT(slotNormalEditingFinished()));

        auto vBox = new QVBoxLayout;
        vBox->addWidget(labelOrigin);
        vBox->addWidget(lineEditOrigin);
        vBox->addWidget(labelNormal);
        vBox->addWidget(lineEditNormal);

        QGroupBox *groupBox = new QGroupBox(rightPane);
        groupBox->setLayout(vBox);
    }
    
//two sides together
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetNoConstraint);
    mainLayout->addWidget(leftPane, 0, 0);
    mainLayout->addWidget(rightPane, 0, 1);

    this->setLayout(mainLayout);
}

pageCuttingPlane::~pageCuttingPlane()
{
    
}

void pageCuttingPlane::slotOriginEditingFinished()
{
    setPlaneOriginNormal(lineEditOrigin);
}

void pageCuttingPlane::slotNormalEditingFinished()
{
    setPlaneOriginNormal(lineEditNormal);
}

void pageCuttingPlane::setPlaneOriginNormal(QLineEdit *aLineEdit)
{
    if (!aLineEdit)
        return;

    QString lTxt;
    if (aLineEdit == lineEditNormal)
        lTxt = lineEditNormal->text();
    else if (aLineEdit == lineEditOrigin)
        lTxt = lineEditOrigin->text();
    else
        return;

    auto lValueSs = lTxt.split(',');
    if (lValueSs.size() != 3)
        return; //suppose to have 3 numbers
    double lCmptValues[3];
    bool ok;
    double lLength = 0.0;
    for (int i = 0; i < 3; ++i)
    {
        lCmptValues[i] = lValueSs[i].toDouble(&ok);
        if (!ok)
            return; //invalid string

        lLength += (lCmptValues[i] * lCmptValues[i]);
    }

    if (aLineEdit == lineEditNormal)
    {
        if (lLength <= 0.0)
            return;//normal can not be zero length
    }

    auto lSelectedItems = listWidget->selectedItems();
    if (lSelectedItems.isEmpty())
        return;

    auto row = listWidget->row(lSelectedItems[0]);
    if (row < 0)
        return;

    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();

    if (row >= lCase.cuttingPlanes().size())
        return;

    auto& lCP = lCase.cuttingPlanes()[row];
    auto lPlane = lCP.m_plane.GetPointer();
    if (!lPlane)
        return;

    auto lOldCmptValues = aLineEdit == lineEditNormal ? lPlane->GetNormal() : lPlane->GetOrigin();
    bool bEq = true;
    for (int i = 0; i < 3; ++i)
    {
        if (fabs(lOldCmptValues[i] - lCmptValues[i]) > 0.0001)
        {
            bEq = false;
        }
    }

    if (bEq)
        return; //no change

    if (aLineEdit == lineEditNormal)
        lPlane->SetNormal(lCmptValues);
    else
        lPlane->SetOrigin(lCmptValues);

    mainWnd()->updateCuttingPlane(lPlane);
    mainWnd()->resetDisplay();
    mainWnd()->repaint();
}

void pageCuttingPlane::slotItemChecked(QListWidgetItem *item)
{
    static bool locked = false;
    
    if (locked)
        return;
    
    locked = true;
    if(item->checkState() == Qt::Checked)
        item->setBackground(QColor("#ffffb2"));
    else
        item->setBackground(QColor("#ffffff"));
    
    locked = false;
    
    auto row = listWidget->row(item);
    if (row < 0)
        return;
    
    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();
    
    if (row >= lCase.cuttingPlanes().size())
        return;
    
    auto& lCP = lCase.cuttingPlanes()[row];
    auto lPlane = lCP.m_plane.GetPointer();
    if (!lPlane)
        return;
        
    if(item->checkState() == Qt::Checked)
    {
        mainWnd()->updateCuttingPlane(lPlane);
        mainWnd()->activateCuttingPlane(lPlane, true);
        mainWnd()->activateCuttingPlaneWidget(lPlane, true);
    } else
    {
        mainWnd()->activateCuttingPlaneWidget(lPlane, false);
        mainWnd()->activateCuttingPlane(lPlane, false);
    }
    
    mainWnd()->resetDisplay();
    mainWnd()->repaint();
}

void pageCuttingPlane::slotItemSelectionChanged()
{
    auto item = listWidget->currentItem();//the selected item
    if (!item)
        return;
    
    auto row = listWidget->row(item);
    if (row < 0)
        return;
    
    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();

    auto &lCP = lCase.cuttingPlanes()[row];
    auto lPlane = lCP.m_plane.GetPointer();
    if (!lPlane)
        return;

    setLineEdits(lPlane);
}

void pageCuttingPlane::slotOnNewCuttingPlane()
{
    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();
    
    double bs[6];
    lCase.GetBounds(bs);
    
    double lCenter[] = {
        0.5 * (bs[0] + bs[1]),
        0.5 * (bs[2] + bs[3]),
        0.5 * (bs[4] + bs[5])
    };
    double lNormal[] = {0.0, 0.0, 1.0};

    auto idx = listWidget->count();
    std::stringstream ss;
    ss << "Plane_" << idx;
    listWidget->addItem(ss.str().c_str());
    auto item = listWidget->item(idx);//the new item
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);

    mainWnd()->createCuttingPlane(lCenter, lNormal);
}

void pageCuttingPlane::slotOnDeleteCuttingPlane()
{
    auto item = listWidget->currentItem();//the selected item
    if (!item)
        return;
    
    auto row = listWidget->row(item);
    if (row < 0)
        return;
    
    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();

    auto &lCP = lCase.cuttingPlanes()[row];
    auto lPlane = lCP.m_plane.GetPointer();
    if (!lPlane)
        return;
    
    mainWnd()->activateCuttingPlaneWidget(lPlane, false);
    mainWnd()->activateCuttingPlane(lPlane, false);

    lCase.cuttingPlanes().erase(lCase.cuttingPlanes().begin() + row);
    
    QListWidgetItem *it = listWidget->takeItem(row);
    delete it;
}

void pageCuttingPlane::onCuttingPlaneModified(vtkPlane *aPlane)
{
    if (!aPlane)
        return;
    
    setLineEdits(aPlane);

    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();

    auto nItems = listWidget->count();
    if (nItems < lCase.cuttingPlanes().size())
    {// a new plane is just created, add it to the list
        std::stringstream ss;
        ss << "Plane_" << nItems;
        listWidget->addItem(ss.str().c_str());
        auto item = listWidget->item(nItems);//the new item
        item->setCheckState(Qt::Checked);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    }
}

void pageCuttingPlane::setLineEdits(vtkPlane* aPlane)
{
    {
        double *lOrigin = aPlane->GetOrigin();
        std::stringstream ss;
        ss << lOrigin[0] << ", " << lOrigin[1] << ", " << lOrigin[2];
        auto s = ss.str();
        lineEditOrigin->setText(s.c_str());
    }

    {
        double *lNormal = aPlane->GetNormal();
        std::stringstream ss;
        ss << lNormal[0] << ", " << lNormal[1] << ", " << lNormal[2];
        auto s = ss.str();
        lineEditNormal->setText(s.c_str());
    }
}
