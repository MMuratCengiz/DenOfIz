using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public static partial class OS
    {
        public static float GetDisplayScale(int display)
        {
            return Methods.DenOfIz_OS_GetDisplayScale(display);
        }

        public static float GetWindowSizeScaleFactor(int display)
        {
            return Methods.DenOfIz_OS_GetWindowSizeScaleFactor(display);
        }

    }
}
