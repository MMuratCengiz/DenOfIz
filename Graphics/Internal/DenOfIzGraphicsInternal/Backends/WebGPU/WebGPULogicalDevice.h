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

#include <deque>
#include <memory>
#include <vector>
#include <webgpu/webgpu.h>
#include "DenOfIzGraphics/Utilities/Common.h"
#include "DenOfIzGraphicsInternal/Backends/Common/InternalAutoSync.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ILogicalDevice.h"
#include "WebGPUBindGroupLayout.h"
#include "WebGPUContext.h"

namespace DenOfIz
{
    class WebGPULogicalDevice final : public ILogicalDevice
    {
        std::unique_ptr<WebGPUContext>      m_context;
        std::vector<DenOfIz_PhysicalDevice> m_physicalDevices;
        std::unique_ptr<InternalAutoSync>   m_autoSync;
        DenOfIz_LogicalDeviceDesc           m_deviceDesc;
        std::deque<std::string>             m_deviceNameStorage;

    public:
        WebGPULogicalDevice( );
        ~WebGPULogicalDevice( ) override;

        void                        CreateDevice( const DenOfIz_LogicalDeviceDesc &desc = { } ) override;
        DenOfIz_PhysicalDeviceArray ListPhysicalDevices( ) override;
        void                        LoadPhysicalDevice( const DenOfIz_PhysicalDevice &device ) override;
        bool                        IsDeviceLost( ) override;
        void                        WaitIdle( ) override;

        ICommandQueue    *CreateCommandQueue( const DenOfIz_CommandQueueDesc &desc ) override;
        ICommandListPool *CreateCommandListPool( const DenOfIz_CommandListPoolDesc &desc ) override;
        IPipeline        *CreatePipeline( const DenOfIz_PipelineDesc &desc ) override;
        ISwapChain       *CreateSwapChain( const DenOfIz_SwapChainDesc &desc ) override;
        IRootSignature   *CreateRootSignature( const DenOfIz_RootSignatureDesc &desc ) override;
        IInputLayout     *CreateInputLayout( const DenOfIz_InputLayoutDesc &desc ) override;
        IBindGroupLayout *CreateBindGroupLayout( const DenOfIz_BindGroupLayoutDesc &desc ) override;
        IBindGroup       *CreateBindGroup( const DenOfIz_BindGroupDesc &desc ) override;
        IFence           *CreateFence( ) override;
        ISemaphore       *CreateSemaphore( ) override;
        IBuffer          *CreateBuffer( const DenOfIz_BufferDesc &desc ) override;
        ITexture         *CreateTexture( const DenOfIz_TextureDesc &desc ) override;
        ISampler         *CreateSampler( const DenOfIz_SamplerDesc &desc ) override;
        IQueryPool       *CreateQueryPool( const DenOfIz_QueryPoolDesc &desc ) override;
        IPipelineCache   *CreatePipelineCache( const DenOfIz_PipelineCacheDesc &desc ) override;

        // Not supported
        ITopLevelAS         *CreateTopLevelAS( const DenOfIz_TopLevelASDesc &desc ) override;
        IBottomLevelAS      *CreateBottomLevelAS( const DenOfIz_BottomLevelASDesc &desc ) override;
        IShaderBindingTable *CreateShaderBindingTable( const DenOfIz_ShaderBindingTableDesc &desc ) override;
        ILocalRootSignature *CreateLocalRootSignature( const DenOfIz_LocalRootSignatureDesc &desc ) override;
        IShaderLocalData    *CreateShaderLocalData( const DenOfIz_ShaderLocalDataDesc &desc ) override;

        WebGPUContext *GetContext( ) const;

    private:
        void CreateDevice( );
        void QueryAdapterCapabilities( ) const;
    };

} // namespace DenOfIz
