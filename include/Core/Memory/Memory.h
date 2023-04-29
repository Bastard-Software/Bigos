#pragma once
#include "Core/CoreTypes.h"

namespace BIGOS
{
    namespace Core
    {
        namespace Memory
        {
#if( BGS_VISUAL_STUDIO )
#    define BGS_ALIGN( _bytes ) __declspec( align( ( _bytes ) ) )
#else
#    error
#endif // ( BGS_VISUAL_STUDIO )

            BGS_FORCEINLINE void* Malloc( size_t size );
            BGS_FORCEINLINE void* MallocAligned( size_t size, size_t alignment );
            BGS_FORCEINLINE void* Realloc( void* pMem, size_t size );
            BGS_FORCEINLINE void* ReallocAligned( void* pMem, size_t size, size_t alignment );
            BGS_FORCEINLINE void  Free( void* pMem );
            BGS_FORCEINLINE void  FreeAligned( void* pMem );

#if( BGS_MEMORY_DEBUG )

            BGS_FORCEINLINE void* MallocDebug( size_t size );
            BGS_FORCEINLINE void* MallocAlignedDebug( size_t size, size_t alignment );
            BGS_FORCEINLINE void* ReallocDebug( void* pMem, size_t size );
            BGS_FORCEINLINE void* ReallocAlignedDebug( void* pMem, size_t size, size_t alignment );
            BGS_FORCEINLINE void  FreeDebug( void* pMem );
            BGS_FORCEINLINE void  FreeAlignedDebug( void* pMem );

#endif // ( BGS_MEMORY_DEBUG )

            BGS_FORCEINLINE void    CopyMemory( const void* pSrc, size_t srcSize, void* pDst, size_t dstSize );
            BGS_FORCEINLINE void    SetMemory( void* pDst, int32_t val, size_t size );
            BGS_FORCEINLINE int32_t CompareMemory( const void* pMem1, const void* pMem2, size_t size );

        } // namespace Memory
    }     // namespace Core
} // namespace BIGOS

#include "Core/Memory/Memory.inl"