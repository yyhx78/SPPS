#include "QtVtkView.h"
#include "VtkExt/sppVtk.h"

#pragma warning( push, 0 )
#include <QMouseEvent>
#include <QDebug>

#include "vtkMapper.h"
#include "vtkActorCollection.h"
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include "vtkPointData.h"
#include "vtkPolyDataMapper.h"
#include "vtkCell.h"
#include "vtkIdList.h"
#include "vtkCutter.h"
#include "vtkExtractEdges.h"
#include "vtkProperty.h"
#include "vtkScalarsToColors.h"
#include "vtkScalarBarActor.h"
#pragma warning(pop)

#include "VtkExt/sppScalarBarWidget.h"
#include "VtkExt/sppScalarBarActorAdapter.h"

// Constructor
QtVtkView::QtVtkView(QWidget* parent, Qt::WindowFlags f)
            :QVTKWidget(parent, f)
{
    m_mouseClick = false;
    mQtVtkViewMode = eQtVtkViewMode_Unknown;
    
    m_pActorCollection = vtkActorCollection::New();
    
    // VTK Renderer
    m_pRenderer = vtkSmartPointer<vtkRenderer>::New();
    
    // VTK/Qt wedded
    GetRenderWindow()->AddRenderer(m_pRenderer);
    
    vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();
    
    m_pVtk = new sppVtk(GetRenderWindow(), m_pRenderer, NULL);
};

QtVtkView::~QtVtkView()
{
    m_pActorCollection->Delete();
        
    if (m_pVtk)
        delete m_pVtk;
    
}

void QtVtkView::AddActor(vtkActor *a) {
    if (m_pRenderer == NULL)
        return;
    
    this->m_pVtk->AddActor(a);
    //    m_pRenderer->AddActor(a);
    m_pActorCollection->AddItem(a);
}

void QtVtkView::RemoveActor(vtkActor *a) {
    if (m_pRenderer == NULL)
        return;
    
    this->m_pVtk->RemoveActor(a);
    //    m_pRenderer->RemoveActor(a);
    m_pActorCollection->RemoveItem(a);
}

void QtVtkView::AddActor2D(vtkActor2D *a) {
    if (m_pRenderer == NULL)
        return;
    
    this->m_pVtk->AddActor2D(a);
    m_pActorCollection->AddItem((vtkActor*)a);
}

void QtVtkView::RemoveActor2D(vtkActor2D *a) {
    if (m_pRenderer == NULL)
        return;
    
    this->m_pVtk->RemoveActor2D(a);
    m_pActorCollection->RemoveItem((vtkActor*)a);
}

void QtVtkView::RemoveAllActors() {
    if (m_pRenderer == NULL)
        return;
    
    //remove all existing actors
    vtkActor *a;
    m_pActorCollection->InitTraversal();
    for (a = m_pActorCollection->GetNextActor(); a != NULL; a = m_pActorCollection->GetNextActor()) {
        this->m_pVtk->RemoveActor(a);
        //        m_pRenderer->RemoveActor(a);
    }
    
    m_pActorCollection->RemoveAllItems();
}

void QtVtkView::Render()
{
    if (this->m_pVtk)
        this->m_pVtk->Render();
}

vtkRenderer* QtVtkView::renderer()
{
    return m_pRenderer.GetPointer();
}

void QtVtkView::ResetView(	double  x,double  y,double  z,
                             double vx,double vy,double vz)
{
    if (this->m_pVtk)
        this->m_pVtk->ResetView(x, y, z, vx, vy, vz);
}

void QtVtkView::AutoCenter()
{
    if (this->m_pVtk)
        this->m_pVtk->AutoCenter();
}

void QtVtkView::resizeEvent(QResizeEvent *event)
{
    QVTKWidget::resizeEvent(event);
}

void QtVtkView::mouseMoveEvent(QMouseEvent* event)
{
	switch (this->m_pVtk->GetStatus())
	{
	case eViewMode_Select:
	case eViewMode_Query:
	case eViewMode_Center:
		//do selection and query by ourself
		break;
	default:
	    QVTKWidget::mouseMoveEvent(event);
		break;
	}
}

void QtVtkView::mousePressEvent(QMouseEvent *e) {
	switch (this->m_pVtk->GetStatus())
	{
	case eViewMode_Select:
	case eViewMode_Query:
	case eViewMode_Center:
		//do selection and query by ourself
		break;
	default:
	    QVTKWidget::mousePressEvent(e);
		break;
	}
    // store click position
    m_lastPoint = e->pos();
    // set the flag meaning "click begin"
    m_mouseClick = true;
}

void QtVtkView::mouseReleaseEvent(QMouseEvent *e)
{
    if (mQtVtkViewMode == eQtVtkViewMode_CuttingPlane)
    {//create/moving cutting plane in the main view
        QVTKWidget::mouseReleaseEvent(e);
    } else
    {
        switch (this->m_pVtk->GetStatus())
        {
            case eViewMode_Select:
                // check if cursor not moved since click beginning
                if ((m_mouseClick) && (e->pos() == m_lastPoint))
                {
                    Query(m_lastPoint.x(), m_lastPoint.y());
                }
                break;
            case eViewMode_Query:
                // check if cursor not moved since click beginning
                if ((m_mouseClick) && (e->pos() == m_lastPoint))
                {
			        QVTKWidget::mouseReleaseEvent(e);
                    Query(m_lastPoint.x(), m_lastPoint.y());
                }
                break;
            case eViewMode_Center:
                if ((m_mouseClick) && (e->pos() == m_lastPoint))
                {
                    this->m_pVtk->DoCenter(m_lastPoint.x(), m_lastPoint.y());
                    this->m_pVtk->Render();
                    emit
                    signal_OnCenter();
                }
                break;
            default:
                QVTKWidget::mouseReleaseEvent(e);
                break;
        }
    }

	m_mouseClick = false;
}

