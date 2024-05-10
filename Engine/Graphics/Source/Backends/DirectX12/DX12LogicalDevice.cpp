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

using namespace DenOfIz;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

DX12LogicalDevice::DX12LogicalDevice()
{
}

DX12LogicalDevice::~DX12LogicalDevice()
{
    WaitIdle();
}

void DX12LogicalDevice::CreateDevice( SDL_Window *window )
{

#if defined(DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        if ( SUCCEEDED( D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())) ) )
        {
            debugController->EnableDebugLayer();
        }
        else
        {
            LOG(Verbosity::Warning, "DX12Device", "WARNING: Direct3D Debug Device is not available" );
        }

        ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
        if ( SUCCEEDED( DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf())) ) )
        {
            m_dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

            dxgiInfoQueue->SetBreakOnSeverity( DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true );
            dxgiInfoQueue->SetBreakOnSeverity( DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true );

            DXGI_INFO_QUEUE_MESSAGE_ID hide[ ] = {
                80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */, };
            DXGI_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof( hide );
            filter.DenyList.pIDList = hide;
            dxgiInfoQueue->AddStorageFilterEntries( DXGI_DEBUG_DXGI, &filter );
        }
    }
#endif

    DX_CHECK_RESULT( CreateDXGIFactory2( m_dxgiFactoryFlags, IID_PPV_ARGS( m_context->DXGIFactory.ReleaseAndGetAddressOf() ) ) );

    // Determines whether tearing support is available for fullscreen borderless windows.
    if ( m_options & c_AllowTearing )
    {
        BOOL allowTearing = FALSE;

        ComPtr<IDXGIFactory5> factory5;
        HRESULT hr = m_context->DXGIFactory.As( &factory5 );
        if ( SUCCEEDED( hr ) )
        {
            hr = factory5->CheckFeatureSupport( DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing) );
        }

        if ( FAILED( hr ) || !allowTearing )
        {
            m_options &= ~c_AllowTearing;
#ifdef DEBUG
            LOG(Verbosity::Warning, "DX12Device", "WARNING: Variable refresh rate displays not supported" );
#endif
        }
    }

    ComPtr<IDXGIAdapter1> adapter;
    GetAdapter( adapter.GetAddressOf() );

    // Create the DX12 API device object.
    DX_CHECK_RESULT( D3D12CreateDevice( adapter.Get(), m_d3dMinFeatureLevel, IID_PPV_ARGS( m_context->D3DDevice.ReleaseAndGetAddressOf() ) ) );

    m_context->D3DDevice->SetName( L"DeviceResources" );

    // Confirm the device supports DXR.
    D3D12_FEATURE_DATA_D3D12_OPTIONS5 opts = {};
    if ( FAILED( m_context->D3DDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &opts, sizeof(opts)) ) || opts.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED )
    {
        OutputDebugStringA( "DirectX Raytracing support not found.\n" );

    }

    // Confirm the device supports Shader Model 6.3 or better.
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_3 };
    if ( FAILED( m_context->D3DDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)) ) || shaderModel.HighestShaderModel <
        D3D_SHADER_MODEL_6_3 )
    {
        OutputDebugStringA( "ERROR: Requires Shader Model 6.3 or better support.\n" );
        throw std::exception( "Requires Shader Model 6.3 or better support" );
    }

#ifndef NDEBUG
    // Configure debug device (if active).
    ComPtr<ID3D12InfoQueue> d3dInfoQueue;
    if ( SUCCEEDED( m_context->D3DDevice.As(&d3dInfoQueue) ) )
    {
#ifdef _DEBUG
        d3dInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, true );
        d3dInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, true );
#endif
        D3D12_MESSAGE_ID hide[ ] = { D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE, D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
                                     // Workarounds for debug layer issues on hybrid-graphics systems
                                     D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE, D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE, };
        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = _countof( hide );
        filter.DenyList.pIDList = hide;
        d3dInfoQueue->AddStorageFilterEntries( &filter );
    }
#endif

    // Determine maximum supported feature level for this device
    static constexpr D3D_FEATURE_LEVEL s_featureLevels[ ] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, };

    D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels = { _countof( s_featureLevels ), s_featureLevels, D3D_FEATURE_LEVEL_11_0 };

    const HRESULT hr = m_context->D3DDevice->CheckFeatureSupport( D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels) );
    if ( SUCCEEDED( hr ) )
    {
        D3D_FEATURE_LEVEL m_d3dFeatureLevel = featLevels.MaxSupportedFeatureLevel;
    }
    else
    {
        m_d3dFeatureLevel = m_d3dMinFeatureLevel;
    }

    // Create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( m_context->CommandQueue.ReleaseAndGetAddressOf() ) ) );

    m_context->CommandQueue->SetName( L"DeviceResources" );

    // Create descriptor heaps for render target views and depth stencil views.
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    rtvDescriptorHeapDesc.NumDescriptors = m_backBufferCount;
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    DX_CHECK_RESULT( m_context->D3DDevice->CreateDescriptorHeap( &rtvDescriptorHeapDesc, IID_PPV_ARGS( m_rtvDescriptorHeap.ReleaseAndGetAddressOf() ) ) );

    m_rtvDescriptorHeap->SetName( L"DeviceResources" );

    m_rtvDescriptorSize = m_context->D3DDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

    if ( m_depthBufferFormat != DXGI_FORMAT_UNKNOWN )
    {
        D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
        dsvDescriptorHeapDesc.NumDescriptors = 1;
        dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

        DX_CHECK_RESULT( m_context->D3DDevice->CreateDescriptorHeap( &dsvDescriptorHeapDesc, IID_PPV_ARGS( m_dsvDescriptorHeap.ReleaseAndGetAddressOf() ) ) );

        m_dsvDescriptorHeap->SetName( L"DeviceResources" );
    }

    // Create a command allocator for each back buffer that will be rendered to.
    for ( UINT n = 0; n < m_backBufferCount; n++ )
    {
        DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( m_commandAllocators[n].ReleaseAndGetAddressOf() ) ) );

        wchar_t name[ 25 ] = {};
        swprintf_s( name, L"Render target %u", n );
        m_commandAllocators[ n ]->SetName( name );
    }

    // Create a command list for recording graphics commands.
    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[ 0 ].Get(), nullptr,
        IID_PPV_ARGS( m_commandList.ReleaseAndGetAddressOf() ) ) );
    DX_CHECK_RESULT( m_commandList->Close() );

    m_commandList->SetName( L"DeviceResources" );

    // Create a fence for tracking GPU execution progress.
    DX_CHECK_RESULT( m_context->D3DDevice->CreateFence( m_fenceValues[ m_backBufferIndex ], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( m_fence.ReleaseAndGetAddressOf() ) ) );

    m_fenceValues[ m_backBufferIndex ]++;
    m_fence->SetName( L"DeviceResources" );

    m_fenceEvent.Attach( CreateEventEx( nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE ) );
    if ( !m_fenceEvent.IsValid() )
    {
        throw std::exception( "CreateEvent" );
    }
}

std::vector<PhysicalDeviceInfo> DX12LogicalDevice::ListPhysicalDevices()
{
}

void DX12LogicalDevice::LoadPhysicalDevice( const PhysicalDeviceInfo &device )
{
}

void DX12LogicalDevice::vWaitIdle()
{
}
