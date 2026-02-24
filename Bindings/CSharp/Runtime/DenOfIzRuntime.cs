using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;

namespace DenOfIz
{
    public static class DenOfIzRuntime
    {
        private static readonly object SyncRoot = new();
        private static bool isInitialised;
        private static string? runtimeOverride;
        private static string? activeRuntimeDirectory;

        private static readonly string NuGetRuntimeIdentifier;
        private static readonly string DevelopmentPlatformIdentifier;
        private static readonly string DevelopmentBinaryDirectory;
        private static readonly string PrimaryLibraryName;

        static DenOfIzRuntime()
        {
            (NuGetRuntimeIdentifier, DevelopmentPlatformIdentifier, DevelopmentBinaryDirectory, PrimaryLibraryName) = DeterminePlatform();
        }

        public static void Initialize()
        {
            if ( isInitialised )
            {
                return;
            }

            lock ( SyncRoot )
            {
                if ( isInitialised )
                {
                    return;
                }

#if NET6_0_OR_GREATER
                if ( OperatingSystem.IsBrowser() )
                {
                    isInitialised = true;
                    return;
                }
#endif

                NativeLibrary.SetDllImportResolver( typeof( DenOfIzRuntime ).Assembly, ResolveLibrary );

                IReadOnlyList<string> candidates;
                string runtimeDirectory = ResolveRuntimeDirectory( out candidates );

                if ( !Directory.Exists( runtimeDirectory ) )
                {
                    string candidateList = string.Join( Environment.NewLine, candidates.Select( path => " - " + path ) );
                    throw new DirectoryNotFoundException(
                        $"Den Of Iz runtime directory '{runtimeDirectory}' was not found." + Environment.NewLine
                        + "Checked locations:" + Environment.NewLine + candidateList );
                }

                activeRuntimeDirectory = runtimeDirectory;

                LoadPrimaryLibrary( runtimeDirectory );
                isInitialised = true;
            }
        }

        public static void SetRuntimeDirectory( string? directory )
        {
            lock ( SyncRoot )
            {
                if ( isInitialised )
                {
                    throw new InvalidOperationException( "Cannot change runtime directory after the native runtime has been initialised." );
                }

                runtimeOverride        = string.IsNullOrWhiteSpace( directory ) ? null : Path.GetFullPath( directory );
                activeRuntimeDirectory = null;
            }
        }

        public static string RuntimeDirectory => activeRuntimeDirectory
            ?? runtimeOverride
            ?? GetDefaultRuntimeDirectories().FirstOrDefault( Directory.Exists )
            ?? GetDefaultRuntimeDirectories().First();

        public static string Platform => NuGetRuntimeIdentifier;

        private static void LoadPrimaryLibrary( string runtimeDirectory )
        {
            string? primary = ResolveLibraryPath( PrimaryLibraryName, runtimeDirectory );
            if ( primary == null )
            {
                throw new DllNotFoundException( $"Failed to locate Den Of Iz native runtime library '{PrimaryLibraryName}' under '{runtimeDirectory}'." );
            }

            NativeLibrary.Load( primary );
        }

        private static IntPtr ResolveLibrary( string libraryName, Assembly assembly, DllImportSearchPath? searchPath )
        {
            string baseDirectory = activeRuntimeDirectory
                ?? runtimeOverride
                ?? GetDefaultRuntimeDirectories().FirstOrDefault( Directory.Exists )
                ?? GetDefaultRuntimeDirectories().First();

            string? candidate = ResolveLibraryPath( libraryName, baseDirectory );
            if ( candidate != null )
            {
                return NativeLibrary.Load( candidate, assembly, searchPath );
            }

            return IntPtr.Zero;
        }

        private static string? ResolveLibraryPath( string libraryName, string baseDirectory )
        {
            foreach ( string candidate in EnumerateCandidateNames( libraryName ) )
            {
                string path = Path.Combine( baseDirectory, candidate );
                if ( File.Exists( path ) )
                {
                    return path;
                }
            }

            return null;
        }

