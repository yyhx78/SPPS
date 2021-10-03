#include "sppScalarBarRepresentation.h"

#pragma warning( push, 0 )
#include "vtkObjectFactory.h"
#include "vtkPropCollection.h"
#include "vtkSmartPointer.h"
#include "vtkTextProperty.h"
#include "vtkNew.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkOpenGLPolyDataMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkRenderer.h"
#include "vtkActor2D.h"
#include "vtkScalarBarActor.h" //VTK_ORIENT_HORIZONTAL
#pragma warning(pop)

#include <algorithm>

#include "sppScalarBarActorAdapter.h"

//=============================================================================
vtkStandardNewMacro(sppScalarBarRepresentation);
//-----------------------------------------------------------------------------
sppScalarBarRepresentation::sppScalarBarRepresentation()
{
	m_bLeftMouseDown = false;
	SetConstantWidth(false);

	this->ProportionalResizeOff();
	this->PositionCoordinate->SetValue(0.82, 0.1);
	this->Position2Coordinate->SetValue(0.17, 0.8);

	this->ScalarBarActorAdapter = NULL;

	{//setting on the cones
		SetConeColor(0.0, 0.0, 0.0);
		SetSelectedConeColor(1.0, 1.0, 0.0);

		{//Upper cone
			UpperConeSelected = false;

			UpperConePos[0] = 0.0;
			UpperConePos[1] = 0.0;
			UpperConeRelativePos = 1.0;//supposed to be in [0, 1]
		}
		{//Lower cone
			LowerConeSelected = false;

			LowerConePos[0] = 0.0;
			LowerConePos[1] = 0.0;
			LowerConeRelativePos = 0.0;//supposed to be in [0, 1]
		}
	}

	this->SetShowBorder(vtkBorderRepresentation::BORDER_ACTIVE);

}

//-----------------------------------------------------------------------------
sppScalarBarRepresentation::~sppScalarBarRepresentation()
{
	this->SetScalarBarActorAdapter(NULL);
}

//-----------------------------------------------------------------------------
vtkActor2D* sppScalarBarRepresentation::GetScalarBarActor()
{
	if (ScalarBarActorAdapter == NULL)
		return NULL;
	else
		return ScalarBarActorAdapter->GetActor2D();
}

//-----------------------------------------------------------------------------
void sppScalarBarRepresentation::SetScalarBarActorAdapter(sppScalarBarActorAdapter* actor)
{
	if (this->ScalarBarActorAdapter != actor)
	{
		if (this->ScalarBarActorAdapter != NULL)
			this->ScalarBarActorAdapter->Delete();

		ScalarBarActorAdapter = actor;

		if (ScalarBarActorAdapter != NULL)
			ScalarBarActorAdapter->Register(this);
	}
}

//-----------------------------------------------------------------------------
void sppScalarBarRepresentation::SetOrientation(int orientation)
{
	if (this->ScalarBarActorAdapter)
	{
		this->ScalarBarActorAdapter->SetOrientation(orientation);
	}
}

//-----------------------------------------------------------------------------
int sppScalarBarRepresentation::GetOrientation()
{
	if (this->ScalarBarActorAdapter)
	{
		return this->ScalarBarActorAdapter->GetOrientation();
	}
	vtkErrorMacro("No scalar bar");
	return 0;
}

//-----------------------------------------------------------------------------
bool sppScalarBarRepresentation::CheckHighlighted(int X, int Y)
{
	int *pos1 = this->PositionCoordinate->GetComputedDisplayValue(this->Renderer);
	int *pos2 = this->Position2Coordinate->GetComputedDisplayValue(this->Renderer);

	bool bRet = true;
	// Figure out where we are in the widget. Exclude outside case first.
	if (X < (pos1[0] - this->Tolerance) || (pos2[0] + this->Tolerance) < X ||
		Y < (pos1[1] - this->Tolerance) || (pos2[1] + this->Tolerance) < Y)
	{
		bRet = false; //outside, should not be highlighted
	}
	else
	{
		bRet = true;
	}

	return bRet;
}

//-----------------------------------------------------------------------------
void sppScalarBarRepresentation::BuildRepresentation()
{
	if (this->ScalarBarActorAdapter)
	{
		this->ScalarBarActorAdapter->SetPosition(this->GetPosition());
		this->ScalarBarActorAdapter->SetPosition2(this->GetPosition2());
	}

	if (!UpperConeSelected && !LowerConeSelected)
		this->Superclass::BuildRepresentation();

	// Set things up
	int *pos1 = this->PositionCoordinate->GetComputedDisplayValue(Renderer.GetPointer());
	//    int *pos2 = this->Position2Coordinate->GetComputedDisplayValue(this->Renderer);

	double scale[3];
	BWTransform->GetScale(scale);

	double xU = pos1[0], yU = pos1[1];
	double xL = pos1[0], yL = pos1[1];

	{
		double colorBarPos[2], colorBarSize[2];
		ScalarBarActorAdapter->GetColorBarPos(colorBarPos);
		ScalarBarActorAdapter->GetColorBarSize(colorBarSize);

		double *barPos = BWTransform->GetPosition();

		double sr, barWidth = 0.0, barLength = 0.0;
		if (ScalarBarActorAdapter->GetOrientation() == VTK_ORIENT_HORIZONTAL)
		{//horizontal
			barWidth = colorBarSize[1];
			barLength = colorBarSize[0];
			sr = barWidth * sqrt(2) * 0.5 / scale[1];

			xU = barPos[0] + colorBarPos[0] + UpperConeRelativePos * barLength;
			yU = barPos[1];// + barWidth;
			xL = barPos[0] + colorBarPos[0] + LowerConeRelativePos * barLength;
			yL = barPos[1];// + barWidth;
		} else
		{//vertical
			barWidth = colorBarSize[0];
			barLength = colorBarSize[1];
			sr = barWidth * sqrt(2) * 0.5 / scale[0];

			xU = barPos[0];// + barWidth;
			yU = barPos[1] + colorBarPos[1] + UpperConeRelativePos * barLength;
			xL = barPos[0];// + barWidth;
			yL = barPos[1] + colorBarPos[1] + LowerConeRelativePos * barLength;
		}

		UpperConePos[0] = xU;
		UpperConePos[1] = yU;
		LowerConePos[0] = xL;
		LowerConePos[1] = yL;
	}
} 

