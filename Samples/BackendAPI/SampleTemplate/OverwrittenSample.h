#pragma once

#include "Driver/Frontend/RenderDevice.h"
#include "Platform/WindowSystem.h"
#include "Sample.h"

using namespace BIGOS::Driver;

class OverwrittenSample : public Sample
{
public:
    OverwrittenSample( APITypes APIType, uint32_t width, uint32_t height, const char* pName );

    virtual BIGOS::RESULT OnInit() override;
    virtual void          OnUpdate() override;
    virtual void          OnRender() override;
    virtual void          OnDestroy() override;

};