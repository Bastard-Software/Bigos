#pragma once

#include "Core/CoreTypes.h"
#include "Platform/PlatformTypes.h"

namespace BIGOS
{
    class Layer;
    class LayerStack;
    class Application;
    class Renderer;

    using LayerArray = HeapArray<Layer*>;
} // namespace BIGOS