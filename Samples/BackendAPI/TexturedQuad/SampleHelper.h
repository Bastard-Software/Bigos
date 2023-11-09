#pragma once
#include "BigosFramework/BigosFramework.h"
#include <fstream>

inline BIGOS::RESULT readDataFromFile( const char* pPath, void** ppData, uint32_t* pSize )
{
    BGS_ASSERT( pPath != nullptr, "Path (pPath) must be a valid string." );
    BGS_ASSERT( *ppData == nullptr, "Data (ppData) has already hold unknown data." );
    BGS_ASSERT( ppData != nullptr, "Data (ppData) must be a valid address." );
    BGS_ASSERT( pSize != nullptr, "Size (pSize) must be a valid address." );

    std::ifstream file( pPath, std::ios::ate | std::ios::binary );

    if( !file.is_open() )
    {
        return BIGOS::Results::FAIL;
    }

    size_t fileSize = static_cast<size_t>( file.tellg() );
    char*  pBytes   = new char[ fileSize ];
    if( pBytes == nullptr )
    {
        return BIGOS::Results::NO_MEMORY;
    }

    file.seekg( 0 );
    file.read( pBytes, fileSize );
    file.close();

    *ppData = pBytes;
    *pSize  = static_cast<uint32_t>( fileSize );

    return BIGOS::Results::OK;
}

inline BIGOS::RESULT GenerateCheckerboardData( uint32_t width, uint32_t height, void** ppData, uint32_t* pSize )
{
    BGS_ASSERT( *ppData == nullptr, "Data (ppData) has already hold unknown data." );
    BGS_ASSERT( ppData != nullptr, "Data (ppData) must be a valid address." );
    BGS_ASSERT( pSize != nullptr, "Size (pSize) must be a valid address." );

    const uint32_t rowPitch    = width * 4;     // 4 is number of bytes per pixel
    const uint32_t cellPitch   = rowPitch >> 3; // The width of a cell in the checkboard texture.
    const uint32_t cellHeight  = width >> 3;    // The height of a cell in the checkerboard texture.
    const uint32_t textureSize = rowPitch * height;

    uint8_t* pData = new uint8_t[ textureSize ];
    if( pData == nullptr )
    {
        return BIGOS::Results::NO_MEMORY;
    }

    for( UINT n = 0; n < textureSize; n += 4 ) // 4 is number of bytes per pixel
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if( i % 2 == j % 2 )
        {
            pData[ n ]     = 0x00; // R
            pData[ n + 1 ] = 0x00; // G
            pData[ n + 2 ] = 0x00; // B
            pData[ n + 3 ] = 0xff; // A
        }
        else
        {
            pData[ n ]     = 0xff; // R
            pData[ n + 1 ] = 0xff; // G
            pData[ n + 2 ] = 0xff; // B
            pData[ n + 3 ] = 0xff; // A
        }
    }

    *ppData = pData;
    *pSize  = textureSize;

    return BIGOS::Results::OK;
}