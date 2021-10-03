#include "Doc.h"

#include "Connect.h"
#pragma warning( push, 0 )
#include "vtkPointData.h"
#include "vtkRenderer.h"
#pragma warning(pop)

SPP::Doc& SPP::Doc::getDoc()
{
	static SPP::Doc d;
	return d;
}

using namespace SPP;

Doc::Doc() : m_renderer(nullptr)
{
}

Doc::~Doc()
{

}

void Doc::LoadCase(const char* aCasePath)
{
	if (!aCasePath)
		return;

	m_study.SetPath(aCasePath);

    auto &cn = SPP::Connect::cn();
	Json::Value jsonCaseInfo;
	cn.getCaseInfo(aCasePath, jsonCaseInfo);

	auto &lCase = m_study;
	//parts
	auto jsonParts = jsonCaseInfo["parts"];
	int nMeshCmpts = jsonParts.size();
	if (nMeshCmpts > 0) {
		auto &lPartList = lCase.GetPartList();
		lPartList.resize(nMeshCmpts);
		for (int i = 0; i < nMeshCmpts; i++)
		{
			auto jsonPart = jsonParts[i];
			lPartList[i].SetName(jsonPart["name"].asString().c_str());
		}
	}

	//results
	auto jsonRlts = jsonCaseInfo["results"];
	int nRlts = jsonRlts.size();
	if (nRlts > 0) {
		auto &lResultList = lCase.GetResultList();
		lResultList.resize(nRlts);
		for (int i = 0; i < nRlts; i++) { //for each new child
			auto jsonRlt = jsonRlts[i];
			auto& lRlt = lResultList[i];

			std::string ws = jsonRlt["name"].asString();
			lRlt.SetName(ws.c_str());
			ws = jsonRlt["unit"].asString();
			lRlt.SetUnit(ws.c_str());

			lRlt.m_indpValues.clear();
			int lIndpCount = jsonRlt["indps"].asInt(); //number of indp variables
			if (lIndpCount > 0)
			{
				auto jsonVals = jsonRlt["indpVals"][0]; //for now, only the first indp variable is considered
				if (jsonVals.type() == Json::arrayValue)
				{
					auto nValues = jsonVals.size();
					lRlt.m_indpValues.resize(nValues);
					for (Json::Value::ArrayIndex i = 0; i < nValues; ++i)
					{
						lRlt.m_indpValues[i] = jsonVals[i].asDouble();
					}
				}
			}
		}
	}
}

const std::vector<CaseAddress>& Doc::GetCaseAddressList(bool aReload)
{
	if (m_studyAddressList.empty() || aReload)
	{
        m_studyAddressList.clear();
        
        auto &cn = Connect::cn();
		Json::Value jsonCaseList;
		if (cn.getCaseList(jsonCaseList))
        {
            std::string s = jsonCaseList.toStyledString();
            if (!jsonCaseList.empty())
            {
                m_studyAddressList.resize(jsonCaseList.size());
                for (auto i = 0; i < (int)jsonCaseList.size(); ++i)
                {
                    m_studyAddressList[i].SetName(jsonCaseList[i]["name"].asString().c_str());
                    m_studyAddressList[i].SetFile(jsonCaseList[i]["file"].asString().c_str());
                }
            }
        }
	}

	return m_studyAddressList;
}
