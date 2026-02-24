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

#pragma once

#include <memory>
#include <vector>
#include <webgpu/webgpu.h>
#include "DenOfIzGraphicsInternal/Backends/Interface/ISwapChain.h"
#include "WebGPUContext.h"
#include "WebGPUTexture.h"

using namespace DenOfIz;

class WebGPUSwapChain final : public ISwapChain
{
    WebGPUContext        *m_context;
    DenOfIz_SwapChainDesc m_desc;
    WGPUSurface           m_surface = nullptr;
    DenOfIz_Viewport      m_viewport{ };
    uint32_t              m_currentImageIndex = 0;

    std::vector<std::unique_ptr<WebGPUTexture>> m_renderTargets{ };
    std::unique_ptr<WebGPUTexture>              m_depthStencil{ };

#if DZ_WEBGPU_USE_DAWN_API
    WGPUSurfaceTexture m_currentSurfaceTexture{ };
    bool               m_surfaceTextureAcquired = false;
#else
    WGPUSwapChain      m_swapChain              = nullptr;
    WGPUTextureView    m_currentTextureView     = nullptr;
    WGPUTextureFormat  m_surfaceFormat          = WGPUTextureFormat_BGRA8Unorm;
#endif

public:
    WebGPUSwapChain( WebGPUContext *context, const DenOfIz_SwapChainDesc &desc );
    ~WebGPUSwapChain( ) override;

    DenOfIz_Format          GetPreferredFormat( ) override;
    uint32_t                AcquireNextImage( ) override;
    DenOfIz_PresentResult   Present( uint32_t imageIndex ) override;
    void                    Resize( uint32_t width, uint32_t height ) override;
    ITexture               *GetRenderTarget( uint32_t image ) override;
    const DenOfIz_Viewport &GetViewport( ) override;

private:
    void CreateSurface( );
    void ConfigureSwapChain( );
    void CreateDepthStencil( );
};
