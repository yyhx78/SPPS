
#ifndef sheetDisplayOptions_H
#define sheetDisplayOptions_H

#pragma warning( push, 0 )
#include <QTabWidget.h>
#include <QModelIndex>
#pragma warning(pop)

class pageResultDisplay;

class sheetDisplayOptions : public QTabWidget
{
    Q_OBJECT

public:

  // Constructor/Destructor
    sheetDisplayOptions(QWidget* parent = NULL);
    ~sheetDisplayOptions();

    void onResultSelectionChanged();
public slots:

	virtual void slotTreeViewItemCheckStatusChanged(const QModelIndex &);

    void slotOnPageActivated(int index);
    
protected:
    pageResultDisplay* m_pageResultDisplay;
};

#endif // SimpleView_H
