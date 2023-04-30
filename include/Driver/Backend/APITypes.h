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

            struct AdapterInfo
            {
                // TODO: Fill
            };

            enum class APITypes : uint8_t
            {
                VULKAN,
                D3D12,
                _MAX_ENUM,
            };
            using API_TYPE = APITypes;

            struct FactoryDesc
            {
                API_TYPE apiType;
                // TODO: Debug flags
            };

            struct DeviceDesc
            {
                // TODO: Fill
            };
        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS