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

#include <DenOfIzGraphics/Backends/DirectX12/DX12Semaphore.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12SwapChain.h>

using namespace DenOfIz;
using namespace Microsoft::WRL;

DX12SwapChain::DX12SwapChain( DX12Context *context, const SwapChainDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_desc.Width  = std::max( 1u, m_desc.Width );
    m_desc.Height = std::max( 1u, m_desc.Height );
    m_viewport    = Viewport( 0.0f, 0.0f, static_cast<float>( m_desc.Width ), static_cast<float>( m_desc.Height ) );
    CreateSwapChain( );
}

void DX12SwapChain::CreateSwapChain( )
{
    DZ_NOT_NULL( m_desc.WindowHandle );
    DZ_NOT_NULL( m_desc.CommandQueue );
    m_commandQueue = static_cast<DX12CommandQueue *>( m_desc.CommandQueue );

    if ( GraphicsWindowSurface surface = m_desc.WindowHandle->GetSurface( ); m_desc.Width != surface.Width || m_desc.Height != surface.Height )
    {
        DLOG( INFO ) << "Swap chain size does not match window size. This could be intentional";
    }

    HWND hwnd = m_desc.WindowHandle->GetNativeHandle( );

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { };
    swapChainDesc.Width                 = m_desc.Width;
    swapChainDesc.Height                = m_desc.Height;
    swapChainDesc.Format                = DX12EnumConverter::ConvertFormat( m_desc.BackBufferFormat );
    swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.BufferCount           = m_desc.NumBuffers;
    swapChainDesc.SampleDesc.Count      = 1;
    swapChainDesc.SampleDesc.Quality    = 0;
    swapChainDesc.Scaling               = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode             = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags                 = m_context->SelectedDeviceInfo.Capabilities.Tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { };
    fsSwapChainDesc.Windowed                        = TRUE;

    // Create a swap chain for the window.
    wil::com_ptr<IDXGISwapChain1> swapChain;

    DX_CHECK_RESULT( m_context->DXGIFactory->CreateSwapChainForHwnd( m_commandQueue->GetCommandQueue( ), hwnd, &swapChainDesc, &fsSwapChainDesc, nullptr, swapChain.put( ) ) );
    DX_CHECK_RESULT( swapChain->QueryInterface( IID_PPV_ARGS( m_swapChain.put( ) ) ) );
    DX_CHECK_RESULT( m_context->DXGIFactory->MakeWindowAssociation( hwnd, DXGI_MWA_VALID ) );

    m_renderTargets.resize( m_desc.NumBuffers );
    m_renderTargetCpuHandles.resize( m_desc.NumBuffers );
    m_buffers.resize( m_desc.NumBuffers );
    for ( uint32_t i = 0; i < m_desc.NumBuffers; i++ )
    {
        DX_CHECK_RESULT( m_swapChain->GetBuffer( i, IID_PPV_ARGS( &m_buffers.at( i ) ) ) );

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = { };
        rtvDesc.Format                        = swapChainDesc.Format;
        rtvDesc.ViewDimension                 = D3D12_RTV_DIMENSION_TEXTURE2D;

        m_renderTargetCpuHandles[ i ] = m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ]->GetNextHandle( 1 ).Cpu;
        auto buffer                   = m_buffers.at( i ).get( );
        buffer->SetName( L"SwapChain Buffer" );
        m_renderTargets[ i ] = std::make_unique<DX12TextureResource>( buffer, m_renderTargetCpuHandles[ i ] );
        m_context->D3DDevice->CreateRenderTargetView( m_renderTargets[ i ]->Resource( ), &rtvDesc, m_renderTargetCpuHandles[ i ] );
    }

    SetColorSpace( );
    CD3DX12_HEAP_PROPERTIES depthHeapProperties( D3D12_HEAP_TYPE_DEFAULT );

    DXGI_FORMAT         depthBufferFormat = DX12EnumConverter::ConvertFormat( m_desc.DepthBufferFormat );
    D3D12_RESOURCE_DESC depthStencilDesc  = CD3DX12_RESOURCE_DESC::Tex2D( depthBufferFormat, m_desc.Width, m_desc.Height,
                                                                          1, // This depth stencil view has only one texture.
                                                                          1  // Use a single mipmap level.
     );
    depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthOptimizedClearValue    = { };
    depthOptimizedClearValue.Format               = depthBufferFormat;
    depthOptimizedClearValue.DepthStencil.Depth   = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommittedResource( &depthHeapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                                    &depthOptimizedClearValue, IID_PPV_ARGS( m_depthStencil.put( ) ) ) );

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = { };
    dsvDesc.Format                        = depthBufferFormat;
    dsvDesc.ViewDimension                 = D3D12_DSV_DIMENSION_TEXTURE2D;

    m_depthStencilCpuHandle = m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_DSV ]->GetNextHandle( 1 ).Cpu;
    m_context->D3DDevice->CreateDepthStencilView( m_depthStencil.get( ), &dsvDesc, m_depthStencilCpuHandle );
}

