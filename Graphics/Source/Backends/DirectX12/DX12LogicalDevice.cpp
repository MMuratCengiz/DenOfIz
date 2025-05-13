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

#include <DenOfIzGraphics/Backends/DirectX12/DX12CommandQueue.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12InputLayout.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12LogicalDevice.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12Semaphore.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12SwapChain.h>
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12BottomLeveLAS.h>
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12LocalRootSignature.h>
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12ShaderBindingTable.h>
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12ShaderLocalData.h>
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12TopLevelAS.h>

#include "SDL2/SDL_syswm.h"

using namespace DenOfIz;

DX12LogicalDevice::DX12LogicalDevice( )
{
    m_context = std::make_unique<DX12Context>( );
}

DX12LogicalDevice::~DX12LogicalDevice( )
{
    WaitIdle( );
}

void DX12LogicalDevice::CreateDevice( )
{
    DWORD dxgiFactoryFlags = 0;
#if not defined( NDEBUG ) && not defined( NSIGHT_ENABLE )
    {
        if ( wil::com_ptr<ID3D12Debug> debugController; SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( debugController.put( ) ) ) ) )
        {
            debugController->EnableDebugLayer( );
        }
        else
        {
            LOG( WARNING ) << "Direct3D Debug Device is not available";
        }

        if ( wil::com_ptr<IDXGIInfoQueue> dxgiInfoQueue; SUCCEEDED( DXGIGetDebugInterface1( 0, IID_PPV_ARGS( dxgiInfoQueue.put( ) ) ) ) )
        {
            dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    DX_CHECK_RESULT( CreateDXGIFactory2( dxgiFactoryFlags, IID_PPV_ARGS( m_context->DXGIFactory.put( ) ) ) );
}

InteropArray<PhysicalDevice> DX12LogicalDevice::ListPhysicalDevices( )
{
    InteropArray<PhysicalDevice> result;
    wil::com_ptr<IDXGIAdapter1>  adapter;
    for ( UINT adapterIndex = 0; SUCCEEDED( m_context->DXGIFactory->EnumAdapters1( adapterIndex, adapter.put( ) ) ); adapterIndex++ )
    {
        PhysicalDevice deviceInfo{ };
        CreateDeviceInfo( *adapter.get( ), deviceInfo );
        result.AddElement( deviceInfo );
    }
    return result;
}

void DX12LogicalDevice::CreateDeviceInfo( IDXGIAdapter1 &adapter, PhysicalDevice &physicalDevice ) const
{
    DXGI_ADAPTER_DESC adapterDesc;
    DX_CHECK_RESULT( adapter.GetDesc( &adapterDesc ) );
    physicalDevice.Id = adapterDesc.DeviceId;
    std::wstring adapterName( adapterDesc.Description );
    physicalDevice.Name = std::string( adapterName.begin( ), adapterName.end( ) ).c_str( );

    DXGI_ADAPTER_DESC1 desc;
    DX_CHECK_RESULT( adapter.GetDesc1( &desc ) );
    physicalDevice.Properties.IsDedicated         = !( desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE );
    physicalDevice.Properties.MemoryAvailableInMb = desc.DedicatedVideoMemory / ( 1024 * 1024 );

    wil::com_ptr<ID3D12Device> device;
    DX_CHECK_RESULT( D3D12CreateDevice( &adapter, m_minFeatureLevel, IID_PPV_ARGS( device.put( ) ) ) );

    physicalDevice.Capabilities.DedicatedCopyQueue = true;
    physicalDevice.Capabilities.ComputeShaders     = true;
    physicalDevice.Capabilities.GeometryShaders    = true;
    physicalDevice.Capabilities.Tessellation       = true;

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 opts5 = { };
    if ( SUCCEEDED( device->CheckFeatureSupport( D3D12_FEATURE_D3D12_OPTIONS5, &opts5, sizeof( opts5 ) ) ) )
    {
        physicalDevice.Capabilities.RayTracing = opts5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS7 opts7 = { };
    if ( SUCCEEDED( device->CheckFeatureSupport( D3D12_FEATURE_D3D12_OPTIONS7, &opts7, sizeof( opts7 ) ) ) )
    {
        physicalDevice.Capabilities.MeshShaders     = opts7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
        physicalDevice.Capabilities.SamplerFeedback = opts7.SamplerFeedbackTier != D3D12_SAMPLER_FEEDBACK_TIER_NOT_SUPPORTED;
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS6 opts6 = { };
    if ( SUCCEEDED( device->CheckFeatureSupport( D3D12_FEATURE_D3D12_OPTIONS6, &opts6, sizeof( opts6 ) ) ) )
    {
        physicalDevice.Capabilities.VariableRateShading = opts6.VariableShadingRateTier != D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED;
    }

    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_6 };
    if ( SUCCEEDED( device->CheckFeatureSupport( D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof( shaderModel ) ) ) )
    {
        physicalDevice.Capabilities.ShaderInt16   = true;
        physicalDevice.Capabilities.ShaderFloat16 = true;
    }

    BOOL allowTearing = false;
    DX_CHECK_RESULT( m_context->DXGIFactory->CheckFeatureSupport( DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof( allowTearing ) ) );
    physicalDevice.Capabilities.Tearing = allowTearing;

    D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = { };
    if ( SUCCEEDED( device->CheckFeatureSupport( D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof( options12 ) ) ) )
    {
        m_context->DX12Capabilities.EnhancedBarriers = options12.EnhancedBarriersSupported;
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS4 opts4 = { };
    if ( SUCCEEDED( device->CheckFeatureSupport( D3D12_FEATURE_D3D12_OPTIONS4, &opts4, sizeof( opts4 ) ) ) )
    {
        physicalDevice.Capabilities.DrawIndirectCount = true;
    }
    physicalDevice.Capabilities.HDR = true;
}

void DX12LogicalDevice::LoadPhysicalDevice( const PhysicalDevice &device )
{
    m_selectedDeviceInfo = device;

    wil::com_ptr<IDXGIAdapter1> adapter;
    for ( UINT adapterIndex = 0; SUCCEEDED( m_context->DXGIFactory->EnumAdapters1( adapterIndex, adapter.put( ) ) ); adapterIndex++ )
    {
        DXGI_ADAPTER_DESC1 adapterDesc;
        DX_CHECK_RESULT( adapter->GetDesc1( &adapterDesc ) );
        if ( device.Id == adapterDesc.DeviceId )
        {
            m_context->Adapter = adapter;
            break;
        }
    }
    // Create the DX12 API device object.
    wil::com_ptr<ID3D12Device> dxDevice;
    DX_CHECK_RESULT( D3D12CreateDevice( m_context->Adapter.get( ), m_minFeatureLevel, IID_PPV_ARGS( dxDevice.put( ) ) ) );
    m_context->D3DDevice = dxDevice.query<ID3D12Device9>( );

    // Confirm the device supports Shader Model 6.3 or better.
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_3 };
    if ( FAILED( m_context->D3DDevice->CheckFeatureSupport( D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof( shaderModel ) ) ) ||
         shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_3 )
    {
        LOG( FATAL ) << "ERROR: Requires Shader Model 6.3 or better support.";
        throw std::exception( "Requires Shader Model 6.3 or better support" );
    }

#if not defined( NDEBUG ) && not defined( NSIGHT_ENABLE )
    // Configure debug device (if active).
    if ( const wil::com_ptr<ID3D12InfoQueue1> d3dInfoQueue = m_context->D3DDevice.query<ID3D12InfoQueue1>( ) )
    {
        DX_CHECK_RESULT( d3dInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, true ) );
        DX_CHECK_RESULT( d3dInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, true ) );
        DX_CHECK_RESULT( d3dInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_WARNING, false ) );
        DX_CHECK_RESULT( d3dInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_INFO, false ) );
        DX_CHECK_RESULT( d3dInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_MESSAGE, false ) );

        D3D12_MESSAGE_ID hide[] = {
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
            // Workarounds for debug layer issues on hybrid-graphics systems
            D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
        };
        D3D12_INFO_QUEUE_FILTER filter = { };
        filter.DenyList.NumIDs         = _countof( hide );
        filter.DenyList.pIDList        = hide;
        DX_CHECK_RESULT( d3dInfoQueue->AddStorageFilterEntries( &filter ) );

        DWORD         callbackCookie;
        const HRESULT hr = d3dInfoQueue->RegisterMessageCallback(
            []( D3D12_MESSAGE_CATEGORY _, D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID iD, LPCSTR description, void *context )
            {
                switch ( severity )
                {
                case D3D12_MESSAGE_SEVERITY_ERROR:
                case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                    LOG( FATAL ) << description;
                    break;
                case D3D12_MESSAGE_SEVERITY_WARNING:
                    LOG( WARNING ) << description;
                    break;
                case D3D12_MESSAGE_SEVERITY_INFO:
                case D3D12_MESSAGE_SEVERITY_MESSAGE:
                    LOG( INFO ) << description;
                    break;
                }
            },
            D3D12_MESSAGE_CALLBACK_FLAG_NONE, this, &callbackCookie );
        DX_CHECK_RESULT( hr );
    }
#endif

    // Determine maximum supported feature level for this device
    static constexpr D3D_FEATURE_LEVEL s_featureLevels[] = { D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0 };

    D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels = { _countof( s_featureLevels ), s_featureLevels, D3D_FEATURE_LEVEL_11_0 };

    if ( const HRESULT hr = m_context->D3DDevice->CheckFeatureSupport( D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof( featLevels ) ); SUCCEEDED( hr ) )
    {
        m_minFeatureLevel = featLevels.MaxSupportedFeatureLevel;
    }
    else
    {
        m_minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = { };
    queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( m_context->GraphicsCommandQueue.put( ) ) ) );
    DX_CHECK_RESULT( m_context->GraphicsCommandQueue->SetName( L"Graphics Command Queue" ) );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_context->GraphicsCommandQueueFence.put( ) ) ) );
    DX_CHECK_RESULT( m_context->GraphicsCommandQueueFence->SetName( L"Graphics Command Queue Fence" ) );

    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( m_context->ComputeCommandQueue.put( ) ) ) );
    DX_CHECK_RESULT( m_context->ComputeCommandQueue->SetName( L"Compute Command Queue" ) );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_context->ComputeCommandQueueFence.put( ) ) ) );
    DX_CHECK_RESULT( m_context->ComputeCommandQueueFence->SetName( L"Compute Command Queue Fence" ) );

    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( m_context->CopyCommandQueue.put( ) ) ) );
    DX_CHECK_RESULT( m_context->CopyCommandQueue->SetName( L"Copy Command Queue" ) );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_context->CopyCommandQueueFence.put( ) ) ) );
    DX_CHECK_RESULT( m_context->CopyCommandQueueFence->SetName( L"Copy Command Queue Fence" ) );

    for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++ )
    {
        m_context->CpuDescriptorHeaps[ i ] = std::make_unique<DX12DescriptorHeap>( m_context->D3DDevice.get( ), static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>( i ), false );
    }

    m_context->RtvDescriptorHeap                    = std::make_unique<DX12DescriptorHeap>( m_context->D3DDevice.get( ), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false );
    m_context->DsvDescriptorHeap                    = std::make_unique<DX12DescriptorHeap>( m_context->D3DDevice.get( ), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false );
    m_context->ShaderVisibleCbvSrvUavDescriptorHeap = std::make_unique<DX12DescriptorHeap>( m_context->D3DDevice.get( ), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true );
    m_context->ShaderVisibleSamplerDescriptorHeap   = std::make_unique<DX12DescriptorHeap>( m_context->D3DDevice.get( ), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true );

    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS( m_context->CopyCommandListAllocator.put( ) ) ) );
    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_COPY, m_context->CopyCommandListAllocator.get( ), nullptr,
                                                              IID_PPV_ARGS( m_context->CopyCommandList.put( ) ) ) );
    DX_CHECK_RESULT( m_context->CopyCommandList->Close( ) );

    D3D12MA::ALLOCATOR_DESC allocatorDesc = { };
    allocatorDesc.pDevice                 = m_context->D3DDevice.get( );
    allocatorDesc.pAdapter                = m_context->Adapter.get( );
    allocatorDesc.Flags                   = D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED | D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;

    DX_CHECK_RESULT( D3D12MA::CreateAllocator( &allocatorDesc, m_context->DX12MemoryAllocator.put( ) ) );

    m_selectedDeviceInfo.Constants.ConstantBufferAlignment   = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    m_selectedDeviceInfo.Constants.BufferTextureAlignment    = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
    m_selectedDeviceInfo.Constants.BufferTextureRowAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
    m_context->SelectedDeviceInfo                            = m_selectedDeviceInfo;
}

