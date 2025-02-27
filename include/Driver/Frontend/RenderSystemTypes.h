#pragma once
#include "Core/CoreTypes.h"
#include "Core/Memory/MemoryTypes.h"

#include "Driver/Backend/API.h"
#include "glm/glm.hpp"

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
            class Camera;
            class RenderSystem;
            class RenderDevice;
            class GraphicsContext;
            class ComputeContext;
            class CopyContext;
            class RenderTarget;
            class Buffer;
            class Texture;
            class Shader;
            class Pipeline;
            class IShaderCompiler;
            class ShaderCompilerFactory;

            using AdapterArray      = Backend::AdapterArray;
            using RenderDeviceArray = HeapArray<RenderDevice*>; // Framework allows to create only one device for each adapter
            using CameraArray       = HeapArray<Camera*>;

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

            enum class CompilerTypes : uint8_t
            {
                DXC,
                GLSLC,
                _MAX_ENUM,
            };
            using COMPILER_TYPE = CompilerTypes;

            struct ShaderCompilerDesc
            {
                COMPILER_TYPE type;
            };

            struct ShaderCompilerFactoryDesc
            {
                // For future use
            };

            enum class ShaderModels : uint8_t
            {
                SHADER_MODEL_6_0,
                SHADER_MODEL_6_1,
                SHADER_MODEL_6_2,
                SHADER_MODEL_6_3,
                SHADER_MODEL_6_4,
                SHADER_MODEL_6_5,
                SHADER_MODEL_6_6,
                SHADER_MODEL_6_7,
                SHADER_MODEL_6_8,
                _MAX_ENUM,
                SHADER_MODEL_LATEST = SHADER_MODEL_6_8,
            };
            using SHADER_MODEL = ShaderModels;

            enum class ShaderLanguages : uint8_t
            {
                HLSL,
                GLSL,
                _MAX_ENUM,
            };
            using SHADER_LANGUAGE = ShaderLanguages;

            enum class ShaderFormats : uint8_t
            {
                DXIL,
                SPIRV,
                _MAX_ENUM,
            };
            using SHADER_FORMAT = ShaderFormats;

            enum class CameraTypes : uint8_t
            {
                DEFAULT,
                _MAX_ENUM,
            };
            using CAMERA_TYPE = CameraTypes;

            struct ShaderSource
            {
                const void* pSourceCode;
                uint32_t    sourceSize;
            };

            struct CompileShaderDesc
            {
                ShaderSource          source;
                const char*           pEntryPoint;
                const wchar_t* const* ppArgs;
                uint32_t              argCount;
                bool_t                compileDebug;
                bool_t                reflect;
                Backend::SHADER_TYPE  type;
                SHADER_MODEL          model;
                SHADER_FORMAT         outputFormat;
            };

            struct ShaderBindingInfo
            {
                char                  pName[ Config::Driver::Shader::MAX_SHADER_BINDING_NAME_LENGHT ];
                uint32_t              baseBindingSlot;
                uint32_t              baseShaderRegister;
                uint32_t              set;
                Backend::BINDING_TYPE type;
            };
            using ShaderBindingArray = HeapArray<ShaderBindingInfo>;

            enum class ShaderInputBaseTypes : uint8_t
            {
                UNKNOWN,
                FLOAT,
                INT,
                UINT,
                _MAX_ENUM
            };
            using SHADER_INPUT_BASE_TYPE = ShaderInputBaseTypes;

            struct ShaderInputBindingInfo
            {
                char                   pSemanticName[ Config::Driver::Shader::MAX_SHADER_BINDING_NAME_LENGHT ];
                uint32_t               location;
                uint32_t               componentCount;
                SHADER_INPUT_BASE_TYPE type;
            };
            using ShaderInputBindingArray = HeapArray<ShaderInputBindingInfo>;

            struct ShaderCompilerOutput
            {
                byte_t*                 pByteCode;
                ShaderBindingInfo*      pBindings;
                ShaderInputBindingInfo* pInputBindings;
                uint32_t                byteCodeSize;
                uint32_t                bindingCount;
                uint32_t                inputBindingCount;
            };

            struct RenderSystemDesc
            {
                Backend::FactoryDesc      factoryDesc;
                ShaderCompilerFactoryDesc compilerFactoryDesc;
            };

            struct CameraDesc
            {
                glm::mat4   projection;
                CAMERA_TYPE type;
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
            };

            struct RenderTargetDesc
            {
                Backend::FORMAT       format;
                uint32_t              width;
                uint32_t              height;
                Backend::SAMPLE_COUNT sampleCount;
                bool_t                allowSampling;
                bool_t                allowCopying;
            };

            struct BufferDesc
            {
                uint32_t                    size;
                Backend::ResourceUsageFlags usage;
            };

            enum class TextureUsageFlagBits : uint32_t
            {
                SAMPLED = 0x00000001,
                STORAGE = 0x00000002,
            };
            using TextureUsageFlags = uint32_t;

            enum class TextureTypes : uint8_t
            {
                TEXTURE_1D,
                TEXTURE_2D,
                TEXTURE_3D,
                TEXTURE_CUBE,
                _MAX_ENUM
            };
            using TEXTURE_TYPE = TextureTypes;

            struct TextureDesc
            {
                Size3D                size;
                Backend::FORMAT       format;
                Backend::SAMPLE_COUNT sampleCount;
                uint32_t              mipLevelCount;
                uint32_t              arrayLayerCount;
                TextureUsageFlags     usage;
                TEXTURE_TYPE          type;
            };

            struct ShaderDesc
            {
                ShaderSource         source;
                Backend::SHADER_TYPE type;
            };

            struct BlendState
            {
                bool_t blendEnable;
            };

            struct DepthStencilState
            {
                bool_t depthTestEnable;
                bool_t depthWriteEnable;
            };

            struct GraphicsPipelineDesc
            {
                Shader*                pVertexShader;
                Shader*                pPixelShader;
                Shader*                pGeometryShader;
                Shader*                pHullShader;
                Shader*                pDomainShader;
                BlendState             blendState;
                DepthStencilState      depthStencilState;
                Backend::SAMPLE_COUNT  sampleCount;
                uint32_t               renderTargetCount;
                const Backend::FORMAT* pRenderTargetFormats;
                Backend::FORMAT        depthStencilFormat;
            };

            struct ComputePipelineDesc
            {
                Shader* pComputeShader;
            };

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS