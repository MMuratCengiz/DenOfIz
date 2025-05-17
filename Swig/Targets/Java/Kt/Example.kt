import com.denofiz.graphics.*

/**
 * Renders a simple triangle
 */
class Example {
    private var window: Window? = null
    private var windowHandle: GraphicsWindowHandle? = null
    private var graphicsApi: GraphicsApi? = null
    private var device: ILogicalDevice? = null
    private var graphicsQueue: ICommandQueue? = null
    private var swapChain: ISwapChain? = null
    private var frameSync: FrameSync? = null
    private var resourceTracking: ResourceTracking? = null
    private var trianglePipeline: IPipeline? = null
    private var vertexBuffer: IBufferResource? = null
    private var shaderProgram: ShaderProgram? = null
    private var inputLayout: IInputLayout? = null
    private var rootSignature: IRootSignature? = null
    private var frameDebugRenderer: FrameDebugRenderer? = null
    private var windowSize = WindowSize()
    private val time = Time()
    private var running = true
    private val constPool = DZConstPool()

    fun run() {
        initializeWindow()
        initializeGraphics()
        createTrianglePipeline()
        createVertexBuffer()

        val frameDebugRendererDesc = FrameDebugRendererDesc()
        frameDebugRendererDesc.graphicsApi = graphicsApi
        frameDebugRendererDesc.logicalDevice = device
        frameDebugRendererDesc.scale = 0.6f
        frameDebugRendererDesc.screenWidth = windowSize.width.toLong()
        frameDebugRendererDesc.screenHeight = windowSize.height.toLong()

        frameDebugRenderer = FrameDebugRenderer(frameDebugRendererDesc)

        val evt = Event()
        while (running) {
            while (InputSystem.pollEvent(evt)) {
                if (evt.type == EventType.Quit) {
                    running = false
                    break
                }
                if (evt.type == EventType.WindowEvent) {
                    val possibleNewSize = window!!.size
                    if (possibleNewSize.width != windowSize.width ||
                        possibleNewSize.height != windowSize.height
                    ) {
                        frameSync!!.waitIdle()
                        swapChain!!.resize(possibleNewSize.width, possibleNewSize.height)
                        windowSize = possibleNewSize

                        for (i in 0..2) {
                            resourceTracking!!.trackTexture(swapChain!!.getRenderTarget(i), ResourceUsage.Common)
                        }
                    }
                }
                if (evt.type == EventType.KeyDown) {
                    if (evt.key.keycode == KeyCode.Escape) {
                        running = false
                    }
                    if (evt.key.keycode == KeyCode.G) {
                        frameSync?.waitIdle()
                        device?.waitIdle()
                        graphicsQueue?.waitIdle()
                        System.gc()
                    }
                }
            }

            renderFrame()
        }

        frameSync!!.waitIdle()
        graphicsQueue!!.waitIdle()
        device!!.waitIdle()
    }

    private fun initializeWindow() {
        val windowDesc = WindowDesc()
        windowDesc.title = InteropString("Triangle Example")
        windowDesc.width = 1920
        windowDesc.height = 1080

        val flags = WindowFlags()
        flags.shown = true
        flags.resizable = true
        windowDesc.flags = flags
        windowDesc.position = WindowPosition.Centered

        window = Window(windowDesc)
        windowHandle = window!!.graphicsWindowHandle
        windowSize = window!!.size
    }

    private fun initializeGraphics() {
        val apiPreference = APIPreference()
        apiPreference.windows = APIPreferenceWindows.DirectX12
        apiPreference.linux = APIPreferenceLinux.Vulkan
        apiPreference.osx = APIPreferenceOSX.Metal

        graphicsApi = GraphicsApi(apiPreference)
        device = graphicsApi!!.createAndLoadOptimalLogicalDevice()

        val queueDesc = CommandQueueDesc()
        queueDesc.queueType = QueueType.Graphics
        val flags = CommandQueueFlags()
        flags.requirePresentationSupport = true
        graphicsQueue = device!!.createCommandQueue(queueDesc)

        val swapChainDesc = createSwapChain()

        val frameSyncDesc = FrameSyncDesc()
        frameSyncDesc.swapChain = swapChain
        frameSyncDesc.device = device
        frameSyncDesc.numFrames = 3
        frameSyncDesc.commandQueue = graphicsQueue
        frameSync = FrameSync(frameSyncDesc)


        // Used to track the current usage for each resource, helps with setting up barriers
        resourceTracking = ResourceTracking()
        for (i in 0..<swapChainDesc.numBuffers) {
            resourceTracking!!.trackTexture(swapChain!!.getRenderTarget(i.toInt()), ResourceUsage.Common)
        }
    }

    private fun createSwapChain(): SwapChainDesc {
        val swapChainDesc = SwapChainDesc()
        swapChainDesc.width = windowSize.width.toLong()
        swapChainDesc.height = windowSize.height.toLong()
        swapChainDesc.windowHandle = windowHandle
        swapChainDesc.commandQueue = graphicsQueue
        swapChainDesc.numBuffers = 3
        swapChain = device!!.createSwapChain(swapChainDesc)
        return swapChainDesc
    }

