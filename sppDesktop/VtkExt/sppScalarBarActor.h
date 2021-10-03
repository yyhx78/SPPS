#ifndef __sppScalarBarActor_h
#define __sppScalarBarActor_h

#pragma warning( push, 0 )
#include "vtkScalarBarActor.h"
#pragma warning(pop)

class vtkPoints;

class sppScalarBarActor : public vtkScalarBarActor
{
public:
	static sppScalarBarActor* New();
	vtkTypeMacro(sppScalarBarActor, vtkScalarBarActor);

	vtkGetMacro(UpperClipValue, double);
	vtkSetMacro(UpperClipValue, double);
	vtkGetMacro(LowerClipValue, double);
	vtkSetMacro(LowerClipValue, double);

	vtkSetMacro(MinimumWidthInPixels, int);
	vtkGetMacro(MinimumWidthInPixels, int);
	vtkSetMacro(MinimumHeightInPixels, int);
	vtkGetMacro(MinimumHeightInPixels, int);

	vtkSetMacro(ClippingHandsVisibility, int);
	vtkGetMacro(ClippingHandsVisibility, int);
	vtkSetVector3Macro(UpperClippingHandColor, double);
	vtkGetVector3Macro(UpperClippingHandColor, double);
	vtkSetVector3Macro(LowerClippingHandColor, double);
	vtkGetVector3Macro(LowerClippingHandColor, double);

	int* GetScalarBarPos();
	int* GetScalarBarSize();

	// To include the upper and lower clipping hands
	virtual int RenderOpaqueGeometry(vtkViewport* viewport) override;
	virtual int RenderOverlay(vtkViewport* viewport) override;
	virtual void ReleaseGraphicsResources(vtkWindow*) override;

protected:
	sppScalarBarActor();
	virtual ~sppScalarBarActor();

	// Description:
	// Generate/configure the annotation labels using the laid-out geometry.
	virtual void ConfigureAnnotations() override;
	// Description:
	// Generate/configure the scalar bar representation from laid-out geometry.
	virtual void ConfigureScalarBar() override;

	//some time the title font size is much larger than the labels', overwrite 
	//this function to fix the bug
	void RebuildLayout(vtkViewport* viewport) override;
	void LayoutTitle() override;
	virtual void LayoutTicks() override;

	//overwrite this to apply Minimum width and height
	virtual void ComputeFrame() override;

	//they are relative, supposed to be within [0, 1]
	double UpperClipValue, LowerClipValue;

	//the max can be set in vtkScalarBarActor
	int MinimumWidthInPixels;
	int MinimumHeightInPixels;

	int ClippingHandsVisibility;
	vtkPolyData* UpperClippingHand_PD;
	vtkPolyDataMapper2D* UpperClippingHand_Mapper;
	vtkActor2D* UpperClippingHand_Actor;

	vtkPolyData* LowerClippingHand_PD;
	vtkPolyDataMapper2D* LowerClippingHand_Mapper;
	vtkActor2D* LowerClippingHand_Actor;

	double UpperClippingHandColor[3], LowerClippingHandColor[3];
private:
	sppScalarBarActor(const sppScalarBarActor&);  //Not implemented
	void operator=(const sppScalarBarActor&);  //Not implemented
};

#endif

