using System.Runtime.InteropServices;
using System.Numerics;
using System.Text;

namespace DenOfIz.Bindings.GraphicsExample;

public class Game
{
    private readonly LogicalDevice _logicalDevice;
    private readonly ResourceTracking _resourceTracking;
    private readonly FrameDebugRenderer _frameDebugRenderer;
    private InputLayout _inputLayout = null!;
    private BindGroupLayout[] _bindGroupLayouts = null!;
    private RootSignature _rootSignature = null!;
    private Pipeline _pipeline = null!;
    private Buffer _vertexBuffer = null!;
    private readonly StepTimer _stepTimer = new();

    private readonly PinnedArray<RenderingAttachmentDesc> _rtAttachments = new(1);

    public Game(LogicalDevice logicalDevice, ResourceTracking resourceTracking)
    {
        _logicalDevice = logicalDevice;
        _resourceTracking = resourceTracking;
        _frameDebugRenderer = new FrameDebugRenderer(new FrameDebugRendererDesc
        {
            Enabled = true,
            LogicalDevice = logicalDevice,
            Width = 1920,
            Height = 1080,
            FontSize = 24,
            TextColor = new Vector4 { X = 1.0f, Y = 1.0f, Z = 1.0f, W = 1.0f }
        });
        CreateBuffers();
        CreatePipeline();
    }

    public void Render(Viewport viewport, CommandList commandList, Texture renderTarget, uint frameIndex)
    {
        _stepTimer.Tick();

        commandList.Begin();

        _resourceTracking.TransitionTexture(commandList, renderTarget,
            (uint)ResourceUsageFlagBits.RenderTarget, QueueType.Graphics);

        _rtAttachments[0] = new RenderingAttachmentDesc
        {
            Resource = renderTarget,
            ClearDepthStencil = new Vector2 { X = 1.0f, Y = 0.0f }
        };

        RenderingDesc renderingDesc = new()
        {
            RTAttachments = RenderingAttachmentDescArray.FromPinned(_rtAttachments.Handle, 1),
            NumLayers = 1
        };

        commandList.BeginRendering(renderingDesc);
        RenderContent(viewport, commandList, frameIndex);
        commandList.EndRendering();

        _resourceTracking.TransitionTexture(commandList, renderTarget,
            (uint)ResourceUsageFlagBits.Present, QueueType.Graphics);

        commandList.End();
    }

    public void RenderContent(Viewport viewport, CommandList commandList, uint frameIndex)
    {
        commandList.BindPipeline(_pipeline);
        commandList.BindViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
        commandList.BindScissorRect(viewport.X, viewport.Y, viewport.Width, viewport.Height);
        commandList.BindVertexBuffer(_vertexBuffer, 0, 0, 0);
        commandList.Draw(3, 1, 0, 0);
        _frameDebugRenderer.Render(commandList);
    }


    private void CreateBuffers()
    {
        var bufferDesc = new BufferDesc
        {
            NumBytes = (ulong)(Triangle.Vertices.Length * sizeof(float)),
            Usage = (uint)BufferUsageFlagBits.Vertex,
            HeapType = HeapType.Gpu
        };
        _vertexBuffer = _logicalDevice.CreateBuffer(bufferDesc);

        var batchCopy = new BatchResourceCopy(new BatchResourceCopyDesc
        {
            Device = _logicalDevice,
            IssueBarriers = false
        });

        batchCopy.Begin();

        var data = new byte[Triangle.Vertices.Length * sizeof(float)];
        System.Buffer.BlockCopy(Triangle.Vertices, 0, data, 0, data.Length);
        var handle = GCHandle.Alloc(data, GCHandleType.Pinned);

        batchCopy.CopyToGPUBuffer(new CopyToGpuBufferDesc()
        {
            Data = new ByteArrayView
            {
                Elements = handle.AddrOfPinnedObject(),
                NumElements = (ulong)data.Length
            },
            DstBuffer = _vertexBuffer,
            DstBufferOffset = 0
        });

        var semaphore = _logicalDevice.CreateSemaphore();
        batchCopy.Submit(semaphore);
    }

    private void CreatePipeline()
    {
        var programDesc = new ShaderProgramDesc();

        var vsDesc = new ShaderStageDesc
        {
            EntryPoint = StringView.Create("VSMain"),
            Data = ByteArray.Create(Encoding.UTF8.GetBytes(Shaders.VertexShaderSource)),
            Stage = (int)ShaderStageFlagBits.Vertex
        };

        var psDesc = new ShaderStageDesc
        {
            EntryPoint = StringView.Create("PSMain"),
            Data = ByteArray.Create(Encoding.UTF8.GetBytes(Shaders.PixelShaderSource)),
            Stage = (int)ShaderStageFlagBits.Pixel
        };

        programDesc.ShaderStages = ShaderStageDescArray.Create([vsDesc, psDesc]);

        var program = new ShaderProgram(programDesc);
        var reflection = program.Reflect();
        _inputLayout = _logicalDevice.CreateInputLayout(reflection.InputLayout);

        var bindGroupLayoutDescs = reflection.BindGroupLayouts.ToArray();
        _bindGroupLayouts = new BindGroupLayout[bindGroupLayoutDescs.Length];
        for (int i = 0; i < bindGroupLayoutDescs.Length; i++)
        {
            _bindGroupLayouts[i] = _logicalDevice.CreateBindGroupLayout(bindGroupLayoutDescs[i]);
        }

        using var bindGroupLayoutArray = BindGroupLayoutArray.Create(_bindGroupLayouts);
        var rootSignatureDesc = new RootSignatureDesc
        {
            BindGroupLayouts = bindGroupLayoutArray,
            RootConstants = reflection.RootConstants
        };
        _rootSignature = _logicalDevice.CreateRootSignature(rootSignatureDesc);

        var pipelineDesc = new PipelineDesc
        {
            InputLayout = _inputLayout,
            RootSignature = _rootSignature,
            ShaderProgram = program,
            Graphics = new GraphicsPipelineDesc
            {
                PrimitiveTopology = PrimitiveTopology.Triangle,
                RenderTargets = RenderTargetDescArray.Create(
                    [
                        new RenderTargetDesc
                        {
                            Format = Format.B8G8R8A8Unorm,
                            Blend = new BlendDesc
                            {
                                RenderTargetWriteMask = 0x0F
                            }
                        }
                    ]
                )
            },
        };
        _pipeline = _logicalDevice.CreatePipeline(pipelineDesc);
    }
}