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

#include <DenOfIzGraphics/Backends/Interface/ISwapChain.h>
#include "DX12Context.h"
#include "DX12EnumConverter.h"
#include "DenOfIzGraphics/Backends/DirectX12/Resource/DX12ImageResource.h"

using namespace Microsoft::WRL;

namespace DenOfIz
{

    class DX12SwapChain : public ISwapChain
    {
    private:
        DX12Context *m_context;
        SwapChainCreateInfo m_swapChainCreateInfo;
        ComPtr<IDXGISwapChain4> m_swapChain;

        std::vector<std::unique_ptr<DX12ImageResource>> m_renderTargets;
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_renderTargetCpuHandles;
        ComPtr<ID3D12Resource> m_depthStencil;
        D3D12_CPU_DESCRIPTOR_HANDLE m_depthStencilCpuHandle;

        DXGI_COLOR_SPACE_TYPE m_colorSpace;

    public:
        DX12SwapChain(DX12Context *context, const SwapChainCreateInfo &swapChainCreateInfo);
        ~DX12SwapChain() override;

        IDXGISwapChain4 *GetSwapChain() { return m_swapChain.Get(); }

        uint32_t AcquireNextImage(ISemaphore *imageAvailableSemaphore) override;
        ImageFormat GetPreferredFormat() override;
        IImageResource *GetRenderTarget(uint32_t frame) override;
        Viewport GetViewport() override;
        void Resize(uint32_t width, uint32_t height) override;
        void CreateSwapChain();

        void SetColorSpace();
    };

} // namespace DenOfIz
