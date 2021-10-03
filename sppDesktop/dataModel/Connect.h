#pragma once
#pragma warning( push, 0 )
#include "vtk_jsoncpp.h"
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkFloatArray.h"
#pragma warning(pop)

class vtkPlane;

namespace SPP //Simulation Result View
{
	class Connect
	{
	public:
        static Connect& cn();
        
		virtual ~Connect();

		void urlRoot(const char* url);
		const char* urlRoot();
        
        void mountedFolder(const char* aFolder);
        const char* mountedFolder();
        
        bool findDockerContainer(std::string &aId, std::string &aVersion, std::string &aIP, int &aPort, bool &aState);
        bool findContainerMountSource(const std::string &aId, std::string &aSource);

        bool createService(std::string &aPort, std::string &aDataFolder);
        bool startService(); //with the hard coded container name
        bool killService(const std::string &aIdName);
        bool removeService(const std::string &aIdName);

		bool getCaseList(Json::Value& jsonData);
		//return the info only, no data loading
		bool getCaseInfo(const char* aCaseFileName, Json::Value& jsonData);
		//a part composites with 3 PDs
		bool getPart(const char* aCaseFileName, int iPart, int iPortion,
			vtkSmartPointer<vtkPolyData>& aMeshTrisPD,
			vtkSmartPointer<vtkPolyData>& aMeshEdgePD,
			vtkSmartPointer<vtkPolyData>& aFeatureEdgePD);
		bool getResult(const char* aCaseFileName, int iRlt, int iIndp,
			float aScalarRange[],
			std::vector<vtkSmartPointer<vtkPolyData> >& aPdArrays, //return streamlines
			std::vector<vtkSmartPointer<vtkFloatArray> >& aRltArrays,
			std::vector<vtkSmartPointer<vtkFloatArray> >& aDispArrays
			);
		bool getSlicesOnParts(const char* aCaseFileName, vtkPlane* aPlane, int aRltIdx,
			std::vector<vtkSmartPointer<vtkPolyData> >& aSlicePDs,
			std::vector<vtkSmartPointer<vtkPolyData> >& aSliceMeshEdgesPDs);

		bool getIsoSurfaces(const char* aCaseFileName, int aNbIsoSs, int aRltIdx,
                            std::vector<float>& aIsoClipValues,
			std::vector<vtkSmartPointer<vtkPolyData> >& aIsoPDs);

		bool doExport();

        static bool IsPortInUse(int aPort);
	protected:
        Connect();
        
        std::string m_mountedFolder;
	};
}

/*
06/08/2021: zlib must be available to build libcurl with compression/uncomression support.
*/
