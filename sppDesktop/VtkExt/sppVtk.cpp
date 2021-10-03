#define __COMPILING_sppVtk_CPP__
#include "sppVtk.h"
#undef __COMPILING_sppVtk_CPP__

#pragma warning( push, 0 )
#include "vtkSmartPointer.h"
#include "vtkRenderWindow.h"
#include "vtkDoubleArray.h"
#include "vtkInteractorStyleTrackball.h"
#include "vtkPolyDataMapper.h"
#include "vtkVectorText.h"
#include "vtkFollower.h"
#include "vtkPropPicker.h"
#include "vtkRenderer.h" 
#include "vtkMath.h"
#include <vtkAppendPolyData.h> 
#include "vtkUnsignedCharArray.h"
#include "vtkDiskSource.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkTextProperty.h"
#include "vtkProperty2D.h"
#include "vtkActor2D.h"
#include "vtkLightCollection.h"
#include "vtkLight.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkCellData.h"
#include "vtkTransform.h"
#include "vtkProperty.h"
#include "vtkPlane.h"
#include "vtkLightKit.h"
#include "vtkLookupTable.h"
#include "vtkScalarBarActor.h"
#include "vtkInteractorStyleRubberBandZoom.h"
#include "vtkInteractorStyleRubberBandPick.h"
#include "vtkSphereSource.h"
#include "vtkConeSource.h"
#include "vtkCylinderSource.h"
#include "vtkAreaPicker.h"

#include "VtkExt/sppScalarBarWidget.h"
#include "VtkExt/sppScalarBarRepresentation.h"
#include "VtkExt/sppPlaneWidget.h"
#include "VtkExt/sppScalarBarActorAdapter.h"
#include "VtkExt/sppScalarBarRepresentation.h"
#pragma warning(pop)

#include "VtkExt/sppScalarBarActor.h"

//----------------------------------------------------------------------------
// functions to convert progress calls.
class sppVtkCmd : public vtkCommand
{
public:
	// generic new method
	static sppVtkCmd *New()
    { 
		return new sppVtkCmd; 
	}
	
	// the execute
	virtual void Execute(vtkObject *caller, unsigned long event, void* vtkNotUsed(v))
    {
		vtkRenderer *r = vtkRenderer::SafeDownCast(caller);
		if (r != NULL)
		{
			switch(event)
			{
			case vtkCommand::StartEvent:
				{
				}
				break;
			case vtkCommand::EndEvent:
				Self->ResetRenderer2();
				break;
			}
        }
    }
	
	// some ivars that should be set
	sppVtk *Self;
};

static void MyResetCamera(vtkRenderer *Renderer)
{
	vtkCamera *CurrentCamera = Renderer->GetActiveCamera();
	if (CurrentCamera == NULL)
		return;

	double bounds[6];
	Renderer->ComputeVisiblePropBounds( bounds );

	int i, j;
	i = 0; j = 1;
	if (bounds[i] == bounds[j] || bounds[i] == VTK_FLOAT_MAX)
	{
		if (bounds[i] == 0 || bounds[i] == VTK_FLOAT_MAX)
		{
			bounds[i] = -0.1;
			bounds[j] =  0.1;
		} else
		{
			bounds[i] -= 0.01 * fabs(bounds[i]);
			bounds[j] += 0.01 * fabs(bounds[j]);
		}
	}
	i = 2; j = 3;
	if (bounds[i] == bounds[j] || bounds[i] == VTK_FLOAT_MAX)
	{
		if (bounds[i] == 0 || bounds[i] == VTK_FLOAT_MAX)
		{
			bounds[i] = -0.1;
			bounds[j] =  0.1;
		} else
		{
			bounds[i] -= 0.01 * fabs(bounds[i]);
			bounds[j] += 0.01 * fabs(bounds[j]);
		}
	}
	i = 4; j = 5;
	if (bounds[i] == bounds[j] || bounds[i] == VTK_FLOAT_MAX)
	{
		if (bounds[i] == 0 || bounds[i] == VTK_FLOAT_MAX)
		{
			bounds[i] = -0.1;
			bounds[j] =  0.1;
		} else
		{
			bounds[i] -= 0.01 * fabs(bounds[i]);
			bounds[j] += 0.01 * fabs(bounds[j]);
		}
	}

	if ( bounds[0] != VTK_FLOAT_MAX )
	{
		double center[3];
		double distance;
		double width;
		double vn[3], *vup;
		
		CurrentCamera->GetViewPlaneNormal(vn);
		
		center[0] = (bounds[0] + bounds[1])/2.0;
		center[1] = (bounds[2] + bounds[3])/2.0;
		center[2] = (bounds[4] + bounds[5])/2.0;
		
		width = bounds[3] - bounds[2];
		if (width < (bounds[1] - bounds[0]))
		{
			width = bounds[1] - bounds[0];
		}
		//My changes on 02/18/2002
		if (width < (bounds[5] - bounds[4]))
		{
			width = bounds[5] - bounds[4];
		}
        
        if (width < 0)
            width = -width;//bounds may be invalid
		//End of My changes
		CurrentCamera->SetViewAngle(2);//make this as small as possible
		distance = 0.8*width/tan(CurrentCamera->GetViewAngle() * vtkMath::Pi()/360.0);
		//My changes on 02/18/2002
		//old  distance = distance + (bounds[5] - bounds[4])/2.0;
		distance = distance + width * 0.5;
		//End of My changes
		
		// check view-up vector against view plane normal
		vup = CurrentCamera->GetViewUp();
		if ( fabs(vtkMath::Dot(vup,vn)) > 0.999 )
		{
//			vtkWarningMacro(<<"Resetting view-up since view plane normal is parallel");
			CurrentCamera->SetViewUp(-vup[2], vup[0], vup[1]);
		}
		
		// update the camera
		CurrentCamera->SetFocalPoint(center[0],center[1],center[2]);
		CurrentCamera->SetPosition(center[0]+distance*vn[0],
			center[1]+distance*vn[1],
			center[2]+distance*vn[2]);
		
		Renderer->ResetCameraClippingRange( bounds );
		
		// setup default parallel scale
		CurrentCamera->SetParallelScale(width);
	}
}

sppVtk::sppVtk(vtkRenderWindow *rw, vtkRenderer *rder, vtkRenderWindowInteractor *rwi)
{
	m_pGnomonLabelX = m_pGnomonLabelY = m_pGnomonLabelZ = NULL;

	m_pOriginActors = NULL;

	RenderWindow = rw;
    if (RenderWindow)
        RenderWindow->Register(NULL);
    
    Renderer = rder;
    if (Renderer)
        Renderer->Register(NULL);

	//does not help?
//	Renderer->SetUseDepthPeeling(true);
    
    Interactor = rwi;
    if (Interactor)
        Interactor->Register(NULL);
    
    Construct();
    CreateAxes();

	this->SetBkColor(1.0, 1.0, 1.0);

    if (Interactor && Interactor->GetRenderWindow() == NULL)
        Interactor->SetRenderWindow(RenderWindow);
}

sppVtk::~sppVtk()
{
	if (m_pOriginActors != NULL)
	{
		vtkActor *a;
		for (m_pOriginActors->InitTraversal(); (a = m_pOriginActors->GetNextActor());)
		{
			a->Delete();
		}
		m_pOriginActors->Delete();
	}

	if (m_pGnomonLabelX)
		m_pGnomonLabelX->Delete();
	if (m_pGnomonLabelY)
		m_pGnomonLabelY->Delete();
	if (m_pGnomonLabelZ)
		m_pGnomonLabelZ->Delete();

	m_pVWStatusNameMapper->Delete();
#if 0//cause problems when more than one vtk windows are opened
	RemoveAllActors();
#endif
	if (m_pCellPicker != NULL)
		m_pCellPicker->Delete();
	if (m_pPropPicker != NULL)
		m_pPropPicker->Delete();
	if (m_pPointPicker != NULL)
		m_pPointPicker->Delete();

	if (m_pClipPlaneScreen != NULL)
		m_pClipPlaneScreen->Delete();

	if (Renderer2)
		Renderer2->Delete();

	if (this->Interactor) 
		this->Interactor->Delete();

	if (Renderer)
		Renderer->Delete();

#if 0//cause problems when more than one vtk windows are opened
	if (this->RenderWindow) 
		this->RenderWindow->Delete();
#endif

	if (m_pLightKit)
		m_pLightKit->Delete();

	for (int i = 0; i < m_nAdditionalRenderers; i++)
		m_AdditionalRenderers[i]->Delete();
}

void sppVtk::Construct()
{
    m_AxiesDispType = eVWAxies_CARTESION_3D;
    
	SelectPointMethod	= NULL;
	SelectCellMethod	= NULL;
	SelectPropMethod	= NULL;
	EndSelectMethod		= NULL;
    
	SelectPointMethodArg= NULL;
	SelectCellMethodArg = NULL;
	SelectPropMethodArg = NULL;
	EndSelectMethodArg	= NULL;
    
	m_pPointPicker = NULL;
	m_pCellPicker = NULL;
	m_pPropPicker = NULL;
    
	m_Clip = false;
    
	m_pLightKit = NULL;
    
#ifdef NotForWnd
    if (Interactor == NULL)
        Interactor = vtkRenderWindowInteractor::New();
#else
	m_BgColor = RGB(1.0, 0, 0);
    if (RenderWindow == NULL)
        RenderWindow = vtkWin32OpenGLRenderWindow::New();
    if (Interactor == NULL)
        Interactor = vtkWin32RenderWindowInteractor::New();
#endif
	RenderWindow->SetNumberOfLayers(2);
    
	m_pClipPlaneScreen = NULL;
	m_pClipPlaneScreenActor = NULL;
    
    if (Renderer == NULL)
    {
        Renderer = vtkRenderer::New();
        RenderWindow->AddRenderer(Renderer);
    }
    sppVtkCmd *cb = sppVtkCmd::New();
    cb->Self = this;
    //	Renderer->AddObserver(vtkCommand::StartEvent, cb);
    Renderer->AddObserver(vtkCommand::EndEvent, cb);
    cb->Delete();
    
    Renderer->SetLayer(0);
    Renderer->SetInteractive(1);
    
	Renderer2 = vtkRenderer::New();
	Renderer2->SetViewport(0.75, 0, 1, 0.25);
	Renderer2->SetLayer(1);
	Renderer2->SetInteractive(0);
	RenderWindow->AddRenderer(Renderer2);
    
    if (Interactor) {
        Interactor->SetLightFollowCamera(1);
	//the default interactor style is vtkInteractorStyleSwitch:
	/*
     vtkRenderWindowInteractor::vtkRenderWindowInteractor()
     {
     ...
     this->SetInteractorStyle(vtkInteractorStyleSwitch::New());
     ...
     }
     */
//        ((vtkInteractorStyleSwitch*)Interactor->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();
    }
	m_pVWStatusNameMapper = vtkTextMapper::New();
    m_pVWStatusNameMapper->SetInput("Mode::Unknown");
    m_pVWStatusNameMapper->GetTextProperty()->SetFontSize(12);
    m_pVWStatusNameMapper->GetTextProperty()->SetJustificationToLeft();
    m_pVWStatusNameMapper->GetTextProperty()->SetVerticalJustificationToBottom();
    
    vtkActor2D *actorName = vtkActor2D::New();
    actorName->GetProperty()->SetColor(1,1,1);
    actorName->SetMapper(m_pVWStatusNameMapper);
    
    vtkCoordinate *pPos = actorName->GetPositionCoordinate();
    //		ASSERT(pPos != NULL);
    pPos->SetCoordinateSystemToNormalizedViewport();
    pPos->SetValue(0.1, 0.005);
    
    Renderer2->AddActor(actorName);
    actorName->Delete();
	SetStatus(eViewMode_Rotate);
	MouseDown=false;
	m_Wireframe=false;
    
	m_CellPickTol = 0.01;
    
	m_nAdditionalRenderers = 0;
	m_AdditionalRenderers[0] = 0;
	m_AdditionalRenderers[1] = 0;
	m_AdditionalRenderers[2] = 0;
	m_AdditionalRenderers[3] = 0;
    
	vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();

}

class myScalarBarActor : public sppScalarBarActorAdapter
{
public:
	virtual vtkScalarBarActor* GetMe() {
		return m_actor;
	}

	virtual vtkActor2D* GetActor2D() {
		return m_actor;
	}

	virtual int GetOrientation()
	{
		return m_actor->GetOrientation();
	}
	virtual void SetOrientation(int i)
	{
		m_actor->SetOrientation(i);
	}

	virtual double* GetPosition()
	{
		return m_actor->GetPosition();
	}

	virtual void SetPosition(double* d)
	{
		m_actor->SetPosition(d);
	}

	virtual double* GetPosition2()
	{
		return m_actor->GetPosition2();
	}

	virtual void SetPosition2(double*d)
	{
		m_actor->SetPosition2(d);
	}

	virtual void GetColorBarPos(double* pos)
	{
		int *ipos = m_actor->GetScalarBarPos();
		pos[0] = ipos[0];
        pos[1] = ipos[1];
	}

	virtual void GetColorBarSize(double* size)
	{
		int *isize = m_actor->GetScalarBarSize();
		if (m_actor->GetOrientation() == 0) //horizontal
		{
			size[0] = isize[1];
			size[1] = isize[0];
		}
		else
		{
			size[0] = isize[0];
			size[1] = isize[1];
		}
	}

	virtual int GetVisibility()
	{
		return m_actor->GetVisibility();
	}

	virtual void SetVisibility(int i)
	{
		m_actor->SetVisibility(i);
	}

	virtual void SetClipHandsVisibility(int iv)
	{
		m_actor->SetClippingHandsVisibility(iv);
	}

	virtual bool SetUpperClipValue(double d)
	{
        if (d != m_actor->GetUpperClipValue())
        {
            m_actor->SetUpperClipValue(d);
            return true;
        }
        
        return false;
	}

	virtual bool SetLowerClipValue(double d)
	{
		if (d != m_actor->GetLowerClipValue())
        {
            m_actor->SetLowerClipValue(d);
            return true;
        }
        
        return false;
	}

	void SetUpperClippingHandColor(double* c)
	{
		m_actor->SetUpperClippingHandColor(c);
	}

	void SetLowerClippingHandColor(double* c)
	{
		m_actor->SetLowerClippingHandColor(c);
	}

	myScalarBarActor()
	{
		m_actor = sppScalarBarActor::New();
		m_actor->SetOrientation(0); //horizental
	}
	~myScalarBarActor()
	{
		m_actor->Delete();
	}

protected:
	sppScalarBarActor *m_actor;
};

sppScalarBarWidget* sppVtk::scalarBarWidget(int index)
{
    if (index < 0)
        return NULL;
    
    int nBars = (int)m_scalarWidgets.size();
    if (index > nBars)
        return NULL;
    
    if (index == nBars)
    {//build but dont' show it
        auto scalarWidget = vtkSmartPointer<sppScalarBarWidget>::New();
        scalarWidget->SetInteractor(Interactor);
		myScalarBarActor *mya = new myScalarBarActor;
		scalarWidget->SetScalarBarActorAdapter(mya);
		mya->Delete();
        
        vtkScalarBarActor *a = mya->GetMe();
		a->SetMaximumNumberOfColors(256);
        {//why lt can not be shared?
            vtkLookupTable *lt = vtkLookupTable::New();
			lt->SetHueRange(0.667, 0.0);
			lt->SetNumberOfTableValues(256) ;
			lt->SetNumberOfColors(256);
			lt->Build(); //the table is
            a->SetLookupTable(lt);
            lt->Delete();
            a->SetVisibility(0);
            Renderer->AddActor(a);
        }
        m_scalarWidgets.push_back(scalarWidget);
        
        sppScalarBarRepresentation* srep = sppScalarBarRepresentation::SafeDownCast(scalarWidget->GetRepresentation());
		if (srep)
		{
			srep->SetPosition(0.3375, 0.0);
			srep->SetPosition2(0.3355, 0.1);
		}
	}
    
	return m_scalarWidgets[index];
}

int sppVtk::GetNumberOfScalarBars()
{
    return (int)m_scalarWidgets.size();
}

sppPlaneWidget* sppVtk::planeWidget()
{
    return m_planeWidget;
}

void sppVtk::ShowScalarBar(int index, bool bShow, bool bResetClipValues)
{
    //create widget if it has not been created
    sppScalarBarWidget *w = this->scalarBarWidget(index);
	if (w == NULL)
		return;

	sppScalarBarActorAdapter *ai = w->GetScalarBarActorAdapter();
	if (ai == NULL)
		return;

    if (bShow) {
        if (bResetClipValues)
        {
            //reset the cone position
            w->GetScalarBarRepresentation()->SetUpperConeRelativePos(1.0);
            w->GetScalarBarRepresentation()->SetLowerConeRelativePos(0.0);
            //make the actor clip values and the cone positions consistent
            ai->SetLowerClipValue(VTK_FLOAT_MIN);
            ai->SetUpperClipValue(VTK_FLOAT_MAX);
        }
        
		float* lContrastColor = this->GetContrastColor();
		vtkScalarBarActor *a = vtkScalarBarActor::SafeDownCast(ai->GetActor2D());
		if (a)
		{
			a->GetTitleTextProperty()->SetColor(lContrastColor[0], lContrastColor[1], lContrastColor[2]);
			a->GetLabelTextProperty()->SetColor(lContrastColor[0], lContrastColor[1], lContrastColor[2]);
			a->SetVisibility(1);
		}
		m_scalarWidgets[index]->GetBorderRepresentation()->GetBorderProperty()->SetColor(lContrastColor[0], lContrastColor[1], lContrastColor[2]);
        m_scalarWidgets[index]->EnabledOn();
    } else {
        ai->GetActor2D()->SetVisibility(0);
        m_scalarWidgets[index]->EnabledOff();
    }
}


void sppVtk::ShowPlaneWidget(bool bShow, double *center, double *normal, double radius)
{
    if (bShow) {
        if (m_planeWidget ==NULL || !m_planeWidget->GetEnabled())
        {//enable it if it has not been
            if (m_planeWidget == NULL)
            {
                m_planeWidget = vtkSmartPointer<sppPlaneWidget>::New();
                vtkPolyData *tpd = vtkPolyData::New();//an empty input to make it work
                m_planeWidget->SetInputData(tpd);
                tpd->Delete();
            }
            m_planeWidget->SetResolution(20);
            m_planeWidget->SetInteractor(Interactor);
            m_planeWidget->SetCenter(center);
            m_planeWidget->SetNormal(normal);
            m_planeWidget->PlaceWidget(radius);
            
            m_planeWidget->EnabledOn();
        } else
        {
            m_planeWidget->SetCenter(center);
            m_planeWidget->SetNormal(normal);
            m_planeWidget->Modified();
        }
    } else {
        m_planeWidget->EnabledOff();
    }
}

void sppVtk::AddLightKit()
{
	if (Renderer != NULL && m_pLightKit == NULL)
	{
		m_pLightKit = vtkLightKit::New();
		  m_pLightKit->AddLightsToRenderer(Renderer);
	}
}

void sppVtk::RemoveLightKit()
{
	if (Renderer != NULL && m_pLightKit != NULL)
	{
		  m_pLightKit->RemoveLightsFromRenderer(Renderer);
	}

	if (m_pLightKit != NULL)
	{
		m_pLightKit->Delete();
		m_pLightKit = NULL;
	}
}

void sppVtk::SetAxiesDispType(EAxiesDispType type) 
{ 
	if (m_AxiesDispType != type)
	{
		m_AxiesDispType = type; 
		CreateAxes();//recreate it
	}
}

void sppVtk::Zoom(double factor)
{
	vtkCamera *CurrentCamera=Renderer->GetActiveCamera();
	if (CurrentCamera != NULL)
	{
		CurrentCamera->Zoom(factor);
	}
}

void sppVtk::Dolly(double distance)
{
	vtkCamera *CurrentCamera=Renderer->GetActiveCamera();
	if (CurrentCamera != NULL)
	{
		CurrentCamera->Dolly(distance);
	}
}

void sppVtk::DoCenter(int centerX, int centerY)
{
	vtkCamera *CurrentCamera=Renderer->GetActiveCamera();
//	ASSERT(CurrentCamera!=NULL);

    //We'll need the window height later
	int *size=RenderWindow->GetSize();
    centerY = size[1] - centerY;

    // Convert the focal point to a display coordinate in order to get the
    // depth of the focal point in display units
    double *oldFocalPoint=CurrentCamera->GetFocalPoint();

	Renderer->SetWorldPoint(oldFocalPoint[0], oldFocalPoint[1], oldFocalPoint[2], 1.0);
    Renderer->WorldToDisplay();
//    double *DPoint=Renderer->GetDisplayPoint();

	float *zBuf = RenderWindow->GetZbufferData(centerX, centerY, centerX, centerY);
	if (zBuf != NULL && zBuf[0] < 0.99999)
	{
		double z = zBuf[0];

		Renderer->SetDisplayPoint(centerX, centerY, z);
	    Renderer->DisplayToWorld();

		double newFocalPoint[4];
		memcpy(newFocalPoint, Renderer->GetWorldPoint(), 4*sizeof(double));
		if (newFocalPoint[3] != 0.0f)
		{
			newFocalPoint[0] /= newFocalPoint[3];
			newFocalPoint[1] /= newFocalPoint[3];
			newFocalPoint[2] /= newFocalPoint[3];
		}

		//displacements
		double dx = newFocalPoint[0] - oldFocalPoint[0];
		double dy = newFocalPoint[1] - oldFocalPoint[1];
		double dz = newFocalPoint[2] - oldFocalPoint[2];

		//move focal point of the camera
		CurrentCamera->SetFocalPoint(newFocalPoint);
		//move the eye position
		double *oldEyePoint = CurrentCamera->GetPosition();
		CurrentCamera->SetPosition(oldEyePoint[0] + dx, 
								   oldEyePoint[1] + dy, 
								   oldEyePoint[2] + dz);
	}
}

void sppVtk::ResetView(double  x,double  y,double  z,
                     double vx,double vy,double vz)
{
	vtkCamera *CurrentCamera=Renderer->GetActiveCamera();
	if (CurrentCamera!=NULL)
	{
		//determine the viewing direction
		CurrentCamera->SetPosition(x,y,z);
		CurrentCamera->SetFocalPoint(0,0,0);
		//the view up direction
		CurrentCamera->SetViewUp(vx,vy,vz);
		//bounds will be calculated in the ResetCamera function
#if 0
		Renderer->ResetCamera();
#else
		MyResetCamera(Renderer);
#endif
	if (Interactor->GetLightFollowCamera())
		{
			vtkLight *light;
			//lights will also have to be moved
			vtkLightCollection *lights=Renderer->GetLights();
			if (lights == NULL || lights->GetNumberOfItems() <= 0)
			{
				light = vtkLight::New();
				light->SetLightTypeToHeadlight();//SetLightTypeToCameraLight();
				Renderer->AddLight(light);
				light->Delete();
				
				lights = Renderer->GetLights();
			}
			if (lights!=NULL)
			{
				for ( lights->InitTraversal(); 
					  (light = lights->GetNextItem());)
				{
					light->SetPosition(CurrentCamera->GetPosition());
					light->SetFocalPoint(CurrentCamera->GetFocalPoint());
					break;//what happens if move than one lights exist ?
				}
			}
		}
		ResetRenderer2();
		Render();
	}
}

static vtkFollower* CreateAxesLabel(vtkRenderer *r, int iAxis, const char *s, double *o, double l)
{
	vtkVectorText *yText = vtkVectorText::New();
		yText->SetText(s);
	vtkPolyDataMapper *yMapper = vtkPolyDataMapper::New();
    yMapper->SetInputConnection(yText->GetOutputPort());
		yText->Delete();
	vtkFollower *yActor = vtkFollower::New();
		yActor->SetMapper(yMapper);
			yMapper->Delete();
		yActor->SetScale(0.2,0.2,0.2);
		switch(iAxis)
		{
		case 0:
			yActor->AddPosition(o[0] + l, o[1], o[2]);
			break;
		case 1:
			yActor->AddPosition(o[0], o[1] + l, o[2]);
			break;
		case 2:
			yActor->AddPosition(o[0], o[1], o[2] + l);
			break;
		}
		yActor->SetCamera(r->GetActiveCamera());
		r->AddActor(yActor);

		return yActor;
}

int sppVtk::CreateAxes()
{
	RemoveAllActors(Renderer2);
	double o[3],l;
	o[0] = o[1] = o[2] = 0.0;
	l = 1.0;
	vtkIdType ids[4];
	vtkAppendPolyData *append = vtkAppendPolyData::New();
//3 axes
	vtkPoints *pts = vtkPoints::New();
		pts->Allocate(4);
		pts->InsertNextPoint(o);
		pts->InsertNextPoint(o[0] + l, o[1], o[2]);
		pts->InsertNextPoint(o[0], o[1] + l, o[2]);
		pts->InsertNextPoint(o[0], o[1], o[2] + l);
	vtkCellArray *cells = vtkCellArray::New();
		ids[0] = 0;		ids[1] = 1;
		cells->InsertNextCell(2,ids);
		ids[0] = 0;		ids[1] = 2;
		cells->InsertNextCell(2,ids);
		switch (GetAxiesDispType())
		{
		case eVWAxies_CARTESION_2D:
		case eVWAxies_CARTESION_3D:
			ids[0] = 0;		ids[1] = 3;
			cells->InsertNextCell(2,ids);
			break;
            default:
                break;
		}
	vtkPolyData *axes = vtkPolyData::New();
		axes->SetPoints(pts);
			pts->Delete();
		axes->SetLines(cells);
			cells->Delete();
	vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
		colors->SetNumberOfComponents(3);
		colors->SetNumberOfTuples(3);
	double tuple[3];
	tuple[0] = 255.0;	tuple[1] = 0.0;	tuple[2] = 0.0;
	colors->SetTuple(0, tuple);
	tuple[0] = 0.0;	tuple[1] = 255.0;	tuple[2] = 0.0;
	colors->SetTuple(1, tuple);
	tuple[0] = 0.0;	tuple[1] = 0.0;	tuple[2] = 255.0;
	colors->SetTuple(2, tuple);
	axes->GetCellData()->SetScalars(colors);
		colors->Delete();
	append->AddInputData(axes);
		axes->Delete();
	if (  GetAxiesDispType() == eVWAxies_CYLINDER_T
	    ||GetAxiesDispType() == eVWAxies_CYLINDER_B)
	{//a disk
		vtkDiskSource *disk = vtkDiskSource::New();
			disk->SetOuterRadius(0.5 * l);
			disk->SetInnerRadius(0.0);
			disk->SetCircumferentialResolution(16);
		vtkTransform *t = vtkTransform::New();
		switch (GetAxiesDispType())
		{
		case eVWAxies_CYLINDER_T:
			t->RotateX(-90);
			break;
		case eVWAxies_CYLINDER_B:
			t->RotateY(90);
			break;
                default:
                break;
		}
		vtkTransformPolyDataFilter *tF = vtkTransformPolyDataFilter::New();
		tF->SetTransform(t);
			t->Delete();
		tF->SetInputConnection(disk->GetOutputPort());
			disk->Delete();

		//all cells the same color
		tF->Update();
		cells = tF->GetOutput()->GetPolys();
	//	ASSERT(cells != NULL);
		colors = vtkUnsignedCharArray::New();
		colors->SetNumberOfComponents(3);
		int nCs = cells->GetNumberOfCells();
		colors->SetNumberOfTuples(nCs);
		tuple[0] = 255.0;	tuple[1] = 255.0;	tuple[2] = 0.0;//yellow
		for (int i = 0; i < nCs; i++)
		{
			colors->SetTuple(i, tuple);
		}
		tF->GetOutput()->GetCellData()->SetScalars(colors);
			colors->Delete();
		append->AddInputData(tF->GetOutput());
			tF->Delete();
	} else
	{//a cube
		pts = vtkPoints::New();
			pts->Allocate(8);
			//bottom
			pts->InsertNextPoint(o);
			pts->InsertNextPoint(o[0]+0.5*l, o[1], o[2]);
			pts->InsertNextPoint(o[0]+0.5*l, o[1]+0.5*l, o[2]);
			pts->InsertNextPoint(o[0], o[1]+0.5*l, o[2]);
			//top
			pts->InsertNextPoint(o[0], o[1], o[2]+0.5*l);
			pts->InsertNextPoint(o[0]+0.5*l, o[1], o[2]+0.5*l);
			pts->InsertNextPoint(o[0]+0.5*l, o[1]+0.5*l, o[2]+0.5*l);
			pts->InsertNextPoint(o[0], o[1]+0.5*l, o[2]+0.5*l);
		cells = vtkCellArray::New();
			ids[0] = 0;	ids[1] = 1;	ids[2] = 2;	ids[3] = 3;
			cells->InsertNextCell(4, ids);//bottom
			ids[0] = 4;	ids[1] = 5;	ids[2] = 6;	ids[3] = 7;
			cells->InsertNextCell(4, ids);//top
			ids[0] = 0;	ids[1] = 1;	ids[2] = 5;	ids[3] = 4;
			cells->InsertNextCell(4, ids);
			ids[0] = 3;	ids[1] = 2;	ids[2] = 6;	ids[3] = 7;
			cells->InsertNextCell(4, ids);
			ids[0] = 0;	ids[1] = 4;	ids[2] = 7;	ids[3] = 3;
			cells->InsertNextCell(4, ids);
			ids[0] = 1;	ids[1] = 5;	ids[2] = 6;	ids[3] = 2;
			cells->InsertNextCell(4, ids);
		vtkPolyData *cube = vtkPolyData::New();
			cube->SetPoints(pts);
				pts->Delete();
			cube->SetPolys(cells);
				cells->Delete();

		colors = vtkUnsignedCharArray::New();
			colors->SetNumberOfComponents(3);
			colors->SetNumberOfTuples(6);
		tuple[0] = 0.0;	tuple[1] = 0.0;	tuple[2] = 255.0;
		colors->SetTuple(0, tuple);
		colors->SetTuple(1, tuple);
		tuple[0] = 0.0;	tuple[1] = 255.0;	tuple[2] = 0.0;
		colors->SetTuple(2, tuple);
		colors->SetTuple(3, tuple);
		tuple[0] = 255.0;	tuple[1] = 0.0;	tuple[2] = 0.0;
		colors->SetTuple(4, tuple);
		colors->SetTuple(5, tuple);
		cube->GetCellData()->SetScalars(colors);
			colors->Delete();
		append->AddInputData(cube);
			cube->Delete();
	}

	vtkPolyDataMapper *pMapper = vtkPolyDataMapper::New();
		pMapper->SetColorModeToDefault();

	pMapper->SetInputConnection(append->GetOutputPort());
		append->Delete();

	vtkActor *pCubeActor = vtkActor::New();
		pCubeActor->SetMapper(pMapper);
			pMapper->Delete();
	Renderer2->AddActor(pCubeActor);
		pCubeActor->Delete();
//Axes labels
	switch(GetAxiesDispType())
	{
	case eVWAxies_CARTESION_2D:
	case eVWAxies_CARTESION_3D:
		m_pGnomonLabelX = CreateAxesLabel(Renderer2, 0, "X", o, l);
		m_pGnomonLabelY = CreateAxesLabel(Renderer2, 1, "Y", o, l);
		m_pGnomonLabelZ = CreateAxesLabel(Renderer2, 2, "Z", o, l);
		break;
	case eVWAxies_CYLINDER_T:
		m_pGnomonLabelX = CreateAxesLabel(Renderer2, 0, "R", o, l);
		m_pGnomonLabelY = CreateAxesLabel(Renderer2, 1, "Z", o, l);
		break;
	case eVWAxies_CYLINDER_B:
		m_pGnomonLabelX = CreateAxesLabel(Renderer2, 0, "Z", o, l);
		m_pGnomonLabelY = CreateAxesLabel(Renderer2, 1, "R", o, l);
		break;
        default:
            break;
	}

	Renderer2->GetActiveCamera()->SetFocalPoint(0,0,0);

	return(0);
}

#ifdef NotForWnd
#else
LRESULT sppVtk::HandleMouseMsg(HWND hwnd, UINT msg, int &ptX, int &ptY)
{
	WPARAM w = 0;
	LPARAM l = MAKELPARAM(ptX, ptY);
	if(Interactor->GetInitialized())
		return(vtkHandleMessage2(hwnd, msg, w, l, Interactor));
	else
		return(-1);
}
#endif

void sppVtk::DoSelect(int ptX, int ptY)
{
	if (SelectPointMethod != NULL)
	{
		vtkPointPicker *pPicker = SelectPoint(ptX, ptY, 0.01);
		if (pPicker != NULL)
			(*SelectPointMethod)(this, pPicker, SelectPointMethodArg);
	}
	if (SelectCellMethod != NULL)
	{
		vtkCellPicker *pPicker = SelectCell(ptX, ptY, m_CellPickTol);
		if (pPicker != NULL)
		(*SelectCellMethod)(this, pPicker, SelectCellMethodArg);
	}
	if (SelectPropMethod != NULL)
	{
		vtkPropPicker *pPicker = SelectProp(ptX, ptY, m_CellPickTol);
		if (pPicker != NULL)
			(*SelectPropMethod)(this, pPicker, SelectPropMethodArg);
	}
}

void sppVtk::Reset() 
{
	switch (GetAxiesDispType())
	{
	case eVWAxies_CARTESION_2D:
	case eVWAxies_CYLINDER_T:
		ResetView(0, 0, 1, 0, 1, 0);//2d view, y is up and viewing in +z direction
		break;
	case eVWAxies_CYLINDER_B:
		ResetView(0, 0, -1, 1, 0, 0);//2d view, x is up and viewing in -z direction
		break;
	case eVWAxies_CARTESION_3D:
		ResetView(1, 0, 0, 0, 0, 1);//z up and y left
		break;
	}
}

void sppVtk::AutoCenter() 
{
	MyResetCamera(Renderer);
	Render();
}

void sppVtk::AddActor(vtkActor *pActor) 
{
	ClipEnd();
	if (!FindActor(pActor))
	{
		this->Renderer->AddViewProp(pActor);
		m_ActiveActors.insert(pActor);
	}
}

void sppVtk::AddActor(vtkActor *pActor, int iRenderer) 
{
	if (iRenderer <= 0)
	{
		AddActor(pActor);
	} else
	{
		iRenderer--;
		if (iRenderer < m_nAdditionalRenderers && m_AdditionalRenderers[iRenderer] != NULL)
			m_AdditionalRenderers[iRenderer]->AddViewProp(pActor);
	}
}

void sppVtk::RemoveActor(vtkActor *pActor) 
{
	ClipEnd();
	if (pActor != NULL)
	{
		pActor->ReleaseGraphicsResources(RenderWindow);
		this->Renderer->RemoveActor(pActor);
		m_ActiveActors.erase(pActor);
	}
}

void sppVtk::RemoveActor(vtkActor *pActor, int iRenderer) 
{
	if (iRenderer <= 0)
		RemoveActor(pActor);
	else
	{
		iRenderer--;
		if (iRenderer < m_nAdditionalRenderers && m_AdditionalRenderers[iRenderer] != NULL)
			m_AdditionalRenderers[iRenderer]->RemoveActor(pActor);
	}
}

void sppVtk::AddActor2D(vtkActor2D *pActor)
{
	ClipEnd();
	if (!FindActor((vtkActor*)pActor))
	{
		this->Renderer->AddActor2D(pActor);
		m_ActiveActors.insert((vtkActor*)pActor);
	}
}


void sppVtk::RemoveActor2D(vtkActor2D *pActor)
{
	ClipEnd();
	if (pActor != NULL)
	{
		pActor->ReleaseGraphicsResources(RenderWindow);
		this->Renderer->RemoveActor2D(pActor);
		m_ActiveActors.erase(pActor);
	}
}

void sppVtk::RemoveAllActors(vtkRenderer *r)
{
	vtkPropCollection *pActors;
	if (r == NULL)
		r = Renderer;
	if (r == NULL)
		return;
	pActors = r->GetViewProps();
	if (pActors != NULL)
	{
		pActors->InitTraversal();
		vtkProp *pActor;
		while ((pActor = pActors->GetNextProp()) != NULL)
		{
			pActor->ReleaseGraphicsResources(RenderWindow);
			r->RemoveViewProp(pActor);
		}
		m_ActiveActors.clear();
	}
}

void sppVtk::AddActors(vtkActorCollection *pActorCollection) 
{
	if (pActorCollection==NULL)
		return;
	ClipEnd();
	pActorCollection->InitTraversal();
	vtkActor *pActor;
	while ((pActor = pActorCollection->GetNextActor()))
    {
		if (!FindActor(pActor))
		{
			this->Renderer->AddActor(pActor);
			m_ActiveActors.insert(pActor);
		}
    }
}

void sppVtk::RemoveActors(vtkActorCollection *pActorCollection) 
{
	if (pActorCollection==NULL)
		return;
	ClipEnd();
	pActorCollection->InitTraversal();
	vtkActor *pActor;
	while ((pActor = pActorCollection->GetNextActor()))
    {
		pActor->ReleaseGraphicsResources(RenderWindow);
		this->Renderer->RemoveActor(pActor);
		m_ActiveActors.erase(pActor);
    }
}

bool sppVtk::FindActor(vtkProp *pP) 
{
	return(pP != NULL && m_ActiveActors.find(pP) != m_ActiveActors.end());
}

void sppVtk::ResetRenderer2()
{
	ResetRenderer(Renderer2);
	for (int i = 0; i < MAX_ADDITIONAL_RENDERERS; i++)
	{
		ResetRenderer(this->GetAdditionalRenderer(i));
	}
}

void sppVtk::ResetRenderer(vtkRenderer *r)
{
	if (r == NULL)
		return;

	vtkCamera *mainCamera = Renderer->GetActiveCamera();
	vtkCamera *axesCamera = r->GetActiveCamera();

	axesCamera->SetPosition(mainCamera->GetPosition());
	axesCamera->SetViewUp(mainCamera->GetViewUp());
	axesCamera->SetFocalPoint(mainCamera->GetFocalPoint());
	if (r == Renderer2)
	{
		double bounds[] = {-1,1,-1,1,-1,1};
		axesCamera->Modified();
		r->ResetCamera(bounds);
	} else
	{
		axesCamera->SetPosition(mainCamera->GetPosition());
		axesCamera->SetFocalPoint(mainCamera->GetFocalPoint());
		axesCamera->SetViewUp(mainCamera->GetViewUp());
		axesCamera->SetParallelScale(mainCamera->GetParallelScale());
		axesCamera->SetDistance(mainCamera->GetDistance());
		axesCamera->SetViewAngle(mainCamera->GetViewAngle());
		axesCamera->Modified();
	}

	vtkLightCollection *lights = r->GetLights();
	vtkLight *light;
	for (  lights->InitTraversal();
	       (light = lights->GetNextItem());)
	{
		light->SetFocalPoint(0,0,0);
		light->SetPosition(axesCamera->GetPosition());
	}
}

void sppVtk::ClipStart()
{
	if (m_pClipPlaneScreen != NULL)
		ClipEnd();

	double bs[6], gbs[6];
	int index = 0;
	vtkActorCollection *actors = Renderer->GetActors();
	vtkActor *actor;
	for (  actors->InitTraversal(); (actor = actors->GetNextActor());)
	{
		actor->GetBounds(bs);
		if (index == 0)
		{
			memcpy(gbs, bs,6*sizeof(double));
		} else
		{
			if (gbs[0] > bs[0])
				gbs[0] = bs[0];
			if (gbs[1] < bs[1])
				gbs[1] = bs[1];

			if (gbs[2] > bs[2])
				gbs[2] = bs[2];
			if (gbs[3] < bs[3])
				gbs[3] = bs[3];

			if (gbs[4] > bs[4])
				gbs[4] = bs[4];
			if (gbs[5] < bs[5])
				gbs[5] = bs[5];
		}
		index++;
	}

	double dMax;
	dMax = gbs[1] - gbs[0];
	if (dMax < (gbs[3] - gbs[2]))
		dMax = gbs[3] - gbs[2];
	if (dMax < (gbs[5] - gbs[4]))
		dMax = gbs[5] - gbs[4];

	if (index <=0 || dMax <1.0e-8)
	{
		gbs[0] =-1;
		gbs[1] = 1;

		gbs[2] =-1;
		gbs[3] = 1;

		gbs[2] =-1;
		gbs[3] = 1;

		dMax = 2;
	}

	dMax *=0.1;
//	if ((gbs[1] - gbs[0])<1.0e-8)
	{
		gbs[0] -= dMax;
		gbs[1] += dMax;
	}
//	if ((gbs[3] - gbs[2])<1.0e-8)
	{
		gbs[2] -= dMax;
		gbs[3] += dMax;
	}
//	if ((gbs[5] - gbs[4])<1.0e-8)
	{
		gbs[4] -= dMax;
		gbs[5] += dMax;
	}

	m_ClipPlanePushStepLength = 0.1*(gbs[5] - gbs[4]);//along x axes

	m_pClipPlaneScreen = vtkPlaneSource::New();

	double center[3];
	center[0] = 0.5*(gbs[1] + gbs[0]);
	center[1] = 0.5*(gbs[3] + gbs[2]);
	center[2] = 0.5*(gbs[5] + gbs[4]);

	m_pClipPlaneScreen->SetOrigin(center[0], gbs[2], gbs[4]);
	m_pClipPlaneScreen->SetPoint2(center[0], gbs[3], gbs[4]);
	m_pClipPlaneScreen->SetPoint1(center[0], gbs[2], gbs[5]);

	vtkPolyDataMapper *planeMapper = vtkPolyDataMapper::New();
	planeMapper->SetInputConnection(m_pClipPlaneScreen->GetOutputPort());
    
	m_pClipPlaneScreenActor = vtkActor::New();
		m_pClipPlaneScreenActor->SetMapper(planeMapper);
			planeMapper->Delete();
	m_pClipPlaneScreenActor->GetProperty()->SetColor(0.7,0.7,0.7);
	m_pClipPlaneScreenActor->GetProperty()->SetOpacity(0.45);
	Renderer->AddViewProp(m_pClipPlaneScreenActor);
		m_pClipPlaneScreenActor->Delete();

	//do the cutting
	m_pClipPlane = vtkPlane::New();
		m_pClipPlane->SetOrigin(m_pClipPlaneScreen->GetCenter());
		m_pClipPlane->SetNormal(m_pClipPlaneScreen->GetNormal());

	actors = Renderer->GetActors();
	for (  actors->InitTraversal(); (actor = actors->GetNextActor());)
	{
		if (   actor->IsA("vtkActor")
			&& actor != m_pClipPlaneScreenActor)
			actor->GetMapper()->AddClippingPlane(m_pClipPlane);
	}
	m_pClipPlane->Delete();

	RenderWindow->Render();
}
void sppVtk::ClipEnd()
{
	m_Clip = false;
	if (m_pClipPlaneScreen == NULL)
		return;

	vtkActorCollection *actors = Renderer->GetActors();
	vtkActor *actor;
	for (  actors->InitTraversal(); (actor = actors->GetNextActor());)
	{
		if (   actor->IsA("vtkActor")
			&& actor != m_pClipPlaneScreenActor)
			actor->GetMapper()->RemoveClippingPlane(m_pClipPlane);
	}

	Renderer->RemoveViewProp(m_pClipPlaneScreenActor);

	m_pClipPlaneScreen->Delete();
	m_pClipPlaneScreen = NULL;
	m_pClipPlaneScreenActor = NULL;

	RenderWindow->Render();
}
void sppVtk::ClipMove(int &ptX, int &ptY)
{
	if ((ptX - m_startPtX)>0)
		m_pClipPlaneScreen->Push( m_ClipPlanePushStepLength);
	else
		m_pClipPlaneScreen->Push(-m_ClipPlanePushStepLength);
	m_pClipPlaneScreen->Modified();

	m_pClipPlane->SetOrigin(m_pClipPlaneScreen->GetCenter());
	m_pClipPlane->SetNormal(m_pClipPlaneScreen->GetNormal());
	m_pClipPlane->Modified();

	RenderWindow->Render();
	m_startPtX = ptX;
	m_startPtY = ptX;
}

