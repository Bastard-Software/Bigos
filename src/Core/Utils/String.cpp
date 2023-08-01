#include "Core/Utils/String.h"

namespace BIGOS
{
    namespace Core
    {
        namespace Utils
        {

            index_t String::FindString( const HeapArray<const char*>& array, const char* pStr )
            {
                BGS_ASSERT( pStr != nullptr, "String (pStr) must be a valid string." );
                BGS_ASSERT( array.size() > 0, "Array (array) can not be empty." );

                for( index_t ndx = 0; ndx < array.size(); ++ndx )
                {
                    if( String::Compare( array[ ndx ], pStr ) )
                    {
                        return ndx;
                    }
                }

                return INVALID_POSITION;
            }

            index_t String::FindString( const char* const* ppArray, size_t elemCount, const char* pStr )
            {
                BGS_ASSERT( pStr != nullptr, "String (pStr) must be a valid string." );
                BGS_ASSERT( ppArray != nullptr, "Array (ppArray) must be a valid array." );

                for( index_t ndx = 0; ndx < elemCount; ++ndx )
                {
                    if( String::Compare( *( ppArray + ndx ), pStr ) )
                    {
                        return ndx;
                    }
                }

                return INVALID_POSITION;
            }

        } // namespace Utils
    }     // namespace Core
} // namespace BIGOS