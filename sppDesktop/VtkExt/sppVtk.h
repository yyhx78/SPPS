#pragma once

#pragma warning( push, 0 )

#define NotForWnd

#ifndef NotForWnd
#include "vtkWin32OpenGLRenderWindow.h"
#include "vtkWin32RenderWindowInteractor.h"
#endif

#include "vtkSmartPointer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPointPicker.h"
#include "vtkCellPicker.h"
#include "vtkPlaneSource.h"
#include "vtkTextMapper.h"
#pragma warning(pop)

#include <set>

class vtkPropPicker;
class vtkLightKit;
class vtkFollower;
class vtkScalarBarActor;
class vtkActor2D;

class sppScalarBarWidget;
class sppPlaneWidget;

typedef std::set<vtkProp*> VTK_PROP_SET;
/////////////////////////////////////////////////////////////////////////////
// CVtkRenderWindow window

enum EViewMode_Status
{
	eViewMode_Rotate,
	eViewMode_Pan,
	eViewMode_Dolly,
    eViewMode_RubberBand_Zoom,
    eViewMode_RubberBand_Select,
	eViewMode_Select,
	eViewMode_Query,//Feb. 26, 2008
	eViewMode_ResetView,
	eViewMode_Center,
	eViewMode_Unknown
};

enum EAxiesDispType
{
	eVWAxies_CARTESION_2D,
	eVWAxies_CARTESION_3D,
	eVWAxies_CYLINDER_T,//for thermoforming: a circle flat disk with x as r and y as z
	eVWAxies_CYLINDER_B//for blowmolding: a tube with y as r and x as z
};

#define IsSHIFTpressed() ( (GetKeyState(VK_SHIFT) & (1 << (sizeof(SHORT)*8-1))) != 0   )
#define IsCTRLpressed()  ( (GetKeyState(VK_CONTROL) & (1 << (sizeof(SHORT)*8-1))) != 0 )

class ScalarBarActor;
class vtkPlaneSource;

#define MAX_ADDITIONAL_RENDERERS 5

class sppVtk
{
public:
    sppVtk(vtkRenderWindow *rw = NULL, vtkRenderer *rder = NULL, vtkRenderWindowInteractor *rwi = NULL);
	virtual ~sppVtk();

	void ResetRenderer2();
	void ResetRenderer(vtkRenderer *r);

	vtkRenderer *Renderer, *Renderer2;
    
#ifdef NotForWnd
    vtkRenderWindow *RenderWindow;
	vtkRenderWindowInteractor *Interactor;
#else
    vtkWin32OpenGLRenderWindow *RenderWindow;
	vtkWin32RenderWindowInteractor *Interactor;
#endif
    
	void Render() {this->RenderWindow->Render();};
	void ResetView(	double  x,double  y,double  z,
                    double vx,double vy,double vz);
	void Reset();
	void AutoCenter();
	void DoCenter(int centerX, int centerY);
	void Zoom(double factor);
	void Dolly(double distance);
	void AddLightKit();
	void RemoveLightKit();

	void AddActor2D(vtkActor2D *pActor);
	void RemoveActor2D(vtkActor2D *pActor);

	void AddActor(vtkActor *pActor);
	void AddActor(vtkActor *pActor, int iRenderer);
	void RemoveActor(vtkActor *pActor);
	void RemoveActor(vtkActor *pActor, int iRenderer);
	bool FindActor(vtkProp *pP);
	void AddActors(vtkActorCollection *pActorCollection);
	void RemoveActors(vtkActorCollection *pActorCollection);
	void RemoveAllActors(vtkRenderer *r = NULL);

	vtkRenderer* GetRenderer(int iRenderer);//0: Renderer, >0: the additional renderer
	void AddAdditionalRenderer(double minX, double minY, double maxX, double maxY);
	vtkRenderer* GetAdditionalRenderer(int i) 
	{ 
		if (i >= 0 && i < m_nAdditionalRenderers)
			return m_AdditionalRenderers[i];
		else
			return 0;
	}
    
    int GetNumberOfScalarBars();
    void ShowScalarBar(int index, bool bShow, bool bResetClipValues = false);

	void SetStatus(EViewMode_Status status);
	EViewMode_Status GetStatus();

	void DoSelect(int ptX, int ptY);

