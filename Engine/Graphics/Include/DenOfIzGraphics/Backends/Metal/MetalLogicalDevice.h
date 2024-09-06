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

#include <DenOfIzCore/Common.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include "MetalContext.h"
#include "MetalBufferResource.h"
#include "MetalCommandListPool.h"
#include "MetalFence.h"
#include "MetalResourceBindGroup.h"
#include "MetalTextureResource.h"

namespace DenOfIz
{

    class MetalLogicalDevice final : public ILogicalDevice
    {
        std::unique_ptr<MetalContext> m_context;
    public:
         MetalLogicalDevice( );
        ~MetalLogicalDevice( ) override;

        // Override methods
        void                        CreateDevice( ) override;
        std::vector<PhysicalDevice> ListPhysicalDevices( ) override;
        void                        LoadPhysicalDevice( const PhysicalDevice &device ) override;
        bool                        IsDeviceLost( ) override
        {
            return false;
        }

        std::unique_ptr<ICommandListPool>   CreateCommandListPool( const CommandListPoolDesc &poolDesc ) override;
        std::unique_ptr<IPipeline>          CreatePipeline( const PipelineDesc &pipelineDesc ) override;
        std::unique_ptr<ISwapChain>         CreateSwapChain( const SwapChainDesc &swapChainDesc ) override;
        std::unique_ptr<IRootSignature>     CreateRootSignature( const RootSignatureDesc &rootSignatureDesc ) override;
        std::unique_ptr<IInputLayout>       CreateInputLayout( const InputLayoutDesc &inputLayoutDesc ) override;
        std::unique_ptr<IResourceBindGroup> CreateResourceBindGroup( const ResourceBindGroupDesc &descriptorTableDesc ) override;
        std::unique_ptr<IFence>             CreateFence( ) override;
        std::unique_ptr<ISemaphore>         CreateSemaphore( ) override;
        std::unique_ptr<IBufferResource>    CreateBufferResource(const BufferDesc &bufferDesc ) override;
        std::unique_ptr<ITextureResource>   CreateTextureResource( const TextureDesc &textureDesc ) override;
        std::unique_ptr<ISampler>           CreateSampler( const SamplerDesc &samplerDesc ) override;

        void WaitIdle( ) override;
        // --
    };

} // namespace DenOfIz
