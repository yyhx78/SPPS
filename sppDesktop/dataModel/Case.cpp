#include "Case.h"

#include "Doc.h"
#include "Connect.h"
#pragma warning( push, 0 )
#include "vtkPointData.h"
#include "vtkRenderer.h"
#pragma warning(pop)

using namespace SPP;

CaseAddress::CaseAddress()
{
}

CaseAddress::~CaseAddress()
{

}

void CaseAddress::SetFile(const char* aFile)
{
	if (aFile)
		m_file = aFile;
}

void CaseAddress::SetName(const char* aName)
{
	if (aName)
		m_name = aName;
}

Case::Case() : m_surfaceVisible(true), m_isoVisible(false), m_activeRlt(-1)
{
}

Case::~Case()
{

}

CuttingPlane* Case::findCP(vtkPlane* aPlane)
{
    auto& lCPs = this->cuttingPlanes();
    for (auto& cp : lCPs)
    {
        if (cp.m_plane.GetPointer() == aPlane)
        {
            return &cp;
        }
    }
    
    return nullptr;
}

void Case::Reset()
{
	auto lRenderer = Doc::getDoc().Renderer();

	auto& lParts = this->GetPartList();
	for (auto& part : lParts)
	{
		if (part.m_surfaceA)
		{//surface
			lRenderer->RemoveActor(part.m_surfaceA);
		}
		if (part.m_meshEdges)
		{//mesh edges
			lRenderer->RemoveActor(part.m_meshEdgesA);
		}
		if (part.m_featureEdges)
		{//feature edges
			lRenderer->RemoveActor(part.m_featureEdgesA);
		}
        
        for (auto& cut : part.m_cuts)
        {
            if (cut.m_cutA)
            {//cutting plane
                lRenderer->RemoveActor(cut.m_cutA);
            }
		}
	}

	auto& lIsoAs = this->m_isoAs;
	for (auto a : lIsoAs)
	{
		lRenderer->RemoveActor(a);
	}

	auto& lStreamLineAs = this->m_streamLineAs;
	for (auto a : lStreamLineAs)
	{
		lRenderer->RemoveActor(a);
	}

	*this = Case();
}

bool Case::GetBounds(double* aBS)
{
	if (!aBS)
		return false;

	double *bs = aBS;

	auto& lParts = this->GetPartList();

	bool bInitialized = false;
	for (auto& part : lParts)
	{
		if (part.m_surface)
		{
			double pdBs[6];
			part.m_surface->GetBounds(pdBs);
			if (!bInitialized)
			{
				for (int i = 0; i < 6; ++i)
					bs[i] = pdBs[i];

				bInitialized = true;
			}
			else
			{
				for (int i = 0; i < 3; ++i)
				{
					if (bs[2 * i] > pdBs[2 * i])
						bs[2 + i] = pdBs[2 * i];
					if (bs[2 * i + 1] < pdBs[2 * i + 1])
						bs[2 + i + 1] = pdBs[2 * i + 1];
				}
			}
		}
	}

	return bInitialized;
}

void Case::LoadPartData(int aPartIdx, int aPortion)
{
	if (aPartIdx < 0 || m_path.empty())
		return;

	vtkSmartPointer<vtkPolyData> lMeshTrisPD, lMeshEdgePD, lFeatureEdgePD;
	auto &cn = Connect::cn();
	if (cn.getPart(m_path.c_str(), aPartIdx, aPortion, lMeshTrisPD, lMeshEdgePD, lFeatureEdgePD))
	{
		auto& lPart = GetPartList()[aPartIdx];
		if (aPortion == 1)
		{
			if (lMeshEdgePD)
			{
//				lMeshEdgePD->SetPoints(lPart.m_surface->GetPoints());
				lPart.m_meshEdges = lMeshEdgePD;
			}
		}
		else
		{
			lPart.m_surface = lMeshTrisPD;
			if (lMeshTrisPD)
				lPart.m_normals = lMeshTrisPD->GetPointData()->GetNormals();
			lPart.m_meshEdges = lMeshEdgePD;
			lPart.m_featureEdges = lFeatureEdgePD;
		}
	}
}

void Case::LoadResultData(int aRltIdx, int aIndpIdx)
{
	if (aRltIdx < 0 || m_path.empty())
		return;

	std::vector<vtkSmartPointer<vtkPolyData> > lPdArrays; //return streamlines
	std::vector<vtkSmartPointer<vtkFloatArray> > lScalarArrays;
	std::vector<vtkSmartPointer<vtkFloatArray> > lDisplacementArrays;
	float lScalarRange[2];
    auto &cn = Connect::cn();
	if (cn.getResult(m_path.c_str(), aRltIdx, aIndpIdx, lScalarRange, lPdArrays, lScalarArrays, lDisplacementArrays))
	{
		m_scalarRange[0] = lScalarRange[0];
		m_scalarRange[1] = lScalarRange[1];

		if (lPdArrays.empty())
		{
			//assign the array to each part
			auto& lParts = GetPartList();
			assert(lParts.size() == lScalarArrays.size());

			auto nParts = lParts.size();
			for (auto iPart = 0; iPart < nParts; ++iPart)
			{
				auto& lPart = lParts[iPart];

				lPart.m_tcoords = lScalarArrays[iPart];
				if (lPart.m_surface)
				{
					lPart.m_surface->GetPointData()->SetScalars(lScalarArrays[iPart]);
				}
				//ignore displacements for now
			}

			m_streamLinePDs.clear(); //a new result has been selected, release streamlines
		}
		else
		{//streamlines loaded
			this->m_streamLinePDs = lPdArrays;
		}
	}
}

void Case::LoadSlicesOnParts(vtkPlane* aPlane)
{
	if (!aPlane)
		return;

	std::vector<vtkSmartPointer<vtkPolyData> > lSlicePDs;
	std::vector<vtkSmartPointer<vtkPolyData> > lSliceMeshEdgePDs;
    auto &cn = Connect::cn();
	if (cn.getSlicesOnParts(m_path.c_str(), aPlane, this->m_activeRlt, lSlicePDs, lSliceMeshEdgePDs))
	{//number of cuts should be the same with number of parts (one cut on each part)
		auto& lParts = this->GetPartList();
		int lCuts = (int)lSlicePDs.size();
		if (lCuts > lParts.size())
			lCuts = (int)lParts.size();//take the smaller one
//		assert(lParts.size() == lSlicePDs.size());
		for (auto i = 0; i < lCuts; ++i)
		{
            auto lExistingCut = lParts[i].FindCut(aPlane);
            if (lExistingCut)
            {
                lExistingCut->m_cut = lSlicePDs[i];
            } else
            {//create a new cut
                lParts[i].m_cuts.push_back(Cut());
                auto& lNewCut = lParts[i].m_cuts.back();
                lNewCut.m_plane = aPlane;
                lNewCut.m_cut = lSlicePDs[i];
            }
		}
	}
}

void Case::LoadIsoSurfaces(int nClipValues)
{
	if (nClipValues < 0)
		return;

	std::vector<vtkSmartPointer<vtkPolyData> > lSlicePDs;
    auto &cn = Connect::cn();
	if (cn.getIsoSurfaces(m_path.c_str(), nClipValues, this->m_activeRlt, m_isoClipValues, lSlicePDs))
	{//number of cuts should be the same with number of parts (one cut on each part)
		this->m_isoPDs = lSlicePDs;
	}
}