	void						SetBkColor(float r, float g, float b);
	void						SetBkColor(float *bkColor);
	float*						GetBkColor(){return(m_bkColor);};
	float*						GetContrastColor() {return(m_ContrastColor);};
//protected:
#ifdef NotForWnd
#else
	LRESULT HandleMouseMsg(HWND hwnd, UINT msg, int &ptX, int &ptY);
#endif
    
	void ClipStart();
	void ClipMove(int &ptX, int &ptY);
	void ClipEnd();

	int CreateAxes();

	bool m_Wireframe,m_Clip;

	int							m_startPtX, m_startPtY, m_endPtX, m_endPtY;
	EViewMode_Status				m_eStatus;
	bool			 				MouseDown;

	float			m_bkColor[3];
	float			m_ContrastColor[3];//supposed to change with bk color
	void			OnBkColorChanged();
	void			SetAxesLineColor(float *color);
	vtkFollower		*m_pGnomonLabelX, *m_pGnomonLabelY, *m_pGnomonLabelZ;
    
	vtkPointPicker* SelectPoint(int x, int y, double tol);
	vtkCellPicker*  SelectCell(int x, int y, double tol);
	vtkPropPicker*  SelectProp(int x, int y, double tol);

	void SetSelectPointMethod( void (*f)(sppVtk* pRW, vtkPointPicker *pPicker, void *arg));
	void SetSelectPointMethodArg( void * pArg);
	void SetSelectCellMethod( void (*f)(sppVtk* pRW, vtkCellPicker *pPicker, void *arg));
	void SetSelectCellMethodArg( void * pArg);
	void SetSelectPropMethod( void (*f)(sppVtk* pRW, vtkPropPicker *pPicker, void *arg));
	void SetSelectPropMethodArg( void * pArg);

	void (*SelectPointMethod)(sppVtk* pRW, vtkPointPicker *pPicker, void *arg);
	void *SelectPointMethodArg;
	void (*SelectCellMethod)(sppVtk* pRW, vtkCellPicker *pPicker, void *arg);
	void *SelectCellMethodArg;
	void (*SelectPropMethod)(sppVtk* pRW, vtkPropPicker *pPicker, void *arg);
	void *SelectPropMethodArg;

	void SetEndSelectMethod( void (*f)(sppVtk *pVW, void *arg));
	void SetEndSelectMethodArg( void * pArg);

	void SaveImage(const char* aFileName);

	EAxiesDispType	
		GetAxiesDispType() { return(m_AxiesDispType); };
	void
		SetAxiesDispType(EAxiesDispType type);

	void ToggleOriginVisible();


	void SetCellPickerTol(double tol) { m_CellPickTol = tol; }
	double GetCellPickerTol() { return(m_CellPickTol); };

    sppScalarBarWidget* scalarBarWidget(int index);
    sppPlaneWidget* planeWidget();
    void ShowPlaneWidget(bool bShow, double *center = NULL, double *normal = NULL, double radius = 1.0);

	vtkTextMapper *m_pVWStatusNameMapper;

	VTK_PROP_SET	m_ActiveActors;
private:
    void Construct();
    
    vtkSmartPointer<sppPlaneWidget> m_planeWidget;
    std::vector<vtkSmartPointer<sppScalarBarWidget> > m_scalarWidgets;
    
	EAxiesDispType	m_AxiesDispType;
	//Clipping
	double							m_ClipPlanePushStepLength;
	vtkActor*						m_pClipPlaneScreenActor;
	vtkLightKit*					m_pLightKit;
	vtkPlane*						m_pClipPlane;
	vtkPlaneSource*					m_pClipPlaneScreen;
	vtkActorCollection			    *m_pOriginActors;

	double m_CellPickTol;//line and point have to have different tol

	void (*EndSelectMethod)(sppVtk *pVW, void *arg);
	void *EndSelectMethodArg;

	vtkPointPicker *m_pPointPicker;
	vtkCellPicker  *m_pCellPicker;
	vtkPropPicker  *m_pPropPicker;

	int m_nAdditionalRenderers;//the max. is MAX_ADDITIONAL_RENDERERS
	vtkRenderer *m_AdditionalRenderers[MAX_ADDITIONAL_RENDERERS];
};
/////////////////////////////////////////////////////////////////////////////
