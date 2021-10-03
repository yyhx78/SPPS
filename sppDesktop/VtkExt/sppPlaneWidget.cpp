#define __COMPILING_sppPlaneWidget_CPP__
#include "sppPlaneWidget.h"
#undef __COMPILING_sppPlaneWidget_CPP__

#pragma warning( push, 0 )
#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkConeSource.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkLineSource.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPlaneSource.h"
#include "vtkPlanes.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkRegularPolygonSource.h"
#include "vtkCubeSource.h"
#pragma warning(pop)

vtkStandardNewMacro(sppPlaneWidget);

vtkCxxSetObjectMacro(sppPlaneWidget,PlaneProperty,vtkProperty);

sppPlaneWidget::sppPlaneWidget() : vtkPolyDataSourceWidget()
{
    this->State = sppPlaneWidget::Start;
    this->EventCallbackCommand->SetCallback(sppPlaneWidget::ProcessEvents);
    
    this->NormalToXAxis = 0;
    this->NormalToYAxis = 0;
    this->NormalToZAxis = 0;
    this->Representation = VTK_PLANE_SURFACE;//VTK_PLANE_WIREFRAME;
    
    //Build the representation of the widget
    // Represent the plane
    this->PlaneSource = vtkPlaneSource::New();
    this->PlaneSource->SetXResolution(4);
    this->PlaneSource->SetYResolution(4);

    this->CubeSource = vtkCubeSource::New();
//    this->CubeSource = vtkSphereSource::New();
//    this->CubeSource->SetPhiResolution(10);
//    this->CubeSource->SetThetaResolution(10);
    
    this->PlaneOutline = vtkPolyData::New();
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(4);
    vtkCellArray *outline = vtkCellArray::New();
    outline->InsertNextCell(4);
    outline->InsertCellPoint(0);
    outline->InsertCellPoint(1);
    outline->InsertCellPoint(2);
    outline->InsertCellPoint(3);
    this->PlaneOutline->SetPoints(pts);
    pts->Delete();
    this->PlaneOutline->SetPolys(outline);
    outline->Delete();
    
    this->PlaneMapper = vtkPolyDataMapper::New();
    this->PlaneMapper->SetInputConnection(this->PlaneSource->GetOutputPort());
    this->PlaneActor = vtkActor::New();
    this->PlaneActor->SetMapper(this->PlaneMapper);

    this->CubeMapper = vtkPolyDataMapper::New();
    this->CubeMapper->SetInputConnection(this->CubeSource->GetOutputPort());
    this->CubeActor = vtkActor::New();
    this->CubeActor->SetMapper(this->CubeMapper);
    
    // Create the + plane normal
    this->LineSource = vtkLineSource::New();
    this->LineSource->SetResolution(1);
    this->LineMapper = vtkPolyDataMapper::New();
    this->LineMapper->SetInputConnection(this->LineSource->GetOutputPort());
    this->LineActor = vtkActor::New();
    this->LineActor->SetMapper(this->LineMapper);
    
    this->ConeSource = vtkConeSource::New();
    this->ConeSource->SetResolution(12);
    this->ConeSource->SetAngle(25.0);
    this->ConeSource->SetHeight(0.1);
    this->ConeMapper = vtkPolyDataMapper::New();
    this->ConeMapper->SetInputConnection(this->ConeSource->GetOutputPort());
    this->ConeActor = vtkActor::New();
    this->ConeActor->SetMapper(this->ConeMapper);
    
    // Define the point coordinates
	Radius = 1.0;

    // Create the - plane normal
    yzCircle = vtkRegularPolygonSource::New();
	yzCircle->SetRadius(Radius);
    yzCircle->SetNumberOfSides(50);
    yzCircle->GeneratePolygonOff();
    yzCircle->GeneratePolylineOn();
    yzCircleMapper = vtkPolyDataMapper::New();
    yzCircleMapper->SetInputConnection(yzCircle->GetOutputPort());
    yzCircleActor  = vtkActor::New();
    yzCircleActor->SetMapper(yzCircleMapper);
    
    zxCircle = vtkRegularPolygonSource::New();
	zxCircle->SetRadius(Radius);
    zxCircle->SetNumberOfSides(50);
    zxCircle->GeneratePolygonOff();
    zxCircle->GeneratePolylineOn();
    zxCircleMapper = vtkPolyDataMapper::New();
    zxCircleMapper->SetInputConnection(zxCircle->GetOutputPort());
    zxCircleActor  = vtkActor::New();
    zxCircleActor->SetMapper(zxCircleMapper);
    
    this->Transform = vtkTransform::New();
    
    //Manage the picking stuff    
    this->PlanePicker = vtkCellPicker::New();
    this->PlanePicker->SetTolerance(0.005); //need some fluff
	//disable plane selection, only the cones and the lines can be selected
    this->PlanePicker->AddPickList(this->CubeActor);
    this->PlanePicker->AddPickList(this->PlaneActor);
    this->PlanePicker->AddPickList(this->ConeActor);
    this->PlanePicker->AddPickList(this->LineActor);
    this->PlanePicker->PickFromListOn();
    
    this->circlePicker = vtkCellPicker::New();
    this->circlePicker->SetTolerance(0.005); //need some fluff
    this->circlePicker->AddPickList(this->yzCircleActor);
    this->circlePicker->AddPickList(this->zxCircleActor);
    this->circlePicker->PickFromListOn();
        
    this->LastPickValid = 0;
    
    // Set up the initial properties
    this->CreateDefaultProperties();
    
    this->SelectRepresentation();
    
    // Initial creation of the widget, serves to initialize it
    // Call PlaceWidget() LAST in the constructor as it depends on ivar
    // values.
	this->PlaceWidget(Radius);
}

sppPlaneWidget::~sppPlaneWidget()
{
    this->PlaneActor->Delete();
    this->PlaneMapper->Delete();
    this->PlaneSource->Delete();
    this->PlaneOutline->Delete();

    this->CubeActor->Delete();
    this->CubeMapper->Delete();
    this->CubeSource->Delete();
    
    this->ConeActor->Delete();
    this->ConeMapper->Delete();
    this->ConeSource->Delete();
    
    this->LineActor->Delete();
    this->LineMapper->Delete();
    this->LineSource->Delete();
    
    
    yzCircle->Delete();
    yzCircleMapper->Delete();
    yzCircleActor->Delete();
    
    zxCircle->Delete();
    zxCircleMapper->Delete();
    zxCircleActor->Delete();
    
    this->PlanePicker->Delete();
    
    if (this->SelectedHandleProperty)
    {
        this->SelectedHandleProperty->Delete();
        this->SelectedHandleProperty = 0;
    }
    
    if (this->PlaneProperty)
    {
        this->PlaneProperty->Delete();
        this->PlaneProperty = 0;
    }
    
    if (this->SelectedPlaneProperty)
    {
        this->SelectedPlaneProperty->Delete();
        this->SelectedPlaneProperty = 0;
    }
    
    if (this->circleProperty)
    {
        circleProperty->Delete();
        circleProperty = 0;
    }
    
    if (this->selectedCircleProperty)
    {
        selectedCircleProperty->Delete();
        selectedCircleProperty = 0;
    }
    
    this->Transform->Delete();
}

void sppPlaneWidget::SetEnabled(int enabling)
{
    if ( ! this->Interactor )
    {
        vtkErrorMacro(<<"The interactor must be set prior to enabling/disabling widget");
        return;
    }
    
    if ( enabling ) //-----------------------------------------------------------
    {
        vtkDebugMacro(<<"Enabling plane widget");
        
        if ( this->Enabled ) //already enabled, just return
        {
            return;
        }
        
        if ( ! this->CurrentRenderer )
        {
            this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(
                                                                         this->Interactor->GetLastEventPosition()[0],
                                                                         this->Interactor->GetLastEventPosition()[1]));
            if (this->CurrentRenderer == NULL)
            {
                return;
            }
        }
        
        this->Enabled = 1;
        
		if (enabling != 2) //if enabling == 2, mouse events are catched and transfered to EventCallbackCommand directly
		{
			// listen for the following events
			vtkRenderWindowInteractor *i = this->Interactor;
			i->AddObserver(vtkCommand::MouseMoveEvent, this->EventCallbackCommand,
				this->Priority);
			i->AddObserver(vtkCommand::LeftButtonPressEvent,
				this->EventCallbackCommand, this->Priority);
			i->AddObserver(vtkCommand::LeftButtonReleaseEvent,
				this->EventCallbackCommand, this->Priority);
			i->AddObserver(vtkCommand::MiddleButtonPressEvent,
				this->EventCallbackCommand, this->Priority);
			i->AddObserver(vtkCommand::MiddleButtonReleaseEvent,
				this->EventCallbackCommand, this->Priority);
			i->AddObserver(vtkCommand::RightButtonPressEvent,
				this->EventCallbackCommand, this->Priority);
			i->AddObserver(vtkCommand::RightButtonReleaseEvent,
				this->EventCallbackCommand, this->Priority);
		}
        // Add the plane
        this->CurrentRenderer->AddActor(this->PlaneActor);
        this->PlaneActor->SetProperty(this->PlaneProperty);

#if 0//the cube is displayed only when it is selected?
        this->CurrentRenderer->AddActor(this->CubeActor);
        this->CubeActor->SetProperty(this->CubeProperty);
#endif
        // add the normal vector
        this->CurrentRenderer->AddActor(this->LineActor);
        this->LineActor->SetProperty(this->HandleProperty);
        this->CurrentRenderer->AddActor(this->ConeActor);
        this->ConeActor->SetProperty(this->HandleProperty);
        
        CurrentRenderer->AddActor(yzCircleActor);
        yzCircleActor->SetProperty(circleProperty);
        CurrentRenderer->AddActor(zxCircleActor);
        zxCircleActor->SetProperty(circleProperty);
        
        this->SelectRepresentation();
        this->InvokeEvent(vtkCommand::EnableEvent,NULL);
    }
    else //disabling----------------------------------------------------------
    {
        vtkDebugMacro(<<"Disabling plane widget");
        
        if ( ! this->Enabled ) //already disabled, just return
        {
            return;
        }
        
        this->Enabled = 0;
        
        // don't listen for events any more
        this->Interactor->RemoveObserver(this->EventCallbackCommand);
        
        // turn off the plane
        this->CurrentRenderer->RemoveActor(this->PlaneActor);
        this->CurrentRenderer->RemoveActor(this->CubeActor);
        
        // turn off the normal vector
        this->CurrentRenderer->RemoveActor(this->LineActor);
        this->CurrentRenderer->RemoveActor(this->ConeActor);
        
        CurrentRenderer->RemoveActor(yzCircleActor);
        CurrentRenderer->RemoveActor(zxCircleActor);
        
        this->InvokeEvent(vtkCommand::DisableEvent,NULL);

        this->SetCurrentRenderer(NULL);
    }
    
    this->Interactor->Render();
}

void sppPlaneWidget::ProcessEvents(vtkObject* vtkNotUsed(object),
                                   unsigned long event,
                                   void* clientdata,
                                   void* vtkNotUsed(calldata))
{
    sppPlaneWidget* self = reinterpret_cast<sppPlaneWidget *>( clientdata );
    
    //okay, let's do the right thing
    switch(event)
    {
        case vtkCommand::LeftButtonPressEvent:
            self->OnLeftButtonDown();
            break;
        case vtkCommand::LeftButtonReleaseEvent:
            self->OnLeftButtonUp();
            break;
        case vtkCommand::MiddleButtonPressEvent:
            self->OnMiddleButtonDown();
            break;
        case vtkCommand::MiddleButtonReleaseEvent:
            self->OnMiddleButtonUp();
            break;
        case vtkCommand::RightButtonPressEvent:
            self->OnRightButtonDown();
            break;
        case vtkCommand::RightButtonReleaseEvent:
            self->OnRightButtonUp();
            break;
        case vtkCommand::MouseMoveEvent:
            self->OnMouseMove();
            break;
    }
}

void sppPlaneWidget::PositionHandles()
{
    double oldCenter[3];
    this->PlaneSource->GetCenter(oldCenter);
    
    double o[3], pt1[3], pt2[3];
    this->PlaneSource->GetOrigin(o);
    this->PlaneSource->GetPoint1(pt1);
    this->PlaneSource->GetPoint2(pt2);

	//adjust plane size, the center should be kept unchanged
	double do1[3], do2[3];
	for (int i = 0; i < 3; i++)
	{
		do1[i] = pt1[i] - o[i];
		do2[i] = pt2[i] - o[i];
	}
	double lo1 = vtkMath::Norm(do1);
	double lo2 = vtkMath::Norm(do2);
	double scaleF = 0.125;//the plane is supposed to be smaller than the circles
    double xSize = scaleF * Radius/lo1;
    double ySize = scaleF * Radius/lo2;
    
	for (int i = 0; i < 3; i++)
	{
        o[i]   = oldCenter[i] - 0.5 * xSize * do1[i] - 0.5 * ySize * do2[i];
        pt1[i] = oldCenter[i] + 0.5 * xSize * do1[i] - 0.5 * ySize * do2[i];
        pt2[i] = oldCenter[i] - 0.5 * xSize * do1[i] + 0.5 * ySize * do2[i];
	}
    PlaneSource->SetOrigin(o);
	PlaneSource->SetPoint1(pt1);
	PlaneSource->SetPoint2(pt2);
    
    double x[3];
    x[0] = pt1[0] + pt2[0] - o[0];
    x[1] = pt1[1] + pt2[1] - o[1];
    x[2] = pt1[2] + pt2[2] - o[2];
    
    // set up the outline
    if ( this->Representation == VTK_PLANE_OUTLINE )
    {
        this->PlaneOutline->GetPoints()->SetPoint(0,o);
        this->PlaneOutline->GetPoints()->SetPoint(1,pt1);
        this->PlaneOutline->GetPoints()->SetPoint(2,x);
        this->PlaneOutline->GetPoints()->SetPoint(3,pt2);
        this->PlaneOutline->Modified();
    }
    this->SelectRepresentation();
    
    // Create the normal vector
    double center[3];
    this->PlaneSource->GetCenter(center);
    this->PlaneSource->GetNormal(this->Normal);
    vtkMath::Normalize(this->Normal);
    
    double p1[3], p2[3];
	for (int i = 0; i < 3; i++)
	{
	    p1[i] = center[i] + Radius * this->Normal[i];
	    p2[i] = center[i] - Radius * this->Normal[i];
	}
    this->LineSource->SetPoint1(p1);
    this->LineSource->SetPoint2(p2);

	double coneHeight = scaleF * Radius;
	double coneCenter[3], reverseN[3];
	for (int i = 0; i < 3; i++)
	{
		reverseN[i] = -Normal[i];
		coneCenter[i] = p2[i] + coneHeight * 0.5 * Normal[i];
	}

    double cubeLen = scaleF * Radius;
    CubeSource->SetCenter(center);
#if 1
    CubeSource->SetXLength(cubeLen);
    CubeSource->SetYLength(cubeLen);
    CubeSource->SetZLength(cubeLen);
#else
    CubeSource->SetRadius(cubeLen);
#endif
    
    this->ConeSource->SetCenter(coneCenter);
    this->ConeSource->SetHeight(coneHeight);
    this->ConeSource->SetRadius(0.06 * Radius);
	//make the cone visible on the cut away side
    this->ConeSource->SetDirection(reverseN);
    
    double *xyNormal = Normal;
    
    double yzNormal[3], delta[3];
    for (int i = 0; i < 3; i++)
        delta[i] = pt1[i] - o[i];
    vtkMath::Cross(xyNormal, delta, yzNormal);
    yzCircle->SetCenter(center);
    vtkMath::Normalize(yzNormal);
    yzCircle->SetNormal(yzNormal);
    yzCircle->SetRadius(Radius);
    
    double zxNormal[3];
    vtkMath::Cross(xyNormal, yzNormal, zxNormal);
    zxCircle->SetCenter(center);
    vtkMath::Normalize(zxNormal);
    zxCircle->SetNormal(zxNormal);
    zxCircle->SetRadius(Radius);
}

void sppPlaneWidget::HighlightNormal(int highlight)
{
    if ( highlight )
    {
        this->ValidPick = 1;
        this->PlanePicker->GetPickPosition(this->LastPickPosition);
        this->LineActor->SetProperty(this->SelectedHandleProperty);
        this->ConeActor->SetProperty(this->SelectedHandleProperty);
    }
    else
    {
        this->LineActor->SetProperty(this->HandleProperty);
        this->ConeActor->SetProperty(this->HandleProperty);
    }
}

void sppPlaneWidget::HighlightPlane(int highlight)
{
    if ( highlight )
    {
        this->ValidPick = 1;
        this->PlanePicker->GetPickPosition(this->LastPickPosition);
        this->PlaneActor->SetProperty(this->SelectedPlaneProperty);
        this->CubeActor->SetProperty(this->SelectedPlaneProperty);
    }
    else
    {
        this->PlaneActor->SetProperty(this->PlaneProperty);
        this->CubeActor->SetProperty(this->PlaneProperty);
    }
}

void sppPlaneWidget::HighlightCircle(int iCircle, int highlight)
{
    if ( highlight )
    {
        this->ValidPick = 1;
		if (iCircle == 1)
		{
	        yzCircleActor->SetProperty(selectedCircleProperty);
		} else
		{
	        zxCircleActor->SetProperty(selectedCircleProperty);
		}
		circlePicker->GetPickPosition(this->LastPickPosition);
    }
    else
    {
		if (iCircle == 1)
		{
	        yzCircleActor->SetProperty(circleProperty);
		} else
		{
	        zxCircleActor->SetProperty(circleProperty);
		}
    }
}

void sppPlaneWidget::OnLeftButtonDown()
{
    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];
    
    // Okay, make sure that the pick is in the current renderer
    if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
        this->State = sppPlaneWidget::Outside;
        return;
    }
    
    // Okay, we can process this. Try to pick handles first;
    // if no handles picked, then try to pick the plane.
    vtkAssemblyPath *path;
    this->PlanePicker->Pick(X,Y,0.0,this->CurrentRenderer);
    path = this->PlanePicker->GetPath();
    if (path != NULL )
    {
        vtkProp *prop = path->GetFirstNode()->GetViewProp();
		if (prop == PlaneActor || prop == CubeActor)
		{
	        this->State = sppPlaneWidget::Moving;
	        this->HighlightPlane(1);
		} else
		{
	        this->State = sppPlaneWidget::Pushing;
	        this->HighlightNormal(1);
		}
    }
    
    if (path == NULL)
    {//if nothing has been picked
        this->circlePicker->Pick(X, Y, 0.0, this->CurrentRenderer);
        path = this->circlePicker->GetPath();
        if (path != NULL)
        {
            vtkProp *prop = path->GetFirstNode()->GetViewProp();
            if ( prop == this->yzCircleActor)
            {
				HighlightCircle(1, 1);
                this->State = sppPlaneWidget::xRotating;
            }
            else
            {
				HighlightCircle(2, 1);
                this->State = sppPlaneWidget::yRotating;
            }
        }
    }

    if (path == NULL)
    {//if nothing has been picked
        this->State = sppPlaneWidget::Outside;
		return; //don't continue to InvokeEvent, it cause the mouse to freeze.
    }
    
    this->EventCallbackCommand->SetAbortFlag(1);
    this->StartInteraction();
    this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
    this->Interactor->Render();
}

void sppPlaneWidget::OnLeftButtonUp()
{
    if ( this->State == sppPlaneWidget::Outside ||
        this->State == sppPlaneWidget::Start )
    {
        return;
    }
    
    this->State = sppPlaneWidget::Start;
	this->HighlightCircle(1, 0);
	this->HighlightCircle(2, 0);
    this->HighlightPlane(0);
    this->HighlightNormal(0);
    this->SizeHandles();
    
    yzCircleActor->SetProperty(circleProperty);
    zxCircleActor->SetProperty(circleProperty);
    
    this->EventCallbackCommand->SetAbortFlag(1);
    this->EndInteraction();
    this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
    this->Interactor->Render();
}

void sppPlaneWidget::OnMiddleButtonDown()
{
    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];
    
    // Okay, make sure that the pick is in the current renderer
    if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
        this->State = sppPlaneWidget::Outside;
        return;
    }
    
    // Okay, we can process this. If anything is picked, then we
    // can start pushing the plane.
    vtkAssemblyPath *path;
    {
        this->PlanePicker->Pick(X,Y,0.0,this->CurrentRenderer);
        path = this->PlanePicker->GetPath();
        if ( path == NULL ) //nothing picked
        {
            this->State = sppPlaneWidget::Outside;
            return;
        }
        else
        {
            this->State = sppPlaneWidget::Pushing;
            this->HighlightNormal(1);
        }
    }
    
    this->EventCallbackCommand->SetAbortFlag(1);
    this->StartInteraction();
    this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
    this->Interactor->Render();
}

void sppPlaneWidget::OnMiddleButtonUp()
{
    if ( this->State == sppPlaneWidget::Outside ||
        this->State == sppPlaneWidget::Start )
    {
        return;
    }
    
    this->State = sppPlaneWidget::Start;
	this->HighlightCircle(1, 0);
	this->HighlightCircle(2, 0);
    this->HighlightPlane(0);
    this->HighlightNormal(0);
    this->SizeHandles();
    yzCircleActor->SetProperty(circleProperty);
    zxCircleActor->SetProperty(circleProperty);
    
    this->EventCallbackCommand->SetAbortFlag(1);
    this->EndInteraction();
    this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
    this->Interactor->Render();
}

void sppPlaneWidget::OnRightButtonDown()
{
    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];
    
    // Okay, make sure that the pick is in the current renderer
    if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
        this->State = sppPlaneWidget::Outside;
        return;
    }
    
    // Okay, we can process this. Try to pick handles first;
    // if no handles picked, then pick the bounding box.
    vtkAssemblyPath *path;
    {
        this->PlanePicker->Pick(X,Y,0.0,this->CurrentRenderer);
        path = this->PlanePicker->GetPath();
        if ( path == NULL )
        {
            this->State = sppPlaneWidget::Outside;
            return;
        }
        else
        {
            this->State = sppPlaneWidget::Scaling;
            this->HighlightPlane(1);
        }
    }
    
    this->EventCallbackCommand->SetAbortFlag(1);
    this->StartInteraction();
    this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
    this->Interactor->Render();
}

void sppPlaneWidget::OnRightButtonUp()
{
    if ( this->State == sppPlaneWidget::Outside ||
        this->State == sppPlaneWidget::Start )
    {
        return;
    }
    
    this->State = sppPlaneWidget::Start;
	this->HighlightCircle(1, 0);
	this->HighlightCircle(2, 0);
    this->HighlightPlane(0);
    this->SizeHandles();
    
    yzCircleActor->SetProperty(circleProperty);
    zxCircleActor->SetProperty(circleProperty);
    
    this->EventCallbackCommand->SetAbortFlag(1);
    this->EndInteraction();
    this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
    this->Interactor->Render();
}

