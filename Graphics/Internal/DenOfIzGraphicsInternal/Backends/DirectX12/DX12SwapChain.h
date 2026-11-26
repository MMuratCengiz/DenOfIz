/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DX12CommandQueue.h"
#include "DX12Context.h"
#include "DX12Texture.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ISwapChain.h"

namespace DenOfIz
{

    class DX12SwapChain final : public ISwapChain
    {
        DX12Context                  *m_context      = nullptr;
        DX12CommandQueue             *m_commandQueue = nullptr;
        DenOfIz_SwapChainDesc         m_desc{ };
        wil::com_ptr<IDXGISwapChain4> m_swapChain = nullptr;

        std::vector<std::unique_ptr<DX12Texture>>  m_renderTargets{ };
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>   m_renderTargetCpuHandles{ };
        std::vector<wil::com_ptr<ID3D12Resource2>> m_buffers;
        std::unique_ptr<DX12Texture>               m_depthBuffer;

        DXGI_COLOR_SPACE_TYPE m_colorSpace{ };
        DenOfIz_Viewport      m_viewport{ };
        UINT m_swapChainFlags = 0;

    public:
        DX12SwapChain( DX12Context *context, const DenOfIz_SwapChainDesc &desc );
        ~DX12SwapChain( ) override;

        [[nodiscard]] IDXGISwapChain4 *GetSwapChain( ) const;

        DenOfIz_Result          AcquireNextImage( uint32_t &outImageIndex ) override;
        DenOfIz_Result          Present( ) override;
        DenOfIz_Format          GetPreferredFormat( ) override;
        ITexture               *GetRenderTarget( uint32_t image ) override;
        ITexture               *GetDepthBuffer( ) override;
        const DenOfIz_Viewport &GetViewport( ) override;
        const DenOfIz_SwapChainDesc &GetDesc( ) override;
        DenOfIz_Result          Resize( uint32_t width, uint32_t height ) override;
        void                    CreateSwapChain( );

        void SetColorSpace( );

    private:
        void CreateBackBuffers( );
        void ReleaseBackBuffers( );
        void NotifyDeviceRemoved( HRESULT hr, const char *operation ) const;
    };

} // namespace DenOfIz
