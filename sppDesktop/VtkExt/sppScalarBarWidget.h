#ifndef __sppScalarBarWidget_h
#define __sppScalarBarWidget_h

#pragma warning( push, 0 )
#include "vtkInteractionWidgetsModule.h" // For export macro
#include "vtkBorderWidget.h"
#pragma warning(pop)

class vtkScalarBarActor;
class sppScalarBarRepresentation;
class sppScalarBarActorAdapter;

class sppScalarBarWidget : public vtkBorderWidget
{
public:
	static sppScalarBarWidget* New();
	vtkTypeMacro(sppScalarBarWidget, vtkBorderWidget);

	// Description:
	// Specify an instance of vtkWidgetRepresentation used to represent this
	// widget in the scene. Note that the representation is a subclass of vtkProp
	// so it can be added to the renderer independent of the widget.
	virtual void SetRepresentation(sppScalarBarRepresentation* rep);

	// Description:
	// Return the representation as a sppScalarBarRepresentation.
	sppScalarBarRepresentation* GetScalarBarRepresentation()
	{
		return reinterpret_cast<sppScalarBarRepresentation*>(this->GetRepresentation());
	}

	// Description:
	// Get the ScalarBar used by this Widget. One is created automatically.
	virtual void SetScalarBarActorAdapter(sppScalarBarActorAdapter* actor);
	virtual sppScalarBarActorAdapter* GetScalarBarActorAdapter();

	// Description:
	// Can the widget be moved. On by default. If off, the widget cannot be moved
	// around.
	//
	// TODO: This functionality should probably be moved to the superclass.
	vtkSetMacro(Repositionable, int);
	vtkGetMacro(Repositionable, int);
	vtkBooleanMacro(Repositionable, int);

	// Description:
	// Create the default widget representation if one is not set.
	virtual void CreateDefaultRepresentation() override;

	vtkCommand* GetEventCallbackCommand() { return (vtkCommand*)EventCallbackCommand; };
protected:
	sppScalarBarWidget();
	~sppScalarBarWidget();

	//handles the events
	static void ProcessEvents(vtkObject* object,
		unsigned long event,
		void* clientdata,
		void* calldata);
	// ProcessEvents() dispatches to these methods.
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnMouseMove();

	int Repositionable;

	// Handle the case of Repositionable == 0
	static void MoveAction(vtkAbstractWidget*);

	// set the cursor to the correct shape based on State argument
	virtual void SetCursor(int State) override;

private:
	sppScalarBarWidget(const sppScalarBarWidget&);  //Not implemented
	void operator=(const sppScalarBarWidget&);  //Not implemented
};
#endif
