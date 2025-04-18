#include "Application.h"
#include "RotatedQuads.h"

// Temporary solution
extern "C"
{

#if( BGS_D3D12_API )

    BGS_API extern const UINT  D3D12SDKVersion = 610;
    BGS_API extern const char* D3D12SDKPath    = ".\\D3D12\\";

#endif // ( BGS_D3D12_API )
};

int main()
{
    RotatedQuads sample( BIGOS::Driver::Backend::APITypes::VULKAN, 1280, 720, "Rotated Quad" );
    return Application::Run( &sample );
}