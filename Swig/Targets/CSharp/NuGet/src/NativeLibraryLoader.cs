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
            if (_initialized)
            {
                return;
            }

            lock (LoadLock)
            {
                if (_initialized)
                {
                    return;
                }

                try
                {
                    string nativePath = GetNativeLibraryPath();
                    if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                    {
                        SetDllDirectoryWindows(nativePath);
                    }
                    LoadAllNativeLibraries(nativePath);
                    _initialized = true;
                }
                catch (Exception ex)
                {
                    Console.Error.WriteLine($"Error initializing native libraries: {ex.Message}");
                    throw;
                }
            }
        }

        private static string GetNativeLibraryPath()
        {
            string assemblyDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            string platform = GetPlatformRid();
            string architecture = GetArchitecture();
            
            string runtimePath = Path.Combine(assemblyDir, "runtimes", $"{platform}-{architecture}", "native");
            if (Directory.Exists(runtimePath))
            {
                return runtimePath;
            }

            runtimePath = Path.Combine(AppContext.BaseDirectory, "runtimes", $"{platform}-{architecture}", "native");
            if (Directory.Exists(runtimePath))
            {
                return runtimePath;
            }
            
            return AppContext.BaseDirectory;
        }

        private static void LoadAllNativeLibraries(string directory)
        {
            if (!Directory.Exists(directory))
            {
                throw new DirectoryNotFoundException($"Native library directory not found: {directory}");
            }
            
            string extension = GetPlatformLibraryExtension();
            string[] libraryFiles = Directory.GetFiles(directory, $"*{extension}");
            
            string coreLibPrefix = RuntimeInformation.IsOSPlatform(OSPlatform.Windows) ?
                "DenOfIzGraphicsCSharp" : "libDenOfIzGraphicsCSharp";
                
            foreach (string file in libraryFiles)
            {
                string filename = Path.GetFileName(file);
                if (filename.StartsWith(coreLibPrefix))
                {
                    LoadLibrary(file);
                    break;
                }
            }
            
            // Now load all remaining libraries
            foreach (string file in libraryFiles)
            {
                string filename = Path.GetFileName(file);
                if (!filename.StartsWith(coreLibPrefix))
                {
                    LoadLibrary(file);
                }
            }
        }
        
        private static void SetDllDirectoryWindows(string pathName)
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                if (!SetDllDirectory(pathName))
                {
                    int errorCode = Marshal.GetLastWin32Error();
                    throw new InvalidOperationException($"Failed to set DLL directory. Error code: {errorCode}");
                }
            }
        }

        private static IntPtr LoadNativeLibrary(string name)
        {
            lock (LoadLock)
            {
                if (LoadedLibraries.TryGetValue(name, out IntPtr handle) && handle != IntPtr.Zero)
                {
                    return handle;
                }
                
                string libraryName = GetPlatformLibraryFileName(name);
                string architecture = GetArchitecture();
                string platform = GetPlatformRid();

                string runtimesPath = Path.Combine(
                    GetAssemblyDirectory(),
                    "runtimes",
                    $"{platform}-{architecture}",
                    "native",
                    libraryName);

                if (File.Exists(runtimesPath))
                {
                    handle = LoadLibrary(runtimesPath);
                    if (handle != IntPtr.Zero)
                    {
                        LoadedLibraries[name] = handle;
                        return handle;
                    }
                }

                string appDirPath = Path.Combine(
                    AppContext.BaseDirectory,
                    libraryName);

                if (File.Exists(appDirPath))
                {
                    handle = LoadLibrary(appDirPath);
                    if (handle != IntPtr.Zero)
                    {
                        LoadedLibraries[name] = handle;
                        return handle;
                    }
                }

                // As a last resort, try loading by name (relying on the system's library search path)
                handle = LoadLibrary(libraryName);
                if (handle != IntPtr.Zero)
                {
                    LoadedLibraries[name] = handle;
                    return handle;
                }

                throw new DllNotFoundException($"Failed to load native library: {name}");
            }
        }

        private static string GetAssemblyDirectory()
        {
            string codeBase = Assembly.GetExecutingAssembly().Location;
            return Path.GetDirectoryName(codeBase);
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

        private static string GetArchitecture()
        {
            switch (RuntimeInformation.ProcessArchitecture)
            {
                case Architecture.X64:
                {
                    return "x64";
                }
                case Architecture.X86:
                {
                    return "x86";
                }
                case Architecture.Arm64:
                {
                    return "arm64";
                }
                case Architecture.Arm:
                {
                    return "arm";
                }
                default:
                {
                    return "unknown";
                }
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

        private static string GetPlatformLibraryFileName(string name)
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return $"{name}.dll";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return $"lib{name}.dylib";
            }
            else
            {
                return $"lib{name}.so";
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

        private const int RTLD_NOW = 2;

        private static IntPtr LinuxLoadLibrary(string path)
        {
            IntPtr handle = LinuxDlopen(path, RTLD_NOW);
            if (handle == IntPtr.Zero)
            {
                Console.Error.WriteLine($"Failed to load Linux library: {path}");
                // Try with libdl.so as a fallback
                handle = DlopenFallback(path, RTLD_NOW);
            }
            return handle;
        }

        [DllImport("libdl.so", EntryPoint = "dlopen", SetLastError = true)]
        private static extern IntPtr DlopenFallback(string fileName, int flags);

        [DllImport("libSystem.dylib", EntryPoint = "dlopen", SetLastError = true)]
        private static extern IntPtr MacOSDlopen(string fileName, int flags);

        private static IntPtr MacOSLoadLibrary(string path)
        {
            return MacOSDlopen(path, RTLD_NOW);
        }
    }
}