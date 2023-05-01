#pragma once
#include "Core/CoreTypes.h"
#include "Core/Memory/MemoryTypes.h"

#include "Driver/Backend/API.h"

namespace BIGOS
{
    class BigosFramework;

    namespace Driver
    {
        namespace Backend
        {

            class VulkanFactory;
            class D3D12Factory;

        } // namespace Backend

        namespace Frontend
        {
            class DriverSystem;

            struct DriverSystemDesc
            {
                Backend::FactoryDesc             factoryDesc;
            };

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS