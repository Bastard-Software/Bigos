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

                for( index_t i = 0; i < array.size(); ++i )
                {
                    if( String::Compare( array[ i ], pStr ) )
                    {
                        return i;
                    }
                }

                return INVALID_POSITION;
            }

            index_t String::FindString( const char* const* ppArray, size_t elemCount, const char* pStr )
            {
                BGS_ASSERT( pStr != nullptr, "String (pStr) must be a valid string." );
                BGS_ASSERT( ppArray != nullptr, "Array (ppArray) must be a valid array." );

                for( index_t i = 0; i < elemCount; ++i )
                {
                    if( String::Compare( *( ppArray + i ), pStr ) )
                    {
                        return i;
                    }
                }

                return INVALID_POSITION;
            }

        } // namespace Utils
    }     // namespace Core
} // namespace BIGOS