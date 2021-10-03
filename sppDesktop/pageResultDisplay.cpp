#include "pageResultDisplay.h"
#pragma warning( push, 0 )
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qmenu.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qapplication.h>

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
pageResultDisplay::pageResultDisplay(QWidget* parent, Qt::WindowFlags f)
            : QWidget(parent, f), m_isoCountChanged(false), m_pAnimationTimer(nullptr)
{
    m_selectedIndpValue = -1;

    auto lCtrlHolder = new QWidget(this);

    auto timeLabel = new QLabel(tr("Time [s]"));

    m_timeComboBox = new QComboBox;
    m_timeComboBox->setEditable(true);

    m_timeSlider = new QSlider(Qt::Horizontal);
    m_timeSlider->setFocusPolicy(Qt::StrongFocus);
    m_timeSlider->setTickPosition(QSlider::TicksBothSides);
    m_timeSlider->setMinimum(0);
    m_timeSlider->setMaximum(3);
    m_timeSlider->setTickInterval(1);
    m_timeSlider->setSingleStep(1);

    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();
    auto& lRlts = lCase.GetResultList();
    if (!lRlts.empty() && lCase.m_activeRlt >= 0)
    {
        auto& lActiveResult = lRlts[lCase.m_activeRlt];

        auto n = lActiveResult.m_indpValues.size();
        for (int i = 0; i < n; ++i)
        {
            m_timeComboBox->addItem(tr(std::to_string(lActiveResult.m_indpValues[i]).c_str()));
        }

        m_timeSlider->setTickInterval((int)(n - 1));
    }

    auto btnWidget = new QWidget;
    
    auto btnPlay = new QPushButton;
    btnPlay->setIcon(QIcon(":/Icons/pqVcrPlay24.png"));
    auto btnPause = new QPushButton;
    btnPause->setIcon(QIcon(":/Icons/pqVcrPause24.png"));
    auto btnBack = new QPushButton;
    btnBack->setIcon(QIcon(":/Icons/pqVcrBack24.png"));
    auto btnForward = new QPushButton;
    btnForward->setIcon(QIcon(":/Icons/pqVcrForward24.png"));
    
    connect(btnPlay, SIGNAL(pressed()), this, SLOT(slotOnPlay()));
    connect(btnPause, SIGNAL(pressed()), this, SLOT(slotOnPause()));
    connect(btnBack, SIGNAL(pressed()), this, SLOT(slotOnBack()));
    connect(btnForward, SIGNAL(pressed()), this, SLOT(slotOnForward()));

    auto btnLayout = new QHBoxLayout;
    btnLayout->addWidget(btnPlay);
    btnLayout->addWidget(btnPause);
    btnLayout->addWidget(btnBack);
    btnLayout->addWidget(btnForward);
    btnWidget->setLayout(btnLayout);

    auto gbox = new QGridLayout;
    gbox->addWidget(timeLabel, 0, 0);
    gbox->addWidget(m_timeComboBox, 0, 1);
    gbox->addWidget(m_timeSlider, 0, 2);
    gbox->addWidget(btnWidget, 0, 3);

    m_enableIsoCheckBox = new QCheckBox(tr("&Enable Contour"));
    gbox->addWidget(m_enableIsoCheckBox, 1, 0);

    QLabel* countLabel = new QLabel(tr("Number of contours:"));
    m_isoCountLineEdit = new QLineEdit;
    m_isoCountLineEdit->setText("5"); //default value
    gbox->addWidget(countLabel, 1, 1);
    gbox->addWidget(m_isoCountLineEdit, 1, 2);

    lCtrlHolder->setLayout(gbox);

    connect(m_timeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTimeItemChanged(int)));
    connect(m_timeSlider, SIGNAL(valueChanged(int)), this, SLOT(slotTimeValueChanged(int)));
    connect(m_enableIsoCheckBox, SIGNAL(stateChanged(int)), this, SLOT(slotOnCheck_Contour(int)));
    connect(m_isoCountLineEdit, SIGNAL(editingFinished()), this, SLOT(slotOnLineEdit_Finished()));
    connect(m_isoCountLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotOnLineEdit_TextChanged(const QString&)));
};

pageResultDisplay::~pageResultDisplay()
{
    delete m_pAnimationTimer;
}

void pageResultDisplay::slotOnPlay()
{
    if (m_pAnimationTimer == NULL)
        m_pAnimationTimer = new QBasicTimer();

    if (m_pAnimationTimer->isActive())
        m_pAnimationTimer->stop();
    else
        m_pAnimationTimer->start(100, this);
}

