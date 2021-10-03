#pragma once
#include <vector>
#include <string>
#include <map>

#pragma warning( push, 0 )
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkActor.h"
#include "vtkFloatArray.h"
#include "vtkPlane.h"
#pragma warning(pop)

namespace SPP //Simulation Result View
{
    struct CuttingPlane
    {
        CuttingPlane() : m_enabled(true)
        {
            
        }
        
        bool enabled()
        {
            return m_enabled;
        }
        void enabled(bool b)
        {
            m_enabled = b;
        }
        vtkSmartPointer<vtkPlane> m_plane;
    protected:
        bool m_enabled;
    };
    struct Cut
    {
        vtkSmartPointer<vtkPlane> m_plane;
        vtkSmartPointer<vtkPolyData> m_cut;
        vtkSmartPointer<vtkActor> m_cutA;
    };

	class Part
	{
	public:
		Part();
		virtual ~Part();

		void SetName(const char* aName)
		{
			m_name = aName;
		}

		const char* GetName()
		{
			return m_name.c_str();
		}
        
        Cut* FindCut(vtkPlane * aPlane);

		vtkSmartPointer<vtkPolyData> m_surface, m_meshEdges, m_featureEdges;
		vtkSmartPointer<vtkActor> m_surfaceA, m_meshEdgesA, m_featureEdgesA;
		vtkSmartPointer<vtkFloatArray> m_tcoords, m_displacements;
		vtkSmartPointer<vtkDataArray> m_normals; //remember this separately to turn on/off Gouraod shading
        
        std::vector<Cut> m_cuts;
	protected:
		std::string m_name;
	};
}
