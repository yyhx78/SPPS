
#ifndef pageQueryResults_H
#define pageQueryResults_H

#pragma warning( push, 0 )
#include <QWidget.h>
#pragma warning(pop)

class QGroupBox;

class pageQueryResults : public QWidget
{
    Q_OBJECT

public:

  // Constructor/Destructor
    pageQueryResults(QWidget* parent = NULL, Qt::WindowFlags f = 0);
    ~pageQueryResults();

protected:
};

#endif // SimpleView_H
