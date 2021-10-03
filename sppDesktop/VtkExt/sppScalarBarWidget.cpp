#define __COMPILING_sppScalarBarWidget_CPP__
#include "sppScalarBarWidget.h"
#undef __COMPILING_sppScalarBarWidget_CPP__

#pragma warning( push, 0 )
#include "vtkCallbackCommand.h"
#include "vtkCoordinate.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkScalarBarActor.h"
#include "sppScalarBarRepresentation.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetEvent.h"
#pragma warning(pop)

#include "sppScalarBarActorAdapter.h"

vtkStandardNewMacro(sppScalarBarWidget);

//-------------------------------------------------------------------------
sppScalarBarWidget::sppScalarBarWidget()
{
    this->EventCallbackCommand->SetCallback(sppScalarBarWidget::ProcessEvents);
	this->Selectable = 0;
	this->Repositionable = 1;

	// Override the subclasses callback to handle the Repositionable flag.
	this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
		vtkWidgetEvent::Move,
		this, sppScalarBarWidget::MoveAction);
}

//-------------------------------------------------------------------------
sppScalarBarWidget::~sppScalarBarWidget()
{
}

//-----------------------------------------------------------------------------
void sppScalarBarWidget::SetRepresentation(sppScalarBarRepresentation *rep)
{
	this->SetWidgetRepresentation(rep);
}

//-----------------------------------------------------------------------------
void sppScalarBarWidget::SetScalarBarActorAdapter(sppScalarBarActorAdapter *actor)
{
	sppScalarBarRepresentation *rep = this->GetScalarBarRepresentation();
	if (!rep)
	{
		this->CreateDefaultRepresentation();
		rep = this->GetScalarBarRepresentation();
	}

	if (rep->GetScalarBarActorAdapter() != actor)
	{
		rep->SetScalarBarActorAdapter(actor);
		this->Modified();
	}
}

//-----------------------------------------------------------------------------
sppScalarBarActorAdapter *sppScalarBarWidget::GetScalarBarActorAdapter()
{
	sppScalarBarRepresentation *rep = this->GetScalarBarRepresentation();
	if (!rep)
		return NULL;

	return rep->GetScalarBarActorAdapter();
}

//-----------------------------------------------------------------------------

	class sppPresentationCB : public vtkCommand
	{
	public:
		static sppPresentationCB *New() { return new sppPresentationCB; }
		virtual void Execute(vtkObject *caller, unsigned long iEvent, void*)
		{
			sppScalarBarRepresentation *rep = reinterpret_cast<sppScalarBarRepresentation*>(caller);
			if (rep)
			{
				switch (iEvent)
				{
				case eScalarBarEvent_UpperConeMoved:
				case eScalarBarEvent_LowerConeMoved:
					{
						//update the display
//						mWidget->OnMouseMove();
						//enable callback
						mWidget->InvokeEvent(iEvent, NULL);
					}
					break;
				default:
					break;
				}
			}
		}

		sppScalarBarWidget *mWidget;
	};


void sppScalarBarWidget::CreateDefaultRepresentation()
{
	if (!this->WidgetRep)
	{
		sppScalarBarRepresentation *rep = sppScalarBarRepresentation::New();
		auto cb = sppPresentationCB::New();
		cb->mWidget = this;
		rep->AddObserver(eScalarBarEvent_UpperConeMoved, cb);
		cb->Delete();
		this->SetRepresentation(rep);
		rep->Delete();
	}
}

//-------------------------------------------------------------------------
void sppScalarBarWidget::SetCursor(int cState)
{
	if (   !this->Repositionable && !this->Selectable
		&& cState == vtkBorderRepresentation::Inside)
	{
		// Don't have a special cursor for the inside if we cannot reposition.
		this->RequestCursorShape(VTK_CURSOR_DEFAULT);
	}
	else
	{
		sppScalarBarRepresentation* rep = this->GetScalarBarRepresentation();
		if (   rep && rep->GetBWActor()->GetVisibility() 
			&& (rep->GetUpperConeSelected() || rep->GetLowerConeSelected()))
		{
			if (rep->GetOrientation() == VTK_ORIENT_HORIZONTAL)
				this->RequestCursorShape(VTK_CURSOR_SIZEWE);
			else
				this->RequestCursorShape(VTK_CURSOR_SIZENS);
		}
		else
			this->Superclass::SetCursor(cState);
	}
}

