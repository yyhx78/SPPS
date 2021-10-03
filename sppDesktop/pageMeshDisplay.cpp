#include "pageMeshDisplay.h"
#pragma warning( push, 0 )
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qmenu.h>
#include <qlineedit.h>
#include <qlabel.h>

#include <QMouseEvent>
#include <QDebug>

#include "vtkProperty.h"
#include "vtkPointData.h"
#include "vtkPolyDataMapper.h"
#pragma warning(pop)

#include "dataModel/Doc.h"

#include "MainWnd.h"
extern MainWnd* mainWnd();

// Constructor
pageMeshDisplay::pageMeshDisplay(QWidget* parent, Qt::WindowFlags f)
            : QWidget(parent, f)
{
    auto lCtrlHolder = new QWidget(this);

    QCheckBox* checkBoxG = new QCheckBox(tr("&Gouraod Shading"), lCtrlHolder);
    checkBoxG->setCheckState(Qt::Checked); //turned on by default

    connect(checkBoxG, SIGNAL(stateChanged(int)), this, SLOT(slotOnCheck_GouraodShading(int)));

    QGroupBox *groupBox = new QGroupBox(tr("Edge Display"));

    QRadioButton *radio1 = new QRadioButton(tr("&Feature Edges"));
    QRadioButton *radio2 = new QRadioButton(tr("&Mesh Edges"));
    QRadioButton *radio3 = new QRadioButton(tr("&Off"));
    radio1->setChecked(true);

    connect(radio1, SIGNAL(clicked()), this, SLOT(slotOnR1Clicked()));
    connect(radio2, SIGNAL(clicked()), this, SLOT(slotOnR2Clicked()));
    connect(radio3, SIGNAL(clicked()), this, SLOT(slotOnR3Clicked()));

    QHBoxLayout *vBox = new QHBoxLayout;
    vBox->addWidget(radio1);
    vBox->addWidget(radio2);
    vBox->addWidget(radio3);
    vBox->addStretch(1);
    groupBox->setLayout(vBox);
    groupBox->setMaximumHeight(60);
    
    QGridLayout *layout = new QGridLayout(this);
    
    layout->addWidget(groupBox, 0, 0);
    layout->addWidget(checkBoxG, 1, 0);

    lCtrlHolder->setLayout(layout);
}

pageMeshDisplay::~pageMeshDisplay()
{
    
}

void pageMeshDisplay::slotOnR1Clicked()
{
    slotOnCheck_MeshEdges(!Qt::Checked);
    slotOnCheck_FeatureEdges(Qt::Checked);
}

void pageMeshDisplay::slotOnR2Clicked()
{
    slotOnCheck_MeshEdges(Qt::Checked);
    slotOnCheck_FeatureEdges(!Qt::Checked);
}

void pageMeshDisplay::slotOnR3Clicked()
{
    slotOnCheck_MeshEdges(!Qt::Checked);
    slotOnCheck_FeatureEdges(!Qt::Checked);
}

void pageMeshDisplay::slotOnCheck_MeshEdges(int aChecked)
{
    bool bChecked = (aChecked & Qt::Checked);

    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();
    auto& lParts = lCase.GetPartList();
    auto nParts = lParts.size();
    for (auto i = 0; i < nParts; i++)
    {
        auto& part = lParts[i];
        if (bChecked && !part.m_meshEdgesA)
        {//load the edges from server
            lCase.LoadPartData(i, 1); //load mesh edges
            if (part.m_meshEdges)
            {//mesh edges
                auto lMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
                lMapper->SetInputData(part.m_meshEdges);

                auto lActor = vtkSmartPointer<vtkActor>::New();
                lActor->SetMapper(lMapper);
                lActor->GetProperty()->SetColor(0, 0, 0);
                part.m_meshEdgesA = lActor;

                mainWnd()->renderer()->AddActor(lActor);
            }
        }
        if (part.m_meshEdgesA)
        {
            part.m_meshEdgesA->SetVisibility(bChecked ? 1 : 0);
        }
    }

    mainWnd()->repaint();
}

void pageMeshDisplay::slotOnCheck_FeatureEdges(int aChecked)
{
    bool bChecked = (aChecked & Qt::Checked);

    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();
    auto& lParts = lCase.GetPartList();
    for (auto& part : lParts)
    {
        if (part.m_featureEdgesA)
        {
            part.m_featureEdgesA->SetVisibility(bChecked ? 1 : 0);
        }
    }

    mainWnd()->repaint();
}

void pageMeshDisplay::slotOnCheck_GouraodShading(int aChecked)
{
    bool bChecked = (aChecked & Qt::Checked);

    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();
    auto& lParts = lCase.GetPartList();
    for (auto& part : lParts)
    {
        if (part.m_surface)
            part.m_surface->GetPointData()->SetNormals(bChecked ? part.m_normals : nullptr);
    }

    mainWnd()->repaint();
}
