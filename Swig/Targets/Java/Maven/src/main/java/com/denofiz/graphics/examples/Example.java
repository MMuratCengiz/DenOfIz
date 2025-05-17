package com.denofiz.graphics.examples;

import com.denofiz.graphics.*;

/**
 * Renders a simple triangle
 */
public class Example {
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
    private FrameDebugRenderer frameDebugRenderer;
    private WindowSize windowSize = new WindowSize();
    private Time time = new Time();
    private boolean running = true;
    private DZConstPool constPool = new DZConstPool();

    public static void main(String[] args) {
        // Very important to ensure necessary libraries are loaded, java evaluates "new EngineDesc" first therefore we have to split this up like this
        DenOfIzRuntime.initializeRuntime();
        DenOfIzRuntime.initializeEngine(new EngineDesc());
        Example example = new Example();
        example.run();
    }

    private void run() {
        initializeWindow();
        initializeGraphics();
        createTrianglePipeline();
        createVertexBuffer();

        FrameDebugRendererDesc frameDebugRendererDesc = new FrameDebugRendererDesc();
        frameDebugRendererDesc.setGraphicsApi(graphicsApi);
        frameDebugRendererDesc.setLogicalDevice(device);
        frameDebugRendererDesc.setScale(0.6f);
        frameDebugRendererDesc.setScreenWidth((long)windowSize.getWidth());
        frameDebugRendererDesc.setScreenHeight((long)windowSize.getHeight());

        frameDebugRenderer = new FrameDebugRenderer(frameDebugRendererDesc);

        Event evt = new Event();

        while (running) {
            while (InputSystem.pollEvent(evt)) {
                if (evt.getType() == EventType.Quit) {
                    running = false;
                    break;
                }
                if (evt.getType() == EventType.WindowEvent) {
                    WindowSize possibleNewSize = window.getSize();
                    if (possibleNewSize.getWidth() != windowSize.getWidth() ||
                        possibleNewSize.getHeight() != windowSize.getHeight()) {

                        frameSync.waitIdle();
                        swapChain.resize(possibleNewSize.getWidth(), possibleNewSize.getHeight());
                        windowSize = possibleNewSize;

                        for (int i = 0; i < 3; i++) {
                            resourceTracking.trackTexture(swapChain.getRenderTarget(i), ResourceUsage.Common);
                        }
                    }
                }
                if (evt.getType() == EventType.KeyDown) {
                    if (evt.getKey().getKeycode() == KeyCode.Escape) {
                        running = false;
                    }
                    if (evt.getKey().getKeycode() == KeyCode.G) {
                        frameSync.waitIdle();
                        device.waitIdle();
                        graphicsQueue.waitIdle();
                        System.gc();
                    }
                }
            }

            renderFrame();
        }

        frameSync.waitIdle();
        graphicsQueue.waitIdle();
        device.waitIdle();
    }

    private void initializeWindow() {
        WindowDesc windowDesc = new WindowDesc();
        windowDesc.setTitle(new InteropString("Triangle Example"));
        windowDesc.setWidth(1920);
        windowDesc.setHeight(1080);

        WindowFlags flags = new WindowFlags();
        flags.setShown(true);
        flags.setResizable(true);
        windowDesc.setFlags(flags);
        windowDesc.setPosition(WindowPosition.Centered);

        window = new Window(windowDesc);
        windowHandle = window.getGraphicsWindowHandle();
        windowSize = window.getSize();
    }

    private void initializeGraphics() {
        APIPreference apiPreference = new APIPreference();
        apiPreference.setWindows(APIPreferenceWindows.DirectX12);
        apiPreference.setLinux(APIPreferenceLinux.Vulkan);
        apiPreference.setOSX(APIPreferenceOSX.Metal);

        graphicsApi = new GraphicsApi(apiPreference);
        device = graphicsApi.createAndLoadOptimalLogicalDevice();

        CommandQueueDesc queueDesc = new CommandQueueDesc();
        queueDesc.setQueueType(QueueType.Graphics);
        CommandQueueFlags flags = new CommandQueueFlags();
        flags.setRequirePresentationSupport(true);
        graphicsQueue = device.createCommandQueue(queueDesc);

        SwapChainDesc swapChainDesc = createSwapChain();

        FrameSyncDesc frameSyncDesc = new FrameSyncDesc();
        frameSyncDesc.setSwapChain(swapChain);
        frameSyncDesc.setDevice(device);
        frameSyncDesc.setNumFrames(3);
        frameSyncDesc.setCommandQueue(graphicsQueue);
        frameSync = new FrameSync(frameSyncDesc);

        // Used to track the current usage for each resource, helps with setting up barriers
        resourceTracking = new ResourceTracking();
        for (int i = 0; i < swapChainDesc.getNumBuffers(); i++) {
            resourceTracking.trackTexture(swapChain.getRenderTarget(i), ResourceUsage.Common);
        }
    }

    private SwapChainDesc createSwapChain() {
        SwapChainDesc swapChainDesc = new SwapChainDesc();
        swapChainDesc.setWidth(windowSize.getWidth());
        swapChainDesc.setHeight(windowSize.getHeight());
        swapChainDesc.setWindowHandle(windowHandle);
        swapChainDesc.setCommandQueue(graphicsQueue);
        swapChainDesc.setNumBuffers(3);
        swapChain = device.createSwapChain(swapChainDesc);
        return swapChainDesc;
    }

    private void createTrianglePipeline() {
        ShaderStageDesc vertexShaderDesc = new ShaderStageDesc();
        vertexShaderDesc.setStage(ShaderStage.Vertex);
        vertexShaderDesc.setEntryPoint(new InteropString("VSMain"));
        vertexShaderDesc.setData(getTriangleVertexShader());

        ShaderStageDesc pixelShaderDesc = new ShaderStageDesc();
        pixelShaderDesc.setStage(ShaderStage.Pixel);
        pixelShaderDesc.setEntryPoint(new InteropString("PSMain"));
        pixelShaderDesc.setData(getTrianglePixelShader());

        ShaderProgramDesc shaderProgramDesc = new ShaderProgramDesc();
        shaderProgramDesc.getShaderStages().addElement(vertexShaderDesc);
        shaderProgramDesc.getShaderStages().addElement(pixelShaderDesc);

        shaderProgram = new ShaderProgram(shaderProgramDesc);
        ShaderReflectDesc reflectDesc = shaderProgram.reflect();
        inputLayout = device.createInputLayout(reflectDesc.getInputLayout());
        rootSignature = device.createRootSignature(reflectDesc.getRootSignature());

        PipelineDesc pipelineDesc = new PipelineDesc();
        pipelineDesc.setInputLayout(inputLayout);
        pipelineDesc.setShaderProgram(shaderProgram);
        pipelineDesc.setRootSignature(rootSignature);
        RenderTargetDesc renderTargetDesc = new RenderTargetDesc();
        renderTargetDesc.setFormat(Format.B8G8R8A8Unorm);
        GraphicsPipelineDesc graphicsPipelineDesc = new GraphicsPipelineDesc();
        graphicsPipelineDesc.getRenderTargets().addElement(renderTargetDesc);
        pipelineDesc.setGraphics(graphicsPipelineDesc);

        trianglePipeline = device.createPipeline(pipelineDesc);
    }

    private void createVertexBuffer() {
        float[] vertices = new float[] {
            // Position (XYZ)    // Color (RGBA)
            0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Top vertex (red)
            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // Bottom left (green)
            0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f // Bottom right (blue)
        };

        long bufferSize = vertices.length * 4; // 4 bytes per float
        BufferDesc bufferDesc = new BufferDesc();
        bufferDesc.getDescriptor().set(ResourceDescriptor.VertexBuffer);
        bufferDesc.setNumBytes(bufferSize);
        bufferDesc.setDebugName(new InteropString("TriangleVertexBuffer"));

        vertexBuffer = device.createBufferResource(bufferDesc);

        BatchResourceCopy batchCopy = new BatchResourceCopy(device);
        batchCopy.begin();

        AlignedDataWriter byteArray = new AlignedDataWriter();
        for (int i = 0; i < vertices.length; i++) {
            byteArray.writeFloat(vertices[i]);
        }
        byteArray.flush();

        CopyToGpuBufferDesc copyDesc = new CopyToGpuBufferDesc();
        copyDesc.setDstBuffer(vertexBuffer);
        copyDesc.setData(byteArray.data());
        batchCopy.copyToGPUBuffer(copyDesc);
        batchCopy.submit();

        resourceTracking.trackBuffer(vertexBuffer, ResourceUsage.VertexAndConstantBuffer);
    }

