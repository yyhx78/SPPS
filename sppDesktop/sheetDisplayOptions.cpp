#include "sheetDisplayOptions.h"

#pragma warning( push, 0 )
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qmenu.h>

#include <QMouseEvent>
#include <QTabBar>
#include <QDebug>
#pragma warning(pop)

#include "TreeModel.h"
#include "MainWnd.h"

#include "pageMeshDisplay.h"
#include "pageResultDisplay.h"
#include "pageQueryResults.h"
#include "pageCuttingPlane.h"

class MainWnd;
extern MainWnd* mainWnd();

// Constructor
sheetDisplayOptions::sheetDisplayOptions(QWidget* parent) : QTabWidget(parent)
{
	m_pageResultDisplay = new pageResultDisplay((QWidget*)mainWnd());
	addTab(new pageMeshDisplay((QWidget*)mainWnd()),"Mesh");
	addTab(m_pageResultDisplay, "Result");
    addTab(new pageCuttingPlane((QWidget*)mainWnd()),"Cutting Plane");
    addTab(new pageQueryResults((QWidget*)mainWnd()),"Query Results");

    // get the QTabBar
    QList<QTabBar *> tabList = findChildren<QTabBar *>();
    if(!tabList.isEmpty()) {
        QTabBar *tabBar = tabList.at(0);
        if (tabBar)
        {
            connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(slotOnPageActivated(int)));
        }
    }
}

sheetDisplayOptions::~sheetDisplayOptions()
{
    
}

void sheetDisplayOptions::slotOnPageActivated(int index)
{
    qDebug() << "current tab is " << index;
}

void sheetDisplayOptions::onResultSelectionChanged()
{
	if (m_pageResultDisplay)
		m_pageResultDisplay->onResultSelectionChanged();
}

void sheetDisplayOptions::slotTreeViewItemCheckStatusChanged(const QModelIndex & index)
{

}
