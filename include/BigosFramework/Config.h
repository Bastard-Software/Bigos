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
            }

            namespace Synchronization
            {
                constexpr uint32_t MAX_FENCES_TO_WAIT_COUNT = 8U;
            } // namespace Synchronization

            namespace Shader
            {
                constexpr uint32_t MAX_SHADER_ENTRY_POINT_NAME_LENGHT = 32U;
                constexpr uint32_t MAX_SHADER_COMPILER_ARGUMENT_COUNT = 32U;
            } // namespace Shader

            namespace Pipeline
            {
                constexpr uint32_t MAX_SCISSOR_COUNT  = 16U;
                constexpr uint32_t MAX_VIEWPORT_COUNT = 16U;
            } // namespace Pipeline
        }     // namespace Driver
    }         // namespace Config
} // namespace BIGOS