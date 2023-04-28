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

#define BGS_DECLARE_HANDLE( name )                                                                                     \
    namespace Private                                                                                                  \
    {                                                                                                                  \
        struct BGS_HANDLE_TAG_NAME( name );                                                                            \
    }                                                                                                                  \
    using name##Handle = BIGOS::Core::Handle<Private::BGS_HANDLE_TAG_NAME( name )>

#if( BGS_VISUAL_STUDIO )
#    define BGS_FORCEINLINE __forceinline
#else // TODO: Support other compilers
#    define BGS_FORCEINLINE inline
#endif

// TODO: Implement
#define BGS_ASSERT_DEBUG( ... )
#define BGS_ASSERT_RELEASE( ... )

#if( BGS_DEBUG )
#    define BGS_ASSERT BGS_ASSERT_DEBUG
#else
#    define BGS_ASSERT BGS_ASSERT_RELEASE
#endif // BGS_DEBUG

namespace BIGOS
{
    inline namespace Core
    {
        using handle_t = uint64_t;
        using hash_t   = uint64_t;
        using index_t  = size_t;
        using byte_t   = std::byte;

        static constexpr uint32_t MAX_UINT32 = ( static_cast<uint32_t>( ~0 ) );
        static constexpr uint64_t MAX_UINT64 = ( static_cast<uint64_t>( ~0 ) );

        static constexpr handle_t INVALID_HANDLE   = ( static_cast<handle_t>( MAX_UINT64 ) );
        static constexpr index_t  INVALID_POSITION = ( static_cast<index_t>( ~0 ) );
        static constexpr uint64_t INVALID_SIZE     = MAX_UINT64;
        static constexpr uint64_t INVALID_OFFSET   = MAX_UINT64;

        // TODO: Use own container implementations
        using String  = std::string;
        using WString = std::wstring;

        template<typename T>
        using HeapArray = std::vector<T>;

        template<typename T, size_t S>
        using StackArray = std::array<T, S>;

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
                static_assert( sizeof( HandleType ) <= sizeof( handle_t ),
                               "Native handle to big to be stored in the Handle<T>." );

                HandleConverter<HandleType> converter;
                converter.nativeHandle = nativeHandle;

                m_value = converter.handle;
            }

            template<typename HandleType = T>
            BGS_FORCEINLINE HandleType GetNativeHandle() const
            {
                HandleConverter<HandleType> converter;
                converter.handle = m_value;

                return converter.nativeHandle;
            }

        private:
            handle_t m_value;
        };

        enum class Results
        {
            OK,
            FAIL,
            NO_MEMORY
            // TODO: Add more during development
        };
        using RESULT = Results;

        struct VersionDesc
        {
            uint16_t major;
            uint16_t minor;
            uint16_t patch;
            uint16_t build;
        };
    } // namespace Core
} // namespace BIGOS