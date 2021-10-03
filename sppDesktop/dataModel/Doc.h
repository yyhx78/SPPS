#pragma once
#include "Case.h"

namespace SPP //Simulation Result View
{
	class Doc
	{
	public:
		//global access
		static Doc& getDoc();

		Doc();
		virtual ~Doc();

		const std::vector<CaseAddress>& GetCaseAddressList(bool aReload = false);

		void LoadCase(const char* aCasePath);

		Case& GetCase()
		{
			return m_study;
		}

		vtkRenderer* Renderer()
		{
			return m_renderer;
		}
		void Renderer(vtkRenderer* r)
		{
			m_renderer = r;
		}
	protected:
		std::vector<CaseAddress> m_studyAddressList;

		Case m_study;
		vtkRenderer* m_renderer;
	};
}
