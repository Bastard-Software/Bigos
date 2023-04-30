#include "Core/CoreTypes.h"

void* BIGOS::Core::Memory::Malloc( size_t size )
{
    BGS_ASSERT( size > 0, "Block size must be greater than 0." );

    return malloc( size );
}

void* BIGOS::Core::Memory::MallocAligned( size_t size, size_t alignment )
{
    BGS_ASSERT( size > 0, "Block size must be greater than 0." );
    BGS_ASSERT( alignment > 0, "Alignment must be greater than 0." );
    BGS_ASSERT( ( alignment & ( alignment - 1 ) ) == 0, "Alignment must be the power of 2." );

#if( BGS_VISUAL_STUDIO )
    return _aligned_malloc( size, alignment );
#else
#    error
#endif // ( BGS_VISUAL_STUDIO )
}

void* BIGOS::Core::Memory::Realloc( void* pMem, size_t size )
{
    BGS_ASSERT( pMem != nullptr, "Memory block (pMem) must be valid pointer." );
    BGS_ASSERT( size > 0, "New block size must be greater than 0." );

    return realloc( pMem, size );
}

void* BIGOS::Core::Memory::ReallocAligned( void* pMem, size_t size, size_t alignment )
{
    BGS_ASSERT( pMem != nullptr, "Memory block (pMem) must be valid pointer." );
    BGS_ASSERT( size > 0, "Block size must be greater than 0." );
    BGS_ASSERT( alignment > 0, "Alignment must be greater than 0." );
    BGS_ASSERT( ( alignment & ( alignment - 1 ) ) == 0, "Alignment must be the power of 2." );

#if( BGS_VISUAL_STUDIO )
    return _aligned_realloc( pMem, size, alignment );
#else
#    error
#endif // ( BGS_VISUAL_STUDIO )
}

void BIGOS::Core::Memory::Free( void* pMem )
{
    BGS_ASSERT( pMem != nullptr, "Memory block pMem must be valid pointer." );

    return free( pMem );
}

void BIGOS::Core::Memory::FreeAligned( void* pMem )
{
    BGS_ASSERT( pMem != nullptr, "Memory block pMem must be valid pointer." );

    return _aligned_free( pMem );
}

#if( BGS_MEMORY_DEBUG )

void* BIGOS::Core::Memory::MallocDebug( size_t size )
{
    BGS_ASSERT( size > 0, "Block size must be greater than 0." );

    uint8_t* pBlock = static_cast<uint8_t*>( Malloc( size + sizeof( MemoryBlockInfo ) ) );

    return pBlock + sizeof( MemoryBlockInfo );
}

void* BIGOS::Core::Memory::MallocAlignedDebug( size_t size, size_t alignment )
{
    BGS_ASSERT( size > 0, "Block size must be greater than 0." );
    BGS_ASSERT( alignment > 0, "Alignment must be greater than 0." );
    BGS_ASSERT( ( alignment & ( alignment - 1 ) ) == 0, "Alignment must be the power of 2." );

    uint8_t* pBlock = static_cast<uint8_t*>( MallocAligned( size + sizeof( MemoryBlockInfo ), alignment ) );

    return pBlock + sizeof( MemoryBlockInfo );
}

void* BIGOS::Core::Memory::ReallocDebug( void* pMem, size_t size )
{
    BGS_ASSERT( pMem != nullptr, "Memory block (pMem) must be valid pointer." );
    BGS_ASSERT( size > 0, "New block size must be greater than 0." );

    uint8_t* pBlock    = static_cast<uint8_t*>( pMem ) - sizeof( MemoryBlockInfo );
    uint8_t* pNewBlock = static_cast<uint8_t*>( Realloc( pBlock, size ) );

    return pNewBlock + sizeof( MemoryBlockInfo );
}

void* BIGOS::Core::Memory::ReallocAlignedDebug( void* pMem, size_t size, size_t alignment )
{
    BGS_ASSERT( pMem != nullptr, "Memory block (pMem) must be valid pointer." );
    BGS_ASSERT( size > 0, "New block size must be greater than 0." );
    BGS_ASSERT( alignment > 0, "Alignment must be greater than 0." );
    BGS_ASSERT( ( alignment & ( alignment - 1 ) ) == 0, "Alignment must be the power of 2." );

    uint8_t* pBlock    = static_cast<uint8_t*>( pMem ) - sizeof( MemoryBlockInfo );
    uint8_t* pNewBlock = static_cast<uint8_t*>( ReallocAligned( pBlock, size, alignment ) );

    return pNewBlock + sizeof( MemoryBlockInfo );
}

void BIGOS::Core::Memory::FreeDebug( void* pMem )
{
    uint8_t* pBlock = static_cast<uint8_t*>( pMem ) - sizeof( MemoryBlockInfo );

    return Free( pBlock );
}

void BIGOS::Core::Memory::FreeAlignedDebug( void* pMem )
{
    uint8_t* pBlock = static_cast<uint8_t*>( pMem ) - sizeof( MemoryBlockInfo );

    return FreeAligned( pBlock );
}

#endif // ( BGS_MEMORY_DEBUG )

void BIGOS::Core::Memory::CopyMemory( const void* pSrc, size_t srcSize, void* pDst, size_t dstSize )
{
    BGS_ASSERT( pSrc != nullptr, "Memory block (pSrc) must be valid pointer." );
    BGS_ASSERT( srcSize > 0, "Block size (srcSize) must be greater than 0." );
    BGS_ASSERT( pDst != nullptr, "Memory block (pDst) must be valid pointer." );
    BGS_ASSERT( srcSize > 0, "Block size (dstSize) must be greater than 0." );

#if( BGS_VISUAL_STUDIO )
    memcpy_s( pDst, dstSize, pSrc, srcSize );
#else
#    error
#endif // ( BGS_VISUAL_STUDIO )
}

void BIGOS::Core::Memory::SetMemory( void* pDst, int32_t val, size_t size )
{
    BGS_ASSERT( pDst != nullptr, "Memory block (pDst) must be valid pointer." );
    BGS_ASSERT( size > 0, "Block size must be greater than 0." );

    memset( pDst, val, size );
}

int32_t BIGOS::Core::Memory::CompareMemory( const void* pMem1, const void* pMem2, size_t size )
{
    BGS_ASSERT( pMem1 != nullptr, "Memory block (pSrc) must be valid pointer." );
    BGS_ASSERT( pMem2 != nullptr, "Memory block (pSrc) must be valid pointer." );
    BGS_ASSERT( size > 0, "Block size (dstSize) must be greater than 0." );

    return memcmp( pMem1, pMem2, size );
}