#include "vtkSmartPointer.h"
//-------------------------------------------------------------------------
void sppScalarBarWidget::MoveAction(vtkAbstractWidget *w)
{
	sppScalarBarWidget *self = reinterpret_cast<sppScalarBarWidget*>(w);
	if (!self)
		return;
	sppScalarBarRepresentation* rep = self->GetScalarBarRepresentation();
	if (!rep)
		return;

	vtkActor2D* barActor = rep->GetScalarBarActor();
	if (!barActor)
		return;

	int X = self->Interactor->GetEventPosition()[0];
	int Y = self->Interactor->GetEventPosition()[1];
	int prevX = self->Interactor->GetLastEventPosition()[0];
	int prevY = self->Interactor->GetLastEventPosition()[1];

	if (X == prevX && Y == prevY)
		return;//no movement

	int *viewPos = self->Interactor->GetRenderWindow()->GetPosition();
	int *viewSize = self->Interactor->GetRenderWindow()->GetSize();
	if (   prevX < viewPos[0] || prevX > (viewPos[0] + viewSize[0])
	    || prevY < viewPos[1] || prevY > (viewPos[1] + viewSize[1]))
		return;//the mouse just enters the view, start interaction at the next move

	if (   X < viewPos[0] || X > (viewPos[0] + viewSize[0])
	    || Y < viewPos[1] || Y > (viewPos[1] + viewSize[1]))
		return;//the mouse moved out of the view, the widget responses to mouse move only when it is inside the view
#if 0
	int deltaX = X - prevX;
	int deltaY = Y - prevY;

	double *barPos = barActor->GetPosition();
	double *barSize = barActor->GetPosition2();

	int iBarPos[2] = {static_cast<int>(barPos[0] * viewSize[0]), static_cast<int>(barPos[1] * viewSize[1])};
	int iBarSize[2] = {static_cast<int>(barSize[0] * viewSize[0]), static_cast<int>(barSize[1] * viewSize[1])};
	if (rep->GetBWActor() && rep->GetBWActor()->GetVisibility())
	{//if it is highlighted
		if ((iBarPos[0] + deltaX) < 0 || (iBarPos[0] + iBarSize[0] + deltaX) > viewSize[0])
		{//going to be out of view, don't allow it to happen
			//move it back
			X = prevX - iBarPos[0];
			self->Interactor->SetEventPosition(X, Y);
		}
	}
#endif
	// The the superclass handle most stuff.
	sppScalarBarWidget::Superclass::MoveAction(w);

	sppScalarBarRepresentation *representation=self->GetScalarBarRepresentation();

	// Handle the case where we suppress widget translation.
	if (   !self->Repositionable
		&& (   representation->GetInteractionState()
		== vtkBorderRepresentation::Inside ) )
	{
		representation->MovingOff();
	}

	representation->SetConeVisibility(representation->GetMoving());
}

void sppScalarBarWidget::ProcessEvents(vtkObject* vtkNotUsed(object),
                                   unsigned long event,
                                   void* clientdata,
                                   void* vtkNotUsed(calldata))
{
    sppScalarBarWidget* self = reinterpret_cast<sppScalarBarWidget *>( clientdata );
	sppScalarBarRepresentation *representation=self->GetScalarBarRepresentation();

	int X = self->Interactor->GetEventPosition()[0];
	int Y = self->Interactor->GetEventPosition()[1];
	double eventPos[] = {static_cast<double>(X), static_cast<double>(Y)};

    //okay, let's do the right thing
    switch(event)
    {
        case vtkCommand::LeftButtonPressEvent:
            self->OnLeftButtonDown();
            representation->OnLeftButtonDown(eventPos);
			self->Render();
			break;
        case vtkCommand::LeftButtonReleaseEvent:
            self->OnLeftButtonUp();
            representation->OnLeftButtonUp(eventPos);
			self->Render();
            break;
        case vtkCommand::MiddleButtonPressEvent:
            self->OnMiddleButtonDown();
            representation->OnMiddleButtonDown(eventPos);
            break;
        case vtkCommand::MiddleButtonReleaseEvent:
            self->OnMiddleButtonUp();
            representation->OnMiddleButtonUp(eventPos);
            break;
        case vtkCommand::RightButtonPressEvent:
            self->OnRightButtonDown();
            representation->OnRightButtonDown(eventPos);
            break;
        case vtkCommand::RightButtonReleaseEvent:
            self->OnRightButtonUp();
            representation->OnRightButtonUp(eventPos);
            break;
        case vtkCommand::MouseMoveEvent:
            representation->OnMouseMove(eventPos);
            self->OnMouseMove();
            break;
    }

	sppScalarBarWidget::Superclass::ProcessEventsHandler(NULL, event, clientdata, NULL);
}

void sppScalarBarWidget::OnLeftButtonDown()
{
   
}

void sppScalarBarWidget::OnLeftButtonUp()
{
}

void sppScalarBarWidget::OnMiddleButtonDown()
{
}

void sppScalarBarWidget::OnMiddleButtonUp()
{
}

void sppScalarBarWidget::OnRightButtonDown()
{
}

void sppScalarBarWidget::OnRightButtonUp()
{
}

void sppScalarBarWidget::OnMouseMove()
{
	sppScalarBarActorAdapter *scalarBarActorI = this->GetScalarBarActorAdapter();
	if (scalarBarActorI)
	{
		sppScalarBarRepresentation *rep = GetScalarBarRepresentation();
		if (rep)
		{//the cones may have been moved, synchronize it with the scalar bar actor
			bool bModifiedA = scalarBarActorI->SetUpperClipValue(rep->GetUpperConeRelativePos());
			bool bModifiedB = scalarBarActorI->SetLowerClipValue(rep->GetLowerConeRelativePos());
			if (bModifiedA || bModifiedB)
				this->Render();
		}
	}
}