    private fun createTrianglePipeline() {
        val vertexShaderDesc = ShaderStageDesc()
        vertexShaderDesc.stage = ShaderStage.Vertex
        vertexShaderDesc.entryPoint = InteropString("VSMain")
        vertexShaderDesc.data = this.triangleVertexShader

        val pixelShaderDesc = ShaderStageDesc()
        pixelShaderDesc.stage = ShaderStage.Pixel
        pixelShaderDesc.entryPoint = InteropString("PSMain")
        pixelShaderDesc.data = this.trianglePixelShader

        val shaderProgramDesc = ShaderProgramDesc()
        shaderProgramDesc.shaderStages.addElement(vertexShaderDesc)
        shaderProgramDesc.shaderStages.addElement(pixelShaderDesc)

        shaderProgram = ShaderProgram(shaderProgramDesc)
        val reflectDesc = shaderProgram!!.reflect()
        inputLayout = device!!.createInputLayout(reflectDesc.inputLayout)
        rootSignature = device!!.createRootSignature(reflectDesc.rootSignature)

        val pipelineDesc = PipelineDesc()
        pipelineDesc.inputLayout = inputLayout
        pipelineDesc.shaderProgram = shaderProgram
        pipelineDesc.rootSignature = rootSignature
        val renderTargetDesc = RenderTargetDesc()
        renderTargetDesc.format = Format.B8G8R8A8Unorm
        val graphicsPipelineDesc = GraphicsPipelineDesc()
        graphicsPipelineDesc.renderTargets.addElement(renderTargetDesc)
        pipelineDesc.graphics = graphicsPipelineDesc

        trianglePipeline = device!!.createPipeline(pipelineDesc)
    }

    private fun createVertexBuffer() {
        val vertices = floatArrayOf( // Position (XYZ)    // Color (RGBA)
            0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // Top vertex (red)
            -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // Bottom left (green)
            0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f // Bottom right (blue)
        )

        val bufferSize = (vertices.size * 4).toLong() // 4 bytes per float
        val bufferDesc = BufferDesc()
        bufferDesc.descriptor.set(ResourceDescriptor.VertexBuffer)
        bufferDesc.numBytes = bufferSize
        bufferDesc.debugName = InteropString("TriangleVertexBuffer")

        vertexBuffer = device!!.createBufferResource(bufferDesc)

        val batchCopy = BatchResourceCopy(device)
        batchCopy.begin()

        val byteArray = AlignedDataWriter()
        for (i in vertices.indices) {
            byteArray.writeFloat(vertices[i])
        }
        byteArray.flush()

        val copyDesc = CopyToGpuBufferDesc()
        copyDesc.dstBuffer = vertexBuffer
        copyDesc.data = byteArray.data()
        batchCopy.copyToGPUBuffer(copyDesc)
        batchCopy.submit()

        resourceTracking!!.trackBuffer(vertexBuffer, ResourceUsage.VertexAndConstantBuffer)
    }

    private fun renderFrame() {
        time.tick()
        frameDebugRenderer!!.updateStats(time.deltaTime.toFloat())

        val frameIndex = frameSync!!.nextFrame()
        val commandList = frameSync!!.getCommandList(frameIndex)

        commandList.begin()

        val nextImage = frameSync!!.acquireNextImage(frameIndex)
        val renderTarget = swapChain!!.getRenderTarget(nextImage.toInt())

        val transitionDesc = constPool.newObject(3, frameIndex.toInt()) { BatchTransitionDesc(commandList) }
        transitionDesc.reset(commandList)
        transitionDesc.transitionTexture(renderTarget, ResourceUsage.RenderTarget)
        resourceTracking!!.batchTransition(transitionDesc)

        val attachmentDesc = constPool.newObject(3, frameIndex.toInt()) { RenderingAttachmentDesc() }
        attachmentDesc.resource = renderTarget

        val renderingDesc = constPool.newObject(3, frameIndex.toInt()) { RenderingDesc() }
        renderingDesc.clearRTAttachments()
        renderingDesc.addRTAttachment(attachmentDesc)

        commandList.beginRendering(renderingDesc)

        val viewport = swapChain!!.viewport
        commandList.bindViewport(viewport.x, viewport.y, viewport.width, viewport.height)
        commandList.bindScissorRect(viewport.x, viewport.y, viewport.width, viewport.height)
        commandList.bindPipeline(trianglePipeline)
        commandList.bindVertexBuffer(vertexBuffer)
        commandList.draw(3, 1, 0, 0)
        frameDebugRenderer!!.render(commandList)
        commandList.endRendering()

        transitionDesc.reset(commandList)
        transitionDesc.transitionTexture(renderTarget, ResourceUsage.Present)
        resourceTracking!!.batchTransition(transitionDesc)

        commandList.end()

        frameSync!!.executeCommandList(frameIndex)
        val presentResult = frameSync!!.present(nextImage)
        if (presentResult == PresentResult.Timeout || presentResult == PresentResult.DeviceLost) {
            frameSync!!.waitIdle()
            swapChain!!.resize(windowSize.width, windowSize.height)
        }
    }

    private val triangleVertexShader: UnsignedCharArray
        get() {
            val shaderCode =
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
                        "}"
            return InteropUtilities.stringToBytes(InteropString(shaderCode))
        }

    private val trianglePixelShader: UnsignedCharArray
        get() {
            val shaderCode =
                "struct PSInput\n" +
                        "{\n" +
                        "    float4 Position : SV_POSITION;\n" +
                        "    float4 Color : COLOR;\n" +
                        "};\n" +
                        "\n" +
                        "float4 PSMain(PSInput input) : SV_TARGET\n" +
                        "{\n" +
                        "    return input.Color;\n" +
                        "}"
            return InteropUtilities.stringToBytes(InteropString(shaderCode))
        }
}