        private static IReadOnlyList<string> GetDefaultRuntimeDirectories()
        {
            var candidates = new List<string>();

            string? assemblyLocation = Path.GetDirectoryName( typeof( DenOfIzRuntime ).Assembly.Location );
            if ( !string.IsNullOrEmpty( assemblyLocation ) )
            {
                candidates.Add( Path.Combine( assemblyLocation, "runtimes", NuGetRuntimeIdentifier, "native" ) );
            }

            candidates.Add( Path.Combine( AppContext.BaseDirectory, "runtimes", NuGetRuntimeIdentifier, "native" ) );
            candidates.Add( Path.Combine( AppContext.BaseDirectory, NuGetRuntimeIdentifier ) );
            candidates.Add( AppContext.BaseDirectory );
            candidates.Add( Path.Combine( AppContext.BaseDirectory, "DenOfIzGraphics", DevelopmentPlatformIdentifier, DevelopmentBinaryDirectory ) );
            candidates.Add( Path.Combine( AppContext.BaseDirectory, "Native", "DenOfIzGraphics", DevelopmentPlatformIdentifier, DevelopmentBinaryDirectory ) );

            return candidates.Distinct( StringComparer.OrdinalIgnoreCase ).ToList();
        }

        private static string ResolveRuntimeDirectory( out IReadOnlyList<string> candidates )
        {
            if ( runtimeOverride != null )
            {
                candidates = new[] { runtimeOverride };
                return runtimeOverride;
            }

            candidates = GetDefaultRuntimeDirectories();

            foreach ( string candidate in candidates )
            {
                if ( Directory.Exists( candidate ) && ContainsNativeLibraries( candidate ) )
                {
                    return candidate;
                }
            }

            foreach ( string candidate in candidates )
            {
                if ( Directory.Exists( candidate ) )
                {
                    return candidate;
                }
            }

            return candidates.Count > 0
                ? candidates[ 0 ]
                : AppContext.BaseDirectory;
        }

        private static bool ContainsNativeLibraries( string directory )
        {
            if ( !Directory.Exists( directory ) )
            {
                return false;
            }

            string extension = RuntimeInformation.IsOSPlatform( OSPlatform.Windows ) ? "*.dll"
                : RuntimeInformation.IsOSPlatform( OSPlatform.OSX ) ? "*.dylib"
                : "*.so";

            return Directory.EnumerateFiles( directory, extension ).Any();
        }

        private static IEnumerable<string> EnumerateCandidateNames( string libraryName )
        {
            if ( string.IsNullOrWhiteSpace( libraryName ) )
            {
                yield break;
            }

            yield return libraryName;

            string baseName = Path.GetFileNameWithoutExtension( libraryName );

            if ( RuntimeInformation.IsOSPlatform( OSPlatform.Windows ) )
            {
                yield return $"{baseName}.dll";
            }
            else if ( RuntimeInformation.IsOSPlatform( OSPlatform.OSX ) )
            {
                string prefix = baseName.StartsWith( "lib", StringComparison.OrdinalIgnoreCase ) ? baseName : $"lib{baseName}";
                yield return $"{prefix}.dylib";
            }
            else
            {
                string prefix = baseName.StartsWith( "lib", StringComparison.OrdinalIgnoreCase ) ? baseName : $"lib{baseName}";
                yield return $"{prefix}.so";
            }
        }

        private static (string nugetRid, string devPlatform, string devBinaryFolder, string primaryLibrary) DeterminePlatform()
        {
#if NET6_0_OR_GREATER
            if ( OperatingSystem.IsBrowser() )
            {
                return ( "browser-wasm", "wasm", "", "__Internal" );
            }
#endif

            if ( RuntimeInformation.IsOSPlatform( OSPlatform.Windows ) )
            {
                Architecture architecture = RuntimeInformation.ProcessArchitecture;
                string archSegment        = architecture == Architecture.Arm64 ? "win-arm64" : "win-x64";
                return ( archSegment, "win32", "bin", "DenOfIzGraphics" );
            }

            if ( RuntimeInformation.IsOSPlatform( OSPlatform.OSX ) )
            {
                Architecture architecture = RuntimeInformation.ProcessArchitecture;
                string archSegment        = architecture == Architecture.Arm64 ? "osx-arm64" : "osx-x64";
                return ( archSegment, archSegment, "lib", "DenOfIzGraphics" );
            }

            Architecture linuxArch    = RuntimeInformation.ProcessArchitecture;
            string linuxArchSegment   = linuxArch == Architecture.Arm64 ? "linux-arm64" : "linux-x64";
            return ( linuxArchSegment, "linux", "lib", "DenOfIzGraphics" );
        }
    }
}
