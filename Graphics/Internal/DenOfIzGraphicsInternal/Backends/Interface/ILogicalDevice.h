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
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "../Common/DeferredDestroyQueue.h"
#include "IBindGroup.h"
#include "IBindGroupLayout.h"
#include "IBuffer.h"
#include "ICommandListPool.h"
#include "ICommandQueue.h"
#include "IFence.h"
#include "IInputLayout.h"
#include "IPipeline.h"
#include "IPipelineCache.h"
#include "IQueryPool.h"
#include "IRootSignature.h"
#include "ISemaphore.h"
#include "ISwapChain.h"
#include "ITexture.h"
#include "RayTracing/IBottomLevelAS.h"
#include "RayTracing/ILocalRootSignature.h"
#include "RayTracing/IShaderBindingTable.h"
#include "RayTracing/IShaderLocalData.h"

namespace DenOfIz
{

    class ILogicalDevice
    {
    protected:
        DenOfIz_PhysicalDevice m_selectedDeviceInfo{ };
        //! Created by the backends that defer destruction, null means objects are deleted as soon as they are destroyed.
        std::unique_ptr<DeferredDestroyQueue> m_destroyQueue;

        //! Deletes everything that is still waiting in the destroy queue. Every backend calls this first thing in its destructor, the objects
        //! need the device to release what they own.
        void ReleaseDestroyedObjects( )
        {
            if ( m_destroyQueue )
            {
                WaitIdle( );
                m_destroyQueue->ReleaseAll( );
            }
        }

    public:
        virtual ~ILogicalDevice( ) = default;

        [[nodiscard]] DeferredDestroyQueue *DestroyQueue( ) const
        {
            return m_destroyQueue.get( );
        }

        virtual void                        CreateDevice( const DenOfIz_LogicalDeviceDesc &desc = { } ) = 0;
        virtual DenOfIz_PhysicalDeviceArray ListPhysicalDevices( )                                      = 0;
        virtual void                        LoadPhysicalDevice( const DenOfIz_PhysicalDevice &device )  = 0;
        virtual bool                        IsDeviceLost( )                                             = 0;
        virtual void                        WaitIdle( )                                                 = 0;

        const DenOfIz_PhysicalDevice &DeviceInfo( )
        {
            return m_selectedDeviceInfo;
        }

        // Factory methods
        virtual ICommandQueue    *CreateCommandQueue( const DenOfIz_CommandQueueDesc &desc )       = 0;
        virtual ICommandListPool *CreateCommandListPool( const DenOfIz_CommandListPoolDesc &desc ) = 0;
        virtual IPipeline        *CreatePipeline( const DenOfIz_PipelineDesc &desc )               = 0;
        virtual IPipelineCache   *CreatePipelineCache( const DenOfIz_PipelineCacheDesc &desc )     = 0;
        virtual ISwapChain       *CreateSwapChain( const DenOfIz_SwapChainDesc &desc )             = 0;
        virtual IRootSignature   *CreateRootSignature( const DenOfIz_RootSignatureDesc &desc )     = 0;
        virtual IInputLayout     *CreateInputLayout( const DenOfIz_InputLayoutDesc &desc )         = 0;
        virtual IBindGroupLayout *CreateBindGroupLayout( const DenOfIz_BindGroupLayoutDesc &desc ) = 0;
        virtual IBindGroup       *CreateBindGroup( const DenOfIz_BindGroupDesc &desc )             = 0;
        virtual IFence           *CreateFence( )                                                   = 0;
        virtual ISemaphore       *CreateSemaphore( )                                               = 0;
        virtual IBuffer          *CreateBuffer( const DenOfIz_BufferDesc &desc )                   = 0;
        virtual ITexture         *CreateTexture( const DenOfIz_TextureDesc &desc )                 = 0;
        virtual ISampler         *CreateSampler( const DenOfIz_SamplerDesc &desc )                 = 0;
        virtual IQueryPool       *CreateQueryPool( const DenOfIz_QueryPoolDesc &desc )             = 0;
        // RayTracing:
        virtual ITopLevelAS         *CreateTopLevelAS( const DenOfIz_TopLevelASDesc &desc )                 = 0;
        virtual IBottomLevelAS      *CreateBottomLevelAS( const DenOfIz_BottomLevelASDesc &desc )           = 0;
        virtual IShaderBindingTable *CreateShaderBindingTable( const DenOfIz_ShaderBindingTableDesc &desc ) = 0;
        virtual ILocalRootSignature *CreateLocalRootSignature( const DenOfIz_LocalRootSignatureDesc &desc ) = 0;
        virtual IShaderLocalData    *CreateShaderLocalData( const DenOfIz_ShaderLocalDataDesc &desc )       = 0;
    };

} // namespace DenOfIz
