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

        public static void Initialize()
        {
            if (_initialized) return;
            
            lock (_lock)
            {
                if (_initialized) return;
                
                try
                {
                    NativeLibraryLoader.Initialize();
                    Engine.Init();
                    _initialized = true;
                    AppDomain.CurrentDomain.ProcessExit += OnProcessExit;
                }
                catch
                {
                    throw;
                }
            }
        }

        private static void OnProcessExit(object sender, EventArgs e)
        {
            try
            {
                GraphicsApi.ReportLiveObjects();
            }
            catch
            {
                // Ignore errors during cleanup
            }
        }
    }
}