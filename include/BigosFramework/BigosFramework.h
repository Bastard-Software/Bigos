#pragma once
#include "Core/CoreTypes.h"

#include "Core/Memory/MemorySystem.h"

namespace BIGOS
{
    struct BigosFrameworkDesc
    {
        BIGOS::Core::Memory::MemorySystemDesc memorySystemDesc;
    };

    class BigosFramework;
} // namespace BIGOS

extern "C"
{
    BGS_API BIGOS::RESULT CreateBigosFramework( const BIGOS::BigosFrameworkDesc& desc, BIGOS::BigosFramework** ppFramework );

    BGS_API void DestroyBigosFramework( BIGOS::BigosFramework** ppFramework );
}

namespace BIGOS
{
    class BGS_API BigosFramework
    {
    public:
        BigosFramework()  = default;
        ~BigosFramework() = default;

        BIGOS::Core::Memory::MemorySystem* GetMemorySystem() { return &m_memorySystem; }

        RESULT Create( const BigosFrameworkDesc& desc );
        void   Destroy();

    private:
        BIGOS::Core::Memory::MemorySystem m_memorySystem;
    };
} // namespace BIGOS