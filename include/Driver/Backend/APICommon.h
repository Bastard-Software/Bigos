#pragma once
#include "APITypes.h"

namespace BIGOS::Driver::Backend
{
    BGS_FORCEINLINE BGS_API uint32_t GetBigosFormatSize( FORMAT format )
    {
        BGS_ASSERT( format != Formats::UNKNOWN, "Invalid format (format)." );

        static const uint32_t translateTable[ BGS_ENUM_COUNT( Formats ) ] = {
            MAX_UINT32, // UNKNOWN - not a valid value to get size
            2,          // B5G5R5A1_UNORM
            2,          // B5G6R5_UNORM
            4,          // B8G8R8A8_UNORM
            4,          // B8G8R8A8_UNORM_SRGB
            8,          // BC1_UNORM
            8,          // BC1_UNORM_SRGB
            16,         // BC2_UNORM
            16,         // BC2_UNORM_SRGB
            16,         // BC3_UNORM
            16,         // BC3_UNORM_SRGB
            8,          // BC4_UNORM
            8,          // BC4_SNORM
            16,         // BC5_UNORM
            16,         // BC5_SNORM
            16,         // BC6H_UFLOAT
            16,         // BC6H_SFLOAT
            16,         // BC7_UNORM
            16,         // BC7_UNORM_SRGB
            1,          // R8_UINT
            1,          // R8_SINT
            1,          // R8_UNORM
            1,          // R8_SNORM
            2,          // R16_UINT
            2,          // R16_SINT
            2,          // R16_UNORM
            2,          // R16_SNORM
            2,          // R16_FLOAT
            2,          // D16_UNORM
            2,          // R8G8_UINT
            2,          // R8G8_SINT
            2,          // R8G8_UNORM
            2,          // R8G8_SNORM
            4,          // D24_UNORM_S8_UINT
            4,          // R32_FLOAT
            4,          // R32_UINT
            4,          // R32_SINT
            4,          // D32_FLOAT
            4,          // R16G16_UINT
            4,          // R16G16_SINT
            4,          // R16G16_UNORM
            4,          // R16G16_SNORM
            4,          // R16G16_FLOAT
            4,          // R8G8B8A8_UNORM
            4,          // R8G8B8A8_SNORM
            4,          // R8G8B8A8_UNORM_SRGB
            4,          // R8G8B8A8_UINT
            4,          // R8G8B8A8_SINT
            8,          // R32G32_UINT
            8,          // R32G32_SINT
            8,          // R32G32_FLOAT
            8,          // R16G16B16A16_UNORM
            8,          // R16G16B16A16_SNORM
            8,          // R16G16B16A16_UINT
            8,          // R16G16B16A16_SINT
            8,          // R16G16B16A16_FLOAT
            12,         // R32G32B32_UINT
            12,         // R32G32B32_SINT
            12,         // R32G32B32_FLOAT
            16,         // R32G32B32A32_UINT
            16,         // R32G32B32A32_SINT
            16,         // R32G32B32A32_FLOAT
        };

        return translateTable[ BGS_ENUM_INDEX( format ) ];
    };
} // namespace BIGOS::Driver::Backend