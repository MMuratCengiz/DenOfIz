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

#include "DX12CommandListPool.h"
#include "DX12Context.h"
#include "DX12Fence.h"
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphics/Utilities/Common.h"

#ifdef _WIN32
#undef CreateSemaphore
#endif

namespace DenOfIz
{

    class DX12LogicalDevice final : public ILogicalDevice
    {
        D3D_FEATURE_LEVEL            m_minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
        std::unique_ptr<DX12Context> m_context;
        std::vector<PhysicalDevice>  m_physicalDevices;

    public:
        DX12LogicalDevice( );
        ~DX12LogicalDevice( ) override;

        // Override methods
        void                CreateDevice( ) override;
        PhysicalDeviceArray ListPhysicalDevices( ) override;
        void                LoadPhysicalDevice( const PhysicalDevice &device ) override;
        bool                IsDeviceLost( ) override;

        ICommandQueue      *CreateCommandQueue( const CommandQueueDesc &desc ) override;
        ICommandListPool   *CreateCommandListPool( const CommandListPoolDesc &poolDesc ) override;
        IPipeline          *CreatePipeline( const PipelineDesc &pipelineDesc ) override;
        ISwapChain         *CreateSwapChain( const SwapChainDesc &swapChainDesc ) override;
        IRootSignature     *CreateRootSignature( const RootSignatureDesc &rootSignatureDesc ) override;
        IInputLayout       *CreateInputLayout( const InputLayoutDesc &inputLayoutDesc ) override;
        IResourceBindGroup *CreateResourceBindGroup( const ResourceBindGroupDesc &descriptorTableDesc ) override;
        IFence             *CreateFence( ) override;
        ISemaphore         *CreateSemaphore( ) override;
        IBufferResource    *CreateBufferResource( const BufferDesc &bufferDesc ) override;
        ITextureResource   *CreateTextureResource( const TextureDesc &textureDesc ) override;
        ISampler           *CreateSampler( const SamplerDesc &samplerDesc ) override;
        // RayTracing:
        ITopLevelAS         *CreateTopLevelAS( const TopLevelASDesc &createDesc ) override;
        IBottomLevelAS      *CreateBottomLevelAS( const BottomLevelASDesc &createDesc ) override;
        IShaderBindingTable *CreateShaderBindingTable( const ShaderBindingTableDesc &createDesc ) override;
        ILocalRootSignature *CreateLocalRootSignature( const LocalRootSignatureDesc &createDesc ) override;
        IShaderLocalData    *CreateShaderLocalData( const ShaderLocalDataDesc &createDesc ) override;

        void WaitIdle( ) override;
        // --
    private:
        void CreateDeviceInfo( IDXGIAdapter1 &adapter, PhysicalDevice &physicalDevice ) const;
    };

} // namespace DenOfIz

#endif
