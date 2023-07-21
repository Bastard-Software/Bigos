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
            class Device;

            using AdapterArray = Backend::AdapterArray;
            using DeviceArray  = HeapArray<Device*>; // Framework allows to create only one device for each adapter

            enum class AdapterTypes : uint8_t
            {
                FIRST_AVAILABLE,
                HIGH_PERFORMANCE,
                LOW_PERFORMANCE,
                DISCRETE,
                INTEGRATED,
                _MAX_ENUM,
            };
            using ADAPTER_TYPE = AdapterTypes;

            struct DriverSystemDesc
            {
                Backend::FactoryDesc factoryDesc;
            };

            struct DeviceDesc
            {
                struct
                {
                    uint8_t index = 0;
                    /* TODO: Handle that better way
                    ADAPTER_TYPE type  = AdapterTypes::FIRST_AVAILABLE;
                    const char*  pText = nullptr;
                    */
                } adapter;
            };

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS