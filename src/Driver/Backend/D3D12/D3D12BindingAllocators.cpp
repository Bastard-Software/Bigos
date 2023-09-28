#include "D3D12BindingAllocators.h"

namespace BIGOS::Driver::Backend
{

    RESULT PoolAllocator::Create( uint32_t capacity )
    {
        m_chunkMap.assign( capacity, BGS_TRUE );
        return Results::OK;
    }

    void PoolAllocator::Destroy() { m_chunkMap.clear(); }

    RESULT PoolAllocator::Allocate( uint32_t* pNdx )
    {
        BGS_ASSERT( pNdx != nullptr, "Uint (pNdx) must be a valid address." );
        for( index_t ndy = 0; ndy < m_chunkMap.size(); ++ndy )
        {
            if( m_chunkMap[ ndy ] )
            {
                *pNdx             = static_cast<uint32_t>( ndy );
                m_chunkMap[ ndy ] = BGS_FALSE;
                return Results::OK;
            }
        }

        *pNdx = MAX_UINT32;

        return Results::NO_MEMORY;
    }

    void PoolAllocator::Free( uint32_t ndx )
    {
        BGS_ASSERT( ndx < static_cast<uint32_t>( m_chunkMap.size() ), "Allocation index (ndx) out of range." );
        BGS_ASSERT( m_chunkMap[ ndx ] == BGS_FALSE, "Given index (ndx) is already free." );
        m_chunkMap[ ndx ] = BGS_TRUE;
    }
} // namespace BIGOS::Driver::Backend