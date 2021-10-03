#pragma once
#include "Part.h"

namespace SPP //Simulation Result View
{
	class Result
	{
	public:
		Result();
		virtual ~Result();

		void SetName(const char* aName)
		{
			m_name = aName;
		}

        const char* GetName()
        {
            return m_name.c_str();
        }

        const char* GetUnit()
        {
            return m_unit.c_str();
        }

        void SetUnit(const char* aUnit)
        {
            m_unit = aUnit;
        }
        
		std::vector<double> m_indpValues;
	protected:
		std::string m_name;
        std::string m_unit;
	};
}
