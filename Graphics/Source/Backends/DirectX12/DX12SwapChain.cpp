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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12SwapChain.h"
#include <algorithm>
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12EnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Semaphore.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;
using namespace Microsoft::WRL;

DX12SwapChain::DX12SwapChain( DX12Context *context, const DenOfIz_SwapChainDesc &desc ) : m_context( context ), m_desc( desc )
{
    if ( m_desc.Width == 0 || m_desc.Height == 0 )
    {
        DenOfIz_GraphicsWindowSurface surface{ };
        DenOfIz_GraphicsWindowHandle_GetSurface( m_desc.WindowHandle, &surface );
        m_desc.Width  = surface.Width;
        m_desc.Height = surface.Height;
    }
    m_desc.Width  = std::max( 1u, m_desc.Width );
    m_desc.Height = std::max( 1u, m_desc.Height );
    m_desc.NumBuffers = std::min( std::max( 2u, m_desc.NumBuffers ), 16u );
    m_viewport        = DenOfIz_Viewport( 0.0f, 0.0f, static_cast<float>( m_desc.Width ), static_cast<float>( m_desc.Height ) );
    CreateSwapChain( );
}

void DX12SwapChain::CreateSwapChain( )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_desc.WindowHandle ) )
    {
        spdlog::error( "WindowHandle cannot be null" );
        return;
    }
    if ( !DENOFIZ_HANDLE_IS_VALID( m_desc.CommandQueue ) )
    {
        spdlog::error( "CommandQueue cannot be null" );
        return;
    }
    m_commandQueue = static_cast<DX12CommandQueue *>( DENOFIZ_FROM_HANDLE( ICommandQueue, m_desc.CommandQueue ) );

    DenOfIz_GraphicsWindowSurface surface{ };
    DenOfIz_GraphicsWindowHandle_GetSurface( m_desc.WindowHandle, &surface );
    if ( m_desc.Width != surface.Width || m_desc.Height != surface.Height )
    {
        spdlog::debug( "Swap chain size does not match window size. This could be intentional" );
    }

    uint32_t windowId = 0;
    DenOfIz_GraphicsWindowHandle_GetSDLWindowId( m_desc.WindowHandle, &windowId );
    SDL_Window *sdlWindow = SDL_GetWindowFromID( windowId );
    if ( sdlWindow == nullptr )
    {
        spdlog::critical( "Swap chain window no longer exists: {}", SDL_GetError( ) );
        return;
    }
    auto hwnd = static_cast<HWND>( SDL_GetPointerProperty( SDL_GetWindowProperties( sdlWindow ), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr ) );

    m_swapChainFlags = ( m_desc.PresentMode == DENOFIZ_PRESENT_MODE_IMMEDIATE && m_context->SelectedDeviceInfo.Capabilities.Tearing ) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { };
    swapChainDesc.Width                 = m_desc.Width;
    swapChainDesc.Height                = m_desc.Height;
    swapChainDesc.Format                = DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.BackBufferFormat );
    swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
    swapChainDesc.BufferCount           = m_desc.NumBuffers;
    swapChainDesc.SampleDesc.Count      = 1;
    swapChainDesc.SampleDesc.Quality    = 0;
    swapChainDesc.Scaling               = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode             = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags                 = m_swapChainFlags;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { };
    fsSwapChainDesc.Windowed                        = TRUE;

    // Create a swap chain for the window.
    wil::com_ptr<IDXGISwapChain1> swapChain;

    HRESULT hr = m_context->DXGIFactory->CreateSwapChainForHwnd( m_commandQueue->GetCommandQueue( ), hwnd, &swapChainDesc, &fsSwapChainDesc, nullptr, swapChain.put( ) );
    if ( FAILED( hr ) || swapChain.get( ) == nullptr )
    {
        spdlog::critical( "Failed to create swap chain: 0x{:08x}", static_cast<uint32_t>( hr ) );
        NotifyDeviceRemoved( hr, "CreateSwapChainForHwnd" );
        return;
    }
    hr = swapChain->QueryInterface( IID_PPV_ARGS( m_swapChain.put( ) ) );
    if ( FAILED( hr ) || m_swapChain.get( ) == nullptr )
    {
        spdlog::critical( "IDXGISwapChain4 not supported: 0x{:08x}", static_cast<uint32_t>( hr ) );
        return;
    }
    DX_CHECK_RESULT( m_context->DXGIFactory->MakeWindowAssociation( hwnd, DXGI_MWA_VALID ) );

    CreateBackBuffers( );
    SetColorSpace( );
}

