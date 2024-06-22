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

#include <DenOfIzGraphics/Backends/DirectX12/DX12SwapChain.h>

using namespace DenOfIz;
using namespace Microsoft::WRL;

DX12SwapChain::DX12SwapChain(DX12Context *context, const SwapChainCreateInfo &swapChainCreateInfo) : m_context(context), m_swapChainCreateInfo(swapChainCreateInfo)
{
    m_swapChainCreateInfo.Width = std::max(1u, m_swapChainCreateInfo.Width);
    m_swapChainCreateInfo.Height = std::max(1u, m_swapChainCreateInfo.Height);
    CreateSwapChain();
}

void DX12SwapChain::CreateSwapChain()
{
    GraphicsWindowSurface surface = m_context->Window->GetSurface();
    if ( m_swapChainCreateInfo.Width != surface.Width || m_swapChainCreateInfo.Height != surface.Height )
    {
        DLOG(INFO) << "DX12SwapChain" << "Swap chain size does not match window size. This could be intentional";
    }

    HWND hwnd = m_context->Window->GetNativeHandle();

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_swapChainCreateInfo.Width;
    swapChainDesc.Height = m_swapChainCreateInfo.Height;
    swapChainDesc.Format = DX12EnumConverter::ConvertImageFormat(m_swapChainCreateInfo.BackBufferFormat);
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = m_context->BackBufferCount;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = (m_context->SelectedDeviceInfo.Capabilities.Tearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
    fsSwapChainDesc.Windowed = TRUE;

    // Create a swap chain for the window.
    wil::com_ptr<IDXGISwapChain1> swapChain;

    THROW_IF_FAILED(m_context->DXGIFactory->CreateSwapChainForHwnd(m_context->GraphicsCommandQueue.get(), hwnd, &swapChainDesc, &fsSwapChainDesc, nullptr, swapChain.put()));
    THROW_IF_FAILED(swapChain->QueryInterface(IID_PPV_ARGS(m_swapChain.put())));
    THROW_IF_FAILED(m_context->DXGIFactory->MakeWindowAssociation(hwnd, DXGI_MWA_VALID));
    //    swapChain->Release();

    m_renderTargets.resize(m_context->BackBufferCount);
    m_renderTargetCpuHandles.resize(m_context->BackBufferCount);

    for ( uint32_t i = 0; i < m_context->BackBufferCount; i++ )
    {
        wil::com_ptr<ID3D12Resource2> buffer;
        THROW_IF_FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&buffer)));

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = swapChainDesc.Format;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        m_renderTargetCpuHandles[ i ] = m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ]->GetNextCPUHandleOffset(1);
        m_renderTargets[ i ] = std::make_unique<DX12ImageResource>(buffer.get(), m_renderTargetCpuHandles[ i ]);
        m_context->D3DDevice->CreateRenderTargetView(m_renderTargets[ i ]->GetResource(), &rtvDesc, m_renderTargetCpuHandles[ i ]);
    }

    SetColorSpace();
    CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

    DXGI_FORMAT depthBufferFormat = DX12EnumConverter::ConvertImageFormat(m_swapChainCreateInfo.DepthBufferFormat);
    D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(depthBufferFormat, m_swapChainCreateInfo.Width, m_swapChainCreateInfo.Height,
                                                                        1, // This depth stencil view has only one texture.
                                                                        1 // Use a single mipmap level.
    );
    depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = depthBufferFormat;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    THROW_IF_FAILED(m_context->D3DDevice->CreateCommittedResource(&depthHeapProperties, D3D12_HEAP_FLAG_NONE, &depthStencilDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                                  &depthOptimizedClearValue, IID_PPV_ARGS(m_depthStencil.put())));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthBufferFormat;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

    m_depthStencilCpuHandle = m_context->CpuDescriptorHeaps[ D3D12_DESCRIPTOR_HEAP_TYPE_DSV ]->GetNextCPUHandleOffset(1);
    m_context->D3DDevice->CreateDepthStencilView(m_depthStencil.get(), &dsvDesc, m_depthStencilCpuHandle);
}

void DX12SwapChain::SetColorSpace()
{
    m_colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    bool isDisplayHDR10 = false;

#if defined(NTDDI_WIN10_RS2)
    wil::com_ptr<IDXGIOutput> output;
    if ( SUCCEEDED(m_swapChain->GetContainingOutput(output.put())) )
    {
        wil::com_ptr<IDXGIOutput6> output6 = output.query<IDXGIOutput6>();
        if ( output6 )
        {
            DXGI_OUTPUT_DESC1 desc;
            THROW_IF_FAILED(output6->GetDesc1(&desc));
            if ( desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 )
            {
                isDisplayHDR10 = true;
            }
        }
    }
#endif

    if ( m_context->SelectedDeviceInfo.Capabilities.HDR && isDisplayHDR10 )
    {
        switch ( m_swapChainCreateInfo.BackBufferFormat )
        {
        case ImageFormat::R10G10B10A2Unorm:
            // The application creates the HDR10 signal.
            m_colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
            break;
        case ImageFormat::R16G16B16A16Float:
            // The system creates the HDR10 signal; application uses linear values.
            m_colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
            break;
        default:
            break;
        }
    }

    UINT colorSpaceSupport = 0;
    if ( SUCCEEDED(m_swapChain->CheckColorSpaceSupport(m_colorSpace, &colorSpaceSupport)) && (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) )
    {
        THROW_IF_FAILED(m_swapChain->SetColorSpace1(m_colorSpace));
    }
}

uint32_t DX12SwapChain::AcquireNextImage(ISemaphore *imageAvailableSemaphore)
{
    uint32_t index = m_swapChain->GetCurrentBackBufferIndex();
    if ( imageAvailableSemaphore )
    {
        imageAvailableSemaphore->Notify();
    }
    return index;
}

void DX12SwapChain::Resize(uint32_t width, uint32_t height)
{
    m_swapChainCreateInfo.Width = width;
    m_swapChainCreateInfo.Height = height;

    HRESULT hr = m_swapChain->ResizeBuffers(m_swapChainCreateInfo.BufferCount, width, height, DX12EnumConverter::ConvertImageFormat(m_swapChainCreateInfo.BackBufferFormat),
                                            m_context->SelectedDeviceInfo.Capabilities.Tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);

    if ( hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET )
    {
        DLOG(INFO) << "DX12SwapChain" <<
            std::format("Device Lost on ResizeBuffers: Reason code 0x{}" , ((hr == DXGI_ERROR_DEVICE_REMOVED) ? m_context->D3DDevice->GetDeviceRemovedReason() : hr));

        m_context->IsDeviceLost = true;
        return;
    }

    THROW_IF_FAILED(hr);
}

Viewport DX12SwapChain::GetViewport()
{
    return Viewport {
        0.0f,
        0.0f,
        static_cast<float>(m_swapChainCreateInfo.Width),
        static_cast<float>(m_swapChainCreateInfo.Height),
    };
}

ITextureResource *DX12SwapChain::GetRenderTarget(uint32_t frame) { return m_renderTargets[ frame ].get(); }

ImageFormat DX12SwapChain::GetPreferredFormat() { return ImageFormat::R8Unorm; }

DX12SwapChain::~DX12SwapChain()
{
    //    for ( uint32_t i = 0; i < m_context->BackBufferCount; i++ )
    //    {
    //        m_renderTargets[ i ].reset();
    //    }
    //    DX_SAFE_RELEASE(m_depthStencil);
    //    DX_SAFE_RELEASE(m_swapChain);
}
