#ifndef __sppScalarBarRepresentation_h
#define __sppScalarBarRepresentation_h

#pragma warning( push, 0 )
#include "vtkInteractionWidgetsModule.h" // For export macro
#include "vtkBorderRepresentation.h"
#include "vtkCommand.h"
#pragma warning(pop)

class vtkScalarBarActor;
class vtkTransform;
class sppScalarBarActorAdapter;

enum EScalarBarEvent
{
	eScalarBarEvent_UpperConeMoved = vtkCommand::UserEvent + 1,
	eScalarBarEvent_LowerConeMoved
};

class sppScalarBarRepresentation : public vtkBorderRepresentation
{
public:
	vtkTypeMacro(sppScalarBarRepresentation, vtkBorderRepresentation);
	static sppScalarBarRepresentation* New();

	vtkGetMacro(ConstantWidth, bool);
	vtkSetMacro(ConstantWidth, bool);

	vtkGetMacro(UpperConeSelected, bool);
	vtkSetMacro(UpperConeSelected, bool);
	vtkGetMacro(LowerConeSelected, bool);
	vtkSetMacro(LowerConeSelected, bool);

	vtkGetMacro(UpperConeRelativePos, double);
	vtkSetClampMacro(UpperConeRelativePos, double, 0., 1.);
	vtkGetMacro(LowerConeRelativePos, double);
	vtkSetClampMacro(LowerConeRelativePos, double, 0., 1.);

	// Description:
	// The prop that is placed in the renderer.
	vtkActor2D* GetScalarBarActor();
	sppScalarBarActorAdapter* GetScalarBarActorAdapter() { return ScalarBarActorAdapter; };
	virtual void SetScalarBarActorAdapter(sppScalarBarActorAdapter*);

	//the boundary rectangle actor, visible only when the widget is highlighted
	vtkActor2D* GetBWActor() { return BWActor; };

	vtkSetVector3Macro(ConeColor, double);
	vtkGetVector3Macro(ConeColor, double);
	vtkSetVector3Macro(SelectedConeColor, double);
	vtkGetVector3Macro(SelectedConeColor, double);

	// Description:
	// Satisfy the superclass' API.
	virtual void BuildRepresentation() override;
	virtual void WidgetInteraction(double eventPos[2]) override;
	virtual void GetSize(double size[2]) override
	{
		size[0] = 2.0; size[1] = 2.0;
	}

	// Description:
	// These methods are necessary to make this representation behave as
	// a vtkProp.
	virtual int GetVisibility() override;
	virtual void SetVisibility(int) override;
	virtual void SetConeVisibility(int);
	virtual void GetActors2D(vtkPropCollection* collection) override;
	virtual void ReleaseGraphicsResources(vtkWindow* window) override;
	virtual int RenderOverlay(vtkViewport*) override;
	virtual int RenderOpaqueGeometry(vtkViewport*) override;
	virtual int RenderTranslucentPolygonalGeometry(vtkViewport*) override;
	virtual int HasTranslucentPolygonalGeometry() override;
	virtual void UpdateShowBorder() override;
	virtual int ComputeInteractionState(int X, int Y, int modify = 0) override; //disable width resizing

	// Description:
	// Get/Set the orientation.
	void SetOrientation(int orient);
	int GetOrientation();

	void OnLeftButtonDown(double eventPos[2]);
	void OnLeftButtonUp(double eventPos[2]);
	void OnMiddleButtonDown(double eventPos[2]);
	void OnMiddleButtonUp(double eventPos[2]);
	void OnRightButtonDown(double eventPos[2]);
	void OnRightButtonUp(double eventPos[2]);
	void OnMouseMove(double eventPos[2]);

	bool CheckHighlighted(int X, int Y);
protected:
	sppScalarBarRepresentation();
	~sppScalarBarRepresentation();

	sppScalarBarActorAdapter* ScalarBarActorAdapter;
	//upper and lower cones
	bool			   UpperConeSelected;
	double			   UpperConePos[2], UpperConeRelativePos;

	bool			   LowerConeSelected;
	double			   LowerConePos[2], LowerConeRelativePos;

	double			   ConeColor[3], SelectedConeColor[3];//unselected and selected colors

	bool               m_bLeftMouseDown;
	void               selectCone(bool bForceUnselect, double eventPos[2]);

	bool			   ConstantWidth; //if true, width resizing is disabled
private:
	sppScalarBarRepresentation(const sppScalarBarRepresentation&); // Not implemented
	void operator=(const sppScalarBarRepresentation&);   // Not implemented
};

#endif //__sppScalarBarRepresentation_h
