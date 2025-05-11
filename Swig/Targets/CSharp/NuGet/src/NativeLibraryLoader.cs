using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace DenOfIz
{
    internal static class NativeLibraryLoader
    {
        private static readonly Dictionary<string, IntPtr> LoadedLibraries = new Dictionary<string, IntPtr>();
        private static readonly object LoadLock = new object();
        private static bool _initialized;

        static NativeLibraryLoader()
        {
            _initialized = false;
        }

        public static void Initialize()
        {
            if (_initialized) return;

            lock (LoadLock)
            {
                if (_initialized) return;

                try
                {
                    string nativePath = GetRuntimeFolder();
                    LoadAllNativeLibraries(nativePath);
                    _initialized = true;
                }
                catch (Exception)
                {
                    throw;
                }
            }
        }

        private static string GetRuntimeFolder()
        {
            string assemblyDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            string platform = GetPlatformRid();
            string architecture = GetArchitecture();
            
            string runtimePath = Path.Combine(assemblyDir, "runtimes", $"{platform}-{architecture}", "native");
            if (Directory.Exists(runtimePath))
            {
                return runtimePath;
            }
            
            // For Apple Silicon, try x64 as fallback since that's what we build in CMake
            if (RuntimeInformation.ProcessArchitecture == Architecture.Arm64 && 
                RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                string fallbackPath = Path.Combine(assemblyDir, "runtimes", $"{platform}-x64", "native");
                if (Directory.Exists(fallbackPath))
                {
                    return fallbackPath;
                }
            }
            
            return Path.Combine(AppContext.BaseDirectory, "runtimes", $"{platform}-{architecture}", "native");
        }

        private static void LoadAllNativeLibraries(string directory)
        {
            if (!Directory.Exists(directory)) return;
            
            string extension = GetPlatformLibraryExtension();
            string[] libraryFiles = Directory.GetFiles(directory, $"*{extension}");
            
            string coreLibPrefix = RuntimeInformation.IsOSPlatform(OSPlatform.Windows) 
                ? "DenOfIzGraphicsCSharp" 
                : "libDenOfIzGraphicsCSharp";
                
            // First load the core library
            foreach (string file in libraryFiles)
            {
                string filename = Path.GetFileName(file);
                if (filename.StartsWith(coreLibPrefix))
                {
                    LoadLibrary(file);
                    break;
                }
            }
            
            // Then load dependencies
            foreach (string file in libraryFiles)
            {
                string filename = Path.GetFileName(file);
                if (!filename.StartsWith(coreLibPrefix))
                {
                    LoadLibrary(file);
                }
            }
        }

        private static string GetArchitecture()
        {
            switch (RuntimeInformation.ProcessArchitecture)
            {
                case Architecture.X64:
                    return "x64";
                case Architecture.X86:
                    return "x86";
                case Architecture.Arm64:
                    return "arm64";
                case Architecture.Arm:
                    return "arm";
                default:
                    return "unknown";
            }
        }

        private static string GetPlatformRid()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return "win";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return "osx";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                return "linux";
            }
            else
            {
                return "unknown";
            }
        }

        private static string GetPlatformLibraryExtension()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return ".dll";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return ".dylib";
            }
            else
            {
                return ".so";
            }
        }

        private static IntPtr LoadLibrary(string path)
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return WindowsLoadLibrary(path);
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return MacOSLoadLibrary(path);
            }
            else
            {
                return LinuxLoadLibrary(path);
            }
        }
        
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool SetDllDirectory(string lpPathName);

        [DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern IntPtr LoadLibraryW(string lpFileName);

        private static IntPtr WindowsLoadLibrary(string path)
        {
            return LoadLibraryW(path);
        }

        [DllImport("libdl.so.2", EntryPoint = "dlopen", SetLastError = true)]
        private static extern IntPtr LinuxDlopen(string fileName, int flags);

        private const int RTLD_NOW = 0x2;
        private const int RTLD_GLOBAL = 0x100;

        private static IntPtr LinuxLoadLibrary(string path)
        {
            return LinuxDlopen(path, RTLD_NOW | RTLD_GLOBAL);
        }

        [DllImport("libSystem.dylib", EntryPoint = "dlopen")]
        private static extern IntPtr MacOSDlopen(string fileName, int flags);

        private static IntPtr MacOSLoadLibrary(string path)
        {
            return MacOSDlopen(path, RTLD_NOW | RTLD_GLOBAL);
        }
    }
}