//-----------------------------------------------------------------------------
/*
	When mouse moves into the boundary rectangle, the bar is highlighted and two thick black lines 
	are displayed at the upper clip and lower clip value, if the mouse is on one of two black lines, 
	the black line is selected and turned yellow.
	When left mouse is pressed and draged after the bar highlighted, the following may happen:
	1. if one of the clipping line is yellow, the yellow line will start moving, the clipping value 
	   will start to change, and the plot is updated. 
	2. if the mouse is on one of the boundary edges, the bar size will start to change.
	3. if the mouse is inside, not on boundary and no yellow line, the bar will start moving
*/
void sppScalarBarRepresentation::WidgetInteraction(double eventPos[2])
{
	// Let superclass move things around.
	bool bHorizontal = 	ScalarBarActorAdapter->GetOrientation() == VTK_ORIENT_HORIZONTAL;
	if (m_bLeftMouseDown)
	{
		if (UpperConeSelected || LowerConeSelected)
		{//move the selected cone
			double XF = eventPos[0];
			double YF = eventPos[1];

			double colorBarPos[2], colorBarSize[2];
			ScalarBarActorAdapter->GetColorBarSize(colorBarSize);
			ScalarBarActorAdapter->GetColorBarPos(colorBarPos);

			double *barPos = BWTransform->GetPosition();

			double barLength = 0.0;
			if (UpperConeSelected)
			{//adjust cone position
				if (bHorizontal)
				{//horizontal
					barLength = colorBarSize[0];
					UpperConeRelativePos = (XF - (barPos[0] + colorBarPos[0])) / barLength;
				} else
				{//vertical
					barLength = colorBarSize[1];
					UpperConeRelativePos = (YF - (barPos[1] + colorBarPos[1])) / barLength;
				}

				if (UpperConeRelativePos < 0)
					UpperConeRelativePos = 0.0;
				else if (UpperConeRelativePos > 1.0)
					UpperConeRelativePos = 1.0;

				if (UpperConeRelativePos < LowerConeRelativePos)
					UpperConeRelativePos = LowerConeRelativePos;

				this->Modified();
				this->InvokeEvent(eScalarBarEvent_UpperConeMoved, NULL);
			} else if (LowerConeSelected)
			{//adjust cone position
				if (bHorizontal)
				{//horizontal
					barLength = colorBarSize[0];
					LowerConeRelativePos = (XF - (barPos[0] + colorBarPos[0])) / barLength;
				} else
				{//vertical
					barLength = colorBarSize[1];
					LowerConeRelativePos = (YF - (barPos[1] + colorBarPos[1])) / barLength;
				}

				if (LowerConeRelativePos < 0)
					LowerConeRelativePos = 0.0;
				else if (LowerConeRelativePos > 1.0)
					LowerConeRelativePos = 1.0;

				if (LowerConeRelativePos > UpperConeRelativePos)
					LowerConeRelativePos = UpperConeRelativePos;

				this->Modified();
				this->InvokeEvent(eScalarBarEvent_LowerConeMoved, NULL);
			}

			return; //no need to continue
		}
	}

	double XF = eventPos[0];
	double YF = eventPos[1];

	// convert to normalized viewport coordinates
	this->Renderer->DisplayToNormalizedDisplay(XF,YF);
	this->Renderer->NormalizedDisplayToViewport(XF,YF);
	this->Renderer->ViewportToNormalizedViewport(XF,YF);

	// there are four parameters that can be adjusted
	//low-left corner position
	double *fpos1 = this->PositionCoordinate->GetValue();
	//size (width and height)
	double *fpos2 = this->Position2Coordinate->GetValue();
	double par1[2];
	double par2[2];
	//low left
	par1[0] = fpos1[0];
	par1[1] = fpos1[1];
	//upper right
	par2[0] = fpos1[0] + fpos2[0];
	par2[1] = fpos1[1] + fpos2[1];

	//mouse displacements
	double delX = XF - this->StartEventPosition[0];
	double delY = YF - this->StartEventPosition[1];
	//adjusted mouse displacements
	double delX2=0.0, delY2=0.0;

	// Based on the state, adjust the representation. Note that we force a
	// uniform scaling of the widget when tugging on the corner points (and
	// when proportional resize is on). This is done by finding the maximum
	// movement in the x-y directions and using this to scale the widget.
	if ( this->ProportionalResize && !this->Moving ) //not moving means it is during resizing
	{
		double sx = fpos2[0]/fpos2[1];
		double sy = fpos2[1]/fpos2[0];
		if ( fabs(delX) > fabs(delY) )
		{
			delY = sy*delX;
			delX2 = delX;
			delY2 = -delY;
		}
		else
		{
			delX = sx*delY;
			delY2 = delY;
			delX2 = -delX;
		}
	}
	else
	{
		delX2 = delX;
		delY2 = delY;
	}

	// The previous "if" statement has taken care of the proportional resize
	// for the most part. However, tugging on edges has special behavior, which
	// is to scale the box about its center.
	switch (this->InteractionState)
	{
	case vtkBorderRepresentation::AdjustingP0:
		par1[0] = par1[0] + delX;
		par1[1] = par1[1] + delY;
		break;
	case vtkBorderRepresentation::AdjustingP1:
		par2[0] = par2[0] + delX2;
		par1[1] = par1[1] + delY2;
		break;
	case vtkBorderRepresentation::AdjustingP2:
		par2[0] = par2[0] + delX;
		par2[1] = par2[1] + delY;
		break;
	case vtkBorderRepresentation::AdjustingP3:
		par1[0] = par1[0] + delX2;
		par2[1] = par2[1] + delY2;
		break;
	case vtkBorderRepresentation::AdjustingE0:
		par1[1] = par1[1] + delY;
		if ( this->ProportionalResize )
		{
			par2[1] = par2[1] - delY;
			par1[0] = par1[0] + delX;
			par2[0] = par2[0] - delX;
		}
		break;
	case vtkBorderRepresentation::AdjustingE1:
		par2[0] = par2[0] + delX;
		if ( this->ProportionalResize )
		{
			par1[0] = par1[0] - delX;
			par1[1] = par1[1] - delY;
			par2[1] = par2[1] + delY;
		}
		break;
	case vtkBorderRepresentation::AdjustingE2:
		par2[1] = par2[1] + delY;
		if ( this->ProportionalResize )
		{
			par1[1] = par1[1] - delY;
			par1[0] = par1[0] - delX;
			par2[0] = par2[0] + delX;
		}
		break;
	case vtkBorderRepresentation::AdjustingE3:
		par1[0] = par1[0] + delX;
		if ( this->ProportionalResize )
		{
			par2[0] = par2[0] - delX;
			par1[1] = par1[1] + delY;
			par2[1] = par2[1] - delY;
		}
		break;
	case vtkBorderRepresentation::Inside:
		if ( this->Moving )
		{
			par1[0] = par1[0] + delX;
			par1[1] = par1[1] + delY;
			par2[0] = par2[0] + delX;
			par2[1] = par2[1] + delY;
		}
		break;
	}

	double center[2];
	center[0] = 0.5 * (par1[0] + par2[0]);
	center[1] = 0.5 * (par1[1] + par2[1]);
	//size does not change
	double sizeX = par2[0] - par1[0];
	double sizeY = par2[1] - par1[1];

	if (par1[0] < 0.0)
	{
		par1[0] = 0.0;
		par2[0] = sizeX;
	}
	else if (par2[0] > 1.0)
	{
		par2[0] = 1.0;
		par1[0] = 1.0 - sizeX;
	}

	if (par1[1] < 0.0)
	{
		par1[1] = 0.0;
		par2[1] = sizeY;
	}
	else if (par2[1] > 1.0)
	{
		par2[1] = 1.0;
		par1[1] = 1.0 - sizeY;
	}

	//determine if orientation should be changed
	bool orientationSwapped = false;
	if (bHorizontal)
	{
		if (center[1] > 0.33 && center[1] < 0.66)
		{
			if (delX < 0)
			{//moving to the left
				if (par1[0] < 0.001)
				{//touch the left
					this->ScalarBarActorAdapter->SetOrientation(VTK_ORIENT_VERTICAL);
					orientationSwapped = true; //change to vertical
				}
			}
			else if (delX > 0)
			{//moving towards the right
				if (par2[0] > 0.999)
				{//touch the right
					this->ScalarBarActorAdapter->SetOrientation(VTK_ORIENT_VERTICAL);
					orientationSwapped = true; //change to vertical
				}
			}
		}
	}
	else
	{
		if (center[0] > 0.33 && center[0] < 0.66)
		{
			if (delY < 0)
			{//moving towards bottom
				if (par1[1] < 0.001)
				{//touch the left
					this->ScalarBarActorAdapter->SetOrientation(VTK_ORIENT_HORIZONTAL);
					orientationSwapped = true; //change to horizontal
				}
			}
			else if (delY > 0)
			{//moving towards the top
				if (par2[1] > 0.999)
				{//touch the right
					this->ScalarBarActorAdapter->SetOrientation(VTK_ORIENT_HORIZONTAL);
					orientationSwapped = true; //change to horizontal
				}
			}
		}
	}

	bool bModified = false;
	if (orientationSwapped)
	{
		//switch the x with the way
		std::swap(sizeX, sizeY);
		//center does not change
		//new lower-left
		par1[0] = center[0] - 0.5 * sizeX;
		par1[1] = center[1] - 0.5 * sizeY;
		//new upper-right
		par2[0] = center[0] + 0.5 * sizeX;
		par2[1] = center[1] + 0.5 * sizeY;

		//adjust if some parts are out, only one side can be guranteed to be visible
		if (par1[0] < 0)
			par1[0] = 0;
		else if (par2[0] > 1.0)
			par1[0] = 1.0 - sizeX;

		if (par1[1] < 0)
			par1[1] = 0;
		else if (par2[1] > 1.0)
			par1[1] = 1.0 - sizeY;

		this->PositionCoordinate->SetValue(par1[0],par1[1]);
		this->Position2Coordinate->SetValue(sizeX, sizeY);

		std::swap(this->ShowHorizontalBorder, this->ShowVerticalBorder);

		bModified = true;
	}
	else if (par2[0] > par1[0] && par2[1] > par1[1])
	{
		this->PositionCoordinate->SetValue(par1[0],par1[1]);
		this->Position2Coordinate->SetValue(par2[0] - par1[0], par2[1] - par1[1]);
		this->StartEventPosition[0] = XF;
		this->StartEventPosition[1] = YF;

		bModified = true;
	}

	if (bModified)
	{
		this->Modified();
		this->UpdateShowBorder();
		this->BuildRepresentation();
	}
}

//-----------------------------------------------------------------------------
int sppScalarBarRepresentation::GetVisibility()
{
	return this->ScalarBarActorAdapter->GetVisibility();
}

//-----------------------------------------------------------------------------
void sppScalarBarRepresentation::SetVisibility(int vis)
{
	this->ScalarBarActorAdapter->SetVisibility(vis);
}

//-----------------------------------------------------------------------------
void sppScalarBarRepresentation::SetConeVisibility(int vis)
{
	//	this->UpperConeActor->SetVisibility(vis);
	//	this->LowerConeActor->SetVisibility(vis);
}

//-----------------------------------------------------------------------------
void sppScalarBarRepresentation::GetActors2D(vtkPropCollection *collection)
{
	if (this->ScalarBarActorAdapter)
	{
		collection->AddItem(this->ScalarBarActorAdapter->GetActor2D());
	}
	this->Superclass::GetActors2D(collection);
}

//-----------------------------------------------------------------------------
void sppScalarBarRepresentation::ReleaseGraphicsResources(vtkWindow *w)
{
	if (this->ScalarBarActorAdapter && ScalarBarActorAdapter->GetActor2D())
	{
		this->ScalarBarActorAdapter->GetActor2D()->ReleaseGraphicsResources(w);
	}
	this->Superclass::ReleaseGraphicsResources(w);
}

//-------------------------------------------------------------------------
int sppScalarBarRepresentation::RenderOverlay(vtkViewport *w)
{
	int count = this->Superclass::RenderOverlay(w);
	if (this->ScalarBarActorAdapter)
	{
		count += this->ScalarBarActorAdapter->GetActor2D()->RenderOverlay(w);
	}
	return count;
}

//-------------------------------------------------------------------------
int sppScalarBarRepresentation::RenderOpaqueGeometry(vtkViewport *w)
{
	assert(!BWActor->GetVisibility() || BWActor->GetPosition()[0] >= 0);

	int count = this->Superclass::RenderOpaqueGeometry(w);
	if (this->ScalarBarActorAdapter)
	{
		count += this->ScalarBarActorAdapter->GetActor2D()->RenderOpaqueGeometry(w);
	}
	return count;
}

//-------------------------------------------------------------------------
int sppScalarBarRepresentation::RenderTranslucentPolygonalGeometry(vtkViewport *w)
{
	int count = this->Superclass::RenderTranslucentPolygonalGeometry(w);
	if (this->ScalarBarActorAdapter)
	{
		count += this->ScalarBarActorAdapter->GetActor2D()->RenderTranslucentPolygonalGeometry(w);
	}
	return count;
}

//-------------------------------------------------------------------------
int sppScalarBarRepresentation::HasTranslucentPolygonalGeometry()
{
	int result = this->Superclass::HasTranslucentPolygonalGeometry();
	if (this->ScalarBarActorAdapter)
	{
		result |= this->ScalarBarActorAdapter->GetActor2D()->HasTranslucentPolygonalGeometry();
	}
	return result;
}

void sppScalarBarRepresentation::OnLeftButtonDown(double eventPos[2])
{
	selectCone(false, eventPos);

	m_bLeftMouseDown = true;
}

void sppScalarBarRepresentation::OnLeftButtonUp(double eventPos[2])
{
	selectCone(true, eventPos);
	m_bLeftMouseDown = false;
}

void sppScalarBarRepresentation::OnMiddleButtonDown(double eventPos[2])
{
}

void sppScalarBarRepresentation::OnMiddleButtonUp(double eventPos[2])
{
}

void sppScalarBarRepresentation::OnRightButtonDown(double eventPos[2])
{
}

void sppScalarBarRepresentation::OnRightButtonUp(double eventPos[2])
{
}

void sppScalarBarRepresentation::OnMouseMove(double eventPos[2])
{
	if (!m_bLeftMouseDown)
		selectCone(false, eventPos);
}

void sppScalarBarRepresentation::selectCone(bool bForceUnselect, double eventPos[2])
{
	UpperConeSelected = false;
	LowerConeSelected = false;
	if (!bForceUnselect)
	{
		{//select uppercone
			if (ScalarBarActorAdapter->GetOrientation() == VTK_ORIENT_HORIZONTAL)
			{//horizontal
				if (fabs(eventPos[0] - UpperConePos[0]) < 3)
				{
					UpperConeSelected = true;
				}
			} else
			{//vertical
				if (fabs(eventPos[1] - UpperConePos[1]) < 3)
				{
					UpperConeSelected = true;
				}
			}
		}

		if (!UpperConeSelected) //only one can be selected
		{//select uppercone
			if (ScalarBarActorAdapter->GetOrientation() == VTK_ORIENT_HORIZONTAL)
			{//horizontal
				if (fabs(eventPos[0] - LowerConePos[0]) < 3)
				{
					LowerConeSelected = true;
				}
			} else
			{//vertical
				if (fabs(eventPos[1] - LowerConePos[1]) < 3)
				{
					LowerConeSelected = true;
				}
			}
		}
	}

	SetConeVisibility(Moving);

	if (UpperConeSelected)
	{
		this->ScalarBarActorAdapter->SetUpperClippingHandColor(SelectedConeColor);
	}
	else
	{
		this->ScalarBarActorAdapter->SetUpperClippingHandColor(ConeColor);
	}

	if (LowerConeSelected)
	{
		this->ScalarBarActorAdapter->SetLowerClippingHandColor(SelectedConeColor);
	}
	else
	{
		this->ScalarBarActorAdapter->SetLowerClippingHandColor(ConeColor);
	}
}