void DX12SwapChain::SetColorSpace( )
{
    m_colorSpace        = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    bool isDisplayHDR10 = false;

#if defined( NTDDI_WIN10_RS2 )
    if ( wil::com_ptr<IDXGIOutput> output; SUCCEEDED( m_swapChain->GetContainingOutput( output.put( ) ) ) )
    {
        if ( const wil::com_ptr<IDXGIOutput6> output6 = output.query<IDXGIOutput6>( ) )
        {
            DXGI_OUTPUT_DESC1 desc;
            DX_CHECK_RESULT( output6->GetDesc1( &desc ) );
            if ( desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 )
            {
                isDisplayHDR10 = true;
            }
        }
    }
#endif

    if ( m_context->SelectedDeviceInfo.Capabilities.HDR && isDisplayHDR10 )
    {
        switch ( m_desc.BackBufferFormat )
        {
        case Format::R10G10B10A2Unorm:
            // The application creates the HDR10 signal.
            m_colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
            break;
        case Format::R16G16B16A16Float:
            // The system creates the HDR10 signal; application uses linear values.
            m_colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
            break;
        default:
            break;
        }
    }

    UINT colorSpaceSupport = 0;
    if ( SUCCEEDED( m_swapChain->CheckColorSpaceSupport( m_colorSpace, &colorSpaceSupport ) ) && colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT )
    {
        DX_CHECK_RESULT( m_swapChain->SetColorSpace1( m_colorSpace ) );
    }
}

uint32_t DX12SwapChain::AcquireNextImage( ISemaphore *imageAvailableSemaphore )
{
    return m_swapChain->GetCurrentBackBufferIndex( );
}

PresentResult DX12SwapChain::Present( const PresentDesc &desc )
{
    uint32_t flags        = 0;
    uint32_t syncInterval = 1;

    if ( m_context->SelectedDeviceInfo.Capabilities.Tearing )
    {
        syncInterval = 0;
        flags |= DXGI_PRESENT_ALLOW_TEARING;
    }

    if ( const HRESULT hr = m_swapChain->Present( syncInterval, flags ); SUCCEEDED( hr ) )
    {
        return PresentResult::Success;
    }

    return PresentResult::DeviceLost;
}

