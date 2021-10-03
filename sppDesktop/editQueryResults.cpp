#include "editQueryResults.h"

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

// Constructor
editQueryResults::editQueryResults(QWidget* parent):QPlainTextEdit(parent)
{

};

editQueryResults::~editQueryResults()
{
    
}

void editQueryResults::slot_OnQuery(QString &queryRlt) {
    this->appendPlainText(queryRlt);
}
