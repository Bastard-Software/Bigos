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
                IAdapter* pAdapter;
            };

            enum class QueueTypes : uint8_t
            {
                GRAPHICS,
                COMPUTE,
                COPY,
                // TODO: Support video (not soon)
                _MAX_ENUM,
            };
            using QUEUE_TYPE = QueueTypes;

            enum class QueuePriorityTypes : uint8_t
            {
                NORMAL,
                HIGH,
                REALTIME,
                _MAX_ENUM,
            };
            using QUEUE_PRIORITY_TYPE = QueuePriorityTypes;

            struct QueueDesc
            {
                QUEUE_TYPE          type;
                QUEUE_PRIORITY_TYPE priority;
            };

            struct QueueLimits
            {
                // TODO: Fill
            };

            struct QueueSubmitDesc
            {
                // TODO: Fill
            };

            struct QueueWaitDesc
            {
                // TODO: Fill
            };

            struct CommandPoolDesc
            {
                IQueue* pQueue;
            };

            enum class PipelineTypes : uint8_t
            {
                // TODO: Fill
            };
            using PIPELINE_TYPE = PipelineTypes;

            enum class PrimitiveTopologies : uint8_t
            {
                POINT_LIST,
                LINE_LIST,
                LINE_STRIP,
                TRIANGLE_LIST,
                TRIANGLE_STRIP,
                LINE_LIST_WITH_ADJACENCY,
                LINE_STRIP_WITH_ADJACENCY,
                TRIANGLE_LIST_WITH_ADJACENCY,
                TRIANGLE_STRIP_WITH_ADJACENCY,
                PATCH_LIST,
                _MAX_ENUM,
            };
            using PRIMITIVE_TOPOLOGY = PrimitiveTopologies;

            struct ViewportDesc
            {
                float upperLeftX;
                float upperLeftY;
                float width;
                float height;
                float minDepth;
                float maxDepth;
            };

            struct ScissorDesc
            {
                uint32_t upperLeftX;
                uint32_t upperLeftY;
                uint32_t width;
                uint32_t height;
            };

            enum class CommandBufferLevels : uint8_t
            {
                PRIMARY,
                SECONDARY, // TODO: Not supported yet
                _MAX_ENUM,
            };
            using COMMAND_BUFFER_LEVEL = CommandBufferLevels;

            struct CommandBufferDesc
            {
                CommandPoolHandle    hCommandPool;
                IQueue*              pQueue;
                COMMAND_BUFFER_LEVEL level;
            };

            struct BeginCommandBufferDesc
            {
                // For future use
            };

            struct BeginRenderingDesc
            {
                // TODO: Fill
            };

            struct DrawDesc
            {
                union
                {
                    uint32_t vertexCount;
                    uint32_t indexCount;
                };
                uint32_t instanceCount;
                union
                {
                    uint32_t firstVertex;
                    uint32_t firstIndex;
                };
                int32_t  vertexOffset;
                uint32_t firstInstance;
            };

            struct BarierDesc
            {
                // TODO: Fill
            };

            struct FenceDesc
            {
                uint64_t initialValue;
            };

            struct WaitForFencesDesc
            {
                FenceHandle* pFences;
                uint64_t*    pWaitValues;
                uint32_t     fenceCount;
                bool_t       waitAll;
            };

            struct SemaphoreDesc
            {
                // For future use
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS