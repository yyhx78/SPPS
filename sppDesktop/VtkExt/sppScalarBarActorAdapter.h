#ifndef __sppScalarBarActorAdapter_h
#define __sppScalarBarActorAdapter_h
#pragma warning( push, 0 )
#include "vtkObject.h"
#pragma warning(pop)

/*
   Interface to make the actor used in the presentation
*/

class vtkActor2D;

class sppScalarBarActorAdapter : public vtkObject
{
public:
	vtkTypeMacro(sppScalarBarActorAdapter, vtkObject);

	//functions the sppScalarBarRepresentation will need:

	virtual vtkActor2D* GetActor2D() = 0;

	virtual int GetOrientation() = 0;
	virtual void SetOrientation(int i) = 0;

	virtual double* GetPosition() = 0;
	virtual void SetPosition(double*) = 0;

	virtual double* GetPosition2() = 0;
	virtual void SetPosition2(double*) = 0;

	virtual int GetVisibility() = 0;
	virtual void SetVisibility(int i) = 0;
	virtual void SetClipHandsVisibility(int) = 0;

	virtual bool SetUpperClipValue(double) = 0;
	virtual bool SetLowerClipValue(double) = 0;

	virtual void SetUpperClippingHandColor(double*) = 0;
	virtual void SetLowerClippingHandColor(double*) = 0;

	//the color rectangle
	virtual void GetColorBarPos(double*) = 0;
	virtual void GetColorBarSize(double*) = 0;
protected:
};

#endif