void sppVtk::SetStatus(EViewMode_Status status)
{
#if 0
	if (m_eStatus == eSelect && status != eSelect)
	{
		if (EndSelectMethod != NULL)
		{
			(*EndSelectMethod)(this, EndSelectMethodArg);
		}
	}
#endif
	m_eStatus = status;
	switch(m_eStatus)
	{
	case eViewMode_Rotate:
		m_pVWStatusNameMapper->SetInput("Mode: Rotate");
		break;
	case eViewMode_Pan:
		m_pVWStatusNameMapper->SetInput("Mode: Pan");
		break;
	case eViewMode_Dolly:
		m_pVWStatusNameMapper->SetInput("Mode: Dolly");
		break;
	case eViewMode_Select:
		m_pVWStatusNameMapper->SetInput("Mode: Select");
		break;
	case eViewMode_Query:
		m_pVWStatusNameMapper->SetInput("Mode: Query");
		break;
	case eViewMode_Unknown:
		m_pVWStatusNameMapper->SetInput("Mode: Unknown");
		break;
        default:
            break;
	}
    
    if (m_eStatus == eViewMode_RubberBand_Zoom)
    {
        auto *style = vtkInteractorStyleRubberBandZoom::New();
        auto *lAreaPicker = vtkAreaPicker::New();
        Interactor->SetPicker(lAreaPicker);
        lAreaPicker->Delete();
        Interactor->SetInteractorStyle(style);
        style->Delete();
    } else if (m_eStatus == eViewMode_RubberBand_Select)
    {
        auto *style = vtkInteractorStyleRubberBandPick::New();
        style->StartSelect();
//        auto *lAreaPicker = vtkAreaPicker::New();
//        Interactor->SetPicker(lAreaPicker);
//        lAreaPicker->Delete();
        Interactor->SetInteractorStyle(style);
        style->Delete();
    } else
    {
        vtkInteractorStyleSwitch *style = vtkInteractorStyleSwitch::New();
        style->SetCurrentStyleToTrackballCamera();
        Interactor->SetInteractorStyle(style);
        style->Delete();
    }

    if (Interactor && Interactor->GetInitialized())
		Render();
}

EViewMode_Status sppVtk::GetStatus()
{
	return(m_eStatus);
}

void sppVtk::SetEndSelectMethod( void (*f)(sppVtk *pVW, void *arg))
{
//	if (EndSelectMethod != NULL)
//		(*EndSelectMethod)(this, EndSelectMethodArg);
	EndSelectMethod = f;
}

void sppVtk::SetEndSelectMethodArg( void * pArg)
{
	EndSelectMethodArg = pArg;
}

void sppVtk::SetSelectPointMethod( void (*f)(sppVtk* pRW, vtkPointPicker *pPicker, void *arg))
{
	SelectPointMethod = f;
}

void sppVtk::SetSelectPointMethodArg( void * pArg)
{
	SelectPointMethodArg = pArg;
}

void sppVtk::SetSelectCellMethod( void (*f)(sppVtk* pRW, vtkCellPicker *pPicker, void *arg))
{
	SelectCellMethod = f;
}

void sppVtk::SetSelectCellMethodArg( void * pArg)
{
	SelectCellMethodArg = pArg;
}

void sppVtk::SetSelectPropMethod( void (*f)(sppVtk* pRW, vtkPropPicker *pPicker, void *arg))
{
	SelectPropMethod = f;
}

void sppVtk::SetSelectPropMethodArg( void * pArg)
{
	SelectPropMethodArg = pArg;
}

vtkPointPicker* sppVtk::SelectPoint(int x, int y, double tol)
{
	if (m_pPointPicker == NULL)
		m_pPointPicker = vtkPointPicker::New();

	m_pPointPicker->SetTolerance(tol);

	if (m_pPointPicker->Pick(x, y, 0, Renderer) != 0)
	{
		if (m_pPointPicker->GetPointId() >= 0)
			return(m_pPointPicker);
	}

	return(NULL);
}

vtkPropPicker* sppVtk::SelectProp(int x, int y, double /*tol*/)
{
	if (m_pPropPicker == NULL)
		m_pPropPicker = vtkPropPicker::New();

//	m_pPropPicker->SetTolerance(tol);
	if (m_pPropPicker->Pick(x, y, 0, Renderer) != 0)
	{
		return(m_pPropPicker);
	}
	return(NULL);
}

vtkCellPicker* sppVtk::SelectCell(int x, int y, double tol)
{
	if (m_pCellPicker == NULL)
		m_pCellPicker = vtkCellPicker::New();

	m_pCellPicker->SetTolerance(tol);

	if (m_pCellPicker->Pick(x, y, 0, Renderer) != 0)
	{
		if (m_pCellPicker->GetCellId() >= 0)
			return(m_pCellPicker);
	}

	return(NULL);
}

void sppVtk::AddAdditionalRenderer(double minX, double minY, double maxX, double maxY)
{
	if (m_nAdditionalRenderers >= MAX_ADDITIONAL_RENDERERS)
		return;

	vtkRenderer *newR = vtkRenderer::New();
		newR->SetBackground(0.0,0.0,0.0);
		newR->SetViewport(minX, minY, maxX, maxY); 
		newR->SetInteractive(0);
		this->RenderWindow->AddRenderer(newR);

	m_AdditionalRenderers[m_nAdditionalRenderers++] = newR;
}

vtkRenderer* sppVtk::GetRenderer(int iRenderer)//0: Renderer, >0: the additional renderer
{
	if (iRenderer <= 0)
		return Renderer;

	if (iRenderer > MAX_ADDITIONAL_RENDERERS)
		return NULL;

	return m_AdditionalRenderers[iRenderer - 1];
}

#include "vtkWindowToImageFilter.h"
#include "vtkJPEGWriter.h"
void sppVtk::SaveImage(const char* aFileName)
{
	if (aFileName == NULL)
		return;

	vtkWindowToImageFilter * w2i  = vtkWindowToImageFilter::New(); 
	w2i->SetInput( this->RenderWindow ); 
	w2i->Modified(); 

	vtkJPEGWriter * image  = vtkJPEGWriter::New();
	image->SetInputConnection( w2i->GetOutputPort());
	image->SetFileName(aFileName);
	image->SetQuality ( 100 ); 
	image->Write (); 

	w2i->Delete();
	image->Delete();

}


void sppVtk::SetBkColor(float* bkColor)
{
	SetBkColor(bkColor[0], bkColor[1], bkColor[2]);
}

void sppVtk::SetBkColor(float r, float g, float b)
{
	m_bkColor[0] = r;
	m_bkColor[1] = g;
	m_bkColor[2] = b;

	Renderer->SetBackground(r, g, b);
//copied from gpws.c
  /* find out luminance */
	double lum = 0.30 * m_bkColor[0] 
		       + 0.59 * m_bkColor[1]
			   + 0.11 * m_bkColor[2];
	/* figure out contrast color */
	if ( lum >= 0.5 ) {
		m_ContrastColor[0] = 0.0f;//black
		m_ContrastColor[1] = 0.0f;//black
		m_ContrastColor[2] = 0.0f;//black
	} else {
		m_ContrastColor[0] = 1.0f;//white
		m_ContrastColor[1] = 1.0f;//white
		m_ContrastColor[2] = 1.0f;//white
	}

	OnBkColorChanged();
}

void sppVtk::OnBkColorChanged() 
{
	SetAxesLineColor(GetContrastColor());
//	UpdateGnomonRuler();
}

