using System;
using System.Runtime.InteropServices;
using DenOfIz;

namespace DenOfIz
{
    public static class DenOfIzGraphicsInitializer
    {
        private static bool _initialized = false;
        private static readonly object _lock = new object();

        static DenOfIzGraphicsInitializer()
        {
            NativeLibraryLoader.Initialize();
        }


        public static void Initialize(EngineDesc engineDesc)
        {
            if (_initialized)
            {
                return;
            }
            lock (_lock)
            {
                if (_initialized)
                {
                    return;
                }
                NativeLibraryLoader.Initialize();
                Engine.Init(engineDesc);
                _initialized = true;
                AppDomain.CurrentDomain.ProcessExit += OnProcessExit;
            }
        }

        private static void OnProcessExit(object sender, EventArgs e)
        {
            GraphicsApi.ReportLiveObjects();
        }
    }
}