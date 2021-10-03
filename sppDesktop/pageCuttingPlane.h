
#ifndef pageCuttingPlane_H
#define pageCuttingPlane_H

#pragma warning( push, 0 )
#include <QWidget.h>
#include "vtkCommand.h"
#pragma warning(pop)

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class vtkPlane;

class cbkCuttingPlaneModified : public vtkCommand
{
public:
    static cbkCuttingPlaneModified* New()
    {
        return new cbkCuttingPlaneModified();
    }
    
    virtual void Execute(vtkObject *caller, unsigned long uEvent, void*d) override;

protected:
    cbkCuttingPlaneModified()
    {
    }
};

class pageCuttingPlane : public QWidget
{
    Q_OBJECT

public:

  // Constructor/Destructor
    pageCuttingPlane(QWidget* parent = NULL, Qt::WindowFlags f = 0);
    ~pageCuttingPlane();

    void onCuttingPlaneModified(vtkPlane*);
public slots:
    void slotOnNewCuttingPlane();
    void slotOnDeleteCuttingPlane();
    void slotItemSelectionChanged();
    void slotItemChecked(QListWidgetItem* item);
    void slotOriginEditingFinished();
    void slotNormalEditingFinished();

protected:
    void setLineEdits(vtkPlane*);
    
    QListWidget* listWidget;
 
    QLineEdit* lineEditOrigin, *lineEditNormal;

private:
    void setPlaneOriginNormal(QLineEdit* aLineEdit);
};

#endif // SimpleView_H
