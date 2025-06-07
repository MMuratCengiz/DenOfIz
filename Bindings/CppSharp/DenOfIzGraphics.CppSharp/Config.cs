namespace CppSharp;

using System.Diagnostics;

public class Config
{
    public string OutputDir { get; }
    public string InstallLocation { get; }
    public string LibraryDir { get; }
    public string LibraryName { get; }
    public string Includes { get; }

    public Config()
    {
        var process = new Process
        {
            StartInfo = new ProcessStartInfo
            {
                FileName = "git",
                Arguments = "rev-parse --show-toplevel",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                CreateNoWindow = true
            }
        };

        process.Start();
        var gitRoot = Path.GetFullPath(process.StandardOutput.ReadToEnd().Trim());
        process.WaitForExit();

        var installLocation = Path.Combine(gitRoot, "install", "DenOfIz");
        var osLibraries = new Dictionary<string, string>
        {
            { "windows", "DenOfIzGraphics.dll" },
            { "osx", "libDenOfIzGraphics.dylib" },
            { "linux", "libDenOfIzGraphics.so" }
        };
        var osDirectories = new Dictionary<string, string>
        {
            { "windows", "Install_Windows" },
            { "osx", "Install_OSX" },
            { "linux", "Install_Linux" }
        };

        if (OperatingSystem.IsWindows())
        {
            InstallLocation = Path.Combine(installLocation, osDirectories["windows"]);
            LibraryDir = Path.Combine(InstallLocation, "bin");
            LibraryName = osLibraries["windows"];
        }
        else if (OperatingSystem.IsMacOS())
        {
            InstallLocation = Path.Combine(installLocation, osDirectories["osx"]);
            LibraryDir = Path.Combine(InstallLocation, "lib");
            LibraryName = osLibraries["osx"];
        }
        else if (OperatingSystem.IsLinux())
        {
            InstallLocation = Path.Combine(installLocation, osDirectories["linux"]);
            LibraryDir = Path.Combine(InstallLocation, "lib");
            LibraryName = osLibraries["linux"];
        }
        else
        {
            throw new Exception("Unsupported OS");
        }
        
        OutputDir = Path.Combine(gitRoot, "Bindings", "CppSharp", "Out", "Code");
        Includes = Path.Combine(InstallLocation, "include");
    }
}