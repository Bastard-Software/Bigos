#pragma once
#include "Core/CoreTypes.h"
#include "Core/Memory/MemoryTypes.h"

#include "Core/Memory/IAllocator.h"

namespace BIGOS
{
    namespace Core
    {
        namespace Memory
        {
            class BGS_API SystemHeapAllocator final : public IAllocator
            {
                friend class MemorySystem;

            public:
                SystemHeapAllocator()          = default;
                virtual ~SystemHeapAllocator() = default;

                BGS_FORCEINLINE RESULT Allocate( size_t size, void** ppMemory, const char* pFile = __FILE__, uint32_t line = __LINE__ ) override;
                BGS_FORCEINLINE RESULT AllocateAligned( size_t size, size_t alignment, void** ppMemory, const char* pFile = __FILE__,
                                                        uint32_t line = __LINE__ ) override;
                BGS_FORCEINLINE void   Free( void** ppMemory ) override;
                BGS_FORCEINLINE void   FreeAligned( void** ppMemory ) override;

            protected:
                RESULT Create( const AllocatorDesc& desc ) override;
                void   Destroy() override;

#if( BGS_MEMORY_DEBUG )
            private:
                BGS_FORCEINLINE void Register( const MemoryBlockInfo* pDebugInfo );
                BGS_FORCEINLINE void Deregister( const MemoryBlockInfo* pDebugInfo );
#endif // ( BGS_MEMORY_DEBUG )

            private:
                MemorySystem* m_pParent;
            };
        } // namespace Memory
    }     // namespace Core
} // namespace BIGOS