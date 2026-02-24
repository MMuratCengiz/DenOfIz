using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public static partial class TextureResource
    {
        public static Format GetFormat(Texture? texture)
        {
            Format outFormat = default;
            Methods.DenOfIz_TextureResource_GetFormat(texture, out outFormat);
            return outFormat;
        }

        public static void Destroy(Texture? texture)
        {
            Methods.DenOfIz_TextureResource_Destroy(texture);
        }

    }
}