void sppVtk::SetAxesLineColor(float *color)
{
	double r = color[0];
	double g = color[1];
	double b = color[2];
#if 0
	if (m_GnomonColors == NULL)
		return;
	m_GnomonColors->SetTableValue(0, r, g, b, 1);//x line
	m_GnomonColors->SetTableValue(1, r, g, b, 1);//y line
	m_GnomonColors->SetTableValue(2, r, g, b, 1);//z line
	m_GnomonColors->Modified();
#endif

	if (m_pGnomonLabelX != NULL && m_pGnomonLabelY != NULL && m_pGnomonLabelZ != NULL )
	{
		m_pGnomonLabelX->GetProperty()->SetColor(r,g,b);
		m_pGnomonLabelY->GetProperty()->SetColor(r,g,b);
		m_pGnomonLabelZ->GetProperty()->SetColor(r,g,b);
	}

#if 0
	if (m_pTextAngles != NULL)
	{
		m_pTextAngles->GetProperty()->SetColor(r,g,b);
	}
#endif
}

static vtkActor* CylinderTransform(vtkCylinderSource *cylinder, int iAxis)
{
	assert(cylinder != NULL);
	vtkPolyData *cylinderPD = cylinder->GetOutput();
	assert(cylinderPD != NULL);
	vtkPoints *cylPts = cylinderPD->GetPoints();
	int nPts = cylPts->GetNumberOfPoints();
	vtkPoints *newPts = vtkPoints::New();
		newPts->SetNumberOfPoints(nPts);
	double *f, halfH = 0.5 * cylinder->GetHeight();
	double gXyz[3];
	for (int i = 0; i < nPts; i++)
	{
		f = cylPts->GetPoint(i);
		switch (iAxis)
		{
		case 0:
			gXyz[0] = f[1] + halfH;
			gXyz[1] = f[0];
			gXyz[2] = f[2];
			break;
		case 1:
			gXyz[0] = f[0];
			gXyz[1] = f[1] + halfH;
			gXyz[2] = f[2];
			break;
		case 2:
			gXyz[0] = f[0];
			gXyz[1] = f[2];
			gXyz[2] = f[1] + halfH;
			break;
		}
		newPts->SetPoint(i, gXyz);
	}

	vtkPolyData *aPD = vtkPolyData::New();
		aPD->SetPoints(newPts);
			newPts->Delete();
		aPD->SetPolys(cylinderPD->GetPolys());

	vtkPolyDataMapper *pLM = vtkPolyDataMapper::New();
		pLM->SetInputData(aPD);
			aPD->Delete();

	vtkActor *pA = vtkActor::New();
		pA->SetMapper(pLM);
			pLM->Delete();
		pA->PickableOff();
		switch(iAxis)
		{
		case 0://x axis
			pA->GetProperty()->SetColor(0, 0, 1);
			break;
		case 1://y axis
			pA->GetProperty()->SetColor(0, 1, 0);
			break;
		case 2://z axis
			pA->GetProperty()->SetColor(1, 0, 0);
			break;
		}

	return(pA);
}

static vtkActor* ConeTransform(vtkPolyData *conePD, int iAxis, double length)
{
	assert(conePD != NULL);
	vtkPoints *conePts = conePD->GetPoints();
	int nPts = conePts->GetNumberOfPoints();
	vtkPoints *newPts = vtkPoints::New();
		newPts->SetNumberOfPoints(nPts);
	double *f;
	double gXyz[3];
	for (int i = 0; i < nPts; i++)
	{
		f = conePts->GetPoint(i);
		switch (iAxis)
		{
		case 0:
			gXyz[0] = f[0] + length;
			gXyz[1] = f[1];
			gXyz[2] = f[2];
			break;
		case 1:
			gXyz[0] = -f[1];
			gXyz[1] = f[0] + length;
			gXyz[2] = f[2];
			break;
		case 2:
			gXyz[0] = -f[2];
			gXyz[1] = f[1];
			gXyz[2] = f[0] + length;
			break;
		}
		newPts->SetPoint(i, gXyz);
	}

	vtkPolyData *aPD = vtkPolyData::New();
		aPD->SetPoints(newPts);
			newPts->Delete();
		aPD->SetPolys(conePD->GetPolys());

	vtkPolyDataMapper *pLM = vtkPolyDataMapper::New();
		pLM->SetInputData(aPD);
			aPD->Delete();

	vtkActor *pA = vtkActor::New();
		pA->SetMapper(pLM);
			pLM->Delete();
		pA->PickableOff();
		switch(iAxis)
		{
		case 0://x axis
			pA->GetProperty()->SetColor(0, 0, 1);
			break;
		case 1://y axis
			pA->GetProperty()->SetColor(0, 1, 0);
			break;
		case 2://z axis
			pA->GetProperty()->SetColor(1, 0, 0);
			break;
		}

	return(pA);
}

void sppVtk::ToggleOriginVisible()
{
	if (Renderer == NULL)
		return;

	double bs[6];
	Renderer->ComputeVisiblePropBounds( bs );
	double maxSize = bs[1] - bs[0];
	if (maxSize < (bs[3] - bs[2]))
		maxSize = (bs[3] - bs[2]);
	if (maxSize < (bs[5] - bs[4]))
		maxSize = (bs[5] - bs[4]);

	double actorScale = maxSize * 0.1;

	vtkActor *a;
	if (m_pOriginActors == NULL)
	{
		m_pOriginActors = vtkActorCollection::New();
	//the three lines
		double length = 1.0;
		double coneR = 0.1 * length;
		double coneH = 3 * coneR;

	//a grey sphere at the center
		vtkSphereSource *sphere = vtkSphereSource::New();
			sphere->SetRadius(coneR);
			sphere->SetCenter(0, 0, 0);
			sphere->Update();

		vtkPolyDataMapper *pLM = vtkPolyDataMapper::New();
			pLM->SetInputConnection(sphere->GetOutputPort());
				sphere->Delete();

		vtkActor *pA = vtkActor::New();
			pA->SetMapper(pLM);
				pLM->Delete();
			pA->PickableOff();
			pA->GetProperty()->SetColor(0.9, 0.9, 0.9);
		m_pOriginActors->AddItem(pA);
	//3 axies (b, g, r)
		vtkCylinderSource *cylinder = vtkCylinderSource::New();
			cylinder->CappingOn();
			cylinder->SetRadius(coneR * 0.25);
			cylinder->SetHeight(length);
			cylinder->SetResolution(8);
			cylinder->SetCenter(0, 0, 0);
		cylinder->Update();
	//x
		m_pOriginActors->AddItem(CylinderTransform(cylinder, 0));
	//y
		m_pOriginActors->AddItem(CylinderTransform(cylinder, 1));
	//z
		m_pOriginActors->AddItem(CylinderTransform(cylinder, 2));

		cylinder->Delete();

		vtkConeSource *lCone = vtkConeSource::New() ;
			lCone->SetHeight(coneH) ;
			lCone->SetRadius(coneR) ;
			lCone->SetResolution(8) ;
			lCone->Update();

		vtkPolyData *conePD = lCone->GetOutput();
	//x
		m_pOriginActors->AddItem(ConeTransform(conePD, 0, length));
	//y
		m_pOriginActors->AddItem(ConeTransform(conePD, 1, length));
	//z
		m_pOriginActors->AddItem(ConeTransform(conePD, 2, length));

		lCone->Delete();

		for (m_pOriginActors->InitTraversal(); (a = m_pOriginActors->GetNextActor());)
		{
			a->SetScale(actorScale);
			AddActor(a);
		}
	} else
	{
		for (m_pOriginActors->InitTraversal(); (a = m_pOriginActors->GetNextActor());)
		{
			a->SetScale(actorScale);
			a->SetVisibility(a->GetVisibility() == 1? 0:1);
		}
	}

	Render();
}
