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

            namespace Queue
            {
                constexpr uint32_t MAX_COMMAND_BUFFER_TO_EXECUTE_COUNT = 8U;
            } // namespace Queue

            namespace Synchronization
            {
                constexpr uint32_t MAX_SEMAPHORES_TO_WAIT_COUNT   = 16U;
                constexpr uint32_t MAX_SEMAPHORES_TO_SIGNAL_COUNT = 16U;
                constexpr uint32_t MAX_FENCES_TO_WAIT_COUNT       = 16U;
                constexpr uint32_t MAX_FENCES_TO_SIGNAL_COUNT     = 16U;
                constexpr uint32_t MAX_GLOBAL_BARRIER_COUNT       = 16U;
                constexpr uint32_t MAX_BUFFER_BARRIER_COUNT       = 16U;
                constexpr uint32_t MAX_TEXTURE_BARRIER_COUNT      = 16U;
            } // namespace Synchronization

            namespace Shader
            {
                constexpr uint32_t MAX_SHADER_ENTRY_POINT_NAME_LENGHT = 32U;
                constexpr uint32_t MAX_SHADER_COMPILER_ARGUMENT_COUNT = 32U;
                constexpr uint32_t MAX_SHADER_BINDING_NAME_LENGHT     = 32U;
            } // namespace Shader

            namespace Binding
            {
                constexpr uint32_t MAX_SHADER_RESOURCE_COUNT           = 4096U;
                constexpr uint32_t MAX_SAMPLER_COUNT                   = 64U;
                constexpr uint32_t MAX_RENDER_TARGET_VIEW_COUNT        = 32U;
                constexpr uint32_t MAX_DEPTH_STENCIL_TARGET_VIEW_COUNT = 16U;

            } // namespace Binding

            namespace Pipeline
            {
                constexpr uint32_t MAX_BINDING_SET_LAYOUT_COUNT = 16U;
                constexpr uint32_t MAX_BINDING_SET_COUNT        = 16U;
                constexpr uint32_t MAX_INPUT_BINDING_COUNT      = 16U;
                constexpr uint32_t MAX_INPUT_ELEMENT_COUNT      = 16U;
                constexpr uint32_t MAX_BLEND_STATE_COUNT        = 8U;
                constexpr uint32_t MAX_RENDER_TARGET_COUNT      = 8U;
                constexpr uint32_t MAX_CLEAR_RECT_COUNT         = 8U;
                constexpr uint32_t MAX_SCISSOR_COUNT            = 16U;
                constexpr uint32_t MAX_VIEWPORT_COUNT           = 16U;
                constexpr uint32_t MAX_BINDING_RANGE_COUNT      = 64U;
                constexpr uint32_t MAX_IMMUTABLE_SAMPLER_COUNT  = 16U;
            } // namespace Pipeline
        } // namespace Driver
    } // namespace Config
} // namespace BIGOS