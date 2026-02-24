using System;
using System.Runtime.InteropServices;
using DenOfIz.Native;

namespace DenOfIz
{
    public sealed partial class LogicalDevice : IDisposable
    {
        internal ulong Handle { get; private set; }
        private readonly bool _ownsHandle;

        public static implicit operator ulong(LogicalDevice wrapper) => wrapper?.Handle ?? 0;

        internal LogicalDevice(ulong handle, bool ownsHandle = false)
        {
            Handle = handle;
            _ownsHandle = ownsHandle;
        }

        public Result CreateDevice(in LogicalDeviceDesc desc)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_LogicalDevice_CreateDevice(Handle, in desc);
        }

        public PhysicalDeviceArray ListPhysicalDevices()
        {
            ThrowIfDisposed();
            PhysicalDeviceArray outDevices = default;
            var __result = Methods.DenOfIz_LogicalDevice_ListPhysicalDevices(Handle, out outDevices);
            if (__result != Result.Success) throw new DenOfIzException(__result);
            return outDevices;
        }

        public Result LoadPhysicalDevice(in PhysicalDevice physicalDevice)
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_LogicalDevice_LoadPhysicalDevice(Handle, in physicalDevice);
        }

        public PhysicalDevice DeviceInfo()
        {
            ThrowIfDisposed();
            PhysicalDevice outDevice = default;
            var __result = Methods.DenOfIz_LogicalDevice_DeviceInfo(Handle, out outDevice);
            if (__result != Result.Success) throw new DenOfIzException(__result);
            return outDevice;
        }

        public bool IsDeviceLost()
        {
            ThrowIfDisposed();
            bool outLost = default;
            var __result = Methods.DenOfIz_LogicalDevice_IsDeviceLost(Handle, out outLost);
            if (__result != Result.Success) throw new DenOfIzException(__result);
            return outLost;
        }

        public Result WaitIdle()
        {
            ThrowIfDisposed();
            return Methods.DenOfIz_LogicalDevice_WaitIdle(Handle);
        }

        public CommandQueue CreateCommandQueue(in CommandQueueDesc desc)
        {
            ThrowIfDisposed();
            ulong outQueue = 0;
            Methods.DenOfIz_LogicalDevice_CreateCommandQueue(Handle, in desc, out outQueue);
            return new CommandQueue(outQueue, ownsHandle: true);
        }

        public CommandListPool CreateCommandListPool(in CommandListPoolDesc desc)
        {
            ThrowIfDisposed();
            ulong outPool = 0;
            Methods.DenOfIz_LogicalDevice_CreateCommandListPool(Handle, in desc, out outPool);
            return new CommandListPool(outPool, ownsHandle: true);
        }

        public Pipeline CreatePipeline(in PipelineDesc desc)
        {
            ThrowIfDisposed();
            ulong outPipeline = 0;
            Methods.DenOfIz_LogicalDevice_CreatePipeline(Handle, in desc, out outPipeline);
            return new Pipeline(outPipeline, ownsHandle: true);
        }

        public PipelineCache CreatePipelineCache(in PipelineCacheDesc desc)
        {
            ThrowIfDisposed();
            ulong outCache = 0;
            Methods.DenOfIz_LogicalDevice_CreatePipelineCache(Handle, in desc, out outCache);
            return new PipelineCache(outCache, ownsHandle: true);
        }

        public SwapChain CreateSwapChain(in SwapChainDesc desc)
        {
            ThrowIfDisposed();
            ulong outSwapChain = 0;
            Methods.DenOfIz_LogicalDevice_CreateSwapChain(Handle, in desc, out outSwapChain);
            return new SwapChain(outSwapChain, ownsHandle: true);
        }

        public RootSignature CreateRootSignature(in RootSignatureDesc desc)
        {
            ThrowIfDisposed();
            ulong outSignature = 0;
            Methods.DenOfIz_LogicalDevice_CreateRootSignature(Handle, in desc, out outSignature);
            return new RootSignature(outSignature, ownsHandle: true);
        }

        public InputLayout CreateInputLayout(in InputLayoutDesc desc)
        {
            ThrowIfDisposed();
            ulong outLayout = 0;
            Methods.DenOfIz_LogicalDevice_CreateInputLayout(Handle, in desc, out outLayout);
            return new InputLayout(outLayout, ownsHandle: true);
        }

        public BindGroupLayout CreateBindGroupLayout(in BindGroupLayoutDesc desc)
        {
            ThrowIfDisposed();
            ulong outLayout = 0;
            Methods.DenOfIz_LogicalDevice_CreateBindGroupLayout(Handle, in desc, out outLayout);
            return new BindGroupLayout(outLayout, ownsHandle: true);
        }

        public BindGroup CreateBindGroup(in BindGroupDesc desc)
        {
            ThrowIfDisposed();
            ulong outGroup = 0;
            Methods.DenOfIz_LogicalDevice_CreateBindGroup(Handle, in desc, out outGroup);
            return new BindGroup(outGroup, ownsHandle: true);
        }

        public Fence CreateFence()
        {
            ThrowIfDisposed();
            ulong outFence = 0;
            Methods.DenOfIz_LogicalDevice_CreateFence(Handle, out outFence);
            return new Fence(outFence, ownsHandle: true);
        }

        public Semaphore CreateSemaphore()
        {
            ThrowIfDisposed();
            ulong outSemaphore = 0;
            Methods.DenOfIz_LogicalDevice_CreateSemaphore(Handle, out outSemaphore);
            return new Semaphore(outSemaphore, ownsHandle: true);
        }

        public Buffer CreateBuffer(in BufferDesc desc)
        {
            ThrowIfDisposed();
            ulong outBuffer = 0;
            Methods.DenOfIz_LogicalDevice_CreateBuffer(Handle, in desc, out outBuffer);
            return new Buffer(outBuffer, ownsHandle: true);
        }

        public Texture CreateTexture(in TextureDesc desc)
        {
            ThrowIfDisposed();
            ulong outTexture = 0;
            Methods.DenOfIz_LogicalDevice_CreateTexture(Handle, in desc, out outTexture);
            return new Texture(outTexture, ownsHandle: true);
        }

        public Sampler CreateSampler(in SamplerDesc desc)
        {
            ThrowIfDisposed();
            ulong outSampler = 0;
            Methods.DenOfIz_LogicalDevice_CreateSampler(Handle, in desc, out outSampler);
            return new Sampler(outSampler, ownsHandle: true);
        }

        public QueryPool CreateQueryPool(in QueryPoolDesc desc)
        {
            ThrowIfDisposed();
            ulong outPool = 0;
            Methods.DenOfIz_LogicalDevice_CreateQueryPool(Handle, in desc, out outPool);
            return new QueryPool(outPool, ownsHandle: true);
        }

        public TopLevelAS CreateTopLevelAS(in TopLevelASDesc desc)
        {
            ThrowIfDisposed();
            ulong outAS = 0;
            Methods.DenOfIz_LogicalDevice_CreateTopLevelAS(Handle, in desc, out outAS);
            return new TopLevelAS(outAS, ownsHandle: true);
        }

        public BottomLevelAS CreateBottomLevelAS(in BottomLevelASDesc desc)
        {
            ThrowIfDisposed();
            ulong outAS = 0;
            Methods.DenOfIz_LogicalDevice_CreateBottomLevelAS(Handle, in desc, out outAS);
            return new BottomLevelAS(outAS, ownsHandle: true);
        }

        public ShaderBindingTable CreateShaderBindingTable(in ShaderBindingTableDesc desc)
        {
            ThrowIfDisposed();
            ulong outTable = 0;
            Methods.DenOfIz_LogicalDevice_CreateShaderBindingTable(Handle, in desc, out outTable);
            return new ShaderBindingTable(outTable, ownsHandle: true);
        }

        public LocalRootSignature CreateLocalRootSignature(in LocalRootSignatureDesc desc)
        {
            ThrowIfDisposed();
            ulong outSignature = 0;
            Methods.DenOfIz_LogicalDevice_CreateLocalRootSignature(Handle, in desc, out outSignature);
            return new LocalRootSignature(outSignature, ownsHandle: true);
        }

        public ShaderLocalData CreateShaderLocalData(in ShaderLocalDataDesc desc)
        {
            ThrowIfDisposed();
            ulong outData = 0;
            Methods.DenOfIz_LogicalDevice_CreateShaderLocalData(Handle, in desc, out outData);
            return new ShaderLocalData(outData, ownsHandle: true);
        }

        public void Dispose()
        {
            if (Handle == 0)
            {
                return;
            }

            if (_ownsHandle)
            {
                Methods.DenOfIz_LogicalDevice_Destroy(Handle);
            }

            Handle = 0;
        }

        private void ThrowIfDisposed()
        {
            if (Handle == 0)
            {
                throw new ObjectDisposedException(nameof(LogicalDevice));
            }
        }
    }
}
