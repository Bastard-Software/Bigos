#pragma once
#include "Core/CoreTypes.h"
#include "Driver/Frontend/DriverSystemTypes.h"

#include "Core/Memory/MemorySystem.h"

namespace BIGOS
{
    struct BigosFrameworkDesc
    {
        Core::Memory::MemorySystemDesc     memorySystemDesc;
        Driver::Frontend::DriverSystemDesc driverSystemDesc;
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

        // TODO: Should that be private and create with framework?
        RESULT CreateDriverSystem( const Driver::Frontend::DriverSystemDesc& desc, Driver::Frontend::DriverSystem** ppSystem );
        void   DestroyDriverSystem( Driver::Frontend::DriverSystem** ppSystem );

        Core::Memory::MemorySystem*     GetMemorySystemPtr() { return &m_memorySystem; }
        Driver::Frontend::DriverSystem* GetDriverSystemPtr() { return m_pDriverSystem; }

        RESULT Create( const BigosFrameworkDesc& desc );
        void   Destroy();

    private:
        Core::Memory::MemorySystem      m_memorySystem;
        Driver::Frontend::DriverSystem* m_pDriverSystem = nullptr;
    };
} // namespace BIGOS