void DX12SwapChain::CreateBackBuffers( )
{
    const DXGI_FORMAT backBufferFormat = DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.BackBufferFormat );

    m_renderTargets.resize( m_desc.NumBuffers );
    m_renderTargetCpuHandles.resize( m_desc.NumBuffers );
    m_buffers.resize( m_desc.NumBuffers );
    for ( uint32_t i = 0; i < m_desc.NumBuffers; i++ )
    {
        DX_CHECK_RESULT( m_swapChain->GetBuffer( i, IID_PPV_ARGS( &m_buffers.at( i ) ) ) );
        auto buffer = m_buffers.at( i ).get( );
        if ( buffer == nullptr )
        {
            spdlog::critical( "Failed to get swap chain buffer {}", i );
            continue;
        }

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = { };
        rtvDesc.Format                        = backBufferFormat;
        rtvDesc.ViewDimension                 = D3D12_RTV_DIMENSION_TEXTURE2D;

        m_renderTargetCpuHandles[ i ] = m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ]->AllocateDescriptors( 1 ).Cpu;
        buffer->SetName( L"SwapChain Buffer" );
        DenOfIz_TextureDesc backBufferDesc{ };
        backBufferDesc.Width     = m_desc.Width;
        backBufferDesc.Height    = m_desc.Height;
        backBufferDesc.Depth     = 1;
        backBufferDesc.ArraySize = 1;
        backBufferDesc.MipLevels = 1;
        backBufferDesc.Format    = m_desc.BackBufferFormat;
        if ( m_renderTargets[ i ] == nullptr )
        {
            m_renderTargets[ i ] = std::make_unique<DX12Texture>( buffer, m_renderTargetCpuHandles[ i ] );
        }
        m_renderTargets[ i ]->UpdateExternal( buffer, m_renderTargetCpuHandles[ i ], backBufferDesc );
        m_context->D3DDevice->CreateRenderTargetView( m_renderTargets[ i ]->Resource( ), &rtvDesc, m_renderTargetCpuHandles[ i ] );
    }

    if ( m_desc.DepthBufferFormat == DENOFIZ_FORMAT_UNDEFINED )
    {
        return;
    }
    if ( m_depthBuffer != nullptr && m_depthBuffer->GetWidth( ) == m_desc.Width && m_depthBuffer->GetHeight( ) == m_desc.Height )
    {
        return;
    }
    m_depthBuffer.reset( );

    DenOfIz_TextureDesc depthDesc{ };
    depthDesc.Width                 = m_desc.Width;
    depthDesc.Height                = m_desc.Height;
    depthDesc.Depth                 = 1;
    depthDesc.ArraySize             = 1;
    depthDesc.MipLevels             = 1;
    depthDesc.Format                = m_desc.DepthBufferFormat;
    depthDesc.Usage                 = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
    depthDesc.HeapType              = DENOFIZ_HEAP_TYPE_GPU;
    depthDesc.ClearDepthStencilHint = { 1.0f, 0.0f };
    depthDesc.DebugName             = DENOFIZ_STRING( "SwapChain Depth Buffer" );
    m_depthBuffer                   = std::make_unique<DX12Texture>( m_context, depthDesc );
}

void DX12SwapChain::ReleaseBackBuffers( )
{
    for ( auto &rt : m_renderTargets )
    {
        if ( rt != nullptr )
        {
            rt->UpdateExternal( nullptr, D3D12_CPU_DESCRIPTOR_HANDLE{ }, DenOfIz_TextureDesc{ } );
        }
    }
    for ( auto &buffer : m_buffers )
    {
        buffer.reset( );
    }

    for ( auto &cpuHandle : m_renderTargetCpuHandles )
    {
        if ( cpuHandle.ptr != 0 )
        {
            DescriptorHandle handle{ };
            handle.Cpu.ptr = cpuHandle.ptr;
            m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ]->FreeDescriptors( handle, 1 );
            cpuHandle.ptr = 0;
        }
    }
}

