#define __COMPILING_sppScalarBarActor_CPP__
#include "sppScalarBarActor.h"
#undef __COMPILING_sppScalarBarActor_CPP__

#pragma warning( push, 0 )
#include "vtkScalarBarActorInternal.h"

#include "vtkNew.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkMath.h"

#include "vtkScalarsToColors.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkPolyData.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkProperty2D.h"
#pragma warning(pop)

#include <stdio.h> // for snprintf

sppScalarBarActor* sppScalarBarActor::New()
{
	return new sppScalarBarActor;
}

//-------------------------------------------------------------------------
sppScalarBarActor::sppScalarBarActor()
{
	this->MinimumWidthInPixels = 50;
	this->MinimumHeightInPixels = 50;

	SetLowerClipValue(VTK_FLOAT_MIN);
	SetUpperClipValue(VTK_FLOAT_MAX);
	this->SetBarRatio(0.3);
	this->SetMaximumNumberOfColors(256);

	SetUpperClippingHandColor(0, 0, 0); //black
	SetLowerClippingHandColor(0, 0, 0); //gray

	SetClippingHandsVisibility(0);//not visible by default
	this->UpperClippingHand_PD = vtkPolyData::New();
	this->UpperClippingHand_Mapper = vtkPolyDataMapper2D::New();
	this->UpperClippingHand_Mapper->SetInputData(UpperClippingHand_PD);
	this->UpperClippingHand_Actor = vtkActor2D::New();
	this->UpperClippingHand_Actor->SetMapper(UpperClippingHand_Mapper);
	this->UpperClippingHand_Actor->GetPositionCoordinate()->SetReferenceCoordinate(this->PositionCoordinate);

	this->LowerClippingHand_PD = vtkPolyData::New();
	this->LowerClippingHand_Mapper = vtkPolyDataMapper2D::New();
	this->LowerClippingHand_Mapper->SetInputData(LowerClippingHand_PD);
	this->LowerClippingHand_Actor = vtkActor2D::New();
	this->LowerClippingHand_Actor->SetMapper(LowerClippingHand_Mapper);
	this->LowerClippingHand_Actor->GetPositionCoordinate()->SetReferenceCoordinate(this->PositionCoordinate);

}

//-------------------------------------------------------------------------
sppScalarBarActor::~sppScalarBarActor()
{
	UpperClippingHand_Mapper->Delete();
	UpperClippingHand_Actor->Delete();
	UpperClippingHand_PD->Delete();
	LowerClippingHand_Mapper->Delete();
	LowerClippingHand_Actor->Delete();
	LowerClippingHand_PD->Delete();
}

int* sppScalarBarActor::GetScalarBarPos()
{
	return P->ScalarBarBox.Posn.GetData();
}

int* sppScalarBarActor::GetScalarBarSize()
{
	return P->ScalarBarBox.Size.GetData();
}

//----------------------------------------------------------------------------
int sppScalarBarActor::RenderOpaqueGeometry(vtkViewport* viewport)
{
	int renderedSomething = vtkScalarBarActor::RenderOpaqueGeometry(viewport);
	if (renderedSomething && ClippingHandsVisibility)
	{//if the bar is rendered
		renderedSomething += UpperClippingHand_Actor->RenderOpaqueGeometry(viewport);
		renderedSomething += LowerClippingHand_Actor->RenderOpaqueGeometry(viewport);
	}

	renderedSomething = (renderedSomething > 0) ? (1) : (0);
	return renderedSomething;
}

//----------------------------------------------------------------------------
int sppScalarBarActor::RenderOverlay(vtkViewport* viewport)
{
	int renderedSomething = vtkScalarBarActor::RenderOverlay(viewport);
	if (renderedSomething && ClippingHandsVisibility)
	{//if the bar is rendered
		renderedSomething += UpperClippingHand_Actor->RenderOverlay(viewport);
		renderedSomething += LowerClippingHand_Actor->RenderOverlay(viewport);
	}

	renderedSomething = (renderedSomething > 0) ? (1) : (0);
	return renderedSomething;
}

//----------------------------------------------------------------------------
void sppScalarBarActor::ReleaseGraphicsResources(vtkWindow* w)
{
	vtkScalarBarActor::ReleaseGraphicsResources(w);
	UpperClippingHand_Actor->vtkProp::ReleaseGraphicsResources(w);
	LowerClippingHand_Actor->vtkProp::ReleaseGraphicsResources(w);
}

//----------------------------------------------------------------------------
void sppScalarBarActor::ConfigureAnnotations()
{
	vtkScalarBarActor::ConfigureAnnotations();
}

static double cosConeHalfAngle = cos(vtkMath::Pi() * 0.25);
static void buildHandPD(vtkPolyData *UpperClippingHand_PD, double UpperClipValue, int Orientation, double *lowerLeftPt, double *upperRightPt)
{
	vtkNew<vtkPoints> lPts;
	vtkNew<vtkCellArray> lSegs;
	vtkNew<vtkCellArray> lTris;

	lPts->SetNumberOfPoints(4); //a line segment + a triangle

	bool bOnRight = false;
	vtkIdType ids[3];
	ids[0] = 0, ids[1] = 1;
	lSegs->InsertNextCell(2, ids); //the segment
	if (bOnRight)
	{
		ids[0] = 1, ids[1] = 2, ids[2] = 3;
	}
	else
	{
		ids[0] = 0, ids[1] = 2, ids[2] = 3;
	}
	lTris->InsertNextCell(3, ids); //the triangle

	if (Orientation == VTK_ORIENT_VERTICAL)
	{
		double dy = UpperClipValue * (upperRightPt[1] - lowerLeftPt[1]);
		lPts->SetPoint(0, lowerLeftPt[0], lowerLeftPt[1] + dy, 0.0);
		lPts->SetPoint(1, upperRightPt[0], lowerLeftPt[1] + dy, 0.0);
		//triangle
		double edgeL = (upperRightPt[0] - lowerLeftPt[0]) * 0.2;//triangle edge length
		if (bOnRight)
		{//on right
			lPts->SetPoint(2, upperRightPt[0] + edgeL * cosConeHalfAngle, lowerLeftPt[1] + dy - 0.5 * edgeL, 0);
			lPts->SetPoint(3, upperRightPt[0] + edgeL * cosConeHalfAngle, lowerLeftPt[1] + dy + 0.5 * edgeL, 0);
		}
		else
		{//on left
			lPts->SetPoint(2, lowerLeftPt[0] - edgeL * cosConeHalfAngle, lowerLeftPt[1] + dy - 0.5 * edgeL, 0);
			lPts->SetPoint(3, lowerLeftPt[0] - edgeL * cosConeHalfAngle, lowerLeftPt[1] + dy + 0.5 * edgeL, 0);
		}
	}
	else
	{
		double dx = UpperClipValue * (upperRightPt[0] - lowerLeftPt[0]);
		double x = lowerLeftPt[0] + dx;
		lPts->SetPoint(0, x, lowerLeftPt[1], 0.0);
		lPts->SetPoint(1, x, upperRightPt[1], 0.0);

		//triangle
		double edgeL = (upperRightPt[1] - lowerLeftPt[1]) * 0.25;//triangle edge length
		if (bOnRight)
		{//on top
			lPts->SetPoint(2, x + 0.5 * edgeL, upperRightPt[1] + edgeL * cosConeHalfAngle, 0);
			lPts->SetPoint(3, x - 0.5 * edgeL, upperRightPt[1] + edgeL * cosConeHalfAngle, 0);
		}
		else
		{//on bottom
			lPts->SetPoint(2, x + 0.5 * edgeL, lowerLeftPt[1] - edgeL * cosConeHalfAngle, 0);
			lPts->SetPoint(3, x - 0.5 * edgeL, lowerLeftPt[1] - edgeL * cosConeHalfAngle, 0);
		}
	}

	UpperClippingHand_PD->SetPoints(lPts.GetPointer());
	UpperClippingHand_PD->SetLines(lSegs.GetPointer());
	UpperClippingHand_PD->SetPolys(lTris.GetPointer());
}

void sppScalarBarActor::ConfigureScalarBar()
{//customize this to apply min/max clipping

#if 0
	vtkScalarBarActor::ConfigureScalarBar();
#else
	vtkScalarsToColors* lut = this->LookupTable;
	double* range = lut->GetRange();
	this->P->NumColors = lut->GetIndexedLookup() ? lut->GetNumberOfAnnotatedValues() : this->MaximumNumberOfColors;
	this->P->NumSwatches = this->P->NumColors + (this->DrawNanAnnotation ? 1 : 0);
	int numPts = 2 * (this->P->NumColors + 1) + (this->DrawNanAnnotation ? 4 : 0);
	this->P->SwatchPts = vtkPoints::New();
	this->P->SwatchPts->SetNumberOfPoints(numPts);
	this->P->Polys = vtkCellArray::New();
	this->P->Polys->Allocate(this->P->Polys->EstimateSize(this->P->NumSwatches,4));
	this->P->SwatchColors = vtkUnsignedCharArray::New();

	unsigned int nComponents = ((this->UseOpacity) ? 4 : 3);
	this->P->SwatchColors->SetNumberOfComponents(nComponents);
	this->P->SwatchColors->SetNumberOfTuples(this->P->NumSwatches);

	this->ScalarBarActor->SetProperty(this->GetProperty());
	this->ScalarBar->Initialize();
	this->ScalarBar->SetPoints(this->P->SwatchPts);
	this->ScalarBar->SetPolys(this->P->Polys);
	this->ScalarBar->GetCellData()->SetScalars(this->P->SwatchColors);
	this->P->SwatchPts->Delete();
	this->P->Polys->Delete();
	this->P->SwatchColors->Delete();

	double delta = static_cast<double>(this->P->ScalarBarBox.Size[1]) / this->P->NumColors;
	double heightU = UpperClipValue * this->P->ScalarBarBox.Size[1];
	double heightL = LowerClipValue * this->P->ScalarBarBox.Size[1];
	double x0[3], x1[3], idelta;
	x0[2] = 0.0; x1[2] = 0.0;
	for (int i = 0; i < numPts / 2 - (this->DrawNanAnnotation ? 2 : 0); ++i)
	{
		//left
		x0[this->P->TL[0]] = this->P->ScalarBarBox.Posn[this->P->TL[0]];
		idelta = i * delta;
		x0[this->P->TL[1]] = this->P->ScalarBarBox.Posn[this->P->TL[1]] + idelta;
		//adjust the height to fit with the clip values
		if (heightU < idelta && heightU > (idelta - delta))
			x0[this->P->TL[1]] = this->P->ScalarBarBox.Posn[this->P->TL[1]] + heightU;
		if (heightL > idelta && heightL < (idelta + delta))
			x0[this->P->TL[1]] = this->P->ScalarBarBox.Posn[this->P->TL[1]] + heightL;
		//right
		x1[this->P->TL[0]] = this->P->ScalarBarBox.Posn[this->P->TL[0]] + this->P->ScalarBarBox.Size[0];
		x1[this->P->TL[1]] = x0[this->P->TL[1]]; //the same height

		//the height may need to be adjusted to fit with the clip value

		this->P->SwatchPts->SetPoint(2 * i, x0);
		this->P->SwatchPts->SetPoint(2 * i + 1, x1);
	}

	int iUpperClipValue = (int)(UpperClipValue * P->NumColors);
	if (UpperClipValue > 1.0)
		iUpperClipValue = P->NumColors;
	int iLowerClipValue = (int)(LowerClipValue * P->NumColors);
	if (LowerClipValue < 0.0)
		iLowerClipValue = 0;

	//polygons & cell colors
	unsigned char* rgb;
	double rgba[4];
	vtkIdType ptIds[4];
	for (int i = 0; i < this->P->NumColors; ++i)
	{
		ptIds[0] = 2 * i;
		ptIds[1] = ptIds[0] + 1;
		ptIds[2] = ptIds[1] + 2;
		ptIds[3] = ptIds[0] + 2;
		this->P->Polys->InsertNextCell(4,ptIds);
		double rgbval;
		if (this->LookupTable->UsingLogScale())
		{
			rgbval = log10(range[0]) + i * (log10(range[1]) - log10(range[0])) / this->P->NumColors;
			rgbval = pow(10.0, rgbval);
		}
		else
		{
			rgbval = range[0] + (range[1] - range[0]) * (i / static_cast<double>(this->P->NumColors));
		}
		lut->GetColor(rgbval, rgba);
		rgba[3] = lut->GetOpacity(rgbval);
		//write into array directly
		rgb = this->P->SwatchColors->GetPointer(nComponents * i);
		if (i < iLowerClipValue || i > iUpperClipValue)
		{
			rgb[0] = rgb[1] = rgb[2] = 222;
		} else
		{
			rgb[0] = static_cast<unsigned char>(rgba[0] * 255.);
			rgb[1] = static_cast<unsigned char>(rgba[1] * 255.);
			rgb[2] = static_cast<unsigned char>(rgba[2] * 255.);
		}
		if (this->P->SwatchColors->GetNumberOfComponents() > 3)
		{
			rgb[3] = static_cast<unsigned char>(this->UseOpacity ? rgba[3] * 255. : 255);
		}
	}

	// Set up a texture actor as an alternative to the 1-quad-per-color
	// scalar bar polydata.
	vtkPoints* texturePoints = vtkPoints::New();
	texturePoints->SetNumberOfPoints(4);
	this->TexturePolyData->SetPoints(texturePoints);
	texturePoints->SetPoint(0, 0.0, 0.0, 0.0);

	double p1[2], p2[2];
	p1[0] = this->P->ScalarBarBox.Posn[0];
	p1[1] = this->P->ScalarBarBox.Posn[1];
	p2[0] = p1[0] + this->P->ScalarBarBox.Size[this->P->TL[0]];
	p2[1] = p1[1] + this->P->ScalarBarBox.Size[this->P->TL[1]];

	texturePoints->SetPoint(0, p1[0], p1[1], 0.0);
	texturePoints->SetPoint(1, p2[0], p1[1], 0.0);
	texturePoints->SetPoint(2, p2[0], p2[1], 0.0);
	texturePoints->SetPoint(3, p1[0], p2[1], 0.0);
	texturePoints->Delete();

	double barWidth = this->P->ScalarBarBox.Size[this->P->TL[0]];
	double barHeight = this->P->ScalarBarBox.Size[this->P->TL[1]];
	vtkDataArray* tc = this->TexturePolyData->GetPointData()->GetTCoords();
	tc->SetTuple2(1, barWidth / this->TextureGridWidth, 0.0);
	tc->SetTuple2(2, barWidth / this->TextureGridWidth, barHeight / this->TextureGridWidth);
	tc->SetTuple2(3, 0.0, barHeight / this->TextureGridWidth);

	//clipping hands
	buildHandPD(UpperClippingHand_PD, UpperClipValue, Orientation, p1, p2);
	UpperClippingHand_Actor->GetProperty()->SetLineWidth(2);
	UpperClippingHand_Actor->GetProperty()->SetColor(UpperClippingHandColor);

	buildHandPD(LowerClippingHand_PD, LowerClipValue, Orientation, p1, p2);
	LowerClippingHand_Actor->GetProperty()->SetLineWidth(2);
	LowerClippingHand_Actor->GetProperty()->SetColor(LowerClippingHandColor);
#endif
}

#if defined(_WIN32) && !defined(__CYGWIN__)
#  define SNPRINTF _snprintf
#else
#  define SNPRINTF snprintf
#endif

void sppScalarBarActor::RebuildLayout(vtkViewport* viewport)
{
	vtkScalarBarActor::RebuildLayout(viewport);
	if (P->ScalarBarBox.Size[1] < 0 || P->ScalarBarBox.Size[1] > P->Frame.Size[1])
	{//ScalarBarBox size is invalid, need rebuild
		vtkScalarBarActor::RebuildLayout(viewport);
	} 
}

void sppScalarBarActor::LayoutTitle()
{
/*	vtkScalarBarActor::LayoutTitle();
    Overwrite this to adjust the targetWidth. 
	when the title contains only one or two letters the old targetWidth is too high
*/
	if (this->Title == NULL || !strlen(this->Title))
	{
		return;
	}

	bool bHorizontal = (this->Orientation == VTK_ORIENT_VERTICAL || this->LookupTable->GetIndexedLookup());

	int targetWidth, targetHeight;
	// Title spans entire width of frame at top, regardless of orientation.
	targetWidth = static_cast<int>(this->P->Frame.Size[this->P->TL[0]]) - 2 * this->TextPad;
	// Height is either: at most half the frame height or
	// a fixed portion of the frame remaining after subtracting the
	// scalar bar's thickness.
	//
	// When laid out horizontally, ticks share vertical space with title.
	// We want the title to be larger (18pt vs 14pt).
	targetHeight = static_cast<int>(bHorizontal ?
		ceil(this->P->Frame.Size[this->P->TL[1]] / 2. - this->TextPad)
		: (this->P->Frame.Size[0] - this->P->ScalarBarBox.Size[0] -	(this->TextPosition == SucceedScalarBar ?
		this->P->ScalarBarBox.Posn[this->P->TL[0]] : 0) - this->TextPad) *
		this->TitleRatio);

	//see above comments
	if (bHorizontal && strlen(Title) < 6)
		targetWidth = static_cast<int>(targetWidth * (strlen(Title) / 6.0));

	this->TitleActor->SetConstrainedFontSize(this->P->Viewport, targetWidth, targetHeight);

	// Now fetch the actual size from the actor and use it to
	// update the box size and position.
	double titleSize[2] = { 0, 0 };
	this->TitleActor->GetSize(this->P->Viewport, titleSize);
	this->TitleActor->GetTextProperty()->SetVerticalJustificationToTop();
	for (int i = 0; i < 2; ++i)
	{
		this->P->TitleBox.Size[this->P->TL[i]] = static_cast<int>(ceil(titleSize[i]));
	}

	this->P->TitleBox.Posn[0] = this->P->Frame.Posn[0] + (this->P->Frame.Size[this->P->TL[0]] - titleSize[0]) / 2;
	this->P->TitleBox.Posn[1] = this->P->Frame.Posn[1] + this->P->Frame.Size[this->P->TL[1]];
	if (
		this->Orientation == VTK_ORIENT_VERTICAL ||
		this->TextPosition == vtkScalarBarActor::SucceedScalarBar)
	{
		this->P->TitleBox.Posn[1] -= this->P->TitleBox.Size[this->P->TL[1]] + this->TextPad;
	}
	else
	{
		this->P->TitleBox.Posn[1] = this->P->Frame.Posn[1] + this->TextPad;
	}
}

void sppScalarBarActor::LayoutTicks()
{
	vtkScalarBarActor::LayoutTicks();

	if (true)
	{//reset title format to use the same font size
		if (this->Title == NULL || !strlen(this->Title))
		{
			return;
		}

		//keep the two same size
		TitleActor->GetTextProperty()->SetFontSize(P->TextActors[0]->GetTextProperty()->GetFontSize());

		// Now fetch the actual size from the actor and use it to
		// update the box size and position.
		double titleSize[2] = { 0, 0 };
		this->TitleActor->GetSize(this->P->Viewport, titleSize);
		this->TitleActor->GetTextProperty()->SetVerticalJustificationToTop();
		for (int i = 0; i < 2; ++i)
		{
			this->P->TitleBox.Size[this->P->TL[i]] = static_cast<int>(ceil(titleSize[i]));
		}

		this->P->TitleBox.Posn[0] = this->P->Frame.Posn[0] + (this->P->Frame.Size[this->P->TL[0]] - titleSize[0]) / 2;
		this->P->TitleBox.Posn[1] = this->P->Frame.Posn[1] + this->P->Frame.Size[this->P->TL[1]];
		if (
			this->Orientation == VTK_ORIENT_VERTICAL ||
			this->TextPosition == vtkScalarBarActor::SucceedScalarBar)
		{
			this->P->TitleBox.Posn[1] -= this->P->TitleBox.Size[this->P->TL[1]] + this->TextPad;
		}
		else
		{
			this->P->TitleBox.Posn[1] = this->P->Frame.Posn[1] + this->TextPad;
		}

	}
}

void sppScalarBarActor::ComputeFrame()
{
	// get the viewport size in display coordinates
	int* p0;
	int* p1;
	int size[2];
	p0 = this->PositionCoordinate->GetComputedViewportValue(this->P->Viewport);
	p1 = this->Position2Coordinate->GetComputedViewportValue(this->P->Viewport);
	for (int i = 0; i < 2; ++i)
	{
		//this->P->Frame.Posn[i] = p0[i];
		this->P->Frame.Posn[i] = 0; // Translate the frame's coordinate system to p0
		size[i] = p1[i] - p0[i];
	}

	// Check if we have bounds on the maximum size
	size[0] = size[0] > this->MaximumWidthInPixels ? this->MaximumWidthInPixels : size[0];
	size[1] = size[1] > this->MaximumHeightInPixels ? this->MaximumHeightInPixels : size[1];

	//lower limits
	if (size[0] < this->MinimumWidthInPixels)
		size[0] = this->MinimumWidthInPixels;
	if (size[1] < this->MinimumHeightInPixels)
		size[1] = this->MinimumHeightInPixels;

	for (int i = 0; i < 2; ++i)
	{
		this->P->Frame.Size[i] = size[this->P->TL[i]];
	}

	this->LastOrigin[0] = p0[0];
	this->LastOrigin[1] = p0[1];
	this->LastSize[0] = size[0];
	this->LastSize[1] = size[1];
}
