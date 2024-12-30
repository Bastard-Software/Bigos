#pragma once

#include "Core/CoreTypes.h"
#include "Platform/PlatformTypes.h"

namespace BIGOS
{
    class Layer;
    class LayerStack;
    class Application;

    using LayerArray = HeapArray<Layer*>;
} // namespace BIGOS