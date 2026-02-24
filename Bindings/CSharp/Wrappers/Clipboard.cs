using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public static partial class Clipboard
    {
        public static void SetText(StringView text)
        {
            Methods.DenOfIz_Clipboard_SetText(text);
        }

        public static StringView GetText()
        {
            return Methods.DenOfIz_Clipboard_GetText();
        }

        public static bool HasText()
        {
            return Methods.DenOfIz_Clipboard_HasText();
        }

        public static void Clear()
        {
            Methods.DenOfIz_Clipboard_Clear();
        }

    }
}
