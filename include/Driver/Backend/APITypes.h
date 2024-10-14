#pragma once
#include "APIHandles.h"

#include "Core/CoreTypes.h"
#include "Platform/PlatformTypes.h"

#include "BigosFramework/Config.h"

#if defined DOMAIN
#    undef DOMAIN
#endif

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

            struct BackBufferInfo;

            using AdapterArray    = HeapArray<IAdapter*>;
            using BackBufferArray = HeapArray<BackBufferInfo>;

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
                char         description[ Config::Driver::Adapter::MAX_DESCRIPTION_TEXT_LENGHT ];
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

            struct DeviceLimits
            {
                uint64_t samplerBindingSize;                // SAMPLER
                uint64_t sampledTextureBindingSize;         // SAMPLED_TEXTURE
                uint64_t storageTextureBindingSize;         // STORAGE_TEXTURE
                uint64_t constantTexelBufferBindingSize;    // CONSTANT_TEXEL_BUFFER
                uint64_t storageTexelBufferBindingSize;     // STORAGE_TEXEL_BUFFER
                uint64_t constantBufferBindingSize;         // CONSTANT_BUFFER
                uint64_t readOnlyStorageBufferBindingSize;  // READ_ONLY_STORAGE_BUFFER
                uint64_t readWriteStorageBufferBindingSize; // READ_WRITE_STORAGE_BUFFER
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
                float timestampPeriod; // Indicates the number of nanoseconds required for a timestamp query to be incremented by 1
            };

            struct QueueSubmitDesc
            {
                const SemaphoreHandle*       phWaitSemaphores;
                const FenceHandle*           phWaitFences;
                const uint64_t*              pWaitValues;
                const SemaphoreHandle*       phSignalSemaphores;
                const FenceHandle*           phSignalFences;
                const uint64_t*              pSignalValues;
                const ICommandBuffer* const* ppCommandBuffers;
                uint32_t                     waitSemaphoreCount;
                uint32_t                     signalSemaphoreCount;
                uint32_t                     waitFenceCount;
                uint32_t                     signalFenceCount;
                uint32_t                     commandBufferCount;
            };

            enum class ShaderVisibilities : uint8_t
            {
                VERTEX,
                PIXEL,
                GEOMETRY,
                HULL,
                DOMAIN,
                COMPUTE,
                ALL_GRAPHICS,
                ALL,
                _MAX_ENUM,
            };
            using SHADER_VISIBILITY = ShaderVisibilities;

            struct PushConstantRangeDesc
            {
                SHADER_VISIBILITY visibility;
                uint32_t          baseConstantSlot;
                uint32_t          constantCount;
                uint32_t          shaderRegister;
            };

            struct PipelineLayoutDesc
            {
                const BindingSetLayoutHandle* phBindigSetLayouts;
                const PushConstantRangeDesc*  pConstantRanges;
                uint32_t                      constantRangeCount;
                uint32_t                      bindingSetLayoutCount;
            };

            enum class Formats : uint8_t
            {
                UNKNOWN,
                B5G5R5A1_UNORM,
                B5G6R5_UNORM,
                B8G8R8A8_UNORM,
                B8G8R8A8_UNORM_SRGB,
                BC1_UNORM,
                BC1_UNORM_SRGB,
                BC2_UNORM,
                BC2_UNORM_SRGB,
                BC3_UNORM,
                BC3_UNORM_SRGB,
                BC4_UNORM,
                BC4_SNORM,
                BC5_UNORM,
                BC5_SNORM,
                BC6H_UFLOAT,
                BC6H_SFLOAT,
                BC7_UNORM,
                BC7_UNORM_SRGB,
                R8_UINT,
                R8_SINT,
                R8_UNORM,
                R8_SNORM,
                R16_UINT,
                R16_SINT,
                R16_UNORM,
                R16_SNORM,
                R16_FLOAT,
                D16_UNORM,
                R8G8_UINT,
                R8G8_SINT,
                R8G8_UNORM,
                R8G8_SNORM,
                D24_UNORM_S8_UINT,
                R32_FLOAT,
                R32_UINT,
                R32_SINT,
                D32_FLOAT,
                R16G16_UINT,
                R16G16_SINT,
                R16G16_UNORM,
                R16G16_SNORM,
                R16G16_FLOAT,
                R8G8B8A8_UNORM,
                R8G8B8A8_SNORM,
                R8G8B8A8_UNORM_SRGB,
                R8G8B8A8_UINT,
                R8G8B8A8_SINT,
                R32G32_UINT,
                R32G32_SINT,
                R32G32_FLOAT,
                R16G16B16A16_UNORM,
                R16G16B16A16_SNORM,
                R16G16B16A16_UINT,
                R16G16B16A16_SINT,
                R16G16B16A16_FLOAT,
                R32G32B32_UINT,
                R32G32B32_SINT,
                R32G32B32_FLOAT,
                R32G32B32A32_UINT,
                R32G32B32A32_SINT,
                R32G32B32A32_FLOAT,
                _MAX_ENUM,
            };
            using FORMAT = Formats;

            struct SwapchainDesc
            {
                Platform::Window* pWindow;
                IQueue*           pQueue;
                uint32_t          backBufferCount;
                bool_t            vSync;
                FORMAT            format;
            };

            struct SwapchainResizeDesc
            {
            };

            struct SwapchainPresentDesc
            {
                SemaphoreHandle* pWaitSemaphores;
                uint32_t         waitSemaphoreCount;
            };

            struct FrameInfo
            {
                SemaphoreHandle hBackBufferAvailableSemaphore;
                uint32_t        swapChainBackBufferIndex;
            };

            struct BackBufferInfo
            {
                ResourceHandle  hBackBuffer;
                SemaphoreHandle hBackBufferAvailableSemaphore;
            };

            struct CommandPoolDesc
            {
                IQueue* pQueue;
            };

            enum class ShaderTypes : uint8_t
            {
                VERTEX,
                PIXEL,
                GEOMETRY,
                HULL,
                DOMAIN,
                COMPUTE,
                // TODO: Ray tracing
                _MAX_ENUM,
            };
            using SHADER_TYPE = ShaderTypes;

            struct ShaderDesc
            {
                const byte_t* pByteCode;
                uint32_t      codeSize;
            };

            struct ShaderStageDesc
            {
                ShaderHandle hShader;
                const char*  pEntryPoint;
            };

            enum class PipelineTypes : uint8_t
            {
                GRAPHICS,
                COMPUTE,
                RAY_TRACING,
                _MAX_ENUM,
            };
            using PIPELINE_TYPE = PipelineTypes;

            struct PipelineDesc
            {
                PipelineLayoutHandle hPipelineLayout;
                PIPELINE_TYPE        type;
            };

            enum class InputStepRates : uint8_t
            {
                PER_VERTEX,
                PER_INSTANCE,
                _MAX_ENUM,
            };
            using INPUT_STEP_RATE = InputStepRates;

            struct InputBindingDesc
            {
                uint32_t        binding;
                INPUT_STEP_RATE inputRate;
            };

            struct InputElementDesc
            {
                const char* pSemanticName;
                uint32_t    location;
                uint32_t    binding;
                uint32_t    offset;
                FORMAT      format;
            };

            struct InputStateDesc
            {
                const InputBindingDesc* pInputBindings;
                const InputElementDesc* pInputElements;
                uint32_t                inputBindingCount;
                uint32_t                inputElementCount;
            };

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

            enum class IndexRestartValues
            {
                DISABLED,         // Check if we need to add value for uint8 index type
                ENABLED_0xFFFF,   // For index type uint16
                ENABLED_0xFFFFFF, // For index type uint32
                _MAX_ENUM,
            };
            using INDEX_RESTART_VALUE = IndexRestartValues;

            struct InputAssemblerStateDesc
            {
                PRIMITIVE_TOPOLOGY  topology;
                INDEX_RESTART_VALUE indexRestartValue;
            };

            enum class PolygonFillModes : uint8_t
            {
                WIREFRAME,
                SOLID,
                _MAX_ENUM,
            };
            using POLYGON_FILL_MODE = PolygonFillModes;

            enum class CullModes : uint8_t
            {
                NONE,
                FRONT,
                BACK,
                _MAX_ENUM,
            };
            using CULL_MODE = CullModes;

            enum class FrontFaceModes
            {
                COUNTER_CLOCKWISE,
                CLOCKWISE,
                _MAX_ENUM,
            };
            using FRONT_FACE_MODE = FrontFaceModes;

            struct RasterizerStateDesc
            {
                float             depthBiasConstantFactor;
                float             depthBiasSlopeFactor;
                float             depthBiasClamp;
                bool_t            depthClipEnable;
                POLYGON_FILL_MODE fillMode;
                CULL_MODE         cullMode;
                FRONT_FACE_MODE   frontFaceMode;
            };

            enum class SampleCount : uint8_t
            {
                COUNT_1,
                COUNT_2,
                COUNT_4,
                COUNT_8,
                COUNT_16,
                COUNT_32,
                COUNT_64,
                _MAX_ENUM,
            };
            using SAMPLE_COUNT = SampleCount;

            struct MultisampleStateDesc
            {
                SAMPLE_COUNT sampleCount;
            };

            enum class CompareOperationTypes : uint8_t
            {
                NEVER,
                LESS,
                EQUAL,
                LESS_OR_EQUAL,
                GREATER,
                NOT_EQUAL,
                GREATER_OR_EQUAL,
                ALWAYS,
                _MAX_ENUM,
            };
            using COMPARE_OPERATION_TYPE = CompareOperationTypes;

            enum class StencilOperationTypes : uint8_t
            {
                KEEP,
                ZERO,
                REPLACE,
                INCREMENT_AND_CLAMP,
                DECREMENT_AND_CLAMP,
                INVERT,
                INCREMENT_AND_WRAP,
                DECREMENT_AND_WRAP,
                _MAX_ENUM,
            };
            using STENCIL_OPERATION_TYPE = StencilOperationTypes;

            struct StencilOperationDesc
            {
                STENCIL_OPERATION_TYPE failOp;
                STENCIL_OPERATION_TYPE passOp;
                STENCIL_OPERATION_TYPE depthFailOp;
                COMPARE_OPERATION_TYPE compareOp;
            };

            struct DepthStencilStateDesc
            {
                bool_t                 depthTestEnable;
                bool_t                 depthWriteEnable;
                bool_t                 stencilTestEnable;
                StencilOperationDesc   frontFace;
                StencilOperationDesc   backFace;
                uint8_t                stencilWriteMask;
                uint8_t                stencilReadMask;
                COMPARE_OPERATION_TYPE depthCompare;
            };

            enum class BlendFactors : uint8_t
            {
                ZERO,
                ONE,
                SRC_COLOR,
                ONE_MINUS_SRC_COLOR,
                DST_COLOR,
                ONE_MINUS_DST_COLOR,
                SRC_ALPHA,
                ONE_MINUS_SRC_ALPHA,
                DST_ALPHA,
                ONE_MINUS_DST_ALPHA,
                SRC1_COLOR,
                ONE_MINUS_SRC1_COLOR,
                SRC1_ALPHA,
                ONE_MINUS_SRC1_ALPHA,
                SRC_ALPHA_SATURATE,
                _MAX_ENUM
            };
            using BLEND_FACTOR = BlendFactors;

            enum class BlendOperationTypes : uint8_t
            {
                ADD,
                SUBTRACT,
                REVERSE_SUBTRACT,
                MIN,
                MAX,
                _MAX_ENUM
            };
            using BLEND_OPERATION_TYPE = BlendOperationTypes;

            enum class ColorComponentFlagBits : uint32_t
            {
                RED   = 0x00000001,
                GREEN = 0x00000002,
                BLUE  = 0x00000004,
                ALPHA = 0x00000008,
                ALL   = RED | GREEN | BLUE | ALPHA
            };
            using ColorComponentFlags = uint32_t;

            struct RenderTargetBlendDesc
            {

                BLEND_FACTOR         srcColorBlendFactor;
                BLEND_FACTOR         dstColorBlendFactor;
                BLEND_OPERATION_TYPE colorBlendOp;
                BLEND_FACTOR         srcAlphaBlendFactor;
                BLEND_FACTOR         dstAlphaBlendFactor;
                BLEND_OPERATION_TYPE alphaBlendOp;
                ColorComponentFlags  writeFlag;
                bool_t               blendEnable;
            };

            enum class LogicOperationTypes : uint8_t
            {
                CLEAR,
                AND,
                AND_REVERSE,
                COPY,
                AND_INVERTED,
                NO_OP,
                XOR,
                OR,
                NOR,
                EQUIVALENT,
                INVERT,
                OR_REVERSE,
                COPY_INVERTED,
                OR_INVERTED,
                NAND,
                SET,
                _MAX_ENUM,
            };
            using LOGIC_OPERATION_TYPE = LogicOperationTypes;

            struct BlendStateDesc
            {
                RenderTargetBlendDesc* pRenderTargetBlendDescs;
                uint32_t               renderTargetBlendDescCount;
                LOGIC_OPERATION_TYPE   logicOperation;
                /// <summary>
                /// Enables logic operations.
                /// </summary>
                bool enableLogicOperations;
            };

            struct GraphicsPipelineDesc : public PipelineDesc
            {
                ShaderStageDesc         vertexShader;
                ShaderStageDesc         pixelShader;
                ShaderStageDesc         domainShader;
                ShaderStageDesc         hullShader;
                ShaderStageDesc         geometryShader;
                InputStateDesc          inputState;
                InputAssemblerStateDesc inputAssemblerState;
                RasterizerStateDesc     rasterizeState;
                MultisampleStateDesc    multisampleState;
                DepthStencilStateDesc   depthStencilState;
                BlendStateDesc          blendState;
                uint32_t                renderTargetCount;
                const FORMAT*           pRenderTargetFormats;
                FORMAT                  depthStencilFormat;
            };

            struct ComputePipelineDesc : public PipelineDesc
            {
                ShaderStageDesc computeShader;
            };

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
                Rect2D                    renderArea;
                const ResourceViewHandle* phColorRenderTargetViews;
                ResourceViewHandle        hDepthStencilTargetView;
                uint32_t                  colorRenderTargetCount;
            };

            struct ColorValue
            {
                float r;
                float g;
                float b;
                float a;
            };

            struct DepthStencilValue
            {
                float   depth;
                uint8_t stencil;
            };

            struct VertexBufferDesc
            {
                ResourceHandle hVertexBuffer;
                uint64_t       offset;
                uint32_t       size;
                uint32_t       elementStride;
            };

            enum class IndexTypes : uint8_t
            {
                UINT8,
                UINT16,
                UINT32,
                _MAX_ENUM,
            };
            using INDEX_TYPE = IndexTypes;

            struct IndexBufferDesc
            {
                ResourceHandle hIndexBuffer;
                uint32_t       offset;
                INDEX_TYPE     indexType;
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

            struct DispatchDesc
            {
                uint32_t groupCountX;
                uint32_t groupCountY;
                uint32_t groupCountZ;
            };

            struct ExecuteIndirectDesc
            {
                CommandLayoutHandle hCommandLayout;
                ResourceHandle      hIndirectBuffer;
                ResourceHandle      hCountBuffer;
                uint64_t            indirectBufferOffset;
                uint64_t            countBufferOffset;
                uint32_t            maxCommandCount;
            };

            enum class TextureComponentFlagBits : uint32_t
            {
                DEPTH   = 0x00000001,
                STENCIL = 0x00000002,
                COLOR   = 0x00000004,
            };
            using TextureComponentFlags = uint32_t;

            struct TextureRangeDesc
            {
                TextureComponentFlags components;
                uint32_t              mipLevel;
                uint32_t              mipLevelCount;
                uint32_t              arrayLayer;
                uint32_t              arrayLayerCount;
            };

            struct BufferRangeDesc
            {
                uint64_t size;
                uint64_t offset;
            };

            enum class PipelineStageFlagBits
            {
                NONE             = 0x00000000,
                INPUT_ASSEMBLER  = 0x00000001,
                VERTEX_SHADING   = 0x00000002,
                PIXEL_SHADING    = 0x00000004,
                COMPUTE_SHADING  = 0x00000008,
                TRANSFER         = 0x00000010,
                RESOLVE          = 0x00000020,
                EXECUTE_INDIRECT = 0x00000040,
                RENDER_TARGET    = 0x00000080,
                DEPTH_STENCIL    = 0x00000100,
                // TODO: Video, raytracing, shading rate
                // TODO: Sum of the flags
            };
            using PipelineStageFlags = uint32_t;

            enum class AccessFlagBits : uint32_t
            {
                NONE                = 0x00000000,
                GENERAL             = 0x00000001,
                VERTEX_BUFFER       = 0x00000002,
                INDEX_BUFFER        = 0x00000004,
                CONSTANT_BUFFER     = 0x00000008,
                INDIRECT_BUFFER     = 0x00000010,
                RENDER_TARGET       = 0x00000020,
                SHADER_READ_ONLY    = 0x00000040,
                SHADER_READ_WRITE   = 0x00000080,
                DEPTH_STENCIL_READ  = 0x00000100,
                DEPTH_STENCIL_WRITE = 0x00000200,
                TRANSFER_SRC        = 0x00000400,
                TRANSFER_DST        = 0x00000800,
                RESOLVE_SRC         = 0x00001000,
                RESOLVE_DST         = 0x00002000,
                // TODO: Video, raytracing, shading rate
                // TODO: Sum of the flags
            };
            using AccessFlags = uint32_t;

            enum class TextureLayouts : uint8_t
            {
                UNDEFINED,
                GENERAL,
                PRESENT,
                RENDER_TARGET,
                DEPTH_STENCIL_READ,
                DEPTH_STENCIL_WRITE,
                SHADER_READ_ONLY,
                SHADER_READ_WRITE,
                TRANSFER_SRC,
                TRANSFER_DST,
                RESOLVE_SRC,
                RESOLVE_DST,
                // TODO: Video, raytracing, shading rate
                _MAX_ENUM,
            };
            using TEXTURE_LAYOUT = TextureLayouts;

            struct GlobalBarrierDesc
            {
                PipelineStageFlags srcStage;
                AccessFlags        srcAccess;
                PipelineStageFlags dstStage;
                AccessFlags        dstAccess;
            };

            struct BufferBarrierDesc
            {
                BufferRangeDesc    bufferRange;
                ResourceHandle     hResouce;
                PipelineStageFlags srcStage;
                AccessFlags        srcAccess;
                PipelineStageFlags dstStage;
                AccessFlags        dstAccess;
            };

            struct TextureBarrierDesc
            {
                TextureRangeDesc   textureRange;
                ResourceHandle     hResouce;
                PipelineStageFlags srcStage;
                AccessFlags        srcAccess;
                TEXTURE_LAYOUT     srcLayout;
                PipelineStageFlags dstStage;
                AccessFlags        dstAccess;
                TEXTURE_LAYOUT     dstLayout;
            };

            struct BarierDesc
            {
                const TextureBarrierDesc* pTextureBarriers;
                const BufferBarrierDesc*  pBufferBarriers;
                const GlobalBarrierDesc*  pGlobalBarriers;
                uint32_t                  textureBarrierCount;
                uint32_t                  bufferBarrierCount;
                uint32_t                  globalBarrierCount;
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

            enum class MemoryHeapTypes : uint8_t
            {
                DEFAULT,
                UPLOAD,
                READBACK,
                CUSTOM,
                _MAX_ENUM
            };
            using MEMORY_HEAP_TYPE = MemoryHeapTypes;

            enum class MemoryHeapUsages : uint8_t
            {
                BUFFERS,
                TEXTURES,
                RENDER_TARGETS,
                _MAX_ENUM,
            };
            using MEMORY_HEAP_USAGE = MemoryHeapUsages;

            enum class MemoryAccessFlagBits : uint32_t
            {
                CPU_READS_FROM_DEVICE = 0x00000001,
                CPU_WRITES_TO_DEVICE  = 0x00000002,
                GPU_READS_FROM_DEVICE = 0x00000004,
                GPU_WRITES_TO_DEVICE  = 0x00000008,
                GPU_WRITES_TO_SYSTEM  = 0x00000010,
                GPU_READS_FROM_SYSTEM = 0x00000020,
                CPU_WRITES_TO_SYSTEM  = 0x00000040,
                CPU_READS_FROM_SYSTEM = 0x00000080,
                CPU_DEVICE_ACCESS     = CPU_READS_FROM_DEVICE | CPU_WRITES_TO_DEVICE,
                CPU_SYSTEM_ACCESS     = CPU_READS_FROM_SYSTEM | CPU_WRITES_TO_SYSTEM,
                GPU_DEVICE_ACCESS     = GPU_READS_FROM_DEVICE | GPU_WRITES_TO_DEVICE,
                GPU_SYSTEM_ACCESS     = GPU_READS_FROM_SYSTEM | GPU_WRITES_TO_SYSTEM,
                GPU_ACCESS            = GPU_DEVICE_ACCESS | GPU_SYSTEM_ACCESS,
                CPU_ACCESS            = CPU_DEVICE_ACCESS | CPU_SYSTEM_ACCESS,
                CPU_READ              = CPU_READS_FROM_DEVICE | CPU_READS_FROM_SYSTEM,
                CPU_WRITE             = CPU_WRITES_TO_SYSTEM | CPU_WRITES_TO_DEVICE,
                GPU_READ              = GPU_READS_FROM_DEVICE | GPU_READS_FROM_SYSTEM,
                GPU_WRITE             = GPU_WRITES_TO_DEVICE,
                ALL                   = CPU_ACCESS | GPU_ACCESS
            };
            using MemoryAccessFlags = uint32_t;

            struct AllocateMemoryDesc
            {
                uint64_t          size;
                uint64_t          alignment;
                MemoryAccessFlags access;
                MEMORY_HEAP_TYPE  heapType;
                MEMORY_HEAP_USAGE heapUsage;
            };

            enum class ResourceTypes : uint8_t
            {
                UNKNOWN,
                BUFFER,
                TEXTURE_1D,
                TEXTURE_2D,
                TEXTURE_3D,
                _MAX_ENUM,
            };
            using RESOURCE_TYPE = ResourceTypes;

            enum class ResourceLayouts : uint8_t
            {
                OPTIMAL,
                LINEAR,
                _MAX_ENUM,
            };
            using RESOURCE_LAYOUT = ResourceLayouts;

            enum class ResourceUsageFlagBits : uint32_t
            {
                TRANSFER_SRC              = 0x00000001,
                TRANSFER_DST              = 0x00000002,
                CONSTANT_TEXEL_BUFFER     = 0x00000004,
                STORAGE_TEXEL_BUFFER      = 0x00000008,
                CONSTANT_BUFFER           = 0x00000010,
                READ_ONLY_STORAGE_BUFFER  = 0x00000020,
                READ_WRITE_STORAGE_BUFFER = 0x00000040,
                INDIRECT_BUFFER           = 0x00000080,
                INDEX_BUFFER              = 0x00000100,
                VERTEX_BUFFER             = 0x00000200,
                STORAGE_TEXTURE           = 0x00000400,
                SAMPLED_TEXTURE           = 0x00000800,
                COLOR_RENDER_TARGET       = 0x00001000,
                DEPTH_STENCIL_TARGET      = 0x00002000,
            };
            using ResourceUsageFlags = uint32_t;

            enum class ResourceSharingModes : uint8_t
            {
                EXCLUSIVE_ACCESS,
                SIMULTANEOUS_ACCESS,
                _MAX_ENUM,
            };
            using RESOURCE_SHARING_MODE = ResourceSharingModes;

            struct ResourceDesc
            {
                union
                {
                    ColorValue        colorClrValue;        // Used only for render targets
                    DepthStencilValue depthStencilClrValue; // Used only for depth stencil targets
                };
                Size3D                size;
                SAMPLE_COUNT          sampleCount;
                FORMAT                format;
                ResourceUsageFlags    resourceUsage;
                uint32_t              mipLevelCount;
                uint32_t              arrayLayerCount;
                RESOURCE_TYPE         resourceType;
                RESOURCE_SHARING_MODE sharingMode;
                RESOURCE_LAYOUT       resourceLayout;
            };

            struct BindResourceMemoryDesc
            {
                ResourceHandle hResource;
                MemoryHandle   hMemory;
                uint64_t       memoryOffset;
            };

            struct ResourceAllocationInfo
            {
                uint64_t size;
                uint64_t alignment;
            };

            struct MapResourceDesc
            {
                union
                {
                    TextureRangeDesc textureRange;
                    BufferRangeDesc  bufferRange;
                };
                ResourceHandle hResource;
            };

            enum class ResourceViewUsageFlagBits : uint32_t
            {
                CONSTANT_TEXEL_BUFFER     = 0x00000001,
                STORAGE_TEXEL_BUFFER      = 0x00000002,
                CONSTANT_BUFFER           = 0x00000004,
                READ_ONLY_STORAGE_BUFFER  = 0x00000008,
                READ_WRITE_STORAGE_BUFFER = 0x00000010,
                STORAGE_TEXTURE           = 0x00000020,
                SAMPLED_TEXTURE           = 0x00000040,
                COLOR_RENDER_TARGET       = 0x00000080,
                DEPTH_STENCIL_TARGET      = 0x00000100,
            };
            using ResourceViewUsageFlags = uint32_t;

            struct ResourceViewDesc
            {
                ResourceHandle         hResource;
                ResourceViewUsageFlags usage;
            };

            struct BufferViewDesc : public ResourceViewDesc
            {
                BufferRangeDesc range;
            };

            struct TexelBufferViewDesc : public ResourceViewDesc
            {
                BufferRangeDesc range;
                FORMAT          format;
            };

            enum class TextureTypes : uint8_t
            {
                TEXTURE_1D,
                TEXTURE_1D_ARRAY,
                TEXTURE_2D,
                TEXTURE_2D_ARRAY,
                TEXTURE_3D,
                TEXURTE_CUBE,
                TEXURTE_CUBE_ARRAY,
                _MAX_ENUM,
            };
            using TEXTURE_TYPE = TextureTypes;

            struct TextureViewDesc : public ResourceViewDesc
            {
                TextureRangeDesc range;
                FORMAT           format;
                TEXTURE_TYPE     textureType;
                TEXTURE_LAYOUT   layout;
            };

            enum class SamplerTypes : uint8_t
            {
                NORMAL,
                IMMUTABLE,
                _MAX_ENUM,
            };
            using SAMPLER_TYPE = SamplerTypes;

            enum class BorderColors : uint8_t
            {
                TRANSPARENT_BLACK,
                OPAQUE_BLACK,
                OPAQUE_WHITE,
                _MAX_ENUM,
            };
            using BORDER_COLOR = BorderColors;

            enum class FilterTypes : uint8_t
            {
                NEAREST,
                LINEAR,
                _MAX_ENUM,
            };
            using FILTER_TYPE = FilterTypes;

            enum class TextureAddressModes : uint8_t
            {
                REPEAT,
                MIRRORED_REPEAT,
                MIRRORED_ONCE,
                CLAMP_TO_EDGE,
                CLAMP_TO_BORDER,
                _MAX_ENUM,
            };
            using TEXTURE_ADDRESS_MODE = TextureAddressModes;

            enum class SamplerReductionModes : uint8_t
            {
                WEIGHTED_AVERAGE,
                MIN,
                MAX,
                _MAX_ENUM,
            };
            using SAMPLER_REDUCTION_MODE = SamplerReductionModes;

            struct SamplerDesc
            {
                union
                {
                    ColorValue   customBorderColor;
                    BORDER_COLOR enumBorderColor;
                };
                float                  mipLodBias;
                float                  maxAnisotropy;
                float                  minLod;
                float                  maxLod;
                bool_t                 anisotropyEnable;
                bool_t                 compareEnable;
                FILTER_TYPE            minFilter;
                FILTER_TYPE            magFilter;
                FILTER_TYPE            mipMapFilter;
                TEXTURE_ADDRESS_MODE   addressU;
                TEXTURE_ADDRESS_MODE   addressV;
                TEXTURE_ADDRESS_MODE   addressW;
                COMPARE_OPERATION_TYPE compareOperation;
                SAMPLER_REDUCTION_MODE reductionMode;
                SAMPLER_TYPE           type;
            };

            enum class IndirectCommandTypes : uint8_t
            {
                DRAW,
                DRAW_INDEXED,
                DISPATCH,
                // TODO: Add more if possible
                _MAX_ENUM,
            };
            using INDIRECT_COMMAND_TYPE = IndirectCommandTypes;

            struct CommandLayoutDesc
            {
                uint32_t              stride;
                INDIRECT_COMMAND_TYPE type;
            };

            enum class BindingTypes : uint8_t
            {
                SAMPLER,
                // TODO: Support Combined image sampler?
                SAMPLED_TEXTURE,
                STORAGE_TEXTURE,
                CONSTANT_TEXEL_BUFFER,
                STORAGE_TEXEL_BUFFER,
                CONSTANT_BUFFER,
                READ_ONLY_STORAGE_BUFFER,
                READ_WRITE_STORAGE_BUFFER,
                _MAX_ENUM,
            };
            using BINDING_TYPE = BindingTypes;

            struct ImmutableSamplerInfo
            {
                const SamplerHandle* phSamplers;
                SHADER_VISIBILITY    visibility;
            };

            struct BindingRangeDesc
            {
                uint32_t     bindingCount;
                uint32_t     baseBindingSlot;
                uint32_t     baseShaderRegister;
                BINDING_TYPE type;
            };

            struct BindingSetLayoutDesc
            {
                const BindingRangeDesc* pBindingRanges;
                uint32_t                bindingRangeCount;
                SHADER_VISIBILITY       visibility;
            };

            enum class BindingHeapTypes : uint8_t
            {
                SHADER_RESOURCE,
                SAMPLER,
                _MAX_ENUM,
            };
            using BINDING_HEAP_TYPE = BindingHeapTypes;

            struct BindingHeapDesc
            {
                uint32_t          bindingCount;
                BINDING_HEAP_TYPE type;
            };

            struct GetBindingOffsetDesc
            {
                BindingSetLayoutHandle hBindingSetLayout;
                uint32_t               bindingNdx;
            };

            struct WriteBindingDesc
            {
                // TODO: Pack them in union (think about those deleted default constructors)
                ResourceViewHandle hResourceView;
                SamplerHandle      hSampler;
                BindingHeapHandle  hDstHeap;
                uint64_t           dstOffset;
                BINDING_TYPE       bindingType;
            };

            struct SetBindingsDesc
            {
                PipelineLayoutHandle hPipelineLayout;
                uint64_t             baseBindingOffset;
                uint32_t             setSpaceNdx;
                uint32_t             heapNdx;
                PIPELINE_TYPE        type;
            };

            struct PushConstantsDesc
            {
                PipelineLayoutHandle hPipelineLayout;
                const void*          pData;
                uint32_t             firstConstant;
                uint32_t             constantCount;
                SHADER_VISIBILITY    shaderVisibility;
                PIPELINE_TYPE        type;
            };

            struct CopyBufferDesc
            {
                ResourceHandle hSrcBuffer;
                ResourceHandle hDstBuffer;
                uint64_t       srcOffset;
                uint64_t       dstOffset;
                uint64_t       size;
            };

            struct CopyTextureDesc
            {
                TextureRangeDesc srcRange;
                TextureRangeDesc dstRange;
                ResourceHandle   hSrcTexture;
                ResourceHandle   hDstTexture;
                Size3D           size;
                Offset3D         srcOffset;
                Offset3D         dstOffset;
            };

            struct CopyBufferTextureDesc
            {
                TextureRangeDesc textureRange;
                ResourceHandle   hBuffer;
                ResourceHandle   hTexture;
                Size3D           size;
                Offset3D         textureOffset;
                uint64_t         bufferOffset;
                FORMAT           textureFormat;
            };

            enum class QueryTypes : uint8_t
            {
                OCCLUSION,
                PIPELINE_STATISTICS,
                TIMESTAMP,
                _MAX_ENUM,
            };
            using QUERY_TYPE = QueryTypes;

            struct QueryPoolDesc
            {
                uint32_t   queryCount;
                QUERY_TYPE type;
            };

            struct CopyQueryResultsDesc
            {
                QueryPoolHandle hQueryPool;
                ResourceHandle  hBuffer;
                uint64_t        bufferOffset;
                uint32_t        firstQuery;
                uint32_t        queryCount;
                QUERY_TYPE      type;
            };

            struct DrawIndirectArguments
            {
                uint32_t vertexCount;
                uint32_t instanceCount;
                uint32_t firstVertex;
                uint32_t firstInstance;
            };

            struct DrawIndexedIndirectArguments
            {
                uint32_t indexCount;
                uint32_t instanceCount;
                uint32_t firstIndex;
                int32_t  vertexOffset;
                uint32_t firstInstance;
            };

            struct DispatchIndirectArguments
            {
                uint32_t threadGroupCountX;
                uint32_t threadGroupCountY;
                uint32_t threadGroupCountZ;
            };

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS