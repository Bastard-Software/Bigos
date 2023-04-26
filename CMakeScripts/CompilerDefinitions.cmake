function(EnableOption option)
    if( ${option} )
        add_definitions("-D${option}=1")
    else() 
        add_definitions("-D${option}=0")
    endif()
endfunction(EnableOption)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(BGS_WINDOWS 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(BGS_LINUX 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(BGS_ANDROID 1)
else()
    message(FATAL_ERROR "Unknown system")
endif()

option(BGS_VULKAN_API "Enable Vulkan API" ON)
option(BGS_D3D12_API "Enable DirectX12 API" ON)

option(BGS_SHADER_DEBUG "Enable shader debug features" ON)
option(BGS_RENDER_DEBUG "Enable render debug features" ON)

option(BGS_DETECT_MEMORY_LEAKS "Enable detecting memory leaks feature" ON)

# platform
EnableOption(BGS_WINDOWS)
EnableOption(BGS_LINUX)
EnableOption(BGS_ANDROID)

# API
EnableOption(BGS_VULKAN_API)
EnableOption(BGS_D3D12_API)

# debug 
EnableOption(BGS_SHADER_DEBUG)
EnableOption(BGS_RENDER_DEBUG)

EnableOption(BGS_DETECT_MEMORY_LEAKS)

if(BGS_VULKAN_API)
set(VOLK_STATIC_DEFINES 1)
    add_definitions(-DVK_NO_PROTOTYPES)
    if(BGS_WINDOWS)
        add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
    endif()
endif()

add_definitions(-DBGS_DLL_EXPORT)