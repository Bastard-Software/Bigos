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

            template<typename T, class AllocatorT>
            BGS_FORCEINLINE RESULT Allocate( AllocatorT* pAllocator, T** ppMem )
            {
                BGS_ASSERT( ( ppMem != nullptr ) && ( *ppMem == nullptr ) );
                BGS_ASSERT( pAllocator != nullptr );

                return pAllocator->Allocate( sizeof( T ), ppMem );
            }

            template<typename T, class AllocatorT>
            BGS_FORCEINLINE void Free( AllocatorT* pAllocator, T** ppMem )
            {
                BGS_ASSERT( ( ppMem != nullptr ) && ( *ppMem != nullptr ) );
                BGS_ASSERT( pAllocator != nullptr );

                return pAllocator->Free( static_cast<void**>( ppMem ) );
            }

            template<typename T, typename... ArgsT>
            BGS_FORCEINLINE T* CreateObject( void* pMem, ArgsT&&... args )
            {
                BGS_ASSERT( pMem != nullptr );

                return ::new( pMem ) T( args... );
            }

            template<typename T, class AllocatorT, typename... ArgsT>
            BGS_FORCEINLINE RESULT AllocateObject( AllocatorT* pAllocator, T** ppObj, ArgsT&&... args )
            {
                BGS_ASSERT( ( ppObj != nullptr ) && ( *ppObj == nullptr ) );
                BGS_ASSERT( pAllocator != nullptr );

                void* pMem = nullptr;
                if( BGS_SUCCESS( pAllocator->Allocate( sizeof( T ), &pMem ) ) )
                {
                    *ppObj = CreateObject<T>( pMem, args... );
                    return Results::OK;
                }

                *ppObj = nullptr;

                return Results::NO_MEMORY;
            }

            template<typename T, class AllocatorT>
            BGS_FORCEINLINE void FreeObject( AllocatorT* pAllocator, T** ppObj )
            {
                BGS_ASSERT( ( ppObj != nullptr ) && ( *ppObj != nullptr ) );
                BGS_ASSERT( pAllocator != nullptr );

                if( *ppObj != nullptr )
                {
                    ( *ppObj )->~T();
                    pAllocator->Free( reinterpret_cast<void**>( ppObj ) );
                }
            }

            template<typename T, class AllocatorT>
            BGS_FORCEINLINE RESULT AllocateArray( AllocatorT* pAllocator, T** ppFirst, uint32_t elemCount, size_t alignment )
            {
                BGS_ASSERT( ( ppFirst != nullptr ) && ( *ppFirst == nullptr ) );
                BGS_ASSERT( pAllocator != nullptr );

                void* pMem = nullptr;
                if( BGS_SUCCESS( pAllocator->AllocateAligned( elemCount * sizeof( T ), alignment, &pMem ) ) )
                {
                    *ppFirst = static_cast<T*>( pMem );
                    return Results::OK;
                }

                *ppFirst = nullptr;

                return Results::NO_MEMORY;
            }

            template<typename T, class AllocatorT>
            BGS_FORCEINLINE void FreeAligned( AllocatorT* pAllocator, T** ppMem )
            {
                BGS_ASSERT( ( ppMem != nullptr ) && ( *ppMem != nullptr ) );
                BGS_ASSERT( pAllocator != nullptr );

                pAllocator->FreeAligned( static_cast<void**>( ppMem ) );
            }

            template<class AllocatorT>
            BGS_FORCEINLINE RESULT AllocateBytes( AllocatorT* pAllocator, byte_t** ppBytes, size_t size )
            {
                BGS_ASSERT( ( ppBytes != nullptr ) && ( *ppBytes == nullptr ) );
                BGS_ASSERT( pAllocator != nullptr );

                void* pMem = nullptr;
                if( BGS_SUCCESS( pAllocator->Allocate( size, &pMem ) ) )
                {
                    *ppBytes = static_cast<byte_t*>( pMem );
                    return Results::OK;
                }

                *ppBytes = nullptr;

                return Results::NO_MEMORY;
            }
        } // namespace Memory
    }     // namespace Core
} // namespace BIGOS

#include "Core/Memory/Memory.inl"