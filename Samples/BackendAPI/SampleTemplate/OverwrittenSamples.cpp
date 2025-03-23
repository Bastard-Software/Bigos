#include "Application.h"
#include "Driver/Frontend/RenderSystem.h"
#include "Driver/Frontend/Shader/IShaderCompiler.h"
#include "OverwrittenSample.h"
#include "Platform/Window/Window.h"

OverwrittenSample::OverwrittenSample( APITypes APIType, uint32_t width, uint32_t height, const char* pName )
    : Sample( APIType, width, height, pName )
{
}

BIGOS::RESULT OverwrittenSample::OnInit()
{
    return BIGOS::Results::OK;
}

void OverwrittenSample::OnUpdate()
{
}

void OverwrittenSample::OnRender()
{
}

void OverwrittenSample::OnDestroy()
{
}