#ifndef __sppPlaneWidget_h
#define __sppPlaneWidget_h

#pragma warning( push, 0 )
#include "vtkPolyDataSourceWidget.h"
#pragma warning(pop)

class vtkActor;
class vtkCellPicker;
class vtkConeSource;
class vtkLineSource;
class vtkPlaneSource;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkProp;
class vtkProperty;
class vtkSphereSource;
class vtkTransform;
class vtkPlane;
class vtkRegularPolygonSource;
class vtkCubeSource;

#define VTK_PLANE_OFF 0
#define VTK_PLANE_OUTLINE 1
#define VTK_PLANE_WIREFRAME 2
#define VTK_PLANE_SURFACE 3

class sppPlaneWidget : public vtkPolyDataSourceWidget
{
public:
	// Description:
	// Instantiate the object.
	static sppPlaneWidget* New();

	vtkTypeMacro(sppPlaneWidget, vtkPolyDataSourceWidget);

	// Description:
	// Methods that satisfy the superclass' API.
	//SetEnabled is specialized for synergy, when the int is 2, it is enabled but the interactor events are not hook up with the widget
	//in synergy, the callback command is directly envoked from the synergy view interactor
	virtual void SetEnabled(int) override;
	virtual void PlaceWidget(double radius); //center and normal are from the plane source
	virtual void PlaceWidget(double* bs) override; //center and normal are from the plane source
	virtual void PlaceWidget() override
	{
		this->Superclass::PlaceWidget();
	}
	void PlaceWidget(double xmin, double xmax, double ymin, double ymax,
		double zmin, double zmax) override
	{
		this->Superclass::PlaceWidget(xmin, xmax, ymin, ymax, zmin, zmax);
	}

	// Description:
	// Set/Get the resolution (number of subdivisions) of the plane.
	void SetResolution(int r);
	int GetResolution();

	// Description:
	// Set/Get the origin of the plane.
	void SetOrigin(double x, double y, double z);
	void SetOrigin(double x[3]);
	double* GetOrigin();
	void GetOrigin(double xyz[3]);

	// Description:
	// Set/Get the position of the point defining the first axis of the plane.
	void SetPoint1(double x, double y, double z);
	void SetPoint1(double x[3]);
	double* GetPoint1();
	void GetPoint1(double xyz[3]);

	// Description:
	// Set/Get the position of the point defining the second axis of the plane.
	void SetPoint2(double x, double y, double z);
	void SetPoint2(double x[3]);
	double* GetPoint2();
	void GetPoint2(double xyz[3]);

	// Description:
	// Get the center of the plane.
	void SetCenter(double x, double y, double z);
	void SetCenter(double x[3]);
	double* GetCenter();
	void GetCenter(double xyz[3]);

	// Description:
	// Get the normal to the plane.
	void SetNormal(double x, double y, double z);
	void SetNormal(double x[3]);
	double* GetNormal();
	void GetNormal(double xyz[3]);

	vtkGetMacro(State, int);

	// Description:
	// Control how the plane appears when GetPolyData() is invoked.
	// If the mode is "outline", then just the outline of the plane
	// is shown. If the mode is "wireframe" then the plane is drawn
	// with the outline plus the interior mesh (corresponding to the
	// resolution specified). If the mode is "surface" then the plane
	// is drawn as a surface.
	vtkSetClampMacro(Representation, int, VTK_PLANE_OFF, VTK_PLANE_SURFACE);
	vtkGetMacro(Representation, int);
	void SetRepresentationToOff()
	{
		this->SetRepresentation(VTK_PLANE_OFF);
	}
	void SetRepresentationToOutline()
	{
		this->SetRepresentation(VTK_PLANE_OUTLINE);
	}
	void SetRepresentationToWireframe()
	{
		this->SetRepresentation(VTK_PLANE_WIREFRAME);
	}
	void SetRepresentationToSurface()
	{
		this->SetRepresentation(VTK_PLANE_SURFACE);
	}

	// Description:
	// Force the plane widget to be aligned with one of the x-y-z axes.
	// Remember that when the state changes, a ModifiedEvent is invoked.
	// This can be used to snap the plane to the axes if it is orginally
	// not aligned.
	vtkSetMacro(Radius, double);
	vtkGetMacro(Radius, double);
	vtkSetMacro(NormalToXAxis, int);
	vtkGetMacro(NormalToXAxis, int);
	vtkBooleanMacro(NormalToXAxis, int);
	vtkSetMacro(NormalToYAxis, int);
	vtkGetMacro(NormalToYAxis, int);
	vtkBooleanMacro(NormalToYAxis, int);
	vtkSetMacro(NormalToZAxis, int);
	vtkGetMacro(NormalToZAxis, int);
	vtkBooleanMacro(NormalToZAxis, int);

	// Description:
	// Grab the polydata (including points) that defines the plane.  The
	// polydata consists of (res+1)*(res+1) points, and res*res quadrilateral
	// polygons, where res is the resolution of the plane. These point values
	// are guaranteed to be up-to-date when either the InteractionEvent or
	// EndInteraction events are invoked. The user provides the vtkPolyData and
	// the points and polyplane are added to it.
	void GetPolyData(vtkPolyData* pd);

	// Description:
	// Get the planes describing the implicit function defined by the plane
	// widget. The user must provide the instance of the class vtkPlane. Note
	// that vtkPlane is a subclass of vtkImplicitFunction, meaning that it can
	// be used by a variety of filters to perform clipping, cutting, and
	// selection of data.
	void GetPlane(vtkPlane* plane);

	// Description:
	// Satisfies superclass API.  This returns a pointer to the underlying
	// PolyData.  Make changes to this before calling the initial PlaceWidget()
	// to have the initial placement follow suit.  Or, make changes after the
	// widget has been initialised and call UpdatePlacement() to realise.
	vtkPolyDataAlgorithm* GetPolyDataAlgorithm() override;

	// Description:
	// Satisfies superclass API.  This will change the state of the widget to
	// match changes that have been made to the underlying PolyDataSource
	void UpdatePlacement(void) override;

	// Description:
	// Get the handle properties (the little balls are the handles). The
	// properties of the handles when selected and normal can be
	// manipulated.
	vtkGetObjectMacro(HandleProperty, vtkProperty);
	vtkGetObjectMacro(SelectedHandleProperty, vtkProperty);

	// Description:
	// Get the plane properties. The properties of the plane when selected
	// and unselected can be manipulated.
	virtual void SetPlaneProperty(vtkProperty*);
	vtkGetObjectMacro(PlaneProperty, vtkProperty);
	vtkGetObjectMacro(SelectedPlaneProperty, vtkProperty);

	vtkCommand* GetEventCallbackCommand() { return (vtkCommand*)EventCallbackCommand; };

	enum WidgetState
	{
		Start = 0,
		Moving, //moving on the same plane
		Scaling,
		Pushing, //moving in the normal direction
		Rotating,
		xRotating,//x axis
		yRotating,//y axis
		Spinning,
		Outside
	};
protected:
	sppPlaneWidget();
	~sppPlaneWidget();

	//BTX - manage the state of the widget
	int State;
	//ETX

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

	// controlling ivars
	int NormalToXAxis;
	int NormalToYAxis;
	int NormalToZAxis;
	int Representation;
	void SelectRepresentation();

	// the plane
	vtkActor* PlaneActor;
	vtkPolyDataMapper* PlaneMapper;
	vtkPlaneSource* PlaneSource;
	vtkPolyData* PlaneOutline;
	void HighlightPlane(int highlight);
	vtkActor* CubeActor;
	vtkPolyDataMapper* CubeMapper;
	vtkCubeSource* CubeSource;
	//    vtkSphereSource     *CubeSource;

	// glyphs representing hot spots (e.g., handles)
	void PositionHandles();

	// the normal cone
	vtkActor* ConeActor;
	vtkPolyDataMapper* ConeMapper;
	vtkConeSource* ConeSource;
	void HighlightNormal(int highlight);

	// the normal line
	vtkActor* LineActor;
	vtkPolyDataMapper* LineMapper;
	vtkLineSource* LineSource;

	//circles
	vtkActor* yzCircleActor;
	vtkPolyDataMapper* yzCircleMapper;
	vtkRegularPolygonSource* yzCircle;

	vtkActor* zxCircleActor;
	vtkPolyDataMapper* zxCircleMapper;
	vtkRegularPolygonSource* zxCircle;
	//1: yz, 2: zx
	void HighlightCircle(int iCircle, int highlight);

	// Do the picking
	vtkCellPicker* PlanePicker;
	vtkCellPicker* circlePicker;

	// Methods to manipulate the hexahedron.
	void MoveOrigin(double* p1, double* p2);
	void MovePoint1(double* p1, double* p2);
	void MovePoint2(double* p1, double* p2);
	void MovePoint3(double* p1, double* p2);
	void Rotate(int X, int Y, double* p1, double* p2, double* vpn);
	void circleRotate(double* p1, double* p2, double* center, double* normal);
	void Spin(double* p1, double* p2);
	void Scale(double* p1, double* p2, int X, int Y);
	void Translate(double* p1, double* p2);
	void Push(double* p1, double* p2);

	// Plane normal, normalized
	double Normal[3];
	double Radius;//size control

	// Transform the hexahedral points (used for rotations)
	vtkTransform* Transform;

	// Properties used to control the appearance of selected objects and
	// the manipulator in general.
	vtkProperty* HandleProperty;
	vtkProperty* SelectedHandleProperty;
	vtkProperty* PlaneProperty;
	vtkProperty* SelectedPlaneProperty;
	vtkProperty* circleProperty, * selectedCircleProperty;

	void CreateDefaultProperties();

	void GeneratePlane();

	int    LastPickValid;

private:
	sppPlaneWidget(const sppPlaneWidget&);  //Not implemented
	void operator=(const sppPlaneWidget&);  //Not implemented
};

#endif
