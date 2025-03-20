#pragma once

#include "Core/CoreTypes.h"
#include "Driver/Backend/APITypes.h"

namespace BIGOS
{
    namespace Driver
    {
        namespace Frontend
        {

            class BGS_API ResourceState final
            {
            public:
                ResourceState( Backend::PipelineStageFlags stage  = BGS_FLAG( Backend::PipelineStageFlagBits::NONE ),
                               Backend::AccessFlags        access = BGS_FLAG( Backend::AccessFlagBits::NONE ),
                               Backend::TEXTURE_LAYOUT     layout = Backend::TextureLayouts::UNDEFINED )
                    : m_stage( stage )
                    , m_access( access )
                    , m_layout( layout )
                {
                }

                ~ResourceState() = default;

                Backend::PipelineStageFlags GetStage() const { return m_stage; }
                Backend::AccessFlags        GetAccess() const { return m_access; }
                Backend::TEXTURE_LAYOUT     GetLayout() const { return m_layout; }

                void SetStage( Backend::PipelineStageFlags stage ) { m_stage = stage; }

                void SetAccess( Backend::AccessFlags access ) { m_access = access; }

                void SetLayout( Backend::TEXTURE_LAYOUT layout ) { m_layout = layout; }

                void SetState( Backend::PipelineStageFlags stage, Backend::AccessFlags access, Backend::TEXTURE_LAYOUT layout )
                {
                    SetStage( stage );
                    SetAccess( access );
                    SetLayout( layout );
                }

                bool operator==( const ResourceState& other ) const
                {
                    return m_layout == other.m_layout && m_stage == other.m_stage && m_access == other.m_access;
                }

                bool operator!=( const ResourceState& other ) const
                {
                    return m_layout != other.m_layout || m_stage != other.m_stage || m_access != other.m_access;
                }

            private:
                Backend::PipelineStageFlags m_stage;
                Backend::AccessFlags        m_access;
                Backend::TEXTURE_LAYOUT     m_layout;
            };
        } // namespace Frontend
    } // namespace Driver
} // namespace BIGOS