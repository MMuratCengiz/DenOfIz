/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <DenOfIzGraphics/Backends/DirectX12/DX12LogicalDevice.h>

#include "SDL2/SDL_syswm.h"

using namespace DenOfIz;

DX12LogicalDevice::DX12LogicalDevice()
{
    m_context = std::make_unique<DX12Context>();
}

DX12LogicalDevice::~DX12LogicalDevice()
{
    WaitIdle();
}

void DX12LogicalDevice::CreateDevice(GraphicsWindowHandle *window)
{
    m_context->Window      = window;
    DWORD dxgiFactoryFlags = 0;
#ifndef NDEBUG
    {
        wil::com_ptr<ID3D12Debug> debugController;
        if ( SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.put()))) )
        {
            debugController->EnableDebugLayer();
        }
        else
        {
            LOG(WARNING) << "Direct3D Debug Device is not available";
        }

        wil::com_ptr<IDXGIInfoQueue> dxgiInfoQueue;
        if ( SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.put()))) )
        {
            dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

            DXGI_INFO_QUEUE_MESSAGE_ID hide[] = {
                80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
            };
            DXGI_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs        = _countof(hide);
            filter.DenyList.pIDList       = hide;
            dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
        }
    }
#endif

    THROW_IF_FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(m_context->DXGIFactory.put())));
}

std::vector<PhysicalDevice> DX12LogicalDevice::ListPhysicalDevices()
{
    std::vector<PhysicalDevice> result;
    wil::com_ptr<IDXGIAdapter1> adapter;
    for ( UINT adapterIndex = 0; SUCCEEDED(m_context->DXGIFactory->EnumAdapters1(adapterIndex, adapter.put())); adapterIndex++ )
    {
        PhysicalDevice deviceInfo{};
        CreateDeviceInfo(*adapter.get(), deviceInfo);
        result.push_back(deviceInfo);
    }

    return result;
}

void DX12LogicalDevice::CreateDeviceInfo(IDXGIAdapter1 &adapter, PhysicalDevice &physicalDevice)
{
    DXGI_ADAPTER_DESC adapterDesc;
    adapter.GetDesc(&adapterDesc);
    physicalDevice.Id = adapterDesc.DeviceId;
    std::wstring adapterName(adapterDesc.Description);
    physicalDevice.Name = std::string(adapterName.begin(), adapterName.end());

    DXGI_ADAPTER_DESC1 desc;
    THROW_IF_FAILED(adapter.GetDesc1(&desc));
    physicalDevice.Properties.IsDedicated         = !(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE);
    physicalDevice.Properties.MemoryAvailableInMb = desc.DedicatedVideoMemory / (1024 * 1024);

    wil::com_ptr<ID3D12Device> device;
    THROW_IF_FAILED(D3D12CreateDevice(&adapter, m_minFeatureLevel, IID_PPV_ARGS(device.put())));

    // Todo actually read these from somewhere:
    physicalDevice.Capabilities.DedicatedTransferQueue = true;
    physicalDevice.Capabilities.ComputeShaders         = true;

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 opts5 = {};
    if ( SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &opts5, sizeof(opts5))) )
    {
        physicalDevice.Capabilities.RayTracing = opts5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    }

    BOOL allowTearing = false;
    m_context->DXGIFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
    physicalDevice.Capabilities.Tearing = allowTearing;

    D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = {};
    HRESULT                            hr        = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(options12));
    if ( SUCCEEDED(hr) )
    {
        m_context->DX12Capabilities.EnhancedBarriers = options12.EnhancedBarriersSupported;
    }
}

void DX12LogicalDevice::LoadPhysicalDevice(const PhysicalDevice &device)
{
    LOG(INFO) << "Loading physical device: " << device.Name;
    LOG(INFO) << "Device Capabilities:";
    LOG(INFO) << "Dedicated GPU " << (device.Properties.IsDedicated ? "Yes" : "No");
    LOG(INFO) << "Available Memory " << device.Properties.MemoryAvailableInMb << "MB";
    LOG(INFO) << "Dedicated Transfer Queue: " << (device.Capabilities.DedicatedTransferQueue ? "Yes" : "No");
    LOG(INFO) << "Compute Shaders: " << (device.Capabilities.ComputeShaders ? "Yes" : "No");
    LOG(INFO) << "Ray Tracing: " << (device.Capabilities.RayTracing ? "Yes" : "No");
    LOG(INFO) << "Tearing: " << (device.Capabilities.Tearing ? "Yes" : "No");
    LOG(INFO) << "DX12 Enhanced Barriers: " << (m_context->DX12Capabilities.EnhancedBarriers ? "Yes" : "No");

    m_selectedDeviceInfo          = device;
    m_context->SelectedDeviceInfo = m_selectedDeviceInfo;

    wil::com_ptr<IDXGIAdapter1> adapter;

    for ( UINT adapterIndex = 0; SUCCEEDED(m_context->DXGIFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.put())));
          adapterIndex++ )
    {
        DXGI_ADAPTER_DESC adapterDesc;
        adapter->GetDesc(&adapterDesc);
        if ( device.Id == adapterDesc.DeviceId )
        {
            THROW_IF_FAILED(adapter->QueryInterface(IID_PPV_ARGS(m_context->Adapter.put())));
            break;
        }
    }
    // Create the DX12 API device object.
    wil::com_ptr<ID3D12Device> dxDevice;
    THROW_IF_FAILED(D3D12CreateDevice(m_context->Adapter.get(), m_minFeatureLevel, IID_PPV_ARGS(dxDevice.put())));
    m_context->D3DDevice = dxDevice.query<ID3D12Device9>();

    // Confirm the device supports Shader Model 6.3 or better.
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_3 };
    if ( FAILED(m_context->D3DDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))) || shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_3 )
    {
        LOG(FATAL) << "ERROR: Requires Shader Model 6.3 or better support.";
        throw std::exception("Requires Shader Model 6.3 or better support");
    }

#ifndef NDEBUG
    // Configure debug device (if active).
    wil::com_ptr<ID3D12InfoQueue1> d3dInfoQueue = m_context->D3DDevice.query<ID3D12InfoQueue1>();
    if ( d3dInfoQueue )
    {
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        D3D12_MESSAGE_ID hide[] = {
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
            // Workarounds for debug layer issues on hybrid-graphics systems
            D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
        };
        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs         = _countof(hide);
        filter.DenyList.pIDList        = hide;
        d3dInfoQueue->AddStorageFilterEntries(&filter);

        DWORD callbackCookie;
        d3dInfoQueue->RegisterMessageCallback(
            [](D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID iD, LPCSTR description, void *context)
            {
                switch ( severity )
                {
                case D3D12_MESSAGE_SEVERITY_ERROR:
                case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                    LOG(FATAL) << description;
                    break;
                case D3D12_MESSAGE_SEVERITY_WARNING:
                    LOG(WARNING) << description;
                    break;
                case D3D12_MESSAGE_SEVERITY_INFO:
                case D3D12_MESSAGE_SEVERITY_MESSAGE:
                    LOG(INFO) << description;
                    break;
                }
            },
            D3D12_MESSAGE_CALLBACK_FLAG_NONE, this, &callbackCookie);
    }
#endif

    // Determine maximum supported feature level for this device
    static constexpr D3D_FEATURE_LEVEL s_featureLevels[] = { D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0 };

    D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels = { _countof(s_featureLevels), s_featureLevels, D3D_FEATURE_LEVEL_11_0 };

    HRESULT hr = m_context->D3DDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));
    if ( SUCCEEDED(hr) )
    {
        m_minFeatureLevel = featLevels.MaxSupportedFeatureLevel;
    }
    else
    {
        m_minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    THROW_IF_FAILED(m_context->D3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_context->GraphicsCommandQueue.put())));

    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
    THROW_IF_FAILED(m_context->D3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_context->ComputeCommandQueue.put())));

    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
    THROW_IF_FAILED(m_context->D3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_context->CopyCommandQueue.put())));

    for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++ )
    {
        m_context->CpuDescriptorHeaps[ i ] = std::make_unique<DX12DescriptorHeap>(m_context->D3DDevice.get(), D3D12_DESCRIPTOR_HEAP_TYPE(i), false);
    }

    m_context->ShaderVisibleCbvSrvUavDescriptorHeap = std::make_unique<DX12DescriptorHeap>(m_context->D3DDevice.get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);
    m_context->ShaderVisibleSamplerDescriptorHeap   = std::make_unique<DX12DescriptorHeap>(m_context->D3DDevice.get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true);

    THROW_IF_FAILED(m_context->D3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(m_context->CopyCommandListAllocator.put())));
    THROW_IF_FAILED(m_context->D3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_context->CopyCommandListAllocator.get(), nullptr,
                                                            IID_PPV_ARGS(m_context->CopyCommandList.put())));
    THROW_IF_FAILED(m_context->CopyCommandList->Close());

    D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
    allocatorDesc.pDevice                 = m_context->D3DDevice.get();
    allocatorDesc.pAdapter                = m_context->Adapter.get();
    allocatorDesc.Flags = static_cast<D3D12MA::ALLOCATOR_FLAGS>(D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED | D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED);

    THROW_IF_FAILED(D3D12MA::CreateAllocator(&allocatorDesc, m_context->DX12MemoryAllocator.put()));
    THROW_IF_FAILED(m_context->D3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_waitIdleFence.put())));

    m_selectedDeviceInfo.Constants.TexturePitchAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}

void DX12LogicalDevice::WaitIdle()
{
    uint16_t fenceValue = 0;
    for ( auto &queue : { m_context->GraphicsCommandQueue, m_context->ComputeCommandQueue, m_context->CopyCommandQueue } )
    {
        if ( queue )
        {
            queue->Signal(m_waitIdleFence.get(), ++fenceValue);
        }
    }

    if ( m_waitIdleFence )
    {
        m_waitIdleFence->SetEventOnCompletion(fenceValue, nullptr);
    }
}

std::unique_ptr<ICommandListPool> DX12LogicalDevice::CreateCommandListPool(const CommandListPoolDesc &poolDesc)
{
    DX12CommandListPool *pool = new DX12CommandListPool(m_context.get(), poolDesc);
    return std::unique_ptr<ICommandListPool>(pool);
}

std::unique_ptr<IPipeline> DX12LogicalDevice::CreatePipeline(const PipelineDesc &pipelineDesc)
{
    DX12Pipeline *pipeline = new DX12Pipeline(m_context.get(), pipelineDesc);
    return std::unique_ptr<IPipeline>(pipeline);
}

std::unique_ptr<ISwapChain> DX12LogicalDevice::CreateSwapChain(const SwapChainDesc &swapChainDesc)
{
    DX12SwapChain *swapChain = new DX12SwapChain(m_context.get(), swapChainDesc);
    return std::unique_ptr<ISwapChain>(swapChain);
}

std::unique_ptr<IRootSignature> DX12LogicalDevice::CreateRootSignature(const RootSignatureDesc &rootSignatureDesc)
{
    DX12RootSignature *rootSignature = new DX12RootSignature(m_context.get(), rootSignatureDesc);
    return std::unique_ptr<IRootSignature>(rootSignature);
}

std::unique_ptr<IInputLayout> DX12LogicalDevice::CreateInputLayout(const InputLayoutDesc &inputLayoutDesc)
{
    DX12InputLayout *inputLayout = new DX12InputLayout(inputLayoutDesc);
    return std::unique_ptr<IInputLayout>(inputLayout);
}

std::unique_ptr<IResourceBindGroup> DX12LogicalDevice::CreateResourceBindGroup(const ResourceBindGroupDesc &descriptorTableDesc)
{
    DX12ResourceBindGroup *descriptorTable = new DX12ResourceBindGroup(m_context.get(), descriptorTableDesc);
    return std::unique_ptr<IResourceBindGroup>(descriptorTable);
}

std::unique_ptr<IFence> DX12LogicalDevice::CreateFence()
{
    DX12Fence *fence = new DX12Fence(m_context.get());
    return std::unique_ptr<IFence>(fence);
}

std::unique_ptr<ISemaphore> DX12LogicalDevice::CreateSemaphore()
{
    DX12Semaphore *semaphore = new DX12Semaphore(m_context.get());
    return std::unique_ptr<ISemaphore>(semaphore);
}

std::unique_ptr<IBufferResource> DX12LogicalDevice::CreateBufferResource(std::string name, const BufferDesc &bufferDesc)
{
    DX12BufferResource *buffer = new DX12BufferResource(m_context.get(), bufferDesc);
    buffer->Name               = name;
    return std::unique_ptr<IBufferResource>(buffer);
}

std::unique_ptr<ITextureResource> DX12LogicalDevice::CreateTextureResource(std::string name, const TextureDesc &textureDesc)
{
    DX12TextureResource *image = new DX12TextureResource(m_context.get(), textureDesc);
    image->Name                = name;
    return std::unique_ptr<ITextureResource>(image);
}

std::unique_ptr<ISampler> DX12LogicalDevice::CreateSampler(std::string name, const SamplerDesc &samplerDesc)
{
    DX12Sampler *sampler = new DX12Sampler(m_context.get(), samplerDesc);
    sampler->Name        = name;
    return std::unique_ptr<ISampler>(sampler);
}
