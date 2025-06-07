namespace CppSharp;

using System.Diagnostics;

public class Config
{
    public string InstallLocation { get; }
    public string LibraryFile { get; }
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
        var gitRoot = process.StandardOutput.ReadToEnd().Trim();
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
            InstallLocation = Path.Combine(installLocation, osDirectories["windows"], "bin");
            LibraryFile = Path.Combine(InstallLocation, osLibraries["windows"]);
            LibraryName = osLibraries["windows"];
        }
        else if (OperatingSystem.IsMacOS())
        {
            InstallLocation = Path.Combine(installLocation, osDirectories["osx"], "bin");
            LibraryFile = Path.Combine(InstallLocation, osLibraries["osx"]);
            LibraryName = osLibraries["osx"];
        }
        else if (OperatingSystem.IsLinux())
        {
            InstallLocation = Path.Combine(installLocation, osDirectories["linux"], "bin");
            LibraryFile = Path.Combine(InstallLocation, osLibraries["linux"]);
            LibraryName = osLibraries["linux"];
        }
        else
        {
            throw new Exception("Unsupported OS");
        }
        
        Includes = Path.Combine(installLocation, "include");
    }
}