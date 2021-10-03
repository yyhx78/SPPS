/*
* Copyright 2007 Sandia Corporation.
* Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
* license for use of this work by or on behalf of the
* U.S. Government. Redistribution and use in source and binary forms, with
* or without modification, are permitted provided that this Notice and any
* statement of authorship are reproduced on all copies.
*/
// QT includes
#pragma warning( push, 0 )
#include <QApplication>
#pragma warning(pop)
//#include <QCleanlooksStyle>
#include "MainWnd.h"
#include "dlgLogIn.h"

extern int qInitResources_icons();

int main( int argc, char** argv )
{
    // QT Stuff
    QApplication app( argc, argv );

    app.setApplicationName("Simulation Post Processing");
    app.setOrganizationName("YYHX78");
    app.setOrganizationDomain("www.yyhx78.com");

    qInitResources_icons();

    dlgLogIn dlg;
//    if(dlg.connected() || dlg.exec() == QDialog::Accepted)
    {
        MainWnd mySimpleView;
        mySimpleView.show();

        return app.exec();
    }
    
    return 0;
}
