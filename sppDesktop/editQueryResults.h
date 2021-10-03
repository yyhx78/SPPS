
#ifndef editQueryResults_H
#define editQueryResults_H

#pragma warning( push, 0 )
#include <QPlainTextEdit.h>
#pragma warning(pop)

class editQueryResults : public QPlainTextEdit
{
    Q_OBJECT

public:

  // Constructor/Destructor
    editQueryResults(QWidget* parent = NULL);
    ~editQueryResults();

public slots:
    void slot_OnQuery(QString &);
protected:
};

#endif // SimpleView_H
