#pragma once
#include "Core/Memory/MemoryTypes.h"

#include "Core/Memory/Memory.h"

namespace BIGOS
{
    namespace Core
    {
        namespace Memory
        {
            class BGS_API BGS_API_INTERFACE IAllocator
            {
                friend class MemorySystem;

            public:
                virtual ~IAllocator() = default;

                virtual RESULT Allocate( size_t size, void** ppMemory, const char* pFile = __FILE__,
                                         uint32_t line = __LINE__ )                                      = 0;
                virtual RESULT AllocateAligned( size_t size, size_t alignment, void** ppMemory,
                                                const char* pFile = __FILE__, uint32_t line = __LINE__ ) = 0;
                virtual void   Free( void** ppMemory )                                                   = 0;
                virtual void   FreeAligned( void** ppMemory )                                            = 0;

            protected:
                virtual RESULT Create( const AllocatorDesc& desc ) = 0;
                virtual void   Destroy()                           = 0;
            };
        } // namespace Memory
    }     // namespace Core
} // namespace BIGOS