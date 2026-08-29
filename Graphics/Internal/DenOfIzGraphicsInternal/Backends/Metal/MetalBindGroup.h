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

#include <DenOfIzGraphicsInternal/Backends/Interface/IBindGroup.h>
#include <algorithm>
#include <memory>
#include "MetalArgumentBuffer.h"
#include "MetalBindGroupLayout.h"
#include "MetalBuffer.h"
#include "MetalContext.h"
#include "MetalShaderLayout.h"
#include "MetalTexture.h"

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
        uint32_t      OffsetBytes = 0;
        id<MTLBuffer> Buffer;

        MetalRootParameterBinding( ) = default;
        MetalRootParameterBinding( uint32_t offsetBytes, id<MTLBuffer> buffer ) : OffsetBytes( offsetBytes ), Buffer( buffer )
        {
        }
    };

    struct MetalBufferBindingWithOffset
    {
        DenOfIz_ResourceBindingSlot Slot;
        IBuffer                    *Resource;
        size_t                      Offset;
    };

    struct MetalTextureArrayIndexBinding
    {
        DenOfIz_ResourceBindingSlot Slot;
        uint32_t                    ArrayIndex;
        ITexture                   *Resource;
    };

    struct MetalTextureBinding
    {
        DenOfIz_ResourceBindingSlot Slot;
        uint32_t                    MipLevel = DZ_ALL_MIP_LEVELS;
        ITexture                   *Resource;
    };

    struct MetalBindGroupTables
    {
        MetalSpaceLayout                       Layout;
        std::unique_ptr<DescriptorTable>       CbvSrvUavTable;
        std::unique_ptr<DescriptorTable>       SamplerTable;
        std::vector<MetalRootParameterBinding> RootParameters;
    };

    class MetalBindGroup final : public IBindGroup
    {
    private:
        DenOfIz_BindGroupDesc                                              m_desc;
        MetalContext                                                      *m_context;
        MetalBindGroupLayout                                              *m_bindGroupLayout;
        std::vector<std::pair<DenOfIz_ResourceBindingSlot, ITopLevelAS *>> m_boundAccelerationStructures;
        std::vector<std::pair<DenOfIz_ResourceBindingSlot, IBuffer *>>     m_boundBuffers;
        std::vector<MetalBufferBindingWithOffset>                          m_boundBuffersWithOffsets;
        std::vector<MetalTextureBinding>                                   m_boundTextures;
        std::vector<MetalTextureArrayIndexBinding>                         m_boundTextureArrayIndices;
        std::vector<std::pair<DenOfIz_ResourceBindingSlot, ISampler *>>    m_boundSamplers;

        std::vector<id<MTLResource>>                   m_indirectResources;
        std::vector<MetalUpdateDescItem<MetalBuffer>>  m_buffers;
        std::vector<MetalUpdateDescItem<MetalTexture>> m_textures;
        std::vector<MetalUpdateDescItem<MetalSampler>> m_samplers;

        mutable std::vector<std::unique_ptr<MetalBindGroupTables>> m_tables;

    public:
        MetalBindGroup( MetalContext *context, DenOfIz_BindGroupDesc desc );

        IBindGroup *BeginUpdate( ) override;
        IBindGroup *Cbv( const uint32_t binding, IBuffer *resource ) override;
        IBindGroup *Cbv( const uint32_t binding, IBuffer *resource, size_t offset ) override;
        IBindGroup *Srv( const uint32_t binding, IBuffer *resource ) override;
        IBindGroup *Srv( const uint32_t binding, IBuffer *resource, size_t offset ) override;
        IBindGroup *Srv( const uint32_t binding, ITexture *resource, uint32_t mipLevel = DZ_ALL_MIP_LEVELS ) override;
        IBindGroup *SrvArray( const uint32_t binding, const DenOfIz_TextureArray &resources ) override;
        IBindGroup *SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITexture *resource ) override;
        IBindGroup *Srv( const uint32_t binding, ITopLevelAS *accelerationStructure ) override;
        IBindGroup *Uav( const uint32_t binding, IBuffer *resource ) override;
        IBindGroup *Uav( const uint32_t binding, IBuffer *resource, size_t offset ) override;
        IBindGroup *Uav( const uint32_t binding, ITexture *resource, uint32_t mipLevel = DZ_ALL_MIP_LEVELS ) override;
        IBindGroup *Sampler( const uint32_t binding, ISampler *sampler ) override;
        void        EndUpdate( ) override;

        [[nodiscard]] const MetalBindGroupTables *TablesFor( const MetalSpaceLayout &spaceLayout ) const;

        const std::vector<id<MTLResource>>                   &IndirectResources( ) const;
        const std::vector<MetalUpdateDescItem<MetalBuffer>>  &Buffers( ) const;
        const std::vector<MetalUpdateDescItem<MetalTexture>> &Textures( ) const;
        const std::vector<MetalUpdateDescItem<MetalSampler>> &Samplers( ) const;
        [[nodiscard]] MetalBindGroupLayout                   *BindGroupLayout( ) const;
        [[nodiscard]] uint32_t                                RegisterSpace( ) const;

    private:
        void                        EncodeTables( MetalBindGroupTables &tables ) const;
        bool                        ValidateSlot( const DenOfIz_ResourceBindingSlot &slot ) const;
        DenOfIz_ResourceBindingSlot GetSlot( uint32_t binding, const DenOfIz_ResourceBindingType &type ) const;
    };
} // namespace DenOfIz
