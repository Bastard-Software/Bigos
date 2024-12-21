#pragma once

#include "Core/CoreTypes.h"

namespace BIGOS
{
    namespace Core
    {
        namespace Utils
        {
            class BGS_API Timestep
            {
            public:
                Timestep( float time = 0.0f )
                    : m_time( time )
                {
                }

                operator float() const { return m_time; }

                float GetSeconds() const { return m_time; }
                float GetMilliseconds() const { return m_time * 1000.0f; }

            private:
                float m_time;
            };

        } // namespace Utils
    } // namespace Core
} // namespace BIGOS