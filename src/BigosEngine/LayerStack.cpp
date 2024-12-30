#include "BigosEngine/LayerStack.h"

namespace BIGOS
{
    LayerStack::~LayerStack()
    {
        for( Layer* layer: m_layers )
        {
            layer->OnDetach();
        }
    }

    void LayerStack::PushLayer( Layer* layer )
    {
        m_layers.emplace( m_layers.begin() + m_layerInsertNdx, layer );
        m_layerInsertNdx++;
    }

    void LayerStack::PushOverlay( Layer* overlay )
    {
        m_layers.emplace_back( overlay );
    }

    void LayerStack::PopLayer( Layer* layer )
    {
        auto it = std::find( m_layers.begin(), m_layers.begin() + m_layerInsertNdx, layer );
        if( it != m_layers.begin() + m_layerInsertNdx )
        {
            layer->OnDetach();
            m_layers.erase( it );
            m_layerInsertNdx--;
        }
    }

    void LayerStack::PopOverlay( Layer* overlay )
    {
        auto it = std::find( m_layers.begin(), m_layers.begin() + m_layerInsertNdx, overlay );
        if( it != m_layers.begin() + m_layerInsertNdx )
        {
            overlay->OnDetach();
            m_layers.erase( it );
        }
    }
} // namespace BIGOS