void DX12SwapChain::Resize( const uint32_t width, const uint32_t height )
{
    if ( width == 0 || height == 0 )
    {
        DLOG( WARNING ) << "Cannot resize swap chain to zero dimensions";
        return;
    }

    m_commandQueue->WaitIdle( );
    m_desc.Width  = width;
    m_desc.Height = height;
    m_viewport    = Viewport( 0.0f, 0.0f, static_cast<float>( m_desc.Width ), static_cast<float>( m_desc.Height ) );

    for ( auto &buffer : m_buffers )
    {
        buffer.reset( );
    }
    for ( auto &rt : m_renderTargets )
    {
        rt.reset( );
    }

    m_depthStencil.reset( );

    HRESULT hr = m_swapChain->ResizeBuffers( m_desc.NumBuffers, width, height, DX12EnumConverter::ConvertFormat( m_desc.BackBufferFormat ),
                                             m_context->SelectedDeviceInfo.Capabilities.Tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u );
    if ( hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET )
    {
        DLOG( ERROR ) << std::format( "Device Lost on ResizeBuffers: Reason code 0x{:x}", hr == DXGI_ERROR_DEVICE_REMOVED ? m_context->D3DDevice->GetDeviceRemovedReason( ) : hr );
        m_context->IsDeviceLost = true;
        return;
    }
    DX_CHECK_RESULT( hr );
    m_renderTargets.resize( m_desc.NumBuffers );
    m_renderTargetCpuHandles.resize( m_desc.NumBuffers );
    m_buffers.resize( m_desc.NumBuffers );

    for ( uint32_t i = 0; i < m_desc.NumBuffers; i++ )
    {
        DX_CHECK_RESULT( m_swapChain->GetBuffer( i, IID_PPV_ARGS( &m_buffers.at( i ) ) ) );
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = { };
        rtvDesc.Format                        = DX12EnumConverter::ConvertFormat( m_desc.BackBufferFormat );
        rtvDesc.ViewDimension                 = D3D12_RTV_DIMENSION_TEXTURE2D;

        m_renderTargetCpuHandles[ i ] = m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ]->GetNextHandle( 1 ).Cpu;
        auto buffer                   = m_buffers.at( i ).get( );
        buffer->SetName( L"SwapChain Buffer" );
        m_renderTargets[ i ] = std::make_unique<DX12TextureResource>( buffer, m_renderTargetCpuHandles[ i ] );
        m_context->D3DDevice->CreateRenderTargetView( m_renderTargets[ i ]->Resource( ), &rtvDesc, m_renderTargetCpuHandles[ i ] );
    }

    const CD3DX12_HEAP_PROPERTIES depthHeapProperties( D3D12_HEAP_TYPE_DEFAULT );

    const DXGI_FORMAT   depthBufferFormat = DX12EnumConverter::ConvertFormat( m_desc.DepthBufferFormat );
    D3D12_RESOURCE_DESC depthStencilDesc  = CD3DX12_RESOURCE_DESC::Tex2D( depthBufferFormat, width, height, 1, 1 );
    depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthOptimizedClearValue    = { };
    depthOptimizedClearValue.Format               = depthBufferFormat;
    depthOptimizedClearValue.DepthStencil.Depth   = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    DX_CHECK_RESULT( m_context->D3DDevice->CreateCommittedResource( &depthHeapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                                    &depthOptimizedClearValue, IID_PPV_ARGS( m_depthStencil.put( ) ) ) );

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = { };
    dsvDesc.Format                        = depthBufferFormat;
    dsvDesc.ViewDimension                 = D3D12_DSV_DIMENSION_TEXTURE2D;

    m_depthStencilCpuHandle = m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_DSV ]->GetNextHandle( 1 ).Cpu;
    m_context->D3DDevice->CreateDepthStencilView( m_depthStencil.get( ), &dsvDesc, m_depthStencilCpuHandle );
}

const Viewport &DX12SwapChain::GetViewport( )
{
    return m_viewport;
}

ITextureResource *DX12SwapChain::GetRenderTarget( const uint32_t image )
{
    return m_renderTargets[ image ].get( );
}

Format DX12SwapChain::GetPreferredFormat( )
{
    return Format::R8Unorm;
}

IDXGISwapChain4 *DX12SwapChain::GetSwapChain( ) const
{
    return m_swapChain.get( );
}

DX12SwapChain::~DX12SwapChain( ) = default;
