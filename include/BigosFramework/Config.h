#pragma once

#include <cstdint>

namespace BIGOS
{
    namespace Config
    {
        namespace Driver
        {
            namespace Adapter
            {
                constexpr uint32_t MAX_DESCRIPTION_TEXT_LENGHT = 256U;
            } // namespace Adapter

            namespace Swapchain
            {
                constexpr uint32_t MAX_BACK_BUFFER_COUNT = 16U;
            } // namespace Swapchain

            namespace Synchronization
            {
                constexpr uint32_t MAX_SEMAPHORES_TO_WAIT_COUNT = 8U;
                constexpr uint32_t MAX_FENCES_TO_WAIT_COUNT     = 8U;
            } // namespace Synchronization

            namespace Shader
            {
                constexpr uint32_t MAX_SHADER_ENTRY_POINT_NAME_LENGHT = 32U;
                constexpr uint32_t MAX_SHADER_COMPILER_ARGUMENT_COUNT = 32U;
            } // namespace Shader

            namespace Binding
            {
                constexpr uint32_t MAX_BINDING_RANGE_COUNT       = 64U;
                constexpr uint32_t MAX_IMMUTABLE_SAMPLER_COUNT   = 16U;
            } // namespace Binding

            namespace Pipeline
            {
                constexpr uint32_t MAX_INPUT_BINDING_COUNT = 16U;
                constexpr uint32_t MAX_INPUT_ELEMENT_COUNT = 16U;
                constexpr uint32_t MAX_BLEND_STATE_COUNT   = 8U;
                constexpr uint32_t MAX_RENDER_TARGET_COUNT = 8U;
                constexpr uint32_t MAX_SCISSOR_COUNT       = 16U;
                constexpr uint32_t MAX_VIEWPORT_COUNT      = 16U;
            } // namespace Pipeline
        }     // namespace Driver
    }         // namespace Config
} // namespace BIGOS