    private void renderFrame() {
        time.tick();
        frameDebugRenderer.updateStats((float) time.getDeltaTime());

        long frameIndex = frameSync.nextFrame();
        ICommandList commandList = frameSync.getCommandList(frameIndex);

        commandList.begin();

        long nextImage = frameSync.acquireNextImage(frameIndex);
        ITextureResource renderTarget = swapChain.getRenderTarget((int)nextImage);

        final int frameIdx = (int)frameIndex;
        BatchTransitionDesc transitionDesc = constPool.newObject(3, frameIdx, () -> new BatchTransitionDesc(commandList), BatchTransitionDesc.class);
        transitionDesc.reset(commandList);
        transitionDesc.transitionTexture(renderTarget, ResourceUsage.RenderTarget);
        resourceTracking.batchTransition(transitionDesc);

        RenderingAttachmentDesc attachmentDesc = constPool.newObject(3, frameIdx, () -> new RenderingAttachmentDesc(), RenderingAttachmentDesc.class);
        attachmentDesc.setResource(renderTarget);

        RenderingDesc renderingDesc = constPool.newObject(3, frameIdx, () -> new RenderingDesc(), RenderingDesc.class);
        renderingDesc.clearRTAttachments();
        renderingDesc.addRTAttachment(attachmentDesc);

        commandList.beginRendering(renderingDesc);

        Viewport viewport = swapChain.getViewport();
        commandList.bindViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
        commandList.bindScissorRect(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
        commandList.bindPipeline(trianglePipeline);
        commandList.bindVertexBuffer(vertexBuffer);
        commandList.draw(3, 1, 0, 0);
        frameDebugRenderer.render(commandList);
        commandList.endRendering();

        transitionDesc.reset(commandList);
        transitionDesc.transitionTexture(renderTarget, ResourceUsage.Present);
        resourceTracking.batchTransition(transitionDesc);
        
        commandList.end();
        
        frameSync.executeCommandList(frameIndex);
        PresentResult presentResult = frameSync.present(nextImage);
        if (presentResult == PresentResult.Timeout || presentResult == PresentResult.DeviceLost) {
            frameSync.waitIdle();
            swapChain.resize(windowSize.getWidth(), windowSize.getHeight());
        }
    }
    
    private UnsignedCharArray getTriangleVertexShader() {
        String shaderCode = 
            "struct VSInput\n" +
            "{\n" +
            "    float3 Position : POSITION;\n" +
            "    float4 Color : COLOR;\n" +
            "};\n" +
            "\n" +
            "struct PSInput\n" +
            "{\n" +
            "    float4 Position : SV_POSITION;\n" +
            "    float4 Color : COLOR;\n" +
            "};\n" +
            "\n" +
            "PSInput VSMain(VSInput input)\n" +
            "{\n" +
            "    PSInput output;\n" +
            "    output.Position = float4(input.Position, 1.0);\n" +
            "    output.Color = input.Color;\n" +
            "    return output;\n" +
            "}";
        return InteropUtilities.stringToBytes(new InteropString(shaderCode));
    }
    
    private UnsignedCharArray getTrianglePixelShader() {
        String shaderCode = 
            "struct PSInput\n" +
            "{\n" +
            "    float4 Position : SV_POSITION;\n" +
            "    float4 Color : COLOR;\n" +
            "};\n" +
            "\n" +
            "float4 PSMain(PSInput input) : SV_TARGET\n" +
            "{\n" +
            "    return input.Color;\n" +
            "}";
        return InteropUtilities.stringToBytes(new InteropString(shaderCode));
    }
}