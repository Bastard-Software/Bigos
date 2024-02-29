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