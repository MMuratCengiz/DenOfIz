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

#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include <algorithm>
#include "MetalArgumentBuffer.h"
#include "MetalBufferResource.h"
#include "MetalContext.h"
#include "MetalRootSignature.h"
#include "MetalTextureResource.h"

namespace DenOfIz
{
    template <typename T>
    struct MetalUpdateDescItem
    {
        T               *Resource     = nullptr;
        MTLRenderStages  ShaderStages = 0;
        MTLResourceUsage Usage        = MTLResourceUsageRead;

        MetalUpdateDescItem( ) = default;
        MetalUpdateDescItem( T *resource, MTLRenderStages shaderStages, MTLResourceUsage usage ) : Resource( resource ), ShaderStages( shaderStages ), Usage( usage )
        {
        }
    };

    struct MetalRootParameterBinding
    {
        uint32_t      TLABOffset = 0;
        id<MTLBuffer> Buffer;

        MetalRootParameterBinding( ) = default;
        MetalRootParameterBinding( uint32_t offset, id<MTLBuffer> buffer ) : TLABOffset( offset ), Buffer( buffer )
        {
        }
    };

    struct MetalBufferBindingWithOffset
    {
        ResourceBindingSlot Slot;
        IBufferResource    *Resource;
        uint32_t            Offset;
    };

    struct MetalTextureArrayIndexBinding
    {
        ResourceBindingSlot Slot;
        uint32_t            ArrayIndex;
        ITextureResource   *Resource;
    };

    struct MetalDescriptorTableBinding
    {
        // Top level argument buffer offset
        uint32_t        TLABOffset = 0;
        uint32_t        NumEntries = 0;
        DescriptorTable Table;

        MetalDescriptorTableBinding( uint32_t tlabOffset, DescriptorTable table ) : TLABOffset( tlabOffset ), Table( table )
        {
        }
    };

    class MetalResourceBindGroup final : public IResourceBindGroup
    {
    private:
        ResourceBindGroupDesc                                           m_desc;
        MetalContext                                                   *m_context;
        MetalRootSignature                                             *m_rootSignature;
        std::vector<std::pair<ResourceBindingSlot, ITopLevelAS *>>      m_boundAccelerationStructures;
        std::vector<std::pair<ResourceBindingSlot, IBufferResource *>>  m_boundBuffers;
        std::vector<MetalBufferBindingWithOffset>                       m_boundBuffersWithOffsets;
        std::vector<std::pair<ResourceBindingSlot, ITextureResource *>> m_boundTextures;
        std::vector<MetalTextureArrayIndexBinding>                      m_boundTextureArrayIndices;
        std::vector<std::pair<ResourceBindingSlot, ISampler *>>         m_boundSamplers;

        std::vector<id<MTLResource>>                           m_indirectResources;
        std::vector<MetalUpdateDescItem<MetalBufferResource>>  m_buffers;
        std::vector<MetalUpdateDescItem<MetalTextureResource>> m_textures;
        std::vector<MetalUpdateDescItem<MetalSampler>>         m_samplers;

        std::vector<Byte>                            m_rootConstant;
        std::vector<MetalRootParameterBinding>       m_rootParameterBindings;
        std::unique_ptr<MetalDescriptorTableBinding> m_cbvSrvUavTable;
        std::unique_ptr<MetalDescriptorTableBinding> m_samplerTable;

    public:
        MetalResourceBindGroup( MetalContext *context, ResourceBindGroupDesc desc );
        void                SetRootConstantsData( uint32_t binding, const InteropArray<Byte> &data ) override;
        void                SetRootConstants( uint32_t binding, void *data ) override;
        IResourceBindGroup *BeginUpdate( ) override;
        IResourceBindGroup *Cbv( const uint32_t binding, IBufferResource *resource ) override;
        IResourceBindGroup *Cbv( const BindBufferDesc &desc ) override;
        IResourceBindGroup *Srv( const uint32_t binding, IBufferResource *resource ) override;
        IResourceBindGroup *Srv( const BindBufferDesc &desc ) override;
        IResourceBindGroup *Srv( const uint32_t binding, ITextureResource *resource ) override;
        IResourceBindGroup *SrvArray( const uint32_t binding, const InteropArray<ITextureResource *> &resources ) override;
        IResourceBindGroup *SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITextureResource *resource ) override;
        IResourceBindGroup *Srv( const uint32_t binding, ITopLevelAS *accelerationStructure ) override;
        IResourceBindGroup *Uav( const uint32_t binding, IBufferResource *resource ) override;
        IResourceBindGroup *Uav( const BindBufferDesc &desc ) override;
        IResourceBindGroup *Uav( const uint32_t binding, ITextureResource *resource ) override;
        IResourceBindGroup *Sampler( const uint32_t binding, ISampler *sampler ) override;
        void                EndUpdate( ) override;

        [[nodiscard]] const std::vector<Byte>                      &RootConstant( ) const;
        [[nodiscard]] const std::vector<MetalRootParameterBinding> &RootParameters( ) const;
        // Nullable if nothing is bound to the pertinent table
        [[nodiscard]] const MetalDescriptorTableBinding *CbvSrvUavTable( ) const;
        [[nodiscard]] const MetalDescriptorTableBinding *SamplerTable( ) const;

        const std::vector<id<MTLResource>>                           &IndirectResources( ) const;
        const std::vector<MetalUpdateDescItem<MetalBufferResource>>  &Buffers( ) const;
        const std::vector<MetalUpdateDescItem<MetalTextureResource>> &Textures( ) const;
        const std::vector<MetalUpdateDescItem<MetalSampler>>         &Samplers( ) const;

        [[nodiscard]] MetalRootSignature *RootSignature( ) const;
        [[nodiscard]] uint32_t            RegisterSpace( ) const;

    private:
        void                BindAccelerationStructure( const ResourceBindingSlot &slot, ITopLevelAS *accelerationStructure );
        void                BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource );
        void                BindBufferWithOffset( const ResourceBindingSlot &slot, IBufferResource *resource, uint32_t offset );
        void                BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource );
        void                BindTextureArrayIndex( const ResourceBindingSlot &slot, uint32_t arrayIndex, ITextureResource *resource );
        void                BindSampler( const ResourceBindingSlot &slot, ISampler *sampler );
        ResourceBindingSlot GetSlot( uint32_t binding, const ResourceBindingType &type ) const;
    };

} // namespace DenOfIz
