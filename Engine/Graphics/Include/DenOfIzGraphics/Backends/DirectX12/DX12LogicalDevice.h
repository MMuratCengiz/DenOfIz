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
#ifdef BUILD_DX12

#include <DenOfIzCore/Common.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include "DX12BufferResource.h"
#include "DX12CommandListPool.h"
#include "DX12Context.h"
#include "DX12Fence.h"
#include "DX12ResourceBindGroup.h"
#include "DX12TextureResource.h"

#include <dxgidebug.h>

namespace DenOfIz
{

    class DX12LogicalDevice final : public ILogicalDevice
    {
    private:
        D3D_FEATURE_LEVEL            m_minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
        std::unique_ptr<DX12Context> m_context;
        wil::com_ptr<ID3D12Fence>    m_waitIdleFence;

    public:
         DX12LogicalDevice( );
        ~DX12LogicalDevice( ) override;

        // Override methods
        void                        CreateDevice( ) override;
        std::vector<PhysicalDevice> ListPhysicalDevices( ) override;
        void                        LoadPhysicalDevice( const PhysicalDevice &device ) override;
        bool                        IsDeviceLost( ) override
        {
            return m_context->IsDeviceLost;
        }

        std::unique_ptr<ICommandListPool>   CreateCommandListPool( const CommandListPoolDesc &poolDesc ) override;
        std::unique_ptr<IPipeline>          CreatePipeline( const PipelineDesc &pipelineDesc ) override;
        std::unique_ptr<ISwapChain>         CreateSwapChain( const SwapChainDesc &swapChainDesc ) override;
        std::unique_ptr<IRootSignature>     CreateRootSignature( const RootSignatureDesc &rootSignatureDesc ) override;
        std::unique_ptr<IInputLayout>       CreateInputLayout( const InputLayoutDesc &inputLayoutDesc ) override;
        std::unique_ptr<IResourceBindGroup> CreateResourceBindGroup( const ResourceBindGroupDesc &descriptorTableDesc ) override;
        std::unique_ptr<IFence>             CreateFence( ) override;
        std::unique_ptr<ISemaphore>         CreateSemaphore( ) override;
        std::unique_ptr<IBufferResource>    CreateBufferResource( const BufferDesc &bufferDesc ) override;
        std::unique_ptr<ITextureResource>   CreateTextureResource( const TextureDesc &textureDesc ) override;
        std::unique_ptr<ISampler>           CreateSampler( const SamplerDesc &samplerDesc ) override;

        void WaitIdle( ) override;
        // --
    private:
        void CreateDeviceInfo( IDXGIAdapter1 &adapter, PhysicalDevice &physicalDevice ) const;
    };

} // namespace DenOfIz

#endif
