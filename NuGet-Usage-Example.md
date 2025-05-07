# Using DenOfIzGraphics NuGet Package

This document provides instructions on how to use the DenOfIzGraphics NuGet package in your .NET projects.

## Creating and Publishing the NuGet Package

### Windows

1. Build the DenOfIzGraphics project first:
   ```powershell
   cmake -S . -B build -DBUILD_EXAMPLES=ON
   cmake --build build --config Debug_MSVC
   ```

2. Run the NuGet packaging script:
   ```powershell
   .\pack-nuget.ps1 Debug_MSVC
   ```

3. The NuGet package will be created in the `NuGet/nupkg` directory.

### Linux/macOS

1. Build the DenOfIzGraphics project first:
   ```bash
   cmake -S . -B build -DBUILD_EXAMPLES=ON
   cmake --build build --config Debug_MSVC
   ```

2. Run the NuGet packaging script:
   ```bash
   bash ./pack-nuget.sh Debug_MSVC
   ```

3. The NuGet package will be created in the `NuGet/nupkg` directory.

## Using the NuGet Package in a .NET Project

### Install from Local Source

1. Add the local NuGet package to your project:
   ```bash
   dotnet add package DenOfIzGraphics --source /path/to/DenOfIz/NuGet/nupkg
   ```

2. Or using Visual Studio:
   - Go to Project > Manage NuGet Packages
   - Click the settings icon
   - Add a new package source pointing to your `/path/to/DenOfIz/NuGet/nupkg` directory
   - Install the package from your local source

### Example Usage

Here's a simple C# example to initialize the graphics library:

```csharp
using System;
using DenOfIzGraphics;

namespace DenOfIzExample
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                // Initialize native libraries - this is done automatically via static constructor
                // but you can explicitly call it if needed
                DenOfIz.NativeLibrary.LoadLibraries();
                
                Console.WriteLine($"Operating System: {PlatformHelper.OperatingSystem}");
                Console.WriteLine($"Preferred Graphics API: {PlatformHelper.PreferredGraphicsApi}");
                
                // Create optimal API for this platform
                var api = PlatformHelper.CreateOptimalGraphicsApi();
                
                // Create a window
                var windowProps = new WindowProperties
                {
                    Title = "DenOfIz Example",
                    Width = 800,
                    Height = 600,
                    WindowFlags = WindowFlags.Resizable | WindowFlags.Shown
                };
                
                var window = new Window(windowProps);
                
                // Create input system
                var inputSystem = new InputSystem();
                
                // Create event handler
                var eventHandler = new EventHandler();
                
                // Add quit handler
                eventHandler.RegisterQuitCallback(() =>
                {
                    Console.WriteLine("Quit event received!");
                    return true; // Return true to allow quit
                });
                
                // Main loop (simplified)
                bool running = true;
                while (running)
                {
                    // Poll events
                    var eventResult = eventHandler.PollEvent(inputSystem);
                    
                    if (!eventResult)
                    {
                        running = false;
                    }
                    
                    // Your rendering code would go here
                    
                    // Present the frame
                    // swapChain.Present();
                }
                
                Console.WriteLine("Clean exit from DenOfIzGraphics example");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
                Console.WriteLine(ex.StackTrace);
            }
        }
    }
}
```

## Troubleshooting

### Missing Native Libraries

If you get a `DllNotFoundException` or similar when running your application, check that:

1. The native libraries are properly copied to your output directory
2. You're running on a supported platform (Windows x64, Linux x64, or macOS)
3. Your platform has the required graphics API (DirectX 12, Vulkan, or Metal)

### Platform Specific Issues

#### Windows
- Make sure you have DirectX 12 compatible hardware and drivers
- For Vulkan backend, ensure you have Vulkan runtime installed

#### Linux
- Install the Vulkan development libraries: `sudo apt install libvulkan-dev` (Ubuntu/Debian)
- Make sure your graphics drivers support Vulkan

#### macOS
- Metal is only supported on macOS 10.13 or newer
- Make sure you're using a compatible GPU

### Debugging Native Library Loading

To debug native library loading issues, you can add more detailed logging:

```csharp
// Add this before initializing DenOfIzGraphics
AppDomain.CurrentDomain.AssemblyResolve += (sender, args) =>
{
    Console.WriteLine($"Attempting to resolve: {args.Name}");
    return null;
};

try 
{
    DenOfIz.NativeLibrary.LoadLibraries();
    Console.WriteLine("Libraries loaded successfully!");
}
catch (Exception ex)
{
    Console.WriteLine($"Failed to load libraries: {ex.Message}");
    Console.WriteLine(ex.StackTrace);
}
```

## Creating a Cross-Platform Package

To create a truly cross-platform package, you need to:

1. Build on Windows to include Windows native libraries
2. Build on Linux to include Linux native libraries
3. Build on macOS to include macOS native libraries

Then, combine the platform-specific libraries into a single NuGet package. You can do this by:

1. Copying the Windows-generated `runtimes/win-x64` folder to a central location
2. Copying the Linux-generated `runtimes/linux-x64` folder to the same location
3. Copying the macOS-generated `runtimes/osx-x64` and `runtimes/osx-arm64` folders to the same location
4. Running the packaging script on any platform with the combined runtimes

## Advanced: Publishing the NuGet Package

If you want to publish the package to NuGet.org:

1. Create an account on [NuGet.org](https://www.nuget.org/)
2. Get your API key
3. Publish using:
   ```bash
   dotnet nuget push /path/to/DenOfIzGraphics.1.0.0.nupkg --api-key YOUR_API_KEY --source https://api.nuget.org/v3/index.json
   ```