using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public static partial class Engine
    {
        public static void Init(in EngineDesc desc)
        {
            Methods.DenOfIz_Engine_Init(in desc);
        }

        public static void Shutdown()
        {
            Methods.DenOfIz_Engine_Shutdown();
        }

    }
}
