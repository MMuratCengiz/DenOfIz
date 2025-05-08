// See https://aka.ms/new-console-template for more information

using DenOfIz;

namespace DZDemo;

internal class Program
{
    private Window _window = null!;
    private GraphicsWindowHandle _windowHandle = null!;
    private GraphicsApi _graphicsApi = null!;
    private ILogicalDevice _device = null!;
    private ICommandQueue _graphicsQueue = null!;
    private ISwapChain _swapChain = null!;
    private FrameSync _frameSync = null!;
    private ResourceTracking _resourceTracking = null!;
    private IPipeline _trianglePipeline = null!;
    private IBufferResource _vertexBuffer = null!;
    private ShaderProgram _shaderProgram = null!;
    private IInputLayout _inputLayout = null!;
    private IRootSignature _rootSignature = null!;
    private FrameDebugRenderer _frameDebugRenderer = null!;
    private WindowSize _windowSize = new();
    private readonly Time _time = new();
    private bool _running = true;

    private class MyQuitCallback(Program program) : QuitEventCallback
    {
        public override void Execute(QuitEventData data)
        {
            program._running = false;
        }
    }

    private class MyResizeCallback(Program program) : WindowEventCallback
    {
        public override void Execute(WindowEventData data)
        {
            var possibleNewSize = program._window.GetSize();
            if (possibleNewSize.Width == program._windowSize.Width &&
                possibleNewSize.Height == program._windowSize.Height)
            {
                return;
            }

            program._frameSync.WaitIdle();
            program._swapChain.Resize((uint)possibleNewSize.Width, (uint)possibleNewSize.Height);
            program._windowSize = possibleNewSize;
            for (uint i = 0; i < 3; i++)
            {
                program._resourceTracking.TrackTexture(program._swapChain.GetRenderTarget(i), ResourceUsage.Common);
            }
        }
    }

    private class MyKeyboardCallback(Program program) : KeyboardEventCallback
    {
        public override void Execute(KeyboardEventData data)
        {
            if (data is { Keycode: KeyCode.Escape, State: (uint)KeyState.Pressed })
            {
                program._running = false;
            }
        }
    }

    private static void Main()
    {
        // Very important to ensure necessary libraries are loaded(especially dxil and dxcompiler for windows)
        DenOfIzGraphicsInitializer.Initialize();

        var program = new Program();
        program.Run();
    }

    private void Run()
    {
        InitializeWindow();
        InitializeGraphics();
        CreateTrianglePipeline();
        CreateVertexBuffer();

        var frameDebugRendererDesc = new FrameDebugRendererDesc();
        frameDebugRendererDesc.GraphicsApi = _graphicsApi;
        frameDebugRendererDesc.LogicalDevice = _device;
        frameDebugRendererDesc.Scale = 0.6f;
        frameDebugRendererDesc.ScreenWidth = (uint)_window.GetSize().Width;
        frameDebugRendererDesc.ScreenHeight = (uint)_window.GetSize().Height;

        _frameDebugRenderer = new FrameDebugRenderer(frameDebugRendererDesc);
        var inputSystem = new InputSystem();
        inputSystem.Initialize();

        var eventHandler = new EventHandler(inputSystem);
        eventHandler.SetOnQuit(new MyQuitCallback(this));
        eventHandler.SetOnKeyDown(new MyKeyboardCallback(this));
        eventHandler.SetOnWindowEvent(new MyResizeCallback(this));

        var evt = new Event();

        while (_running)
        {
            while (InputSystem.PollEvent(evt))
            {
                eventHandler.ProcessEvent(evt);
            }

            RenderFrame();
        }

        // We always need to wait for ongoing operations to finish before exiting
        _frameSync.WaitIdle();
        _graphicsQueue.WaitIdle();
        _device.WaitIdle();
    }

    private void InitializeWindow()
    {
        var windowProps = new WindowProperties();
        windowProps.Title = new InteropString("Triangle Example");
        windowProps.Width = 1920;
        windowProps.Height = 1080;
        windowProps.Flags.Shown = true;
        windowProps.Flags.Resizable = true;
        windowProps.Position = WindowPosition.Centered;

        _window = new Window(windowProps);
        _windowHandle = _window.GetGraphicsWindowHandle();
        _windowSize = new WindowSize();
        _windowSize.Width = _window.GetSize().Width;
        _windowSize.Height = _window.GetSize().Height;
    }

    private void InitializeGraphics()
    {
        var apiPreference = new APIPreference();
        apiPreference.Windows = APIPreferenceWindows.DirectX12;
        apiPreference.Linux = APIPreferenceLinux.Vulkan;
        apiPreference.OSX = APIPreferenceOSX.Metal;

        _graphicsApi = new GraphicsApi(apiPreference);
        _device = _graphicsApi.CreateAndLoadOptimalLogicalDevice();

        var queueDesc = new CommandQueueDesc();
        queueDesc.QueueType = QueueType.Graphics;
        queueDesc.Flags.RequirePresentationSupport = true;
        _graphicsQueue = _device.CreateCommandQueue(queueDesc);

        var swapChainDesc = CreateSwapChain();

        var frameSyncDesc = new FrameSyncDesc();
        frameSyncDesc.SwapChain = _swapChain;
        frameSyncDesc.Device = _device;
        frameSyncDesc.NumFrames = 3;
        frameSyncDesc.CommandQueue = _graphicsQueue;
        _frameSync = new FrameSync(frameSyncDesc);

        // Used to track the current usage for each resource, helps with setting up barriers
        _resourceTracking = new ResourceTracking();
        for (uint i = 0; i < swapChainDesc.NumBuffers; i++)
        {
            _resourceTracking.TrackTexture(_swapChain.GetRenderTarget(i), ResourceUsage.Common);
        }
    }

    private SwapChainDesc CreateSwapChain()
    {
        var swapChainDesc = new SwapChainDesc();
        swapChainDesc.Width = (uint)_window.GetSize().Width;
        swapChainDesc.Height = (uint)_window.GetSize().Height;
        swapChainDesc.WindowHandle = _windowHandle;
        swapChainDesc.CommandQueue = _graphicsQueue;
        swapChainDesc.NumBuffers = 3;
        _swapChain = _device.CreateSwapChain(swapChainDesc);
        return swapChainDesc;
    }

    private void CreateTrianglePipeline()
    {
        var vertexShaderDesc = new ShaderStageDesc();
        vertexShaderDesc.Stage = ShaderStage.Vertex;
        vertexShaderDesc.EntryPoint = new InteropString("VSMain");
        vertexShaderDesc.Data = GetTriangleVertexShader();

        var pixelShaderDesc = new ShaderStageDesc();
        pixelShaderDesc.Stage = ShaderStage.Pixel;
        pixelShaderDesc.EntryPoint = new InteropString("PSMain");
        pixelShaderDesc.Data = GetTrianglePixelShader();

        var shaderProgramDesc = new ShaderProgramDesc();
        shaderProgramDesc.ShaderStages.AddElement(vertexShaderDesc);
        shaderProgramDesc.ShaderStages.AddElement(pixelShaderDesc);

        _shaderProgram = new ShaderProgram(shaderProgramDesc);
        var reflectDesc = _shaderProgram.Reflect();
        _inputLayout = _device.CreateInputLayout(reflectDesc.InputLayout);
        _rootSignature = _device.CreateRootSignature(reflectDesc.RootSignature);

        // Create graphics pipeline
        var pipelineDesc = new PipelineDesc();
        pipelineDesc.InputLayout = _inputLayout;
        pipelineDesc.ShaderProgram = _shaderProgram;
        pipelineDesc.RootSignature = _rootSignature;
        // pipelineDesc.Graphics.PrimitiveTopology = PrimitiveTopology.Triangle;
        pipelineDesc.Graphics.RenderTargets.AddElement(new RenderTargetDesc { Format = Format.B8G8R8A8Unorm });

        _trianglePipeline = _device.CreatePipeline(pipelineDesc);
    }

