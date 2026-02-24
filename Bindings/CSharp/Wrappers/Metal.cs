using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public static partial class Metal
    {
        public static bool BeginGpuCapture(LogicalDevice? device)
        {
            return Methods.DenOfIz_Metal_BeginGpuCapture(device);
        }

        public static void EndGpuCapture(LogicalDevice? device)
        {
            Methods.DenOfIz_Metal_EndGpuCapture(device);
        }

        public static bool IsCapturing(LogicalDevice? device)
        {
            return Methods.DenOfIz_Metal_IsCapturing(device);
        }

    }
}
