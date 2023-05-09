#include "Core/Utils/String.h"

void BIGOS::Core::Utils::String::Copy( char* pDst, size_t dstSize, const char* pSrc )
{
    BGS_ASSERT( pDst != nullptr, "Buffer (pDst) must be a valid buffer." );
    BGS_ASSERT( dstSize > 0, "Buffer size (dstSize) must be greater than zero." );
    BGS_ASSERT( pSrc != nullptr, "String (pSrc) must be a valid string." );
#if( BGS_VISUAL_STUDIO )
    strcpy_s( pDst, dstSize, pSrc );
#else
#    error
#endif // ( BGS_VISUAL_STUDIO )
}

void BIGOS::Core::Utils::String::Copy( wchar_t* pDst, size_t dstSize, const wchar_t* pSrc )
{
    BGS_ASSERT( pDst != nullptr, "Buffer (pDst) must be a valid buffer." );
    BGS_ASSERT( dstSize > 0, "Buffer size (dstSize) must be greater than zero." );
    BGS_ASSERT( pSrc != nullptr, "String (pSrc) must be a valid string." );
#if( BGS_VISUAL_STUDIO )
    wcscpy_s( pDst, dstSize, pSrc );
#else
#    error
#endif // ( BGS_VISUAL_STUDIO )
}

size_t BIGOS::Core::Utils::String::Utf16ToUtf8( char* pDst, size_t dstSize, const wchar_t* pSrc, size_t srcSize )
{
    BGS_ASSERT( pDst != nullptr, "Buffer (pDst) must be a valid buffer." );
    BGS_ASSERT( dstSize > 0, "Buffer size (dstSize) must be greater than zero." );
    BGS_ASSERT( pSrc != nullptr, "String (pSrc) must be a valid string." );
    BGS_ASSERT( srcSize > 0, "String size (srcSize) must be greater than zero." );

    size_t length = 0;
#if( BGS_VISUAL_STUDIO )
    wcstombs_s( &length, pDst, dstSize, pSrc, srcSize );
#else
#    error
#endif // ( BGS_VISUAL_STUDIO )
    return length;
}

size_t BIGOS::Core::Utils::String::Utf8ToUtf16( wchar_t* pDst, size_t dstSize, const char* pSrc, size_t srcSize )
{
    BGS_ASSERT( pDst != nullptr, "Buffer (pDst) must be a valid buffer." );
    BGS_ASSERT( dstSize > 0, "Buffer size (dstSize) must be greater than zero." );
    BGS_ASSERT( pSrc != nullptr, "String (pSrc) must be a valid string." );
    BGS_ASSERT( srcSize > 0, "String size (srcSize) must be greater than zero." );

    size_t length = 0;
#if( BGS_VISUAL_STUDIO )
    mbstowcs_s( &length, pDst, dstSize, pSrc, srcSize );
#else
#    error
#endif // ( BGS_VISUAL_STUDIO )
    return length;
}

size_t BIGOS::Core::Utils::String::Length( const char* pStr )
{
    BGS_ASSERT( pStr != nullptr, "String (pStr) must be a valid string." );

#if( BGS_VISUAL_STUDIO )
    return strlen( pStr );
#else
#    error
#endif // ( BGS_VISUAL_STUDIO )
}

size_t BIGOS::Core::Utils::String::Length( const wchar_t* pStr )
{
    BGS_ASSERT( pStr != nullptr, "String (pStr) must be a valid string." );

#if( BGS_VISUAL_STUDIO )
    return wcslen( pStr );
#else
#    error
#endif // ( BGS_VISUAL_STUDIO )
}

BIGOS::bool_t BIGOS::Core::Utils::String::Compare( const char* pFirst, const char* pSecond )
{
    BGS_ASSERT( pFirst != nullptr, "String (pFirst) must be a valid string." );
    BGS_ASSERT( pSecond != nullptr, "String (pSecond) must be a valid string." );

#if( BGS_VISUAL_STUDIO )
    return strcmp( pFirst, pSecond ) == 0 ? BGS_TRUE : BGS_FALSE;
#else
#    error
#endif // ( BGS_VISUAL_STUDIO )
}

BIGOS::bool_t BIGOS::Core::Utils::String::Compare( const wchar_t* pFirst, const wchar_t* pSecond )
{
    BGS_ASSERT( pFirst != nullptr, "String (pFirst) must be a valid string." );
    BGS_ASSERT( pSecond != nullptr, "String (pSecond) must be a valid string." );

#if( BGS_VISUAL_STUDIO )
    return wcscmp( pFirst, pSecond ) == 0 ? BGS_TRUE : BGS_FALSE;
#else
#    error
#endif // ( BGS_VISUAL_STUDIO )
}