void sppScalarBarRepresentation::UpdateShowBorder()
{
	vtkBorderRepresentation::UpdateShowBorder();
	if (ScalarBarActorAdapter) //display the hands only when the bar is highlighted
		ScalarBarActorAdapter->SetClipHandsVisibility(BWActor->GetVisibility());
}

int sppScalarBarRepresentation::ComputeInteractionState(int X, int Y, int vtkNotUsed(modify))
{
	int *pos1 = this->PositionCoordinate->GetComputedDisplayValue(this->Renderer);
	int *pos2 = this->Position2Coordinate->GetComputedDisplayValue(this->Renderer);

	// Figure out where we are in the widget. Exclude outside case first.
	if (X < (pos1[0] - this->Tolerance) || (pos2[0] + this->Tolerance) < X ||
		Y < (pos1[1] - this->Tolerance) || (pos2[1] + this->Tolerance) < Y)
	{
		this->InteractionState = vtkBorderRepresentation::Outside;
	}

	else // we are on the boundary or inside the border
	{
		// Now check for proximinity to edges and points
		int e0 = (Y >= (pos1[1] - this->Tolerance) && Y <= (pos1[1] + this->Tolerance));
		int e1 = (X >= (pos2[0] - this->Tolerance) && X <= (pos2[0] + this->Tolerance));
		int e2 = (Y >= (pos2[1] - this->Tolerance) && Y <= (pos2[1] + this->Tolerance));
		int e3 = (X >= (pos1[0] - this->Tolerance) && X <= (pos1[0] + this->Tolerance));

		int adjustHorizontalEdges = (this->ShowHorizontalBorder != BORDER_OFF);
		int adjustVerticalEdges = (this->ShowVerticalBorder != BORDER_OFF);
		if (GetConstantWidth())
		{
			if (GetOrientation() == VTK_ORIENT_HORIZONTAL)
				adjustHorizontalEdges = 0;
			else
				adjustVerticalEdges = 0;
		}

		int adjustPoints = (adjustHorizontalEdges && adjustVerticalEdges);

		if (e0 && e1 && adjustPoints)
		{
			this->InteractionState = vtkBorderRepresentation::AdjustingP1;
		}
		else if (e1 && e2 && adjustPoints)
		{
			this->InteractionState = vtkBorderRepresentation::AdjustingP2;
		}
		else if (e2 && e3 && adjustPoints)
		{
			this->InteractionState = vtkBorderRepresentation::AdjustingP3;
		}
		else if (e3 && e0 && adjustPoints)
		{
			this->InteractionState = vtkBorderRepresentation::AdjustingP0;
		}

		// Edges
		else if (e0 || e1 || e2 || e3)
		{
			if (e0 && adjustHorizontalEdges)
			{
				this->InteractionState = vtkBorderRepresentation::AdjustingE0;
			}
			else if (e1 && adjustVerticalEdges)
			{
				this->InteractionState = vtkBorderRepresentation::AdjustingE1;
			}
			else if (e2 && adjustHorizontalEdges)
			{
				this->InteractionState = vtkBorderRepresentation::AdjustingE2;
			}
			else if (e3 && adjustVerticalEdges)
			{
				this->InteractionState = vtkBorderRepresentation::AdjustingE3;
			}
		}

		else // must be interior
		{
			if (this->Moving)
			{
				// FIXME: This must be wrong.  Moving is not an entry in the
				// _InteractionState enum.  It is an ivar flag and it has no business
				// being set to InteractionState.  This just happens to work because
				// Inside happens to be 1, and this gets set when Moving is 1.
				this->InteractionState = vtkBorderRepresentation::Moving;
			}
			else
			{
				this->InteractionState = vtkBorderRepresentation::Inside;
			}
		}
	}//else inside or on border
	this->UpdateShowBorder();

	return this->InteractionState;
}

