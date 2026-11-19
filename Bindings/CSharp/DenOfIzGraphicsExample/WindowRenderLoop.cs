using DenOfIz;

namespace DenOfIz.Bindings.GraphicsExample;

public class WindowRenderLoop : IDisposable
{
    private readonly LogicalDevice _logicalDevice;
    private readonly Window _window;
    private readonly CommandQueue _commandQueue;
    private readonly SwapChain _swapChain;
    private readonly FrameSync _frameSync;
    private readonly ResourceTracking _resourceTracking;
    private readonly Game _game;
    private bool _disposed;

    public WindowRenderLoop(LogicalDevice logicalDevice, uint width, uint height, string title)
    {
        _logicalDevice = logicalDevice;

        _window = new Window(new WindowDesc
        {
            Width = (int)width,
            Height = (int)height,
            Title = StringView.Create(title),
            Flags = new WindowFlags { Resizable = true }
        });

        var commandQueueDesc = new CommandQueueDesc
        {
            QueueType = QueueType.Graphics
        };
        _commandQueue = _logicalDevice.CreateCommandQueue(commandQueueDesc);

        var swapChainDesc = new SwapChainDesc
        {
            PresentMode = PresentMode.Immediate,
            BackBufferFormat = Format.B8G8R8A8Unorm,
            DepthBufferFormat = Format.D32Float,
            CommandQueue = _commandQueue,
            WindowHandle = _window.GetGraphicsWindowHandle(),
            NumBuffers = 3
        };
        _swapChain = _logicalDevice.CreateSwapChain(swapChainDesc);

        _resourceTracking = new ResourceTracking();
        _frameSync = new FrameSync(new FrameSyncDesc
        {
            Device = _logicalDevice,
            CommandQueue = _commandQueue,
            SwapChain = _swapChain,
            NumFrames = 3,
            ResourceTracking = _resourceTracking
        });

        _window.Show();

        _game = new Game(_logicalDevice, _resourceTracking);
    }

    public bool PollAndRender()
    {
        while (InputSystem.PollEvent(out var ev))
        {
            switch (ev.Type)
            {
                case EventType.Quit:
                case EventType.KeyDown when ev.Key.KeyCode == KeyCode.Escape:
                    return false;
            }
        }

        var result = _frameSync.BeginFrame(out var frame);
        if (result == Result.ErrorDeviceLost)
        {
            return false;
        }

        if (result != Result.Success)
        {
            return true;
        }

        _game.Render(_swapChain.GetViewport(), _frameSync.CommandList!, _frameSync.BackBuffer!, frame.Index);

        return _frameSync.EndFrame() != Result.ErrorDeviceLost;
    }

    public void Dispose()
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;

        _frameSync.WaitIdle();
        _commandQueue.WaitIdle();

        _frameSync.Dispose();
        _swapChain.Dispose();
        _commandQueue.Dispose();
    }
}
