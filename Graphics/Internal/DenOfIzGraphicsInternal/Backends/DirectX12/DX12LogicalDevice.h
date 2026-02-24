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

#include <deque>
#include "../Common/InternalAutoSync.h"
#include "DX12BindGroupLayout.h"
#include "DX12CommandListPool.h"
#include "DX12Context.h"
#include "DX12Fence.h"
#include "DenOfIzGraphics/Utilities/Common.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ILogicalDevice.h"

#ifdef _WIN32
#undef CreateSemaphore
#endif

namespace DenOfIz
{

    class DX12LogicalDevice final : public ILogicalDevice
    {
        D3D_FEATURE_LEVEL                   m_minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
        std::unique_ptr<DX12Context>        m_context;
        std::vector<DenOfIz_PhysicalDevice> m_physicalDevices;
        std::unique_ptr<InternalAutoSync>   m_autoSync;
        DenOfIz_LogicalDeviceDesc           m_deviceDesc;
        std::deque<std::string>             m_deviceNameStorage;

    public:
        DX12LogicalDevice( );
        ~DX12LogicalDevice( ) override;

        // Override methods
        void                        CreateDevice( const DenOfIz_LogicalDeviceDesc &desc = { } ) override;
        DenOfIz_PhysicalDeviceArray ListPhysicalDevices( ) override;
        void                        LoadPhysicalDevice( const DenOfIz_PhysicalDevice &device ) override;
        bool                        IsDeviceLost( ) override;

        ICommandQueue    *CreateCommandQueue( const DenOfIz_CommandQueueDesc &desc ) override;
        ICommandListPool *CreateCommandListPool( const DenOfIz_CommandListPoolDesc &poolDesc ) override;
        IPipeline        *CreatePipeline( const DenOfIz_PipelineDesc &pipelineDesc ) override;
        IPipelineCache   *CreatePipelineCache( const DenOfIz_PipelineCacheDesc &desc ) override;
        ISwapChain       *CreateSwapChain( const DenOfIz_SwapChainDesc &swapChainDesc ) override;
        IRootSignature   *CreateRootSignature( const DenOfIz_RootSignatureDesc &rootSignatureDesc ) override;
        IInputLayout     *CreateInputLayout( const DenOfIz_InputLayoutDesc &inputLayoutDesc ) override;
        IBindGroupLayout *CreateBindGroupLayout( const DenOfIz_BindGroupLayoutDesc &desc ) override;
        IBindGroup       *CreateBindGroup( const DenOfIz_BindGroupDesc &descriptorTableDesc ) override;
        IFence           *CreateFence( ) override;
        ISemaphore       *CreateSemaphore( ) override;
        IBuffer          *CreateBuffer( const DenOfIz_BufferDesc &bufferDesc ) override;
        ITexture         *CreateTexture( const DenOfIz_TextureDesc &textureDesc ) override;
        ISampler         *CreateSampler( const DenOfIz_SamplerDesc &samplerDesc ) override;
        IQueryPool       *CreateQueryPool( const DenOfIz_QueryPoolDesc &desc ) override;
        // RayTracing:
        ITopLevelAS         *CreateTopLevelAS( const DenOfIz_TopLevelASDesc &createDesc ) override;
        IBottomLevelAS      *CreateBottomLevelAS( const DenOfIz_BottomLevelASDesc &createDesc ) override;
        IShaderBindingTable *CreateShaderBindingTable( const DenOfIz_ShaderBindingTableDesc &createDesc ) override;
        ILocalRootSignature *CreateLocalRootSignature( const DenOfIz_LocalRootSignatureDesc &createDesc ) override;
        IShaderLocalData    *CreateShaderLocalData( const DenOfIz_ShaderLocalDataDesc &createDesc ) override;

        void WaitIdle( ) override;

        [[nodiscard]] DX12Context *GetContext( ) const
        {
            return m_context.get( );
        }
        // --
    private:
        void CreateDeviceInfo( IDXGIAdapter1 &adapter, DenOfIz_PhysicalDevice &physicalDevice );
    };

} // namespace DenOfIz

#endif
