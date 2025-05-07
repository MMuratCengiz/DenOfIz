using System;

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
                    _initialized = true;
                }
                catch (Exception ex)
                {
                    throw new InvalidOperationException($"Failed to initialize DenOfIzGraphics: {ex.Message}", ex);
                }
            }
        }
    }
}