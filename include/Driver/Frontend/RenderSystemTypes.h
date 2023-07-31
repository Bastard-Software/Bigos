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

            class RenderSystem;
            class RenderDevice;

            using AdapterArray      = Backend::AdapterArray;
            using RenderDeviceArray = HeapArray<RenderDevice*>; // Framework allows to create only one device for each adapter
            using QueueArray        = HeapArray<Backend::IQueue*>;

            using QueueDesc = Backend::QueueDesc;

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

            struct RenderSystemDesc
            {
                Backend::FactoryDesc factoryDesc;
            };

            struct RenderDeviceDesc
            {
                struct
                {
                    uint8_t index = 0;
                    /* TODO: Handle that better way
                    ADAPTER_TYPE type  = AdapterTypes::FIRST_AVAILABLE;
                    const char*  pText = nullptr;
                    */
                } adapter;
                /* TODO: Expose to framework, for now we create all three type of queues
                struct
                {
                    QueueDesc* pQueueDescs;
                    uint32_t   queueCount;
                } queue;
                */
            };

        } // namespace Frontend
    }     // namespace Driver
} // namespace BIGOS