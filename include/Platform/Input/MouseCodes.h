#pragma once

#include "Core/CoreTypes.h"

namespace BIGOS
{
    namespace Platform
    {
        namespace Input
        {
            using MouseCode = uint32_t;

            namespace Mouse
            {
                enum MouseCode
                {
                    Button0 = 0,
                    Button1 = 1,
                    Button2 = 2,
                    Button3 = 3,
                    Button4 = 4,
                    Button5 = 5,
                    Button6 = 6,
                    Button7 = 7,

                    ButtonLast   = Button7,
                    ButtonLeft   = 0x01,
                    ButtonRight  = 0x02,
                    ButtonMiddle = 0x04
                };
            }
        } // namespace Input
    }     // namespace Platform
} // namespace BIGOS