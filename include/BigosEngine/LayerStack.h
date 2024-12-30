#pragma once

#include "BigosEngine/BigosTypes.h"

#include "BigosEngine/Layer.h"

namespace BIGOS
{
    class BGS_API LayerStack
    {
    public:
        LayerStack() = default;
        ~LayerStack();

        void PushLayer( Layer* layer );
        void PushOverlay( Layer* overlay );
        void PopLayer( Layer* layer );
        void PopOverlay( Layer* overlay );

        LayerArray::iterator         begin() { return m_layers.begin(); }
        LayerArray::iterator         end() { return m_layers.end(); }
        LayerArray::reverse_iterator rbegin() { return m_layers.rbegin(); }
        LayerArray::reverse_iterator rend() { return m_layers.rend(); }

        LayerArray::const_iterator         begin() const { return m_layers.begin(); }
        LayerArray::const_iterator         end() const { return m_layers.end(); }
        LayerArray::const_reverse_iterator rbegin() const { return m_layers.rbegin(); }
        LayerArray::const_reverse_iterator rend() const { return m_layers.rend(); }

    private:
        LayerArray m_layers;
        uint32_t   m_layerInsertNdx = 0;
    };
} // namespace BIGOS