void sppPlaneWidget::OnMouseMove()
{
    // See whether we're active
    if ( this->State == sppPlaneWidget::Outside ||
        this->State == sppPlaneWidget::Start )
    {
        return;
    }
    
    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];
    
    // Do different things depending on state
    // Calculations everybody does
    double focalPoint[4], pickPoint[4], prevPickPoint[4];
    double z, vpn[3];
    
    vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
    if ( !camera )
    {
        return;
    }
    
    // Compute the two points defining the motion vector
    this->ComputeWorldToDisplay(this->LastPickPosition[0],
                                this->LastPickPosition[1],
                                this->LastPickPosition[2], focalPoint);
    z = focalPoint[2];
    this->ComputeDisplayToWorld(
                                double(this->Interactor->GetLastEventPosition()[0]),
                                double(this->Interactor->GetLastEventPosition()[1]),
                                z, prevPickPoint);
    this->ComputeDisplayToWorld(double(X), double(Y), z, pickPoint);
    
    // Process the motion
    if ( this->State == sppPlaneWidget::Moving )
    {
        this->MoveOrigin(prevPickPoint, pickPoint);
    }
    else if ( this->State == sppPlaneWidget::Scaling )
    {
        this->Scale(prevPickPoint, pickPoint, X, Y);
    }
    else if ( this->State == sppPlaneWidget::Pushing )
    {
        this->Push(prevPickPoint, pickPoint);
    }
    else if ( this->State == sppPlaneWidget::Rotating )
    {
        camera->GetViewPlaneNormal(vpn);
        this->Rotate(X, Y, prevPickPoint, pickPoint, vpn);
    }
    else if ( this->State == sppPlaneWidget::xRotating )
    {
        this->circleRotate(prevPickPoint, pickPoint, yzCircle->GetCenter(), yzCircle->GetNormal());
    }
    else if ( this->State == sppPlaneWidget::yRotating )
    {
        this->circleRotate(prevPickPoint, pickPoint, zxCircle->GetCenter(), zxCircle->GetNormal());
    }
    else if ( this->State == sppPlaneWidget::Spinning )
    {
        this->Spin(prevPickPoint, pickPoint);
    }
    
    
    // Interact, if desired
    this->EventCallbackCommand->SetAbortFlag(1);
    this->InvokeEvent(vtkCommand::InteractionEvent,NULL);
    
    this->Interactor->Render();
}

void sppPlaneWidget::MoveOrigin(double *p1, double *p2)
{
    //Get the plane definition
    double *o = this->PlaneSource->GetOrigin();
    double *pt1 = this->PlaneSource->GetPoint1();
    double *pt2 = this->PlaneSource->GetPoint2();
    
	double projP1[3], projP2[3];
	vtkPlane::ProjectPoint(p1, o, PlaneSource->GetNormal(), projP1);
	vtkPlane::ProjectPoint(p2, o, PlaneSource->GetNormal(), projP2);
    
    double point1[3], point2[3], origin[3];
    for (int i = 0; i < 3; i++)
    {
        point1[i] = pt1[i] + projP2[i] - projP1[i];
        point2[i] = pt2[i] + projP2[i] - projP1[i];
        origin[i] = o[i] + projP2[i] - projP1[i];
    }
    
    this->PlaneSource->SetOrigin(origin);
    this->PlaneSource->SetPoint1(point1);
    this->PlaneSource->SetPoint2(point2);
    this->PlaneSource->Update();
    
    this->PositionHandles();
}

void sppPlaneWidget::MovePoint1(double *p1, double *p2)
{
    //Get the plane definition
    double *o = this->PlaneSource->GetOrigin();
    double *pt1 = this->PlaneSource->GetPoint1();
    double *pt2 = this->PlaneSource->GetPoint2();
    
    //Get the vector of motion
    double v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];
    
    // Need the point opposite the origin (pt3)
    double pt3[3];
    pt3[0] = o[0] + (pt1[0] - o[0]) + (pt2[0] - o[0]);
    pt3[1] = o[1] + (pt1[1] - o[1]) + (pt2[1] - o[1]);
    pt3[2] = o[2] + (pt1[2] - o[2]) + (pt2[2] - o[2]);
    
    // Define vectors from point pt2
    double p32[3], p02[3];
    p02[0] = o[0] - pt2[0];
    p02[1] = o[1] - pt2[1];
    p02[2] = o[2] - pt2[2];
    p32[0] = pt3[0] - pt2[0];
    p32[1] = pt3[1] - pt2[1];
    p32[2] = pt3[2] - pt2[2];
    
    double vN = vtkMath::Norm(v);
    double n02 = vtkMath::Norm(p02);
    double n32 = vtkMath::Norm(p32);
    
    // Project v onto these vector to determine the amount of motion
    // Scale it by the relative size of the motion to the vector length
    double d1 = (vN/n02) * vtkMath::Dot(v,p02) / (vN*n02);
    double d2 = (vN/n32) * vtkMath::Dot(v,p32) / (vN*n32);
    
    double point1[3], origin[3];
    for (int i=0; i<3; i++)
    {
        origin[i] = pt2[i] + (1.0+d1)*p02[i];
        point1[i] = pt2[i] + (1.0+d1)*p02[i] + (1.0+d2)*p32[i];
    }
    
    this->PlaneSource->SetOrigin(origin);
    this->PlaneSource->SetPoint1(point1);
    this->PlaneSource->Update();
    
    this->PositionHandles();
}

void sppPlaneWidget::MovePoint2(double *p1, double *p2)
{
    //Get the plane definition
    double *o = this->PlaneSource->GetOrigin();
    double *pt1 = this->PlaneSource->GetPoint1();
    double *pt2 = this->PlaneSource->GetPoint2();
    
    //Get the vector of motion
    double v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];
    
    // The point opposite point2 (pt1) stays fixed
    double pt3[3];
    pt3[0] = o[0] + (pt1[0] - o[0]) + (pt2[0] - o[0]);
    pt3[1] = o[1] + (pt1[1] - o[1]) + (pt2[1] - o[1]);
    pt3[2] = o[2] + (pt1[2] - o[2]) + (pt2[2] - o[2]);
    
    // Define vectors from point pt1
    double p01[3], p31[3];
    p31[0] = pt3[0] - pt1[0];
    p31[1] = pt3[1] - pt1[1];
    p31[2] = pt3[2] - pt1[2];
    p01[0] = o[0] - pt1[0];
    p01[1] = o[1] - pt1[1];
    p01[2] = o[2] - pt1[2];
    
    double vN = vtkMath::Norm(v);
    double n31 = vtkMath::Norm(p31);
    double n01 = vtkMath::Norm(p01);
    
    // Project v onto these vector to determine the amount of motion
    // Scale it by the relative size of the motion to the vector length
    double d1 = (vN/n31) * vtkMath::Dot(v,p31) / (vN*n31);
    double d2 = (vN/n01) * vtkMath::Dot(v,p01) / (vN*n01);
    
    double point2[3], origin[3];
    for (int i=0; i<3; i++)
    {
        point2[i] = pt1[i] + (1.0+d1)*p31[i] + (1.0+d2)*p01[i];
        origin[i] = pt1[i] + (1.0+d2)*p01[i];
    }
    
    this->PlaneSource->SetOrigin(origin);
    this->PlaneSource->SetPoint2(point2);
    this->PlaneSource->Update();
    
    this->PositionHandles();
}

void sppPlaneWidget::MovePoint3(double *p1, double *p2)
{
    //Get the plane definition
    double *o = this->PlaneSource->GetOrigin();
    double *pt1 = this->PlaneSource->GetPoint1();
    double *pt2 = this->PlaneSource->GetPoint2();
    
    //Get the vector of motion
    double v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];
    
    // Define vectors from point pt3
    double p10[3], p20[3];
    p10[0] = pt1[0] - o[0];
    p10[1] = pt1[1] - o[1];
    p10[2] = pt1[2] - o[2];
    p20[0] = pt2[0] - o[0];
    p20[1] = pt2[1] - o[1];
    p20[2] = pt2[2] - o[2];
    
    double vN = vtkMath::Norm(v);
    double n10 = vtkMath::Norm(p10);
    double n20 = vtkMath::Norm(p20);
    
    // Project v onto these vector to determine the amount of motion
    // Scale it by the relative size of the motion to the vector length
    double d1 = (vN/n10) * vtkMath::Dot(v,p10) / (vN*n10);
    double d2 = (vN/n20) * vtkMath::Dot(v,p20) / (vN*n20);
    
    double point1[3], point2[3];
    for (int i=0; i<3; i++)
    {
        point1[i] = o[i] + (1.0+d1)*p10[i];
        point2[i] = o[i] + (1.0+d2)*p20[i];
    }
    
    this->PlaneSource->SetPoint1(point1);
    this->PlaneSource->SetPoint2(point2);
    this->PlaneSource->Update();
    
    this->PositionHandles();
}

void sppPlaneWidget::Rotate(int X, int Y, double *p1, double *p2, double *vpn)
{
    double *o = this->PlaneSource->GetOrigin();
    double *pt1 = this->PlaneSource->GetPoint1();
    double *pt2 = this->PlaneSource->GetPoint2();
    double *center = this->PlaneSource->GetCenter();
    
    double v[3]; //vector of motion
    double axis[3]; //axis of rotation
    double theta; //rotation angle
    
    // mouse motion vector in world space
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];
    
    // Create axis of rotation and angle of rotation
    vtkMath::Cross(vpn,v,axis);
    if ( vtkMath::Normalize(axis) == 0.0 )
    {
        return;
    }
    int *size = this->CurrentRenderer->GetSize();
    int Px = this->Interactor->GetLastEventPosition()[0];
    int Py = this->Interactor->GetLastEventPosition()[1];
    double l2 = (X - Px) * (X - Px) + (Y - Py)* (Y - Py);
    theta = 360.0 * sqrt(l2/(size[0]*size[0]+size[1]*size[1]));
    
    //Manipulate the transform to reflect the rotation
    this->Transform->Identity();
    this->Transform->Translate(center[0],center[1],center[2]);
    this->Transform->RotateWXYZ(theta,axis);
    this->Transform->Translate(-center[0],-center[1],-center[2]);
    
    //Set the corners
    double oNew[3], pt1New[3], pt2New[3];
    this->Transform->TransformPoint(o,oNew);
    this->Transform->TransformPoint(pt1,pt1New);
    this->Transform->TransformPoint(pt2,pt2New);
    
    this->PlaneSource->SetOrigin(oNew);
    this->PlaneSource->SetPoint1(pt1New);
    this->PlaneSource->SetPoint2(pt2New);
    this->PlaneSource->Update();
    
    this->PositionHandles();
}

void sppPlaneWidget::circleRotate(double *p1, double *p2, double *center, double *normal)
{
    if (p1 == NULL || p2 == NULL || center == NULL || normal == NULL)
        return;
    
    //project p1 and p2 onto the circle plane
    double proj_p1[3], proj_p2[3];
	vtkPlane::ProjectPoint(p1, center, normal, proj_p1);
	vtkPlane::ProjectPoint(p2, center, normal, proj_p2);

    double delta1[3], delta2[3];
    for (int i = 0; i < 3; i++)
    {
        delta1[i] = proj_p1[i] - center[i];
        delta2[i] = proj_p2[i] - center[i];
    }
    
    double cross[3];
    vtkMath::Cross(delta1, delta2, cross);
    double norm = vtkMath::Norm(cross);
    if (norm == 0.0)
        return;//no rotation

    double sign = vtkMath::Dot(normal, cross);
    
    double theta;
    theta = asin(norm/(vtkMath::Norm(delta1) * vtkMath::Norm(delta2)));
    theta = vtkMath::DegreesFromRadians(theta);
    if (sign < 0)
        theta = -theta;
    
    double *axis = normal;
    
    //Manipulate the transform to reflect the rotation
    this->Transform->Identity();
    this->Transform->Translate(center[0],center[1],center[2]);
    this->Transform->RotateWXYZ(theta,axis);
    this->Transform->Translate(-center[0],-center[1],-center[2]);
    
    //Set the corners
    double *o = this->PlaneSource->GetOrigin();
    double *pt1 = this->PlaneSource->GetPoint1();
    double *pt2 = this->PlaneSource->GetPoint2();
    
    double oNew[3], pt1New[3], pt2New[3];
    this->Transform->TransformPoint(o,oNew);
    this->Transform->TransformPoint(pt1,pt1New);
    this->Transform->TransformPoint(pt2,pt2New);
    
    this->PlaneSource->SetOrigin(oNew);
    this->PlaneSource->SetPoint1(pt1New);
    this->PlaneSource->SetPoint2(pt2New);
    this->PlaneSource->Update();
    
    this->PositionHandles();
}

void sppPlaneWidget::Spin(double *p1, double *p2)
{
    // Mouse motion vector in world space
    double v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];
    
    double* normal = this->PlaneSource->GetNormal();
    // Axis of rotation
    double axis[3] = { normal[0], normal[1], normal[2] };
    vtkMath::Normalize(axis);
    
    double *o = this->PlaneSource->GetOrigin();
    double *pt1 = this->PlaneSource->GetPoint1();
    double *pt2 = this->PlaneSource->GetPoint2();
    double *center = this->PlaneSource->GetCenter();
    
    // Radius vector (from center to cursor position)
    double rv[3] = {p2[0] - center[0],
        p2[1] - center[1],
        p2[2] - center[2]};
    
    // Distance between center and cursor location
    double rs = vtkMath::Normalize(rv);
    
    // Spin direction
    double ax_cross_rv[3];
    vtkMath::Cross(axis,rv,ax_cross_rv);
    
    // Spin angle
    double theta = vtkMath::DegreesFromRadians( vtkMath::Dot( v, ax_cross_rv ) / rs );
    
    // Manipulate the transform to reflect the rotation
    this->Transform->Identity();
    this->Transform->Translate(center[0],center[1],center[2]);
    this->Transform->RotateWXYZ(theta,axis);
    this->Transform->Translate(-center[0],-center[1],-center[2]);
    
    //Set the corners
    double oNew[3], pt1New[3], pt2New[3];
    this->Transform->TransformPoint(o,oNew);
    this->Transform->TransformPoint(pt1,pt1New);
    this->Transform->TransformPoint(pt2,pt2New);
    
    this->PlaneSource->SetOrigin(oNew);
    this->PlaneSource->SetPoint1(pt1New);
    this->PlaneSource->SetPoint2(pt2New);
    this->PlaneSource->Update();
    
    this->PositionHandles();
}

// Loop through all points and translate them
void sppPlaneWidget::Translate(double *p1, double *p2)
{
    double *normal = PlaneSource->GetNormal();
    
    //Get the motion vector
    double v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];
    
    //allow only normal direction move
    double dist = vtkMath::Dot(normal, v);
    
    //int res = this->PlaneSource->GetXResolution();
    double *o = this->PlaneSource->GetOrigin();
    double *pt1 = this->PlaneSource->GetPoint1();
    double *pt2 = this->PlaneSource->GetPoint2();
    
    double delta;
    double origin[3], point1[3], point2[3];
    for (int i=0; i<3; i++)
    {
        delta = dist * normal[i];
        origin[i] = o[i] + delta;
        point1[i] = pt1[i] + delta;
        point2[i] = pt2[i] + delta;
    }
    
    this->PlaneSource->SetOrigin(origin);
    this->PlaneSource->SetPoint1(point1);
    this->PlaneSource->SetPoint2(point2);
    this->PlaneSource->Update();
    
    this->PositionHandles();
}

void sppPlaneWidget::Scale(double *p1, double *p2, int vtkNotUsed(X), int Y)
{
    //Get the motion vector
    double v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];
    
    //int res = this->PlaneSource->GetXResolution();
    double *o = this->PlaneSource->GetOrigin();
    double *pt1 = this->PlaneSource->GetPoint1();
    double *pt2 = this->PlaneSource->GetPoint2();
    
    double center[3];
    center[0] = 0.5 * ( pt1[0] + pt2[0] );
    center[1] = 0.5 * ( pt1[1] + pt2[1] );
    center[2] = 0.5 * ( pt1[2] + pt2[2] );
    
    // Compute the scale factor
    double sf =
    vtkMath::Norm(v) / sqrt(vtkMath::Distance2BetweenPoints(pt1,pt2));
    if ( Y > this->Interactor->GetLastEventPosition()[1] )
    {
        sf = 1.0 + sf;
    }
    else
    {
        sf = 1.0 - sf;
    }
    
    // Move the corner points
    double origin[3], point1[3], point2[3];
    for (int i=0; i<3; i++)
    {
        origin[i] = sf * (o[i] - center[i]) + center[i];
        point1[i] = sf * (pt1[i] - center[i]) + center[i];
        point2[i] = sf * (pt2[i] - center[i]) + center[i];
    }
    
    this->PlaneSource->SetOrigin(origin);
    this->PlaneSource->SetPoint1(point1);
    this->PlaneSource->SetPoint2(point2);
    this->PlaneSource->Update();
    
    this->PositionHandles();
}

void sppPlaneWidget::Push(double *p1, double *p2)
{
    //Get the motion vector
    double v[3];
    v[0] = p2[0] - p1[0];
    v[1] = p2[1] - p1[1];
    v[2] = p2[2] - p1[2];
    
    this->PlaneSource->Push( vtkMath::Dot(v,this->Normal) );
    this->PlaneSource->Update();
    this->PositionHandles();
}

void sppPlaneWidget::CreateDefaultProperties()
{
    // Handle properties
    this->HandleProperty = vtkProperty::New();
    this->HandleProperty->SetColor(0,0,0);
    this->HandleProperty->SetLineWidth(2);
    
    this->SelectedHandleProperty = vtkProperty::New();
    this->SelectedHandleProperty->SetColor(1,0,0);
    this->SelectedHandleProperty->SetLineWidth(3);
    
    // Plane properties
    this->PlaneProperty = vtkProperty::New();
    this->PlaneProperty->SetOpacity(0.1);
    this->PlaneProperty->SetColor(0.0,0.0,0.0);
    
    this->SelectedPlaneProperty = vtkProperty::New();
    this->SelectRepresentation();
    this->SelectedPlaneProperty->SetColor(1.0,0.0,0.0);
    
    circleProperty = vtkProperty::New();
	circleProperty->SetColor(0, 0, 0);
    circleProperty->SetLineWidth(2);
    
    selectedCircleProperty = vtkProperty::New();
    selectedCircleProperty->SetColor(1, 0, 0);
    selectedCircleProperty->SetLineWidth(3.0);
}

void sppPlaneWidget::PlaceWidget(double *)//bs)
{
//	bs;
}

void sppPlaneWidget::PlaceWidget(double radius)
{
	SetRadius(radius);

    this->PlaneSource->Update();

    // Position the handles at the end of the planes
    this->PositionHandles();
    
    // Set the radius on the sphere handles
    this->SizeHandles();
}

void sppPlaneWidget::SelectRepresentation()
{
    if ( ! this->CurrentRenderer )
    {
        return;
    }
    
    if ( this->Representation == VTK_PLANE_OFF )
    {
        this->CurrentRenderer->RemoveActor(this->PlaneActor);
        this->CurrentRenderer->RemoveActor(this->CubeActor);
    }
    else if ( this->Representation == VTK_PLANE_OUTLINE )
    {
        this->CurrentRenderer->RemoveActor(this->PlaneActor);
        this->CurrentRenderer->AddActor(this->PlaneActor);
        this->PlaneMapper->SetInputData( this->PlaneOutline );
        this->PlaneActor->GetProperty()->SetRepresentationToWireframe();

        this->CurrentRenderer->RemoveActor(this->CubeActor);
        this->CurrentRenderer->AddActor(this->CubeActor);
        this->CubeMapper->SetInputConnection( this->CubeSource->GetOutputPort() );
        this->CubeActor->GetProperty()->SetRepresentationToWireframe();
    }
    else if ( this->Representation == VTK_PLANE_SURFACE )
    {
        this->CurrentRenderer->RemoveActor(this->PlaneActor);
        this->CurrentRenderer->AddActor(this->PlaneActor);
        this->PlaneMapper->SetInputConnection( this->PlaneSource->GetOutputPort() );
        this->PlaneActor->GetProperty()->SetRepresentationToSurface();

        this->CurrentRenderer->RemoveActor(this->CubeActor);
        this->CurrentRenderer->AddActor(this->CubeActor);
        this->CubeMapper->SetInputConnection( this->CubeSource->GetOutputPort() );
        this->CubeActor->GetProperty()->SetRepresentationToSurface();
    }
    else //( this->Representation == VTK_PLANE_WIREFRAME )
    {
        this->CurrentRenderer->RemoveActor(this->PlaneActor);
        this->CurrentRenderer->AddActor(this->PlaneActor);
        this->PlaneMapper->SetInputConnection( this->PlaneSource->GetOutputPort() );
        this->PlaneActor->GetProperty()->SetRepresentationToWireframe();
        this->CurrentRenderer->RemoveActor(this->CubeActor);
        this->CurrentRenderer->AddActor(this->CubeActor);
        this->CubeMapper->SetInputConnection( this->CubeSource->GetOutputPort() );
        this->CubeActor->GetProperty()->SetRepresentationToWireframe();
    }
}

// Description:
// Set/Get the resolution (number of subdivisions) of the plane.
void sppPlaneWidget::SetResolution(int r)
{
    this->PlaneSource->SetXResolution(r); 
    this->PlaneSource->SetYResolution(r); 
}

int sppPlaneWidget::GetResolution()
{ 
    return this->PlaneSource->GetXResolution(); 
}

// Description:
// Set/Get the origin of the plane.
void sppPlaneWidget::SetOrigin(double x, double y, double z) 
{
    this->PlaneSource->SetOrigin(x,y,z);
    this->PositionHandles();
}

void sppPlaneWidget::SetOrigin(double x[3]) 
{
    this->SetOrigin(x[0], x[1], x[2]);
}

double* sppPlaneWidget::GetOrigin() 
{
    return this->PlaneSource->GetOrigin();
}

void sppPlaneWidget::GetOrigin(double xyz[3]) 
{
    this->PlaneSource->GetOrigin(xyz);
}

// Description:
// Set/Get the position of the point defining the first axis of the plane.
void sppPlaneWidget::SetPoint1(double x, double y, double z) 
{
    this->PlaneSource->SetPoint1(x,y,z);
    this->PositionHandles();
}

void sppPlaneWidget::SetPoint1(double x[3]) 
{
    this->SetPoint1(x[0], x[1], x[2]);
}

double* sppPlaneWidget::GetPoint1() 
{
    return this->PlaneSource->GetPoint1();
}

void sppPlaneWidget::GetPoint1(double xyz[3]) 
{
    this->PlaneSource->GetPoint1(xyz);
}

// Description:
// Set/Get the position of the point defining the second axis of the plane.
void sppPlaneWidget::SetPoint2(double x, double y, double z) 
{
    this->PlaneSource->SetPoint2(x,y,z);
    this->PositionHandles();
}

void sppPlaneWidget::SetPoint2(double x[3]) 
{
    this->SetPoint2(x[0], x[1], x[2]);
}

double* sppPlaneWidget::GetPoint2() 
{
    return this->PlaneSource->GetPoint2();
}

void sppPlaneWidget::GetPoint2(double xyz[3]) 
{
    this->PlaneSource->GetPoint2(xyz);
}

// Description:
// Set the center of the plane.
void sppPlaneWidget::SetCenter(double x, double y, double z) 
{
    this->PlaneSource->SetCenter(x, y, z);
    this->PositionHandles();
}

// Description:
// Set the center of the plane.
void sppPlaneWidget::SetCenter(double c[3]) 
{
    this->SetCenter(c[0], c[1], c[2]);
}

// Description:
// Get the center of the plane.
double* sppPlaneWidget::GetCenter() 
{
    return this->PlaneSource->GetCenter();
}

void sppPlaneWidget::GetCenter(double xyz[3]) 
{
    this->PlaneSource->GetCenter(xyz);
}

// Description:
// Set the normal to the plane.
void sppPlaneWidget::SetNormal(double x, double y, double z) 
{
    this->PlaneSource->SetNormal(x, y, z);
    this->PositionHandles();
}

// Description:
// Set the normal to the plane.
void sppPlaneWidget::SetNormal(double n[3]) 
{
    this->SetNormal(n[0], n[1], n[2]);
}

// Description:
// Get the normal to the plane.
double* sppPlaneWidget::GetNormal() 
{
    return this->PlaneSource->GetNormal();
}

void sppPlaneWidget::GetNormal(double xyz[3]) 
{
    this->PlaneSource->GetNormal(xyz);
}

void sppPlaneWidget::GetPolyData(vtkPolyData *pd)
{ 
    pd->ShallowCopy(this->PlaneSource->GetOutput()); 
}

vtkPolyDataAlgorithm *sppPlaneWidget::GetPolyDataAlgorithm()
{
    return this->PlaneSource;
}

void sppPlaneWidget::GetPlane(vtkPlane *plane)
{
    if ( plane == NULL )
    {
        return;
    }
    
    plane->SetNormal(this->GetNormal());
    plane->SetOrigin(this->GetCenter());
}

void sppPlaneWidget::UpdatePlacement(void)
{
    this->PlaneSource->Update();
    this->PositionHandles();
}
