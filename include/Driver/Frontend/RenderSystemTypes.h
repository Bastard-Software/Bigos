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
            class IShaderCompiler;
            class ShaderCompilerFactory;

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
                _MAX_ENUM,
                SHADER_MODEL_LATEST = SHADER_MODEL_6_7,
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
                Backend::SHADER_TYPE  type;
                SHADER_MODEL          model;
                SHADER_FORMAT         outputFormat;
            };

            struct ShaderCompilerOutput
            {
                byte_t*  pByteCode;
                uint32_t byteCodeSize;
                // TODO: Add more if needed
            };

            struct RenderSystemDesc
            {
                Backend::FactoryDesc      factoryDesc;
                ShaderCompilerFactoryDesc compilerFactoryDesc;
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