/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphics/Utilities/Common.h>
#include <DenOfIzGraphicsInternal/Backends/Interface/ILogicalDevice.h>
#include <deque>
#include "MetalBindGroup.h"
#include "MetalBindGroupLayout.h"
#include "MetalBuffer.h"
#include "MetalCommandListPool.h"
#include "MetalContext.h"
#include "MetalFence.h"
#include "MetalTexture.h"
#include "RayTracing/MetalShaderLocalData.h"

namespace DenOfIz
{

    class MetalLogicalDevice final : public ILogicalDevice
    {
        std::unique_ptr<MetalContext>       m_context;
        std::vector<DenOfIz_PhysicalDevice> m_physicalDevices;
        std::unique_ptr<InternalAutoSync>   m_autoSync;
        DenOfIz_LogicalDeviceDesc           m_deviceDesc;
        std::deque<std::string>             m_deviceNameStorage;

        DeferredDestroyQueue::Completion CaptureGpuCompletion( ) const;

    public:
        MetalLogicalDevice( );
        ~MetalLogicalDevice( ) override;

        void                        CreateDevice( const DenOfIz_LogicalDeviceDesc &desc = { } ) override;
        DenOfIz_PhysicalDeviceArray ListPhysicalDevices( ) override;
        void                        LoadPhysicalDevice( const DenOfIz_PhysicalDevice &device ) override;
        bool                        IsDeviceLost( ) override;

        ICommandQueue       *CreateCommandQueue( const DenOfIz_CommandQueueDesc &desc ) override;
        ICommandListPool    *CreateCommandListPool( const DenOfIz_CommandListPoolDesc &poolDesc ) override;
        IPipeline           *CreatePipeline( const DenOfIz_PipelineDesc &pipelineDesc ) override;
        ISwapChain          *CreateSwapChain( const DenOfIz_SwapChainDesc &swapChainDesc ) override;
        IRootSignature      *CreateRootSignature( const DenOfIz_RootSignatureDesc &rootSignatureDesc ) override;
        IInputLayout        *CreateInputLayout( const DenOfIz_InputLayoutDesc &inputLayoutDesc ) override;
        IBindGroupLayout    *CreateBindGroupLayout( const DenOfIz_BindGroupLayoutDesc &desc ) override;
        IBindGroup          *CreateBindGroup( const DenOfIz_BindGroupDesc &descriptorTableDesc ) override;
        IFence              *CreateFence( ) override;
        ISemaphore          *CreateSemaphore( ) override;
        IBuffer             *CreateBuffer( const DenOfIz_BufferDesc &bufferDesc ) override;
        ITexture            *CreateTexture( const DenOfIz_TextureDesc &textureDesc ) override;
        ISampler            *CreateSampler( const DenOfIz_SamplerDesc &samplerDesc ) override;
        IQueryPool          *CreateQueryPool( const DenOfIz_QueryPoolDesc &desc ) override;
        IPipelineCache      *CreatePipelineCache( const DenOfIz_PipelineCacheDesc &desc ) override;
        IBottomLevelAS      *CreateBottomLevelAS( const DenOfIz_BottomLevelASDesc &desc ) override;
        ITopLevelAS         *CreateTopLevelAS( const DenOfIz_TopLevelASDesc &desc ) override;
        IShaderBindingTable *CreateShaderBindingTable( const DenOfIz_ShaderBindingTableDesc &desc ) override;
        ILocalRootSignature *CreateLocalRootSignature( const DenOfIz_LocalRootSignatureDesc &desc ) override;
        IShaderLocalData    *CreateShaderLocalData( const DenOfIz_ShaderLocalDataDesc &createDesc ) override;

        void WaitIdle( ) override;

        MetalContext *Context( ) const
        {
            return m_context.get( );
        }

    private:
        void InitializeCaptureManager( );
    };

} // namespace DenOfIz
