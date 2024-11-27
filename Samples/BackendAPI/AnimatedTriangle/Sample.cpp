#include "Sample.h"

Sample::Sample( APITypes APIType, uint32_t width, uint32_t height, const char* pName )
    : m_width( width )
    , m_height( height )
    , m_pName( pName )
    , m_APIType( APIType )
    , m_running( true )
    , m_windowCloseHandler( [ this ]( const WindowCloseEvent& e ) { OnWindowClose( e ); } )
{
}

Sample::~Sample()
{
}

void Sample::OnWindowClose( const WindowCloseEvent& e )
{
    e;
    m_running = false;
}