void DX12LogicalDevice::WaitIdle( )
{
    DX_CHECK_RESULT( m_context->GraphicsCommandQueue->Signal( m_context->GraphicsCommandQueueFence.get( ), 1 ) );
    DX_CHECK_RESULT( m_context->ComputeCommandQueue->Signal( m_context->ComputeCommandQueueFence.get( ), 1 ) );
    DX_CHECK_RESULT( m_context->CopyCommandQueue->Signal( m_context->CopyCommandQueueFence.get( ), 1 ) );

    auto waitForFence = []( ID3D12Fence *fence )
    {
        Wrappers::Event eventHandle;
        eventHandle.Attach( CreateEventEx( nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE ) );
        DX_CHECK_RESULT( fence->SetEventOnCompletion( 1, eventHandle.Get( ) ) );
        WaitForSingleObjectEx( eventHandle.Get( ), INFINITE, FALSE );
    };

    waitForFence( m_context->GraphicsCommandQueueFence.get( ) );
    waitForFence( m_context->ComputeCommandQueueFence.get( ) );
    waitForFence( m_context->CopyCommandQueueFence.get( ) );
}

ICommandQueue *DX12LogicalDevice::CreateCommandQueue( const CommandQueueDesc &desc )
{
    return new DX12CommandQueue( m_context.get( ), desc );
}

ICommandListPool *DX12LogicalDevice::CreateCommandListPool( const CommandListPoolDesc &poolDesc )
{
    return new DX12CommandListPool( m_context.get( ), poolDesc );
}

IPipeline *DX12LogicalDevice::CreatePipeline( const PipelineDesc &pipelineDesc )
{
    return new DX12Pipeline( m_context.get( ), pipelineDesc );
}

ISwapChain *DX12LogicalDevice::CreateSwapChain( const SwapChainDesc &swapChainDesc )
{
    return new DX12SwapChain( m_context.get( ), swapChainDesc );
}

IRootSignature *DX12LogicalDevice::CreateRootSignature( const RootSignatureDesc &rootSignatureDesc )
{
    return new DX12RootSignature( m_context.get( ), rootSignatureDesc );
}

IInputLayout *DX12LogicalDevice::CreateInputLayout( const InputLayoutDesc &inputLayoutDesc )
{
    return new DX12InputLayout( inputLayoutDesc );
}

IResourceBindGroup *DX12LogicalDevice::CreateResourceBindGroup( const ResourceBindGroupDesc &descriptorTableDesc )
{
    return new DX12ResourceBindGroup( m_context.get( ), descriptorTableDesc );
}

IFence *DX12LogicalDevice::CreateFence( )
{
    return new DX12Fence( m_context.get( ) );
}

ISemaphore *DX12LogicalDevice::CreateSemaphore( )
{
    return new DX12Semaphore( m_context.get( ) );
}

IBufferResource *DX12LogicalDevice::CreateBufferResource( const BufferDesc &bufferDesc )
{
    return new DX12BufferResource( m_context.get( ), bufferDesc );
}

ITextureResource *DX12LogicalDevice::CreateTextureResource( const TextureDesc &textureDesc )
{
    return new DX12TextureResource( m_context.get( ), textureDesc );
}

ISampler *DX12LogicalDevice::CreateSampler( const SamplerDesc &samplerDesc )
{
    return new DX12Sampler( m_context.get( ), samplerDesc );
}

ITopLevelAS *DX12LogicalDevice::CreateTopLevelAS( const TopLevelASDesc &createDesc )
{
    return new DX12TopLevelAS( m_context.get( ), createDesc );
}

IBottomLevelAS *DX12LogicalDevice::CreateBottomLevelAS( const BottomLevelASDesc &createDesc )
{
    return new DX12BottomLevelAS( m_context.get( ), createDesc );
}

IShaderBindingTable *DX12LogicalDevice::CreateShaderBindingTable( const ShaderBindingTableDesc &createDesc )
{
    return new DX12ShaderBindingTable( m_context.get( ), createDesc );
}

ILocalRootSignature *DX12LogicalDevice::CreateLocalRootSignature( const LocalRootSignatureDesc &createDesc )
{
    return new DX12LocalRootSignature( m_context.get( ), createDesc );
}

IShaderLocalData *DX12LogicalDevice::CreateShaderLocalData( const ShaderLocalDataDesc &createDesc )
{
    return new DX12ShaderLocalData( m_context.get( ), createDesc );
}

bool DX12LogicalDevice::IsDeviceLost( )
{
    return m_context->IsDeviceLost;
}
