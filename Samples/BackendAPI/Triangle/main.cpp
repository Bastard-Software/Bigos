#include "Application.h"
#include "Triangle.h"

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
    Triangle sample( BIGOS::Driver::Backend::APITypes::D3D12, 1280, 720, "Triangle" );
    return Application::Run( &sample );
}