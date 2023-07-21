#pragma once
#include "APIHandles.h"

#include "Core/CoreTypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            class IAdapter;
            class IFactory;
            class IDevice;
            class IQueue;
            class ICommandBuffer;
            class ISwapchain;

            using AdapterArray = HeapArray<IAdapter*>;

            enum class AdapterTypes : uint8_t
            {
                UNKNOWN,
                INTEGRATED,
                DISCRETE,
                SOFTWARE,
                _MAX_ENUM,
            };
            using ADAPTER_TYPE = AdapterTypes;

            struct AdapterInfo
            {
                char         description[ 256 ]; // TODO: Make config file
                VersionDesc  apiVersion;
                VersionDesc  driverVersion;
                uint32_t     vendorID;
                uint32_t     deviceID;
                ADAPTER_TYPE type;
            };

            enum class APITypes : uint8_t
            {
                VULKAN,
                D3D12,
                _MAX_ENUM,
            };
            using API_TYPE = APITypes;

            enum class FactoryFlagBits : uint32_t
            {
                NONE                                    = 0x00000000,
                ENABLE_DEBUG_LAYERS                     = 0x00000001,
                ENABLE_DEBUG_LAYERS_IF_AVAILABLE        = 0x00000002,
                ENABLE_GPU_BASED_VALIDATION             = 0x00000004, // Those flags implicitly declares ENABLE_DEBUG_LAYER
                ENABLE_QUEUE_SYNCHRONIZATION_VALIDATION = 0x00000008, // Those flags implicitly declares ENABLE_DEBUG_LAYER
            };
            using FactoryFlags = uint32_t;

            struct FactoryDesc
            {
                FactoryFlags flags;
                API_TYPE     apiType;
            };

            struct DeviceDesc
            {
                AdapterHandle h_adapter;
            };

            enum class QueueTypes : uint8_t
            {
                GENERAL,
                COMPUTE,
                COPY,
                _MAX_ENUM,
            };
            using QUEUE_TYPE = QueueTypes;

            enum class QueuePriority : uint8_t
            {
                NORMAL,
                HIGH,
                _MAX_ENUM,
            };
            using QUEUE_PRIORITY = QueuePriority;

            struct QueueDesc
            {
                QUEUE_TYPE type;
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS