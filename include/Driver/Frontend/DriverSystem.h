#pragma once
#include "DriverSystemTypes.h"

namespace BIGOS
{
    class BigosEngine;

    namespace Driver
    {
        namespace Frontend
        {
            class BGS_API DriverSystem final
            {
            public:
                friend class BigosEngine;

                const DriverSystemDesc& GetDesc() const { return m_desc; }

            protected:
                RESULT Create( const DriverSystemDesc& desc, BigosEngine* pEngine );
                void   Destroy();

            private:
                DriverSystemDesc m_desc;
                BigosEngine*     m_pEngine = nullptr;
            };
        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS