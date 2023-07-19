#define BGS_USE_D3D12_HANDLES 1

#include "D3D12Adapter.h"

#include <initguid.h>

#include "Core/Utils/String.h"
#include <SetupAPI.h>
#include <devpkey.h>

namespace BIGOS
{
    namespace Driver
    {
        namespace Backend
        {
            static const GUID   BIGOS_GUID_DEVCLASS_DISPLAY_ADAPTERS = { 0x4d36e968, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 };
            static ADAPTER_TYPE MapD3D12AdapterTypeTypeToBIGOSAdapterType( IDXGIAdapter4* pNativeAdapter4 );
            static VersionDesc  GetD3D12AdapterDriverVersion( const DXGI_ADAPTER_DESC3& desc );

            RESULT D3D12Adapter::Create( IDXGIAdapter1* pAdapter, D3D12Factory* pFactory )
            {
                BGS_ASSERT( pAdapter != nullptr, "Adapter (pAdapter) must be a valid pointer." );
                BGS_ASSERT( pFactory != nullptr, "Factory (pFactory) must be a valid pointer." );

                m_handle  = AdapterHandle( pAdapter );
                m_pParent = pFactory;

                if( BGS_FAILED( GetInternalInfo() ) )
                {
                    Destroy();
                    return Results::FAIL;
                }

                return Results::OK;
            }

            void D3D12Adapter::Destroy()
            {
                IDXGIAdapter1* pNativeAdapter = m_handle.GetNativeHandle();
                RELEASE_COM_PTR( pNativeAdapter );

                m_handle  = AdapterHandle();
                m_pParent = nullptr;
            }

            RESULT D3D12Adapter::GetInternalInfo()
            {
                BGS_ASSERT( m_handle != AdapterHandle() );
                IDXGIAdapter1* pNativeAdapter = m_handle.GetNativeHandle();
                BGS_ASSERT( pNativeAdapter != nullptr );

                IDXGIAdapter4* pNativeAdapter4 = nullptr;
                if( FAILED( pNativeAdapter->QueryInterface( IID_PPV_ARGS( &pNativeAdapter4 ) ) ) )
                {
                    return Results::FAIL;
                }
                BGS_ASSERT( pNativeAdapter4 != nullptr );

                DXGI_ADAPTER_DESC3 desc;
                if( FAILED( pNativeAdapter4->GetDesc3( &desc ) ) )
                {
                    return Results::FAIL;
                }

                char* pDesc = m_info.description;
                Core::Utils::String::Utf16ToUtf8( pDesc, 256, desc.Description, 256 );
                m_info.vendorID      = desc.VendorId;
                m_info.deviceID      = desc.DeviceId;
                m_info.type          = MapD3D12AdapterTypeTypeToBIGOSAdapterType( pNativeAdapter4 );
                m_info.driverVersion = GetD3D12AdapterDriverVersion( desc );

                RELEASE_COM_PTR( pNativeAdapter4 );

                return Results::OK;
            }

            static ADAPTER_TYPE MapD3D12AdapterTypeTypeToBIGOSAdapterType( IDXGIAdapter4* pNativeAdapter4 )
            {
                DXGI_ADAPTER_DESC3 desc;
                pNativeAdapter4->GetDesc3( &desc );

                DXGI_QUERY_VIDEO_MEMORY_INFO nonLocalInfo;
                pNativeAdapter4->QueryVideoMemoryInfo( 0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &nonLocalInfo );

                if( desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE )
                {
                    return AdapterTypes::SOFTWARE;
                }
                else if( nonLocalInfo.Budget == 0 )
                {
                    return AdapterTypes::INTEGRATED;
                }

                return AdapterTypes::DISCRETE;
            }

            static VersionDesc GetD3D12AdapterDriverVersion( const DXGI_ADAPTER_DESC3& desc )
            {
                VersionDesc driverVersion = {};

                // Construct device instance path
                wchar_t deviceInstancePath[ 45 ] = {};
                Utils::String::Format( deviceInstancePath, L"PCI\\VEN_%04X&DEV_%04X&SUBSYS_%08X&REV_%02X", desc.VendorId & 0xFFFF,
                                       desc.DeviceId & 0xFFFF, desc.SubSysId & 0xFFFFFFFF, desc.Revision & 0xFF );

                // Obtain handle to the requested device information set
                HDEVINFO hDeviceInfoSet = SetupDiGetClassDevsW( &BIGOS_GUID_DEVCLASS_DISPLAY_ADAPTERS, deviceInstancePath, nullptr, 0 );

                if( hDeviceInfoSet != INVALID_HANDLE_VALUE )
                {
                    SP_DEVINFO_DATA deviceInfoData;
                    deviceInfoData.cbSize = sizeof( deviceInfoData );

                    // Enumerate all devices in the filtred display adapter class
                    if( SetupDiEnumDeviceInfo( hDeviceInfoSet, 0, &deviceInfoData ) )
                    {
                        DWORD deviceDriverVersionPropertyType = 0;
                        DWORD deviceDriverVersionPropertySize = 0;

                        // Get driver version string length
                        SetupDiGetDevicePropertyW( hDeviceInfoSet, &deviceInfoData, &DEVPKEY_Device_DriverVersion, &deviceDriverVersionPropertyType,
                                                   nullptr, 0, &deviceDriverVersionPropertySize, 0 );

                        if( ( deviceDriverVersionPropertyType == DEVPROP_TYPE_STRING ) && ( deviceDriverVersionPropertySize > 0 ) )
                        {
                            uint32_t deviceDriverVersionStringLength = deviceDriverVersionPropertySize / sizeof( wchar_t );

                            HeapArray<wchar_t> deviceDriverVersionString( deviceDriverVersionStringLength );

                            // Get driver version string
                            if( SetupDiGetDevicePropertyW(
                                    hDeviceInfoSet, &deviceInfoData, &DEVPKEY_Device_DriverVersion, &deviceDriverVersionPropertyType,
                                    reinterpret_cast<PBYTE>( deviceDriverVersionString.data() ), deviceDriverVersionPropertySize, nullptr, 0 ) )
                            {
                                // Parse version string
                                Utils::String::Parse( deviceDriverVersionString.data(), deviceDriverVersionStringLength, L"%hu.%hu.%hu.%hu",
                                                      &driverVersion.major, &driverVersion.minor, &driverVersion.patch, &driverVersion.build );
                            }
                        }
                    }

                    // Close obtained handle
                    SetupDiDestroyDeviceInfoList( hDeviceInfoSet );
                }

                return driverVersion;
            }

        } // namespace Backend
    }     // namespace Driver
} // namespace BIGOS