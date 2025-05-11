using System;
using System.Runtime.InteropServices;
using DenOfIz;

namespace DenOfIz
{
    public static class DenOfIzGraphicsInitializer
    {
        private static bool _initialized = false;
        private static readonly object _lock = new object();

        public static void Initialize()
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
                
                try
                {
                    NativeLibraryLoader.Initialize();
                    Engine.Init();
                    _initialized = true;
                    AppDomain.CurrentDomain.ProcessExit += OnProcessExit;
                }
                catch (Exception ex)
                {
                    Console.Error.WriteLine($"Failed to initialize DenOfIzGraphics: {ex.Message}");
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
            catch (Exception ex)
            {
                Console.Error.WriteLine($"Error during process exit cleanup: {ex.Message}");
            }
        }
    }
}