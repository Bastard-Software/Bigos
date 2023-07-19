#pragma once

#include "Core/CoreTypes.h"

namespace BIGOS
{
    namespace Core
    {
        namespace Utils
        {
            class BGS_API String
            {
            public:
                static BGS_FORCEINLINE void Copy( char* pDst, size_t dstSize, const char* pSrc );
                static BGS_FORCEINLINE void Copy( wchar_t* pDst, size_t dstSize, const wchar_t* pSrc );

                static BGS_FORCEINLINE size_t Utf16ToUtf8( char* pDst, size_t dstSize, const wchar_t* pSrc, size_t srcSize );
                static BGS_FORCEINLINE size_t Utf8ToUtf16( wchar_t* pDst, size_t dstSize, const char* pSrc, size_t srcSize );

                template<typename... ArgsT>
                static BGS_FORCEINLINE int32_t Format( char* pDst, uint32_t bufferSize, const char* pFormat, ArgsT&&... args );
                template<typename... ArgsT>
                static BGS_FORCEINLINE int32_t Format( wchar_t* pDst, uint32_t bufferSize, const wchar_t* pFormat, ArgsT&&... args );

                template<size_t BUFFER_SIZE, typename... ArgsT>
                static BGS_FORCEINLINE int32_t Format( char ( &pDst )[ BUFFER_SIZE ], const char* pFormat, ArgsT&&... args )
                {
                    return Format( pDst, static_cast<uint32_t>( BUFFER_SIZE ), pFormat, std::forward<ArgsT>( args )... );
                }
                template<size_t BUFFER_SIZE, typename... ArgsT>
                static BGS_FORCEINLINE int32_t Format( wchar_t ( &pDst )[ BUFFER_SIZE ], const wchar_t* pFormat, ArgsT&&... args )
                {
                    return Format( pDst, static_cast<uint32_t>( BUFFER_SIZE ), pFormat, std::forward<ArgsT>( args )... );
                }

                template<typename... ArgsT>
                static BGS_FORCEINLINE int32_t Parse( const char* pSrc, uint32_t bufferSize, const char* pFormat, ArgsT&&... args );
                template<typename... ArgsT>
                static BGS_FORCEINLINE int32_t Parse( const wchar_t* pSrc, uint32_t bufferSize, const wchar_t* pFormat, ArgsT&&... args );

                template<size_t BUFFER_SIZE, typename... ArgsT>
                static BGS_FORCEINLINE int32_t Parse( const char ( &pSrc )[ BUFFER_SIZE ], const char* pFormat, ArgsT&&... args )
                {
                    return Parse( pSrc, static_cast<uint32_t>( BUFFER_SIZE ), pFormat, std::forward<ArgsT>( args )... );
                }
                template<size_t BUFFER_SIZE, typename... ArgsT>
                static BGS_FORCEINLINE int32_t Parse( const wchar_t ( &pSrc )[ BUFFER_SIZE ], const wchar_t* pFormat, ArgsT&&... args )
                {
                    return Parse( pSrc, static_cast<uint32_t>( BUFFER_SIZE ), pFormat, std::forward<ArgsT>( args )... );
                }

                static BGS_FORCEINLINE size_t Length( const char* pStr );
                static BGS_FORCEINLINE size_t Length( const wchar_t* pStr );

                static BGS_FORCEINLINE bool_t Compare( const char* pFirst, const char* pSecond );
                static BGS_FORCEINLINE bool_t Compare( const wchar_t* pFirst, const wchar_t* pSecond );

                static index_t FindString( const HeapArray<const char*>& array, const char* pStr );
                static index_t FindString( const char* const* ppArray, size_t elemCount, const char* pStr );
            };
        } // namespace Utils
    }     // namespace Core
} // namespace BIGOS

#include "Core/Utils/String.inl"