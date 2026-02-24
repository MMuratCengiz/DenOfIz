using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public static partial class FSConfig
    {
        public static void InitDefaults()
        {
            Methods.DenOfIz_FSConfig_InitDefaults();
        }

        public static void Init(in FSDesc config)
        {
            Methods.DenOfIz_FSConfig_Init(in config);
        }

        public static StringView AssetPath()
        {
            return Methods.DenOfIz_FSConfig_AssetPath();
        }

        public static StringView BundleResourcePath()
        {
            return Methods.DenOfIz_FSConfig_BundleResourcePath();
        }

    }
}
