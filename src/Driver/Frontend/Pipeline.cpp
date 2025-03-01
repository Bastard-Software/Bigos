#include "Driver/Frontend/Pipeline.h"

#include "Core/Utils/String.h"
#include "Driver/Backend/APICommon.h"
#include "Driver/Frontend/RenderDevice.h"
#include "Driver/Frontend/Shader/Shader.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            Backend::FORMAT GetInputElemFormat( SHADER_INPUT_BASE_TYPE type, uint32_t compCnt )
            {
                switch( type )
                {
                    case ShaderInputBaseTypes::FLOAT:
                        switch( compCnt )
                        {
                            case 1:
                                return Backend::Formats::R32_FLOAT;
                            case 2:
                                return Backend::Formats::R32G32_FLOAT;
                            case 3:
                                return Backend::Formats::R32G32B32_FLOAT;
                            case 4:
                                return Backend::Formats::R32G32B32A32_FLOAT;
                        }
                        break;
                    case ShaderInputBaseTypes::UINT:
                        switch( compCnt )
                        {
                            case 1:
                                return Backend::Formats::R32_UINT;
                            case 2:
                                return Backend::Formats::R32G32_UINT;
                            case 3:
                                return Backend::Formats::R32G32B32_UINT;
                            case 4:
                                return Backend::Formats::R32G32B32A32_UINT;
                        }
                        break;
                    case ShaderInputBaseTypes::INT:
                        switch( compCnt )
                        {
                            case 1:
                                return Backend::Formats::R32_SINT;
                            case 2:
                                return Backend::Formats::R32G32_SINT;
                            case 3:
                                return Backend::Formats::R32G32B32_SINT;
                            case 4:
                                return Backend::Formats::R32G32B32A32_SINT;
                        }
                        break;
                }

                return Backend::Formats::UNKNOWN;
            }

            void AddNextStageBindings( Shader* pShader, index_t* pCurrNdx, ShaderBindingInfo** ppBindingInfos )
            {
                ShaderCompilerOutput* pOutput         = pShader->GetCompiledShader();
                const index_t         constBindingCnt = ( *pCurrNdx );
                index_t               bindingCnt      = ( *pCurrNdx );

                ShaderBindingInfo* pBindingInfos = ( *ppBindingInfos );
                for( index_t ndx = 0; ndx < pOutput->bindingCount; ++ndx )
                {
                    const ShaderBindingInfo& binding = pOutput->pBindings[ ndx ];
                    bool_t                   found   = false;
                    for( index_t ndy = 0; ndy < constBindingCnt; ++ndy )
                    {
                        if( Utils::String::Compare( pBindingInfos[ ndy ].pName, binding.pName ) )
                        {
                            found = true;
                            break;
                        }
                    }
                    if( !found )
                    {
                        pBindingInfos[ bindingCnt++ ] = binding;
                    }
                }
                ( *pCurrNdx ) = bindingCnt;
            }

            Pipeline::Pipeline()
                : m_computeDesc()
                , m_graphicsDesc()
                , m_pParent( nullptr )
                , m_hShaderResourceLayout()
                , m_hSamplerLayout()
                , m_hPipelineLayout()
                , m_hPipeline()
                , m_type( Backend::PipelineTypes::_MAX_ENUM )
            {
            }

            RESULT Pipeline::Create( const GraphicsPipelineDesc& desc, RenderDevice* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Render device (pDevice) must be a valid pointer." );
                m_pParent = pDevice;

                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                m_type = Backend::PipelineTypes::GRAPHICS;
                if( BGS_FAILED(
                        CreateGraphicsLayout( desc.pVertexShader, desc.pPixelShader, desc.pDomainShader, desc.pHullShader, desc.pGeometryShader ) ) )
                {
                    return Results::FAIL;
                }

                m_graphicsDesc.type                     = m_type;
                m_graphicsDesc.vertexShader.hShader     = desc.pVertexShader != nullptr ? desc.pVertexShader->GetHandle() : Backend::ShaderHandle();
                m_graphicsDesc.vertexShader.pEntryPoint = "VSMain";
                m_graphicsDesc.pixelShader.hShader      = desc.pPixelShader != nullptr ? desc.pPixelShader->GetHandle() : Backend::ShaderHandle();
                m_graphicsDesc.pixelShader.pEntryPoint  = "PSMain";
                m_graphicsDesc.hullShader.hShader       = desc.pHullShader != nullptr ? desc.pHullShader->GetHandle() : Backend::ShaderHandle();
                m_graphicsDesc.hullShader.pEntryPoint   = "HSMain";
                m_graphicsDesc.domainShader.hShader     = desc.pDomainShader != nullptr ? desc.pDomainShader->GetHandle() : Backend::ShaderHandle();
                m_graphicsDesc.domainShader.pEntryPoint = "DSMain";
                m_graphicsDesc.geometryShader.hShader = desc.pGeometryShader != nullptr ? desc.pGeometryShader->GetHandle() : Backend::ShaderHandle();
                m_graphicsDesc.geometryShader.pEntryPoint = "GSMain";
                m_graphicsDesc.hPipelineLayout            = m_hPipelineLayout;
                m_graphicsDesc.pRenderTargetFormats       = desc.pRenderTargetFormats;
                m_graphicsDesc.renderTargetCount          = desc.renderTargetCount;
                m_graphicsDesc.depthStencilFormat         = desc.depthStencilFormat;
                // States (note that input state is already set in CreateGraphicsLayout())
                // Inpot assembler state
                m_graphicsDesc.inputAssemblerState.indexRestartValue = Backend::IndexRestartValues::DISABLED;
                m_graphicsDesc.inputAssemblerState.topology          = Backend::PrimitiveTopologies::TRIANGLE_LIST;
                // Rasterizer state
                m_graphicsDesc.rasterizeState.frontFaceMode           = Backend::FrontFaceModes::COUNTER_CLOCKWISE;
                m_graphicsDesc.rasterizeState.cullMode                = Backend::CullModes::BACK;
                m_graphicsDesc.rasterizeState.fillMode                = Backend::PolygonFillModes::SOLID;
                m_graphicsDesc.rasterizeState.depthBiasClamp          = 0.0f;
                m_graphicsDesc.rasterizeState.depthBiasConstantFactor = 0.0f;
                m_graphicsDesc.rasterizeState.depthBiasSlopeFactor    = 0.0f;
                m_graphicsDesc.rasterizeState.depthClipEnable         = BGS_FALSE;
                // Multisample state
                m_graphicsDesc.multisampleState.sampleCount = desc.sampleCount;
                // Depth stencil states
                m_graphicsDesc.depthStencilState.depthTestEnable   = desc.depthStencilState.depthTestEnable;
                m_graphicsDesc.depthStencilState.depthWriteEnable  = desc.depthStencilState.depthWriteEnable;
                m_graphicsDesc.depthStencilState.stencilTestEnable = BGS_FALSE;
                m_graphicsDesc.depthStencilState.depthCompare      = Backend::CompareOperationTypes::LESS;
                // Blend state
                Backend::RenderTargetBlendDesc blendDescs[ Config::Driver::Pipeline::MAX_BLEND_STATE_COUNT ];
                for( index_t ndx = 0; ndx < static_cast<index_t>( desc.renderTargetCount ); ++ndx )
                {
                    Backend::RenderTargetBlendDesc& blendDesc = blendDescs[ ndx ];
                    blendDesc.blendEnable                     = desc.blendState.blendEnable;
                    blendDesc.srcColorBlendFactor             = Backend::BlendFactors::SRC_ALPHA;
                    blendDesc.dstColorBlendFactor             = Backend::BlendFactors::ONE_MINUS_SRC_ALPHA;
                    blendDesc.colorBlendOp                    = Backend::BlendOperationTypes::ADD;
                    blendDesc.srcAlphaBlendFactor             = Backend::BlendFactors::ONE;
                    blendDesc.dstAlphaBlendFactor             = Backend::BlendFactors::ZERO;
                    blendDesc.alphaBlendOp                    = Backend::BlendOperationTypes::ADD;
                    blendDesc.writeFlag                       = BGS_FLAG( Backend::ColorComponentFlagBits::ALL );
                }
                m_graphicsDesc.blendState.pRenderTargetBlendDescs    = blendDescs;
                m_graphicsDesc.blendState.renderTargetBlendDescCount = desc.renderTargetCount;
                m_graphicsDesc.blendState.enableLogicOperations      = BGS_FALSE;

                if( BGS_FAILED( pAPIDevice->CreatePipeline( m_graphicsDesc, &m_hPipeline ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT Pipeline::Create( const ComputePipelineDesc& desc, RenderDevice* pDevice )
            {
                BGS_ASSERT( pDevice != nullptr, "Render device (pDevice) must be a valid pointer." );
                m_pParent = pDevice;

                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                m_type = Backend::PipelineTypes::COMPUTE;
                if( BGS_FAILED( CreateComputeLayout( desc.pComputeShader ) ) )
                {
                    return Results::FAIL;
                }

                m_computeDesc.type                      = m_type;
                m_computeDesc.computeShader.hShader     = desc.pComputeShader->GetHandle();
                m_computeDesc.computeShader.pEntryPoint = "CSMain";
                m_computeDesc.hPipelineLayout           = m_hPipelineLayout;
                if( BGS_FAILED( pAPIDevice->CreatePipeline( m_computeDesc, &m_hPipeline ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void Pipeline::Destroy()
            {
                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                if( m_hPipeline != Backend::PipelineHandle() )
                {
                    pAPIDevice->DestroyPipeline( &m_hPipeline );
                }
                if( m_hPipelineLayout != Backend::PipelineLayoutHandle() )
                {
                    pAPIDevice->DestroyPipelineLayout( &m_hPipelineLayout );
                }
                if( m_hShaderResourceLayout != Backend::BindingSetLayoutHandle() )
                {
                    pAPIDevice->DestroyBindingSetLayout( &m_hShaderResourceLayout );
                }
                if( m_hSamplerLayout != Backend::BindingSetLayoutHandle() )
                {
                    pAPIDevice->DestroyBindingSetLayout( &m_hSamplerLayout );
                }
            }

            RESULT Pipeline::CreateComputeLayout( Shader* pCS )
            {
                BGS_ASSERT( m_type == Backend::PipelineTypes::COMPUTE );

                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                ShaderCompilerOutput* pOutput = pCS->GetCompiledShader();

                Backend::BindingRangeDesc srRanges[ Config::Driver::Binding::MAX_SHADER_RESOURCE_COUNT ];
                Backend::BindingRangeDesc samplerRanges[ Config::Driver::Binding::MAX_SAMPLER_COUNT ];
                uint32_t                  srCount      = 0;
                uint32_t                  samplerCount = 0;
                for( index_t ndx = 0; ndx < pOutput->bindingCount; ++ndx )
                {
                    const ShaderBindingInfo& binding = pOutput->pBindings[ ndx ];
                    if( binding.type == Backend::BindingTypes::SAMPLER )
                    {
                        Backend::BindingRangeDesc& range = samplerRanges[ samplerCount++ ];
                        range.baseBindingSlot            = binding.baseBindingSlot;
                        range.baseShaderRegister         = binding.baseShaderRegister;
                        range.bindingCount               = 1;
                        range.type                       = binding.type;
                    }
                    else
                    {
                        Backend::BindingRangeDesc& range = srRanges[ srCount++ ];
                        range.baseBindingSlot            = binding.baseBindingSlot;
                        range.baseShaderRegister         = binding.baseShaderRegister;
                        range.bindingCount               = 1;
                        range.type                       = binding.type;
                    }
                }
                Backend::BindingSetLayoutDesc srLayoutDesc;
                srLayoutDesc.pBindingRanges    = srRanges;
                srLayoutDesc.bindingRangeCount = srCount;
                srLayoutDesc.visibility        = Backend::ShaderVisibilities::COMPUTE;
                if( BGS_FAILED( pAPIDevice->CreateBindingSetLayout( srLayoutDesc, &m_hShaderResourceLayout ) ) )
                {
                    return Results::FAIL;
                }

                Backend::BindingSetLayoutDesc samplerLayoutDesc;
                samplerLayoutDesc.pBindingRanges    = samplerRanges;
                samplerLayoutDesc.bindingRangeCount = samplerCount;
                samplerLayoutDesc.visibility        = Backend::ShaderVisibilities::COMPUTE;
                if( BGS_FAILED( pAPIDevice->CreateBindingSetLayout( samplerLayoutDesc, &m_hSamplerLayout ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                Backend::BindingSetLayoutHandle bindingSetLayouts[] = { m_hShaderResourceLayout, m_hSamplerLayout };
                Backend::PipelineLayoutDesc     plDesc;
                plDesc.bindingSetLayoutCount = 2;
                plDesc.phBindigSetLayouts    = bindingSetLayouts;
                // TODO: Support push constants
                plDesc.constantRangeCount = 0;
                plDesc.pConstantRanges    = nullptr;
                if( BGS_FAILED( pAPIDevice->CreatePipelineLayout( plDesc, &m_hPipelineLayout ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            RESULT Pipeline::CreateGraphicsLayout( Shader* pVS, Shader* pPS, Shader* pDS, Shader* pHS, Shader* pGS )
            {
                BGS_ASSERT( m_type == Backend::PipelineTypes::GRAPHICS );

                Backend::IDevice* pAPIDevice = m_pParent->GetNativeAPIDevice();

                // Graphics pipeline requires vertex shader so we start getting bindings from it
                // Input bindings
                ShaderCompilerOutput* pOutput = pVS->GetCompiledShader();

                Backend::InputBindingDesc inputBinding;
                inputBinding.binding   = 0;
                inputBinding.inputRate = Backend::InputStepRates::PER_VERTEX; // TODO: Support per instance

                Backend::InputElementDesc inputElements[ Config::Driver::Pipeline::MAX_INPUT_ELEMENT_COUNT ];
                uint32_t                  inputElementCount = 0;
                uint32_t                  vertexOffset      = 0;
                for( index_t ndx = 0; ndx < pOutput->inputBindingCount; ++ndx )
                {
                    const ShaderInputBindingInfo& currInputBinding = pOutput->pInputBindings[ ndx ];
                    Backend::InputElementDesc&    elem             = inputElements[ inputElementCount++ ];
                    elem.binding                                   = 0;
                    elem.location                                  = currInputBinding.location;
                    elem.pSemanticName                             = currInputBinding.pSemanticName;
                    elem.format                                    = GetInputElemFormat( currInputBinding.type, currInputBinding.componentCount );
                    elem.offset                                    = vertexOffset;

                    vertexOffset += Backend::GetBigosFormatSize( elem.format );

                    BGS_ASSERT( elem.format != Backend::Formats::UNKNOWN, "Unknown input element format." );
                }
                m_graphicsDesc.inputState.inputBindingCount = 1;
                m_graphicsDesc.inputState.pInputBindings    = &inputBinding;
                m_graphicsDesc.inputState.inputElementCount = inputElementCount;
                m_graphicsDesc.inputState.pInputElements    = inputElements;

                // Resource bindings
                ShaderBindingInfo
                        allStageResources[ Config::Driver::Binding::MAX_SHADER_RESOURCE_COUNT + Config::Driver::Binding::MAX_SAMPLER_COUNT ];
                index_t allBindingCount               = 0;
                ShaderBindingInfo* pAllStageResources = allStageResources;

                AddNextStageBindings( pVS, &allBindingCount, &pAllStageResources );
                if( pPS != nullptr )
                {
                    AddNextStageBindings( pPS, &allBindingCount, &pAllStageResources );
                }
                if( pDS != nullptr )
                {
                    AddNextStageBindings( pDS, &allBindingCount, &pAllStageResources );
                }
                if( pHS != nullptr )
                {
                    AddNextStageBindings( pHS, &allBindingCount, &pAllStageResources );
                }
                if( pGS != nullptr )
                {
                    AddNextStageBindings( pGS, &allBindingCount, &pAllStageResources );
                }

                Backend::BindingRangeDesc srRanges[ Config::Driver::Binding::MAX_SHADER_RESOURCE_COUNT ];
                Backend::BindingRangeDesc samplerRanges[ Config::Driver::Binding::MAX_SAMPLER_COUNT ];
                uint32_t                  srCount      = 0;
                uint32_t                  samplerCount = 0;
                for( index_t ndx = 0; ndx < allBindingCount; ++ndx )
                {
                    const ShaderBindingInfo& binding = allStageResources[ ndx ];
                    if( binding.type == Backend::BindingTypes::SAMPLER )
                    {
                        Backend::BindingRangeDesc& range = samplerRanges[ samplerCount++ ];
                        range.baseBindingSlot            = binding.baseBindingSlot;
                        range.baseShaderRegister         = binding.baseShaderRegister;
                        range.bindingCount               = 1;
                        range.type                       = binding.type;
                    }
                    else
                    {
                        Backend::BindingRangeDesc& range = srRanges[ srCount++ ];
                        range.baseBindingSlot            = binding.baseBindingSlot;
                        range.baseShaderRegister         = binding.baseShaderRegister;
                        range.bindingCount               = 1;
                        range.type                       = binding.type;
                    }
                }
                Backend::BindingSetLayoutDesc srLayoutDesc;
                srLayoutDesc.pBindingRanges    = srRanges;
                srLayoutDesc.bindingRangeCount = srCount;
                srLayoutDesc.visibility        = Backend::ShaderVisibilities::ALL_GRAPHICS;
                if( BGS_FAILED( pAPIDevice->CreateBindingSetLayout( srLayoutDesc, &m_hShaderResourceLayout ) ) )
                {
                    return Results::FAIL;
                }

                Backend::BindingSetLayoutDesc samplerLayoutDesc;
                samplerLayoutDesc.pBindingRanges    = samplerRanges;
                samplerLayoutDesc.bindingRangeCount = samplerCount;
                samplerLayoutDesc.visibility        = Backend::ShaderVisibilities::ALL_GRAPHICS;
                if( BGS_FAILED( pAPIDevice->CreateBindingSetLayout( samplerLayoutDesc, &m_hSamplerLayout ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                Backend::BindingSetLayoutHandle bindingSetLayouts[] = { m_hShaderResourceLayout, m_hSamplerLayout };
                Backend::PipelineLayoutDesc     plDesc;
                plDesc.bindingSetLayoutCount = 2;
                plDesc.phBindigSetLayouts    = bindingSetLayouts;
                // TODO: Support push constants
                plDesc.constantRangeCount = 0;
                plDesc.pConstantRanges    = nullptr;
                if( BGS_FAILED( pAPIDevice->CreatePipelineLayout( plDesc, &m_hPipelineLayout ) ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS