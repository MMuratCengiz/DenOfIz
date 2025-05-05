using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace DenOfIzGraphics
{
    /// <summary>
    /// Handles native library loading on different platforms
    /// </summary>
    internal static class NativeLibrary
    {
        private static readonly object LoadLock = new object();
        private static bool _librariesLoaded = false;
        private static readonly List<string> _loadedLibraries = new List<string>();

        // Constants for library names 
        private const string LibraryNameWin = "DenOfIzGraphicsCSharp.dll";
        private const string GraphicsLibraryNameWin = "DenOfIzGraphics.dll";
        private const string LibraryNameLinux = "libDenOfIzGraphicsCSharp.so";
        private const string GraphicsLibraryNameLinux = "libDenOfIzGraphics.so";
        private const string LibraryNameMac = "libDenOfIzGraphicsCSharp.dylib";
        private const string GraphicsLibraryNameMac = "libDenOfIzGraphics.dylib";

        // Windows imports
        [DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern IntPtr LoadLibraryW(string lpFileName);

        [DllImport("kernel32", SetLastError = true)]
        private static extern IntPtr GetModuleHandleW(string lpFileName);

        [DllImport("kernel32", SetLastError = true)]
        private static extern bool FreeLibrary(IntPtr hLibModule);

        // Linux imports
        [DllImport("libdl.so.2", EntryPoint = "dlopen")]
        private static extern IntPtr dlopen_linux(string filename, int flags);

        [DllImport("libdl.so.2", EntryPoint = "dlclose")]
        private static extern int dlclose_linux(IntPtr handle);

        // macOS imports
        [DllImport("libdl.dylib", EntryPoint = "dlopen")]
        private static extern IntPtr dlopen_macos(string filename, int flags);

        [DllImport("libdl.dylib", EntryPoint = "dlclose")]
        private static extern int dlclose_macos(IntPtr handle);

        // dlopen constants
        private const int RTLD_NOW = 2;

        /// <summary>
        /// Initializes static members of the NativeLibrary class.
        /// This is implicitly called when the class is first accessed.
        /// </summary>
        static NativeLibrary()
        {
            LoadLibraries();
        }

        /// <summary>
        /// Explicitly loads the native libraries
        /// </summary>
        public static void LoadLibraries()
        {
            if (_librariesLoaded)
            {
                return;
            }

            lock (LoadLock)
            {
                if (_librariesLoaded)
                {
                    return;
                }
                
                try
                {
                    LoadPlatformLibraries();
                    _librariesLoaded = true;
                }
                catch (Exception ex)
                {
                    Debug.WriteLine($"Failed to load native libraries: {ex.Message}");
                    throw new PlatformNotSupportedException("Failed to load DenOfIzGraphics native libraries. See inner exception for details.", ex);
                }
            }
        }

        /// <summary>
        /// Loads platform-specific native libraries
        /// </summary>
        private static void LoadPlatformLibraries()
        {
            string[] searchPaths = GetSearchPaths();

            // First load the main graphics library 
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                LoadWindowsLibrary(GraphicsLibraryNameWin, searchPaths);
                LoadWindowsLibrary(LibraryNameWin, searchPaths);
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                LoadLinuxLibrary(GraphicsLibraryNameLinux, searchPaths);
                LoadLinuxLibrary(LibraryNameLinux, searchPaths);
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                LoadMacOSLibrary(GraphicsLibraryNameMac, searchPaths);
                LoadMacOSLibrary(LibraryNameMac, searchPaths);
            }
            else
            {
                throw new PlatformNotSupportedException("Unsupported platform");
            }
        }

        /// <summary>
        /// Gets a list of directories to search for native libraries
        /// </summary>
        private static string[] GetSearchPaths()
        {
            var paths = new List<string>();
            
            // Base directory of the application
            paths.Add(AppDomain.CurrentDomain.BaseDirectory);
            
            // NuGet package runtime locations
            string runtimePath = Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory,
                "runtimes",
                GetRuntimeIdentifier(),
                "native");
            paths.Add(runtimePath);
            
            // Current directory
            paths.Add(Environment.CurrentDirectory);
            
            // Assembly location directory
            string assemblyLocation = Assembly.GetExecutingAssembly().Location;
            if (!string.IsNullOrEmpty(assemblyLocation))
            {
                paths.Add(Path.GetDirectoryName(assemblyLocation));
            }
            
            return paths.ToArray();
        }

        /// <summary>
        /// Loads a Windows native library
        /// </summary>
        private static void LoadWindowsLibrary(string libraryName, string[] searchPaths)
        {
            // Check if already loaded
            IntPtr libraryHandle = GetModuleHandleW(libraryName);
            if (libraryHandle != IntPtr.Zero)
            {
                _loadedLibraries.Add(libraryName);
                return;
            }

            // Try each search path
            foreach (string path in searchPaths)
            {
                string fullPath = Path.Combine(path, libraryName);
                if (File.Exists(fullPath))
                {
                    libraryHandle = LoadLibraryW(fullPath);
                    if (libraryHandle != IntPtr.Zero)
                    {
                        _loadedLibraries.Add(fullPath);
                        return;
                    }
                }
            }

            // Try loading by name only (relying on the system search path)
            libraryHandle = LoadLibraryW(libraryName);
            if (libraryHandle != IntPtr.Zero)
            {
                _loadedLibraries.Add(libraryName);
                return;
            }

            throw new DllNotFoundException($"Could not find or load library '{libraryName}'");
        }

        /// <summary>
        /// Loads a Linux native library
        /// </summary>
        private static void LoadLinuxLibrary(string libraryName, string[] searchPaths)
        {
            // Try each search path
            foreach (string path in searchPaths)
            {
                string fullPath = Path.Combine(path, libraryName);
                if (File.Exists(fullPath))
                {
                    IntPtr libraryHandle = dlopen_linux(fullPath, RTLD_NOW);
                    if (libraryHandle != IntPtr.Zero)
                    {
                        _loadedLibraries.Add(fullPath);
                        return;
                    }
                }
            }

            // Try loading by name only (relying on the system search path)
            IntPtr handle = dlopen_linux(libraryName, RTLD_NOW);
            if (handle != IntPtr.Zero)
            {
                _loadedLibraries.Add(libraryName);
                return;
            }

            throw new DllNotFoundException($"Could not find or load library '{libraryName}'");
        }

        /// <summary>
        /// Loads a macOS native library
        /// </summary>
        private static void LoadMacOSLibrary(string libraryName, string[] searchPaths)
        {
            // Try each search path
            foreach (string path in searchPaths)
            {
                string fullPath = Path.Combine(path, libraryName);
                if (File.Exists(fullPath))
                {
                    IntPtr libraryHandle = dlopen_macos(fullPath, RTLD_NOW);
                    if (libraryHandle != IntPtr.Zero)
                    {
                        _loadedLibraries.Add(fullPath);
                        return;
                    }
                }
            }

            // Try loading by name only (relying on the system search path)
            IntPtr handle = dlopen_macos(libraryName, RTLD_NOW);
            if (handle != IntPtr.Zero)
            {
                _loadedLibraries.Add(libraryName);
                return;
            }

            throw new DllNotFoundException($"Could not find or load library '{libraryName}'");
        }

        /// <summary>
        /// Gets the runtime identifier for the current platform
        /// </summary>
        private static string GetRuntimeIdentifier()
        {
            string architecture = RuntimeInformation.ProcessArchitecture switch
            {
                Architecture.X64 => "x64",
                Architecture.X86 => "x86",
                Architecture.Arm64 => "arm64",
                _ => throw new PlatformNotSupportedException("Unsupported architecture")
            };

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return $"win-{architecture}";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                return $"linux-{architecture}";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return $"osx-{architecture}";
            }
            else
            {
                throw new PlatformNotSupportedException("Unsupported platform");
            }
        }
    }
}