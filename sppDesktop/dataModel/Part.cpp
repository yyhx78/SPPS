#include "Result.h"

#include "Doc.h"
#include "Connect.h"
#pragma warning( push, 0 )
#include "vtkPointData.h"
#include "vtkRenderer.h"
#pragma warning(pop)

using namespace SPP;

Part::Part()
{
}

Part::~Part()
{

}

Cut* Part::FindCut(vtkPlane * aPlane)
{
    for (auto& c : m_cuts)
    {
        if (c.m_plane.GetPointer() == aPlane)
        {
            return &c;
        }
    }
    
    return nullptr;
}
