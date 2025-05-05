using System;
using System.IO;
using System.Runtime.InteropServices;

namespace DenOfIzGraphics
{
    /// <summary>
    /// Helper methods for platform-specific functionality
    /// </summary>
    public static class PlatformHelper
    {
        /// <summary>
        /// Gets whether the current platform is Windows
        /// </summary>
        public static bool IsWindows => RuntimeInformation.IsOSPlatform(OSPlatform.Windows);

        /// <summary>
        /// Gets whether the current platform is macOS
        /// </summary>
        public static bool IsMacOS => RuntimeInformation.IsOSPlatform(OSPlatform.OSX);

        /// <summary>
        /// Gets whether the current platform is Linux
        /// </summary>
        public static bool IsLinux => RuntimeInformation.IsOSPlatform(OSPlatform.Linux);

        /// <summary>
        /// Gets whether the current architecture is x64 (64-bit)
        /// </summary>
        public static bool IsX64 => RuntimeInformation.ProcessArchitecture == Architecture.X64;

        /// <summary>
        /// Gets whether the current architecture is x86 (32-bit)
        /// </summary>
        public static bool IsX86 => RuntimeInformation.ProcessArchitecture == Architecture.X86;

        /// <summary>
        /// Gets whether the current architecture is ARM64
        /// </summary>
        public static bool IsArm64 => RuntimeInformation.ProcessArchitecture == Architecture.Arm64;

        /// <summary>
        /// Gets the operating system name and version
        /// </summary>
        public static string OperatingSystem
        {
            get
            {
                if (IsWindows)
                {
                    return $"Windows {Environment.OSVersion.Version}";
                }
                else if (IsMacOS)
                {
                    return $"macOS {Environment.OSVersion.Version}";
                }
                else if (IsLinux)
                {
                    return $"Linux {Environment.OSVersion.Version}";
                }
                else
                {
                    return "Unknown";
                }
            }
        }

        /// <summary>
        /// Gets the preferred graphics API for the current platform based on the GraphicsApi structure
        /// </summary>
        public static string PreferredGraphicsApi
        {
            get
            {
                if (IsWindows)
                {
                    return "DirectX12";
                }
                else if (IsMacOS)
                {
                    return "Metal";
                }
                else
                {
                    return "Vulkan";
                }
            }
        }

        /// <summary>
        /// Gets the fallback graphics API for the current platform
        /// </summary>
        public static string FallbackGraphicsApi
        {
            get
            {
                if (IsWindows || IsLinux)
                {
                    return "Vulkan";
                }
                else if (IsMacOS)
                {
                    return "Metal";
                }
                else
                {
                    return "Unknown";
                }
            }
        }

        /// <summary>
        /// Gets the file extension for native libraries on the current platform
        /// </summary>
        public static string NativeLibraryExtension
        {
            get
            {
                if (IsWindows)
                    return ".dll";
                else if (IsLinux)
                    return ".so";
                else if (IsMacOS)
                    return ".dylib";
                else
                    return string.Empty;
            }
        }

        /// <summary>
        /// Gets the runtime identifier for the current platform
        /// </summary>
        public static string RuntimeIdentifier
        {
            get
            {
                string architecture = RuntimeInformation.ProcessArchitecture switch
                {
                    Architecture.X64 => "x64",
                    Architecture.X86 => "x86",
                    Architecture.Arm64 => "arm64",
                    _ => "unknown"
                };

                if (IsWindows)
                {
                    return $"win-{architecture}";
                }
                else if (IsLinux)
                {
                    return $"linux-{architecture}";
                }
                else if (IsMacOS)
                {
                    return $"osx-{architecture}";
                }
                else
                {
                    return "unknown";
                }
            }
        }

        /// <summary>
        /// Gets the path to native libraries in the NuGet package
        /// </summary>
        public static string GetNuGetNativeLibraryPath()
        {
            return Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory,
                "runtimes",
                RuntimeIdentifier,
                "native");
        }

        /// <summary>
        /// Creates a DenOfIzGraphics GraphicsApi instance with the most appropriate settings for the current platform
        /// </summary>
        public static DenOfIz.GraphicsApi CreateOptimalGraphicsApi()
        {
            var apiPreference = new DenOfIz.APIPreference();
            if (IsWindows)
            {
                apiPreference.Windows = DenOfIz.APIPreferenceWindows.DirectX12;
            }
            
            if (IsMacOS)
            {
                apiPreference.OSX = DenOfIz.APIPreferenceOSX.Metal;
            }
            
            return new DenOfIz.GraphicsApi(apiPreference);
        }
    }
}