#include "pageQueryResults.h"
#pragma warning( push, 0 )

#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qmenu.h>

#include <QMouseEvent>
#include <QDebug>
#pragma warning(pop)

#include "editQueryResults.h"
#include "QtVtkView.h"
#include "MainWnd.h"

extern MainWnd* mainWnd();

// Constructor
pageQueryResults::pageQueryResults(QWidget* parent, Qt::WindowFlags f)
            : QWidget(parent, f)
{
     QGridLayout *grid = new QGridLayout;
//     grid->addWidget(createFirstExclusiveGroup(), 0, 0);

     //query view
     auto qView = new editQueryResults(this);
     grid->addWidget(qView, 0, 0);
     connect(mainWnd()->qvtkWidget(), SIGNAL(signal_OnQuery(QString&)), qView, SLOT(slot_OnQuery(QString&)));

     setLayout(grid);

     setWindowTitle(tr("Group Boxes"));
};

pageQueryResults::~pageQueryResults()
{
    
}