void QtVtkView::ShowPlaneWidget(bool bShow, double *center, double *normal, double radius)
{
    if (m_pVtk)
        m_pVtk->ShowPlaneWidget(bShow, center, normal, radius);
}

void QtVtkView::qtVtkViewMode(EQtVtkViewMode e)
{
    mQtVtkViewMode = e;
    if (m_pVtk != NULL) //disable others, status and view mode are excluded from each other
        m_pVtk->SetStatus(eViewMode_Unknown);
}

class sppScalarBarCallback : public vtkCommand
{
public:
	static sppScalarBarCallback *New() { return new sppScalarBarCallback; }
	virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
		auto *widget = reinterpret_cast<sppScalarBarWidget*>(caller);
		if (widget)
		{
            m_qtVtkView->onScalarBarModified(vtkScalarBarActor::SafeDownCast(widget->GetScalarBarActorAdapter()->GetActor2D()));
		}
    }
    
    QtVtkView *m_qtVtkView;
};
vtkNew<sppScalarBarCallback> sbCB;

void QtVtkView::onScalarBarModified(vtkScalarBarActor* sba)
{
    emit
    signal_OnScalarBarModified(sba);
}

void QtVtkView::ShowScalarBar(int index, bool bShow)
{
    if (m_pVtk)
    {
        m_pVtk->ShowScalarBar(index, bShow);
        if (bShow)
        {
            auto w = m_pVtk->scalarBarWidget(index);
            if (w)
            {
                if (!w->HasObserver(vtkCommand::InteractionEvent, sbCB.GetPointer()))
                {
                    sbCB->m_qtVtkView = this;
                    w->AddObserver(vtkCommand::InteractionEvent, sbCB.GetPointer());
                }
            }
        }
    }
}

int QtVtkView::GetNumberOfScalarBars()
{
    if (m_pVtk)
        return m_pVtk->GetNumberOfScalarBars();
    
    return 0;
}

vtkScalarBarActor* QtVtkView::scalarBarActor(int index)
{
	if (m_pVtk == NULL)
		return NULL;
	if (m_pVtk->scalarBarWidget(index) == NULL)
		return NULL;

	return vtkScalarBarActor::SafeDownCast(m_pVtk->scalarBarWidget(index)->GetScalarBarActorAdapter()->GetActor2D());
}

void QtVtkView::Query(int x, int y)
{
    int wHeight = this->height();
    //do selection
	vtkCellPicker* retPicker  = m_pVtk->SelectCell(x, wHeight - y, 0.001);
	if (retPicker == NULL)
		return;
    
	vtkPoints *lPickedPositions = retPicker->GetPickedPositions();
	vtkDataSet *pPickedData = retPicker->GetDataSet();
	if (lPickedPositions != NULL && pPickedData != NULL)
	{        
        QString lAllStr;
        
        QString lText = "";
        lText.sprintf("CellId = %d", (int)retPicker->GetCellId());
        lAllStr = lText;
        
        vtkCell *pCell = pPickedData->GetCell(retPicker->GetCellId());
        if (pCell == NULL)
            return;
        
        //find the closest point in the cell
        vtkIdList *pIdList = pCell->GetPointIds();
        if (pIdList == NULL)
            return;
        
        vtkPoints *pCellPoints = pCell->GetPoints();
        if (pCellPoints == NULL)
            return;
        
        vtkIdType nCellPts = pIdList->GetNumberOfIds();
        if (nCellPts <= 0)
            return;//must be a triangle
        
        for (int i = 0; i < nCellPts; i++)
        {
            if (i == 0)
            {
                lText.sprintf(", CellPts = (%d", (int)pIdList->GetId(i));
                lAllStr += lText;
            } else
            {
                lText.sprintf(", %d", (int)pIdList->GetId(i));
                lAllStr += lText;
            }
        }
        
        lAllStr += ")";
        
        auto clickPoint = retPicker->GetMapperPosition();
        lText.sprintf(", Click Position = (%g, %g, %g)", clickPoint[0], clickPoint[1], clickPoint[2]);
        lAllStr += lText;
        
        vtkDataArray *pScalars = pPickedData->GetPointData()->GetScalars();
        if (pScalars)
        {
            double *pcoords = retPicker->GetPCoords();
            if (pcoords == NULL)
                return;
            
            double *weights = new double[(int)nCellPts];//number of points in a polygon may be much higher than 5
            int subId = 0;
            double clickPoint[3];
            pCell->EvaluateLocation (subId, pcoords,  clickPoint, weights);
            
            //a cell with color has been picked
            if (pScalars != NULL)
            {
                double s = 0.0;
                for (int i = 0; i < nCellPts; i++)
                {
                    s += (weights[i] * pScalars->GetComponent(pIdList->GetId(i), 0));
                }
                lText.sprintf(", Scalar Value = %g", s);
                lAllStr += lText;
            }
            
            delete[] weights;
        }
    emit
        signal_OnQuery(lAllStr);
	}

}

void QtVtkView::slotDoRubberBandZoom()
{
    if (m_pVtk == NULL)
        return;
    
    static bool enabled = false;
    
    enabled = !enabled;
    if (enabled)
        m_pVtk->SetStatus(eViewMode_RubberBand_Zoom);
    else
        m_pVtk->SetStatus(eViewMode_Rotate);

}

void QtVtkView::slotDoRubberBandSelect()
{
    if (m_pVtk == NULL)
        return;
    
    static bool enabled = false;
    
    enabled = !enabled;
    if (enabled)
        m_pVtk->SetStatus(eViewMode_RubberBand_Select);
    else
        m_pVtk->SetStatus(eViewMode_Rotate);
    
}
