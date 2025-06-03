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

#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "IBufferResource.h"
#include "ICommandListPool.h"
#include "ICommandQueue.h"
#include "IPipeline.h"
#include "IResourceBindGroup.h"
#include "IRootSignature.h"
#include "ISemaphore.h"
#include "ISwapChain.h"
#include "ITextureResource.h"
#include "RayTracing/ILocalRootSignature.h"
#include "RayTracing/IShaderBindingTable.h"
#include "RayTracing/IShaderLocalData.h"

namespace DenOfIz
{
    class DZ_API ILogicalDevice
    {
    protected:
        PhysicalDevice m_selectedDeviceInfo{ };

    public:
        virtual ~ILogicalDevice( ) = default;

        virtual void                         CreateDevice( )                                    = 0;
        virtual InteropArray<PhysicalDevice> ListPhysicalDevices( )                             = 0;
        virtual void                         LoadPhysicalDevice( const PhysicalDevice &device ) = 0;
        virtual bool                         IsDeviceLost( )                                    = 0;
        virtual void                         WaitIdle( )                                        = 0;

        const PhysicalDevice &DeviceInfo( )
        {
            return m_selectedDeviceInfo;
        };

        // Factory methods
        virtual ICommandQueue      *CreateCommandQueue( const CommandQueueDesc &desc )           = 0;
        virtual ICommandListPool   *CreateCommandListPool( const CommandListPoolDesc &desc )     = 0;
        virtual IPipeline          *CreatePipeline( const PipelineDesc &desc )                   = 0;
        virtual ISwapChain         *CreateSwapChain( const SwapChainDesc &desc )                 = 0;
        virtual IRootSignature     *CreateRootSignature( const RootSignatureDesc &desc )         = 0;
        virtual IInputLayout       *CreateInputLayout( const InputLayoutDesc &desc )             = 0;
        virtual IResourceBindGroup *CreateResourceBindGroup( const ResourceBindGroupDesc &desc ) = 0;
        virtual IFence             *CreateFence( )                                               = 0;
        virtual ISemaphore         *CreateSemaphore( )                                           = 0;
        virtual IBufferResource    *CreateBufferResource( const BufferDesc &desc )               = 0;
        virtual ITextureResource   *CreateTextureResource( const TextureDesc &desc )             = 0;
        virtual ISampler           *CreateSampler( const SamplerDesc &desc )                     = 0;
        // RayTracing:
        virtual ITopLevelAS         *CreateTopLevelAS( const TopLevelASDesc &desc )                 = 0;
        virtual IBottomLevelAS      *CreateBottomLevelAS( const BottomLevelASDesc &desc )           = 0;
        virtual IShaderBindingTable *CreateShaderBindingTable( const ShaderBindingTableDesc &desc ) = 0;
        virtual ILocalRootSignature *CreateLocalRootSignature( const LocalRootSignatureDesc &desc ) = 0;
        virtual IShaderLocalData    *CreateShaderLocalData( const ShaderLocalDataDesc &desc )       = 0;
    };

} // namespace DenOfIz