void pageResultDisplay::slotOnPause()
{
    if (m_pAnimationTimer != NULL)
    {
        if (m_pAnimationTimer->isActive())
            m_pAnimationTimer->stop();
        else
            m_pAnimationTimer->start(100, this);
    }
}

void pageResultDisplay::slotOnBack()
{
    if (m_timeComboBox->count() <= 0)
        return;
    
    auto idx = m_timeComboBox->currentIndex();
    --idx;
    if (idx < 0)
        idx = m_timeComboBox->count() - 1;
    
    m_timeComboBox->setCurrentIndex(idx);
}

void pageResultDisplay::slotOnForward()
{
    if (m_timeComboBox->count() <= 0)
        return;

    auto idx = m_timeComboBox->currentIndex();
    ++idx;
    if (idx >= m_timeComboBox->count())
        idx = 0;
    
    m_timeComboBox->setCurrentIndex(idx);
}

void pageResultDisplay::timerEvent(QTimerEvent *event)
{
    if (m_pAnimationTimer != NULL && event->timerId() == m_pAnimationTimer->timerId())
    {
        slotOnForward();
    } else
    {
        QWidget::timerEvent(event);
    }
}

void pageResultDisplay::slotOnLineEdit_TextChanged(const QString&)
{
    m_isoCountChanged = true;
}

void pageResultDisplay::slotOnLineEdit_Finished()
{
    if (m_isoCountChanged)
    {
        updateIsoSurfaces();
        m_isoCountChanged = false;
    }
}

void pageResultDisplay::slotOnCheck_Contour(int)
{
    updateIsoSurfaces();
}

void pageResultDisplay::updateIsoSurfaces()
{
    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();

    if (lCase.m_activeRlt < 0)
        return;

    bool bEnabled = m_enableIsoCheckBox->isChecked();
    if (bEnabled)
    {//enable iso-surface
        lCase.surfaceVisible(false);
        lCase.isoSurfaceVisible(true);

        auto txt = m_isoCountLineEdit->text();
        int nIsos = txt.toInt();
        if (nIsos <= 0)
            nIsos = 5;
        lCase.LoadIsoSurfaces(nIsos);
        //the actors need to rebuild,
        auto lRenderer = mainWnd()->renderer();
        for (auto a : lCase.m_isoAs)
        {
            lRenderer->RemoveActor(a);
        }
        lCase.m_isoAs.clear();
    }
    else
    {
        lCase.surfaceVisible(true);
        lCase.isoSurfaceVisible(false);
    }

    mainWnd()->resetDisplay();
    mainWnd()->repaint();
}

void pageResultDisplay::onResultSelectionChanged()
{
    if (!m_timeComboBox)
        return;

    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();

    //a new result is selected, set to default display
    lCase.surfaceVisible(true);
    lCase.isoSurfaceVisible(false);

    //reset the combo box
    m_timeComboBox->clear();
    if (lCase.m_activeRlt >= 0)
    {
        auto& lRlts = lCase.GetResultList();
        auto& lRlt = lRlts[lCase.m_activeRlt];
        for (auto val : lRlt.m_indpValues)
        {
            m_timeComboBox->addItem(tr(std::to_string(val).c_str()));
        }

        m_timeSlider->setMinimum(0);
        m_timeSlider->setMaximum((int)lRlt.m_indpValues.size() - 1);
        m_timeSlider->setTickInterval(1);
    }

    //reset the iso-surface checkbox and lineedit
    m_enableIsoCheckBox->setChecked(false);
    m_isoCountLineEdit->setText("5"); //default value
}

void pageResultDisplay::slotTimeValueChanged(int aIndex)
{
    if (m_selectedIndpValue == aIndex)
        return;

    m_timeComboBox->setCurrentIndex(aIndex);
}

void pageResultDisplay::slotTimeItemChanged(int aIndex)
{
    if (aIndex < 0)
        return;

    if (m_selectedIndpValue == aIndex)
        return;

    auto& lSrvDoc = SPP::Doc::getDoc();
    auto& lCase = lSrvDoc.GetCase();

    if (lCase.m_activeRlt < 0)
        return;

    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        lCase.LoadResultData(lCase.m_activeRlt, aIndex);

        updateIsoSurfaces();

        for (auto &cp : lCase.cuttingPlanes())
        {
            mainWnd()->updateCuttingPlane(cp.m_plane);
        }

        mainWnd()->repaint();
        QApplication::restoreOverrideCursor();
    }

    m_selectedIndpValue = aIndex;
    m_timeSlider->setValue(aIndex);
}
