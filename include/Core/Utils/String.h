#pragma once

#include "Core/CoreTypes.h"

namespace BIGOS
{
    namespace Core
    {
        namespace Utils
        {
            namespace String
            {

                BGS_FORCEINLINE void Copy( char* pDst, size_t dstSize, const char* pSrc );
                BGS_FORCEINLINE void Copy( wchar_t* pDst, size_t dstSize, const wchar_t* pSrc );

                BGS_FORCEINLINE size_t Utf16ToUtf8( char* pDst, size_t dstSize, const wchar_t* pSrc, size_t srcSize );
                BGS_FORCEINLINE size_t Utf8ToUtf16( wchar_t* pDst, size_t dstSize, const char* pSrc, size_t srcSize );

                BGS_FORCEINLINE index_t FindString( const HeapArray<const char*>& array, const char* pStr );
                BGS_FORCEINLINE index_t FindString( const char* const* ppArray, size_t elemCount, const char* pStr );

                BGS_FORCEINLINE size_t Length( const char* pStr );
                BGS_FORCEINLINE size_t Length( const wchar_t* pStr );

                BGS_FORCEINLINE bool_t Compare( const char* pFirst, const char* pSecond );
                BGS_FORCEINLINE bool_t Compare( const wchar_t* pFirst, const wchar_t* pSecond );

            } // namespace String
        }     // namespace Utils
    }         // namespace Core
} // namespace BIGOS

#include "Core/Utils/String.inl"