#include "Driver/Frontend/DriverSystem.h"

namespace BIGOS
{
	namespace Driver
	{
		namespace Frontend
		{
			RESULT DriverSystem::Create(const DriverSystemDesc& desc, BigosEngine* pEngine)
			{
				BGS_ASSERT(pEngine != nullptr);
				m_pEngine = pEngine;
				m_desc = desc;

				return Results::OK;
			}

			void DriverSystem::Destroy()
			{
			}
		}
	}
}