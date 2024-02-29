#pragma once

#include "BigosFramework/BigosFramework.h"
#include "SampleHelper.h"

using namespace BIGOS::Driver::Backend;

class Sample
{
public:
    Sample( APITypes APIType, uint32_t width, uint32_t height, const char* pName );
    virtual ~Sample();

    virtual BIGOS::RESULT OnInit()    = 0;
    virtual void          OnUpdate()  = 0;
    virtual void          OnRender()  = 0;
    virtual void          OnDestroy() = 0;

    const API_TYPE GetAPIType() const { return m_APIType; }

protected:
    const char* m_pName;
    uint32_t    m_width;
    uint32_t    m_height;
    API_TYPE    m_APIType;
};