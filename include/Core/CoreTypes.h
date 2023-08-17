#pragma once
#include "CoreDependencies.h"

#ifdef _DEBUG
#    define BGS_DEBUG   1
#    define BGS_RELEASE 0
#else
#    define BGS_DEBUG   0
#    define BGS_RELEASE 1
#endif // _DEBUG

#if defined( BGS_WINDOWS )
#    if defined( BGS_DLL_EXPORT )
#        define BGS_API __declspec( dllexport )
#    else
#        define BGS_API __declspec( dllimport )
#    endif // BGS_DLL_EXPORT
#    define BGS_API_INTERFACE __declspec( novtable )
#else
#    error "not implemented"
#endif

#define BGS_HANDLE_TAG_NAME( name ) name##T

#define BGS_DECLARE_HANDLE( name )                                                                                                                   \
    namespace Private                                                                                                                                \
    {                                                                                                                                                \
        struct BGS_HANDLE_TAG_NAME( name );                                                                                                          \
    }                                                                                                                                                \
    using name##Handle = BIGOS::Core::Handle<Private::BGS_HANDLE_TAG_NAME( name )>

#if( BGS_VISUAL_STUDIO )
#    define BGS_FORCEINLINE __forceinline
#else // TODO: Support other compilers
#    define BGS_FORCEINLINE inline
#endif

// TODO: Implement better
#define BGS_ASSERT_DEBUG( x, ... )                                                                                                                   \
    {                                                                                                                                                \
        if( !( x ) )                                                                                                                                 \
        {                                                                                                                                            \
            __debugbreak();                                                                                                                          \
        }                                                                                                                                            \
    }

#if( BGS_DEBUG )
#    define BGS_ASSERT BGS_ASSERT_DEBUG
#else
#    define BGS_ASSERT
#endif // BGS_DEBUG

#define BGS_FAILED( result )  ( ( result ) != BIGOS::Core::Results::OK )
#define BGS_SUCCESS( result ) ( ( result ) == BIGOS::Core::Results::OK )

#define SET_BITS( var, ndx ) ( ( var ) |= ~( ~0u << ( ndx ) ) )
#define SET_BIT( var, ndx )  ( ( var ) |= 1 << ( ndx ) )
#define CLEAR_BIT( var, ndx ) ( ( var ) &= ~( 1u << ( ndx ) )

#define BGS_ENUM_COUNT( enumClass ) static_cast<uint32_t>( ( enumClass::_MAX_ENUM ) )
#define BGS_ENUM_INDEX( enumVal )   static_cast<uint32_t>( ( enumVal ) )

namespace BIGOS
{
    class BigosFramework;

    inline namespace Core
    {
        using handle_t = uint64_t;
        using hash_t   = uint64_t;
        using index_t  = size_t;
        using byte_t   = std::byte;
        using bool_t   = uint32_t;

        static constexpr bool_t BGS_TRUE  = 1u;
        static constexpr bool_t BGS_FALSE = 0u;

        static constexpr uint32_t MAX_UINT32 = ( static_cast<uint32_t>( ~0 ) );
        static constexpr int32_t  MAX_INT32  = INT32_MAX;
        static constexpr int32_t  MIN_INT32  = INT32_MIN;
        static constexpr uint64_t MAX_UINT64 = ( static_cast<uint64_t>( ~0 ) );
        static constexpr int64_t  MAX_INT64  = INT64_MAX;
        static constexpr int64_t  MIN_INT64  = INT64_MIN;
        static constexpr size_t   MAX_SIZET  = ( static_cast<size_t>( ~0 ) );

        static constexpr handle_t INVALID_HANDLE    = ( static_cast<handle_t>( MAX_UINT64 ) );
        static constexpr index_t  INVALID_POSITION  = ( static_cast<index_t>( ~0 ) );
        static constexpr uint64_t INVALID_SIZE      = MAX_UINT64;
        static constexpr uint64_t INVALID_OFFSET    = MAX_UINT64;
        static constexpr size_t   DEFAULT_ALIGNMENT = MAX_SIZET;

        // TODO: Use own container implementations
        using String  = std::string;
        using WString = std::wstring;

        template<typename T>
        using HeapArray = std::vector<T>;

        template<typename T, size_t S>
        using StackArray = std::array<T, S>;

        template<typename T>
        struct HandleTraits
        {
            using NativeHandleTypes = handle_t;
        };

        template<typename T>
        class Handle final
        {
        private:
            template<typename HandleType>
            union HandleConverter
            {
                handle_t   handle;
                HandleType nativeHandle;
            };

        public:
            BGS_FORCEINLINE constexpr Handle()
                : m_value( INVALID_HANDLE )
            {
            }

            template<typename HandleType>
            BGS_FORCEINLINE constexpr explicit Handle( HandleType nativeHandle )
            {
                static_assert( sizeof( HandleType ) <= sizeof( handle_t ), "Native handle to big to be stored in the Handle<T>." );

                HandleConverter<HandleType> converter;
                converter.nativeHandle = nativeHandle;

                m_value = converter.handle;
            }

            template<typename HandleType = HandleTraits<T>::NativeHandleTypes>
            BGS_FORCEINLINE HandleType GetNativeHandle() const
            {
                HandleConverter<HandleType> converter;
                converter.handle = m_value;

                return converter.nativeHandle;
            }

            BGS_FORCEINLINE bool operator==( const Handle<T>& other ) const { return m_value == other.m_value; }
            BGS_FORCEINLINE bool operator!=( const Handle<T>& other ) const { return m_value != other.m_value; }

        private:
            handle_t m_value;
        };

        enum class Results
        {
            OK,
            FAIL,
            NO_MEMORY,
            NOT_FOUND,
            TIMEOUT,
            NOT_READY,
            // TODO: Add more during development
            _MAX_ENUM,
        };
        using RESULT = Results;

        struct VersionDesc
        {
            uint16_t major;
            uint16_t minor;
            uint16_t patch;
            uint16_t build;
        };

        template<typename T>
        struct TExtent2D
        {
            union
            {
                struct
                {
                    T width;
                    T height;
                };
                struct
                {
                    T x;
                    T y;
                };
            };
        };

        using Offset2D = TExtent2D<int32_t>;
        using Size2D   = TExtent2D<uint32_t>;

        template<typename T>
        struct TRect2D
        {
            TExtent2D<T> offset;
            TExtent2D<T> size;
        };

        using Rect2D = TRect2D<uint32_t>;

        template<typename T>
        struct TExtent3D
        {
            union
            {
                struct
                {
                    T width;
                    T height;
                    T depth;
                };
                struct
                {
                    T x;
                    T y;
                    T z;
                };
            };
        };

        using Offset3D = TExtent3D<int32_t>;
        using Size3D   = TExtent3D<uint32_t>;
    } // namespace Core
} // namespace BIGOS