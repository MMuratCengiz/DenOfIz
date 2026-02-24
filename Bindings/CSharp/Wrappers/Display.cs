using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public partial struct Display
    {
        public static DisplayArray GetDisplays()
        {
            return Methods.DenOfIz_Display_GetDisplays();
        }

        public static Display GetPrimaryDisplay()
        {
            return Methods.DenOfIz_Display_GetPrimaryDisplay();
        }

        public static void RefreshDisplays()
        {
            Methods.DenOfIz_Display_RefreshDisplays();
        }
    }
}