void DX12SwapChain::NotifyDeviceRemoved( const HRESULT hr, const char *operation ) const
{
    if ( hr != DXGI_ERROR_DEVICE_REMOVED && hr != DXGI_ERROR_DEVICE_RESET )
    {
        return;
    }
    if ( m_context->IsDeviceLost )
    {
        return;
    }
    m_context->IsDeviceLost = true;
    const HRESULT reason    = hr == DXGI_ERROR_DEVICE_REMOVED ? m_context->D3DDevice->GetDeviceRemovedReason( ) : hr;
    spdlog::critical( "Device lost during {}: 0x{:08x}", operation, static_cast<uint32_t>( reason ) );
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

    if ( m_desc.EnableHDR && m_context->SelectedDeviceInfo.Capabilities.HDR && isDisplayHDR10 )
    {
        switch ( m_desc.ColorSpace )
        {
        case DENOFIZ_COLOR_SPACE_HDR10:
            m_colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
            break;
        case DENOFIZ_COLOR_SPACE_SCRGB:
            m_colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
            break;
        case DENOFIZ_COLOR_SPACE_SRGB:
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

DenOfIz_Result DX12SwapChain::AcquireNextImage( uint32_t &outImageIndex )
{
    outImageIndex = 0;
    if ( m_context->IsDeviceLost )
    {
        return DENOFIZ_ERROR_DEVICE_LOST;
    }
    if ( m_swapChain.get( ) == nullptr )
    {
        return DENOFIZ_ERROR_INITIALIZATION;
    }
    DenOfIz_GraphicsWindowSurface surface{ };
    DenOfIz_GraphicsWindowHandle_GetSurface( m_desc.WindowHandle, &surface );
    if ( surface.Width == 0 || surface.Height == 0 )
    {
        return DENOFIZ_NOT_READY;
    }
    if ( surface.Width != m_desc.Width || surface.Height != m_desc.Height )
    {
        if ( const DenOfIz_Result resizeResult = Resize( surface.Width, surface.Height ); resizeResult != DENOFIZ_SUCCESS )
        {
            return resizeResult;
        }
    }
    outImageIndex = m_swapChain->GetCurrentBackBufferIndex( );
    return DENOFIZ_SUCCESS;
}

DenOfIz_Result DX12SwapChain::Present( )
{
    if ( m_context->IsDeviceLost )
    {
        return DENOFIZ_ERROR_DEVICE_LOST;
    }
    if ( m_swapChain.get( ) == nullptr )
    {
        return DENOFIZ_ERROR_INITIALIZATION;
    }

    const bool     tearing      = ( m_swapChainFlags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING ) != 0;
    const uint32_t syncInterval = m_desc.PresentMode == DENOFIZ_PRESENT_MODE_FIFO ? 1 : 0;
    const uint32_t flags        = tearing && syncInterval == 0 ? DXGI_PRESENT_ALLOW_TEARING : 0;

    const HRESULT hr = m_swapChain->Present( syncInterval, flags );
    if ( SUCCEEDED( hr ) )
    {
        return DENOFIZ_SUCCESS;
    }
    if ( hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET )
    {
        NotifyDeviceRemoved( hr, "Present" );
        return DENOFIZ_ERROR_DEVICE_LOST;
    }
    spdlog::error( "Present failed: 0x{:08x}", static_cast<uint32_t>( hr ) );
    return DENOFIZ_ERROR_UNKNOWN;
}

DenOfIz_Result DX12SwapChain::Resize( uint32_t width, uint32_t height )
{
    if ( m_context->IsDeviceLost )
    {
        return DENOFIZ_ERROR_DEVICE_LOST;
    }
    if ( m_swapChain.get( ) == nullptr )
    {
        return DENOFIZ_ERROR_INITIALIZATION;
    }
    if ( width == 0 || height == 0 )
    {
        DenOfIz_GraphicsWindowSurface surface{ };
        DenOfIz_GraphicsWindowHandle_GetSurface( m_desc.WindowHandle, &surface );
        width  = surface.Width;
        height = surface.Height;
    }
    if ( width == 0 || height == 0 )
    {
        return DENOFIZ_NOT_READY;
    }
    if ( width == m_desc.Width && height == m_desc.Height && !m_buffers.empty( ) && m_buffers[ 0 ].get( ) != nullptr )
    {
        return DENOFIZ_SUCCESS;
    }

    m_commandQueue->WaitIdle( );
    ReleaseBackBuffers( );

    const HRESULT hr = m_swapChain->ResizeBuffers( m_desc.NumBuffers, width, height, DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.BackBufferFormat ), m_swapChainFlags );
    if ( hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET )
    {
        NotifyDeviceRemoved( hr, "ResizeBuffers" );
        return DENOFIZ_ERROR_DEVICE_LOST;
    }
    if ( FAILED( hr ) )
    {
        spdlog::error( "ResizeBuffers to {}x{} failed: 0x{:08x}", width, height, static_cast<uint32_t>( hr ) );
        CreateBackBuffers( );
        return DENOFIZ_ERROR_UNKNOWN;
    }

    m_desc.Width  = width;
    m_desc.Height = height;
    m_viewport    = DenOfIz_Viewport( 0.0f, 0.0f, static_cast<float>( m_desc.Width ), static_cast<float>( m_desc.Height ) );
    CreateBackBuffers( );
    return DENOFIZ_SUCCESS;
}

const DenOfIz_Viewport &DX12SwapChain::GetViewport( )
{
    return m_viewport;
}

ITexture *DX12SwapChain::GetDepthBuffer( )
{
    return m_depthBuffer.get( );
}

const DenOfIz_SwapChainDesc &DX12SwapChain::GetDesc( )
{
    return m_desc;
}

ITexture *DX12SwapChain::GetRenderTarget( const uint32_t image )
{
    if ( image >= m_renderTargets.size( ) )
    {
        return nullptr;
    }
    return m_renderTargets[ image ].get( );
}

DenOfIz_Format DX12SwapChain::GetPreferredFormat( )
{
    return DENOFIZ_FORMAT_B8G8R8A8_UNORM;
}

IDXGISwapChain4 *DX12SwapChain::GetSwapChain( ) const
{
    return m_swapChain.get( );
}

DX12SwapChain::~DX12SwapChain( )
{
    if ( m_commandQueue != nullptr && !m_context->IsDeviceLost )
    {
        m_commandQueue->WaitIdle( );
    }
    ReleaseBackBuffers( );
    m_renderTargets.clear( );
    m_depthBuffer.reset( );
}
