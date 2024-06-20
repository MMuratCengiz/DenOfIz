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

#include "DenOfIzCore/Logger.h"
#include "SDL2/SDL_syswm.h"

using namespace DenOfIz;

DX12LogicalDevice::DX12LogicalDevice() { m_context = std::make_unique<DX12Context>(); }

DX12LogicalDevice::~DX12LogicalDevice()
{
    WaitIdle();
//    m_context = nullptr;
//    m_context.reset();
//    for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++ )
//    {
//        m_context->CpuDescriptorHeaps[ i ].reset();
//    }
//    m_context->ShaderVisibleCbvSrvUavDescriptorHeap.reset();
//    m_context->ShaderVisibleSamplerDescriptorHeap.reset();
    //    m_context->CopyCommandListAllocator.reset();
    //    m_context->CopyCommandList.reset();
    //    m_context->DX12MemoryAllocator.reset();
    //    m_context->CopyCommandQueue.reset();
    //    m_context->ComputeCommandQueue.reset();
    //    m_context->GraphicsCommandQueue.reset();
    //    m_context->D3DDevice.reset();
    //    m_context->DXGIFactory.reset();
    //    m_context->Adapter.reset();
}

void DX12LogicalDevice::CreateDevice(GraphicsWindowHandle *window)
{
    m_context->Window = window;
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
            LOG(Verbosity::Warning, "DX12Device", "WARNING: Direct3D Debug Device is not available");
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
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
        }
    }
#endif

    THROW_IF_FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(m_context->DXGIFactory.put())));
}

std::vector<PhysicalDeviceInfo> DX12LogicalDevice::ListPhysicalDevices()
{
    std::vector<PhysicalDeviceInfo> result;
    wil::com_ptr<IDXGIAdapter1> adapter;
    for ( UINT adapterIndex = 0; SUCCEEDED(m_context->DXGIFactory->EnumAdapters1(adapterIndex, adapter.put())); adapterIndex++ )
    {
        PhysicalDeviceInfo deviceInfo{};
        CreateDeviceInfo(*adapter.get(), deviceInfo);
        result.push_back(deviceInfo);
    }

    return result;
}

void DX12LogicalDevice::CreateDeviceInfo(IDXGIAdapter1 &adapter, PhysicalDeviceInfo &deviceInfo)
{
    DXGI_ADAPTER_DESC adapterDesc;
    adapter.GetDesc(&adapterDesc);
    deviceInfo.Id = adapterDesc.DeviceId;
    std::wstring adapterName(adapterDesc.Description);
    deviceInfo.Name = std::string(adapterName.begin(), adapterName.end());

    // Todo actually read these from somewhere:
    deviceInfo.Capabilities.DedicatedTransferQueue = true;
    deviceInfo.Capabilities.ComputeShaders = true;

    DXGI_ADAPTER_DESC1 desc;
    THROW_IF_FAILED(adapter.GetDesc1(&desc));

    deviceInfo.Properties.IsDedicated = !(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE);

    wil::com_ptr<ID3D12Device> device;
    THROW_IF_FAILED(D3D12CreateDevice(&adapter, m_minFeatureLevel, IID_PPV_ARGS(device.put())));
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 opts = {};
    if ( SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &opts, sizeof(opts))) )
    {
        deviceInfo.Capabilities.RayTracing = opts.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    }

    BOOL allowTearing = false;
    HRESULT hr = m_context->DXGIFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
    if ( FAILED(hr) || !allowTearing )
    {
        deviceInfo.Capabilities.Tearing = false;
        LOG(Verbosity::Warning, "DX12Device", "WARNING: Variable refresh rate displays not supported");
    }
    else
    {
        deviceInfo.Capabilities.Tearing = true;
    }
}

void DX12LogicalDevice::LoadPhysicalDevice(const PhysicalDeviceInfo &device)
{
    m_selectedDeviceInfo = device;
    m_context->SelectedDeviceInfo = m_selectedDeviceInfo;

    wil::com_ptr<IDXGIAdapter1> adapter;

    for ( UINT adapterIndex = 0;
          SUCCEEDED(m_context->DXGIFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.put()))); adapterIndex++ )
    {
        DXGI_ADAPTER_DESC adapterDesc;
        adapter->GetDesc(&adapterDesc);
        if ( device.Id == adapterDesc.DeviceId )
        {
            break;
        }
    }

    m_context->Adapter = adapter.detach();
    // Create the DX12 API device object.
    wil::com_ptr<ID3D12Device> dxDevice;
    THROW_IF_FAILED(D3D12CreateDevice(m_context->Adapter.get(), m_minFeatureLevel, IID_PPV_ARGS(dxDevice.put())));
    THROW_IF_FAILED(dxDevice->QueryInterface(IID_PPV_ARGS(m_context->D3DDevice.put())));
    dxDevice->Release();

    // Confirm the device supports DXR.
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 opts = {};
    if ( FAILED(m_context->D3DDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &opts, sizeof(opts))) || opts.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED )
    {
        LOG(Verbosity::Warning, "DX12Device", "WARNING: DirectX Raytracing support not found.");
    }

    // Confirm the device supports Shader Model 6.3 or better.
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_3 };
    if ( FAILED(m_context->D3DDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))) || shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_3 )
    {
        LOG(Verbosity::Critical, "DX12Device", "ERROR: Requires Shader Model 6.3 or better support.\n.");
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
        filter.DenyList.NumIDs = _countof(hide);
        filter.DenyList.pIDList = hide;
        d3dInfoQueue->AddStorageFilterEntries(&filter);

        DWORD callbackCookie;
        d3dInfoQueue->RegisterMessageCallback(
            [](D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID iD, LPCSTR description, void *context)
            {
                switch ( severity )
                {
                case D3D12_MESSAGE_SEVERITY_ERROR:
                case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                    LOG(Verbosity::Critical, "DX12Device", description);
                    break;
                case D3D12_MESSAGE_SEVERITY_WARNING:
                    LOG(Verbosity::Warning, "DX12Device", description);
                    break;
                case D3D12_MESSAGE_SEVERITY_INFO:
                case D3D12_MESSAGE_SEVERITY_MESSAGE:
                    LOG(Verbosity::Information, "DX12Device", description);
                    break;
                };
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
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
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
    m_context->ShaderVisibleSamplerDescriptorHeap = std::make_unique<DX12DescriptorHeap>(m_context->D3DDevice.get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true);

    THROW_IF_FAILED(m_context->D3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(m_context->CopyCommandListAllocator.put())));
    THROW_IF_FAILED(m_context->D3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_context->CopyCommandListAllocator.get(), nullptr,
                                                            IID_PPV_ARGS(m_context->CopyCommandList.put())));
    THROW_IF_FAILED(m_context->CopyCommandList->Close());

    D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
    allocatorDesc.pDevice = m_context->D3DDevice.get();
    allocatorDesc.pAdapter = m_context->Adapter.get();

    allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED | D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;
    THROW_IF_FAILED(D3D12MA::CreateAllocator(&allocatorDesc, m_context->DX12MemoryAllocator.put()));
}

void DX12LogicalDevice::WaitIdle() {}

std::unique_ptr<ICommandList> DX12LogicalDevice::CreateCommandList(const CommandListCreateInfo &createInfo)
{
    DX12CommandList *commandList = new DX12CommandList(m_context.get(), createInfo);
    return std::unique_ptr<ICommandList>(commandList);
}

std::unique_ptr<IPipeline> DX12LogicalDevice::CreatePipeline(const PipelineCreateInfo &createInfo)
{
    DX12Pipeline *pipeline = new DX12Pipeline(m_context.get(), createInfo);
    return std::unique_ptr<IPipeline>(pipeline);
}

std::unique_ptr<ISwapChain> DX12LogicalDevice::CreateSwapChain(const SwapChainCreateInfo &createInfo)
{
    DX12SwapChain *swapChain = new DX12SwapChain(m_context.get(), createInfo);
    return std::unique_ptr<ISwapChain>(swapChain);
}

std::unique_ptr<IRootSignature> DX12LogicalDevice::CreateRootSignature(const RootSignatureCreateInfo &createInfo)
{
    DX12RootSignature *rootSignature = new DX12RootSignature(m_context.get(), createInfo);
    return std::unique_ptr<IRootSignature>(rootSignature);
}

std::unique_ptr<IInputLayout> DX12LogicalDevice::CreateInputLayout(const InputLayoutCreateInfo &createInfo)
{
    DX12InputLayout *inputLayout = new DX12InputLayout(createInfo);
    return std::unique_ptr<IInputLayout>(inputLayout);
}

std::unique_ptr<IDescriptorTable> DX12LogicalDevice::CreateDescriptorTable(const DescriptorTableCreateInfo &createInfo)
{
    DX12DescriptorTable *descriptorTable = new DX12DescriptorTable(m_context.get(), createInfo);
    return std::unique_ptr<IDescriptorTable>(descriptorTable);
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

std::unique_ptr<IBufferResource> DX12LogicalDevice::CreateBufferResource(std::string name, const BufferCreateInfo &createInfo)
{
    DX12BufferResource *buffer = new DX12BufferResource(m_context.get(), createInfo);
    buffer->Name = name;
    return std::unique_ptr<IBufferResource>(buffer);
}

std::unique_ptr<ITextureResource> DX12LogicalDevice::CreateImageResource(std::string name, const ImageCreateInfo &createInfo)
{
    DX12ImageResource *image = new DX12ImageResource(m_context.get(), createInfo);
    image->Name = name;
    return std::unique_ptr<ITextureResource>(image);
}
