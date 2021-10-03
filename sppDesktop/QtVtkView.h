
#ifndef QtVtkView_H
#define QtVtkView_H

#pragma warning( push, 0 )
#include "vtkSmartPointer.h"    // Required for smart pointer internal ivars.
#include "QVTKWidget.h"
#include "VtkExt/sppVtk.h"
#pragma warning(pop)

class QAbstractItemDelegate;

class sppVtk;

class vtkActorCollection;
class vtkRenderer;
class vtkActor;
class vtkActor2D;
class vtkPlane;
class vtkPlaneSource;
class vtkCellPicker;
class vtkScalarBarActor;

enum EQtVtkViewMode {
    eQtVtkViewMode_CuttingPlane,
    eQtVtkViewMode_Unknown
};

class QtVtkView : public QVTKWidget
{
    Q_OBJECT

public:

  // Constructor/Destructor
    QtVtkView(QWidget* parent = NULL, Qt::WindowFlags f = 0);
    ~QtVtkView();

    void AddActor(vtkActor *a);
    void RemoveActor(vtkActor *a);
    void AddActor2D(vtkActor2D *a);
    void RemoveActor2D(vtkActor2D *a);
    void RemoveAllActors();

	void Render();
	void AutoCenter();
	void ResetView(	double  x,double  y,double  z,
                   double vx,double vy,double vz);
    
    int  GetNumberOfScalarBars();
    void ShowScalarBar(int index, bool bShow);
    virtual void onScalarBarModified(vtkScalarBarActor*);
    
    void ShowPlaneWidget(bool bShow, double *center = NULL, double *normal = NULL, double radius = 1.0);
    
	vtkScalarBarActor* scalarBarActor(int index);

    void Query(int x, int y);
    sppVtk *m_pVtk;
        
    vtkRenderer* renderer();
    
    EQtVtkViewMode qtVtkViewMode() { return mQtVtkViewMode; };
    void qtVtkViewMode(EQtVtkViewMode e);
public slots:
    void slotDoRubberBandZoom();
    void slotDoRubberBandSelect();

signals:
    void signal_OnQuery(QString &queryRlt);
    void signal_OnSelect();
    void signal_OnCenter();
    void signal_OnScalarBarModified(vtkScalarBarActor*);
protected:
    EQtVtkViewMode mQtVtkViewMode;
    // member variable to store click position
    QPoint m_lastPoint;
    // member variable - flag of click beginning
    bool m_mouseClick;
    //mouse events
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    virtual void resizeEvent(QResizeEvent *event) override;
        
    vtkSmartPointer<vtkCellPicker>      m_pCellPicker;
    vtkSmartPointer<vtkRenderer>        m_pRenderer;
    vtkSmartPointer<vtkActorCollection> m_pActorCollection;
};

#endif // SimpleView_H
