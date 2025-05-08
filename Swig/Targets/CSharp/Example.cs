// See https://aka.ms/new-console-template for more information

using DenOfIz;

// Todo this is quite crude, to be fleshed out before including it in docs
namespace DZDemo;

class Program
{
    private Window window;
    private GraphicsWindowHandle windowHandle;
    private GraphicsApi graphicsApi;
    private ILogicalDevice device;
    private ICommandQueue graphicsQueue;
    private ISwapChain swapChain;
    private FrameSync frameSync;
    private ResourceTracking resourceTracking;
    private IPipeline trianglePipeline;
    private IBufferResource vertexBuffer;
    private ShaderProgram shaderProgram;
    private IInputLayout inputLayout;
    private IRootSignature rootSignature;
    private bool running = true;

    // Custom event handler for quit events
    private class MyQuitCallback : QuitEventCallback
    {
        private Program program;

        public MyQuitCallback(Program program)
        {
            this.program = program;
        }

        public override void Execute(QuitEventData data)
        {
            program.running = false;
        }
    }

    // Custom event handler for keyboard events
    private class MyKeyboardCallback : KeyboardEventCallback
    {
        private Program program;

        public MyKeyboardCallback(Program program)
        {
            this.program = program;
        }

        public override void Execute(KeyboardEventData data)
        {
            if (data.Keycode == KeyCode.Escape && data.State == (uint)KeyState.Pressed)
            {
                program.running = false;
            }
        }
    }

    static void Main(string[] args)
    {
        // Initialize DenOfIzGraphics native libraries
        // DenOfIzGraphicsInitializer.Initialize();

        Program program = new Program();
        program.Run();
    }

    public void Run()
    {
        InitializeWindow();
        InitializeGraphics();
        CreateTrianglePipeline();
        CreateVertexBuffer();

        // Event handling
        EventHandler eventHandler = new EventHandler();
        eventHandler.SetOnQuit(new MyQuitCallback(this));
        eventHandler.SetOnKeyDown(new MyKeyboardCallback(this));

        // Main loop
        InputSystem inputSystem = new InputSystem();
        inputSystem.Initialize();
        Event evt = new Event();

        while (running)
        {
            // Process events
            while (InputSystem.PollEvent(evt))
            {
                eventHandler.ProcessEvent(evt);
            }

            // Render frame
            RenderFrame();
        }

        // Cleanup
        graphicsQueue.WaitIdle();
        device.WaitIdle();
    }

    private void InitializeWindow()
    {
        WindowProperties windowProps = new WindowProperties();
        windowProps.Title = new InteropString("Triangle Example");
        windowProps.Width = 800;
        windowProps.Height = 600;
        windowProps.Flags.Shown = true;
        windowProps.Flags.Resizable = true;

        window = new Window(windowProps);
        windowHandle = window.GetGraphicsWindowHandle();
    }

    private void InitializeGraphics()
    {
        // Set API preferences
        APIPreference apiPreference = new APIPreference();
        apiPreference.Windows = APIPreferenceWindows.DirectX12;
        apiPreference.Linux = APIPreferenceLinux.Vulkan;
        apiPreference.OSX = APIPreferenceOSX.Metal;

        // Create GraphicsApi and device
        graphicsApi = new GraphicsApi(apiPreference);
        device = graphicsApi.CreateAndLoadOptimalLogicalDevice();

        // Create command queue
        CommandQueueDesc queueDesc = new CommandQueueDesc();
        queueDesc.QueueType = QueueType.Graphics;
        queueDesc.Flags.RequirePresentationSupport = true;
        graphicsQueue = device.CreateCommandQueue(queueDesc);

        // Create swap chain
        SwapChainDesc swapChainDesc = new SwapChainDesc();
        swapChainDesc.Width = (uint)window.GetSize().Width;
        swapChainDesc.Height = (uint)window.GetSize().Height;
        swapChainDesc.WindowHandle = windowHandle;
        swapChainDesc.CommandQueue = graphicsQueue;
        swapChainDesc.NumBuffers = 3;
        swapChain = device.CreateSwapChain(swapChainDesc);

        // Create FrameSync
        FrameSyncDesc frameSyncDesc = new FrameSyncDesc();
        frameSyncDesc.SwapChain = swapChain;
        frameSyncDesc.Device = device;
        frameSyncDesc.NumFrames = 3;
        frameSyncDesc.CommandQueue = graphicsQueue;
        frameSync = new FrameSync(frameSyncDesc);

        // Create ResourceTracking
        resourceTracking = new ResourceTracking();

        // Track swap chain resources
        for (uint i = 0; i < swapChainDesc.NumBuffers; i++)
        {
            resourceTracking.TrackTexture(swapChain.GetRenderTarget(i), ResourceUsage.Common);
        }
    }

    private void CreateTrianglePipeline()
    {
        // Create shader stages
        ShaderStageDesc vertexShaderDesc = new ShaderStageDesc();
        vertexShaderDesc.Stage = ShaderStage.Vertex;
        vertexShaderDesc.EntryPoint = new InteropString("VSMain");

        var vertexShader = GetTriangleVertexShader();
        var pixelShader = GetTrianglePixelShader();
        vertexShaderDesc.Data.CopyBytes(vertexShader, vertexShader.Length);
        ;

        ShaderStageDesc pixelShaderDesc = new ShaderStageDesc();
        pixelShaderDesc.Stage = ShaderStage.Pixel;
        pixelShaderDesc.EntryPoint = new InteropString("PSMain");
        pixelShaderDesc.Data.CopyBytes(pixelShader, pixelShader.Length);

        ShaderStageDescArray shaderStages = new ShaderStageDescArray();
        shaderStages.AddElement(vertexShaderDesc);
        shaderStages.AddElement(pixelShaderDesc);

        ShaderProgramDesc shaderProgramDesc = new ShaderProgramDesc();
        shaderProgramDesc.ShaderStages = shaderStages;

        shaderProgram = new ShaderProgram(shaderProgramDesc);
        var reflectDesc = shaderProgram.Reflect();
        inputLayout = device.CreateInputLayout(reflectDesc.InputLayout);
        rootSignature = device.CreateRootSignature(reflectDesc.RootSignature);

        // Create graphics pipeline
        PipelineDesc pipelineDesc = new PipelineDesc();
        pipelineDesc.InputLayout = inputLayout;
        pipelineDesc.ShaderProgram = shaderProgram;
        pipelineDesc.RootSignature = rootSignature;
        pipelineDesc.Graphics.PrimitiveTopology = PrimitiveTopology.Triangle;
        pipelineDesc.Graphics.RenderTargets.AddElement(new RenderTargetDesc { Format = Format.R8G8B8A8Unorm });

        trianglePipeline = device.CreatePipeline(pipelineDesc);
    }

    private void CreateVertexBuffer()
    {
        // Define triangle vertices (position + color)
        float[] vertices = new float[]
        {
            // Position (XYZ)    // Color (RGBA)
            0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Top vertex (red)
            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // Bottom left (green)
            0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f // Bottom right (blue)
        };

        // Create buffer
        uint bufferSize = (uint)(vertices.Length * sizeof(float));
        BufferDesc bufferDesc = new BufferDesc();
        bufferDesc.Descriptor.Set(ResourceDescriptor.VertexBuffer);
        bufferDesc.NumBytes = bufferSize;
        bufferDesc.DebugName = new InteropString("TriangleVertexBuffer");

        vertexBuffer = device.CreateBufferResource(bufferDesc);

        // Copy vertex data
        BatchResourceCopy batchCopy = new BatchResourceCopy(device);
        batchCopy.Begin();


        // Convert vertices to byte array
        byte[] vertexBytes = new byte[bufferSize];
        Buffer.BlockCopy(vertices, 0, vertexBytes, 0, (int)bufferSize);

        CopyToGpuBufferDesc copyDesc = new CopyToGpuBufferDesc();
        copyDesc.DstBuffer = vertexBuffer;
        copyDesc.Data.CopyBytes(vertexBytes, vertexBytes.Length);
        batchCopy.CopyToGPUBuffer(copyDesc);
        batchCopy.Submit();

        // Track resource
        resourceTracking.TrackBuffer(vertexBuffer, ResourceUsage.VertexAndConstantBuffer);
    }

    private void RenderFrame()
    {
        // Get frame index and command list
        ulong frameIndex = frameSync.NextFrame();
        ICommandList commandList = frameSync.GetCommandList(frameIndex);

        // Begin command list
        commandList.Begin();

        // Get current render target
        ITextureResource renderTarget = swapChain.GetRenderTarget(frameSync.AcquireNextImage(frameIndex));

        // Transition render target to render target state
        BatchTransitionDesc transitionDesc = new BatchTransitionDesc(commandList);
        transitionDesc.TransitionTexture(renderTarget, ResourceUsage.RenderTarget);
        resourceTracking.BatchTransition(transitionDesc);

        // Begin rendering
        RenderingAttachmentDesc attachmentDesc = new RenderingAttachmentDesc();
        attachmentDesc.Resource = renderTarget;

        RenderingDesc renderingDesc = new RenderingDesc();
        renderingDesc.RTAttachments.AddElement(attachmentDesc);

        commandList.BeginRendering(renderingDesc);

        // Set viewport and scissor
        Viewport viewport = swapChain.GetViewport();
        commandList.BindViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
        commandList.BindScissorRect(viewport.X, viewport.Y, viewport.Width, viewport.Height);

        // Bind pipeline
        commandList.BindPipeline(trianglePipeline);
        commandList.BindVertexBuffer(vertexBuffer);
        // Draw triangle
        commandList.Draw(3, 1, 0, 0);

        // End rendering
        commandList.EndRendering();

        // Transition render target to present state
        transitionDesc = new BatchTransitionDesc(commandList);
        transitionDesc.TransitionTexture(renderTarget, ResourceUsage.Present);
        resourceTracking.BatchTransition(transitionDesc);

        // End command list
        commandList.End();

        // Execute command list and present
        frameSync.ExecuteCommandList(frameIndex);
        frameSync.Present((uint)frameIndex); // todo casting shouldn't be necessary
    }

    // Helper method to get triangle vertex shader
    private byte[] GetTriangleVertexShader()
    {
        string shaderCode = @"
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
        }";

        return System.Text.Encoding.UTF8.GetBytes(shaderCode);
    }

    // Helper method to get triangle pixel shader
    private byte[] GetTrianglePixelShader()
    {
        // Simple pixel shader for triangle
        string shaderCode = @"
            struct PSInput
            {
                float4 Position : SV_POSITION;
                float4 Color : COLOR;
            };

            float4 PSMain(PSInput input) : SV_TARGET
            {
                return input.Color;
            }";
        return System.Text.Encoding.UTF8.GetBytes(shaderCode);
    }
}