    private void CreateVertexBuffer()
    {
        var vertices = new[]
        {
            // Position (XYZ)    // Color (RGBA)
            0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Top vertex (red)
            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // Bottom left (green)
            0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f // Bottom right (blue)
        };

        var bufferSize = (uint)(vertices.Length * sizeof(float));
        var bufferDesc = new BufferDesc();
        bufferDesc.Descriptor.Set(ResourceDescriptor.VertexBuffer);
        bufferDesc.NumBytes = bufferSize;
        bufferDesc.DebugName = new InteropString("TriangleVertexBuffer");

        _vertexBuffer = _device.CreateBufferResource(bufferDesc);

        var batchCopy = new BatchResourceCopy(_device);
        batchCopy.Begin();

        var vertexBytes = new byte[bufferSize];
        Buffer.BlockCopy(vertices, 0, vertexBytes, 0, (int)bufferSize);

        var copyDesc = new CopyToGpuBufferDesc();
        copyDesc.DstBuffer = _vertexBuffer;
        copyDesc.Data.CopyBytes(vertexBytes, vertexBytes.Length);
        batchCopy.CopyToGPUBuffer(copyDesc);
        batchCopy.Submit();

        _resourceTracking.TrackBuffer(_vertexBuffer, ResourceUsage.VertexAndConstantBuffer);
    }

    private void RenderFrame()
    {
        _time.Tick();
        _frameDebugRenderer.UpdateStats((float)_time.GetDeltaTime());

        // Get frame index and command list
        var frameIndex = _frameSync.NextFrame();
        var commandList = _frameSync.GetCommandList(frameIndex);

        commandList.Begin();

        var nextImage = _frameSync.AcquireNextImage(frameIndex);
        var renderTarget = _swapChain.GetRenderTarget(nextImage);

        var transitionDesc = new BatchTransitionDesc(commandList);
        transitionDesc.TransitionTexture(renderTarget, ResourceUsage.RenderTarget);
        _resourceTracking.BatchTransition(transitionDesc);

        var attachmentDesc = new RenderingAttachmentDesc();
        attachmentDesc.Resource = renderTarget;

        var renderingDesc = new RenderingDesc();
        renderingDesc.RTAttachments.AddElement(attachmentDesc);

        commandList.BeginRendering(renderingDesc);

        var viewport = _swapChain.GetViewport();
        commandList.BindViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
        commandList.BindScissorRect(viewport.X, viewport.Y, viewport.Width, viewport.Height);
        commandList.BindPipeline(_trianglePipeline);
        commandList.BindVertexBuffer(_vertexBuffer);
        commandList.Draw(3, 1, 0, 0);
        _frameDebugRenderer.Render(commandList);
        commandList.EndRendering();

        transitionDesc = new BatchTransitionDesc(commandList);
        transitionDesc.TransitionTexture(renderTarget, ResourceUsage.Present);
        _resourceTracking.BatchTransition(transitionDesc);

        commandList.End();

        _frameSync.ExecuteCommandList(frameIndex);
        switch (_frameSync.Present(nextImage))
        {
            case PresentResult.Timeout:
            case PresentResult.DeviceLost:
                _frameSync.WaitIdle();
                _swapChain.Resize((uint)_window.GetSize().Width, (uint)_window.GetSize().Height);
                break;
            case PresentResult.Success:
            case PresentResult.Suboptimal:
            default:
                break;
        }
    }

    private static UnsignedCharArray GetTriangleVertexShader()
    {
        const string shaderCode =
            """
            struct VSInput
            {
                float3 Position : POSITION;
                float4 Color : COLOR;
            };

            struct PSInput
            {
                float4 Position : SV_POSITION;
                float4 Color : COLOR;
            };

            PSInput VSMain(VSInput input)
            {
                PSInput output;
                output.Position = float4(input.Position, 1.0);
                output.Color = input.Color;
                return output;
            }
            """;
        return InteropUtilities.StringToBytes(new InteropString(shaderCode));
    }

    private static UnsignedCharArray GetTrianglePixelShader()
    {
        const string shaderCode =
            """
            struct PSInput
            {
                float4 Position : SV_POSITION;
                float4 Color : COLOR;
            };

            float4 PSMain(PSInput input) : SV_TARGET
            {
                return input.Color;
            }
            """;
        return InteropUtilities.StringToBytes(new InteropString(shaderCode));
    }
}