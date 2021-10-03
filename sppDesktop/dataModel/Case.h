#pragma once
#include "Result.h"

namespace SPP //Simulation Result View
{
	//information to locate a study on disk
	class CaseAddress
	{
	public:
		CaseAddress();
		virtual ~CaseAddress();

		void SetFile(const char* aFile);
		const char* GetFile()
		{
			return m_file.c_str();
		}
		void SetName(const char* aName);
		const char* GetName()
		{
			return m_name.c_str();
		}
	protected:
		std::string m_file;
		std::string m_name;
	};

	class Case
	{
	public:
		Case();
		virtual ~Case();

		void SetPath(const char* aPath)
		{
			m_path = aPath;
		}

		const char* GetPath()
		{
			return m_path.c_str();
		}

		void Reset(); //release all memory, initialize.

		void LoadPartData(int aPartIdx, int aPartPortion);//Portion 0: surface, 1: mesh edges, 2: vertex normals
		void LoadResultData(int aRltIdx, int aIndpIdx);
		void LoadSlicesOnParts(vtkPlane* aPlane);
		void LoadIsoSurfaces(int nClipValues);

		std::vector<Part>& GetPartList()
		{
			return m_partList;
		}

		std::vector<Result>& GetResultList()
		{
			return m_resultList;
		}

		Result* activeResult()
		{
			if (m_activeRlt < 0 || m_activeRlt >= m_resultList.size())
				return nullptr;

			return &m_resultList[m_activeRlt];

		}

		//return false if no data exists, bs can not be computed
		bool GetBounds(double* bs);

		vtkPlane* activePlane()
		{
			return m_activePlane.GetPointer();//the one that is being modified by the widget
		}

		void activePlane(vtkPlane* aPlane)
		{
			m_activePlane = aPlane;
		}
        
        std::vector<CuttingPlane>& cuttingPlanes()
        {
            return m_cuttingPlanes;
        }
        
        CuttingPlane* findCP(vtkPlane* aPlane);

		void surfaceVisible(bool b)
		{
			m_surfaceVisible = b;
		}
		bool surfaceVisible()
		{
			return m_surfaceVisible;
		}

		void isoSurfaceVisible(bool b)
		{
			m_isoVisible = b;
		}
		bool isoSurfaceVisible()
		{
			return m_isoVisible;
		}

		float m_scalarRange[2];
		int m_activeRlt;//the index of the result being displayed, if -1, mesh is displayed

		//isos on all parts are put together
		std::vector<float> m_isoClipValues;
		std::vector<vtkSmartPointer<vtkPolyData> > m_isoPDs;
		std::vector<vtkSmartPointer<vtkActor> > m_isoAs;

		std::vector<vtkSmartPointer<vtkPolyData> > m_streamLinePDs;
		std::vector<vtkSmartPointer<vtkActor> > m_streamLineAs;
	protected:
		vtkSmartPointer<vtkPlane> m_activePlane;//the one that is being modified by the widget
        std::vector<CuttingPlane> m_cuttingPlanes;
        
		bool m_surfaceVisible, m_isoVisible;

		std::vector<Part> m_partList;
		std::vector<Result> m_resultList;

		std::string m_path;//folder or file path while study data is stored
	};
}
