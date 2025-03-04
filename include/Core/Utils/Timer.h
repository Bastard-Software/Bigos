#pragma once

#include "Core/CoreTypes.h"

#include <chrono>

namespace BIGOS
{
    namespace Core
    {
        namespace Utils
        {
            class BGS_API Timer
            {
            public:
                Timer() { Reset(); }

                void Reset() { m_start = std::chrono::high_resolution_clock::now(); }

                float Elapsed()
                {
                    return std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now() - m_start ).count() *
                           0.001f * 0.001f * 0.001f;
                }

                float ElapsedMillis() { return Elapsed() * 1000.0f; }

            private:
                std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
            };

        } // namespace Utils
    } // namespace Core
} // namespace BIGOS