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
        T               *Resource;
        MTLRenderStages  ShaderStages;
        MTLResourceUsage Usage;
    };

    struct MetalRootParameterBinding
    {
        uint32_t      TLABOffset = 0;
        id<MTLBuffer> Buffer;
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

    class MetalResourceBindGroup : public IResourceBindGroup
    {
    private:
        MetalContext       *m_context;
        MetalRootSignature *m_rootSignature;
        UpdateDesc          m_updateDesc;

        std::vector<MetalUpdateDescItem<MetalBufferResource>>  m_buffers;
        std::vector<MetalUpdateDescItem<MetalTextureResource>> m_textures;
        std::vector<MetalUpdateDescItem<MetalSampler>>         m_samplers;

        std::vector<Byte>                            m_rootConstant;
        std::vector<MetalRootParameterBinding>       m_rootParameterBindings;
        std::unique_ptr<MetalDescriptorTableBinding> m_cbvSrvUavTable;
        std::unique_ptr<MetalDescriptorTableBinding> m_samplerTable;

    public:
        MetalResourceBindGroup( MetalContext *context, ResourceBindGroupDesc desc );
        void SetRootConstants( uint32_t binding, void *data ) override;
        void Update( const UpdateDesc &desc ) override;

        [[nodiscard]] const std::vector<Byte>                      &RootConstant( ) const;
        [[nodiscard]] const std::vector<MetalRootParameterBinding> &RootParameters( ) const;
        // Nullable if nothing is bound to the pertinent table
        [[nodiscard]] const MetalDescriptorTableBinding *CbvSrvUavTable( ) const;
        [[nodiscard]] const MetalDescriptorTableBinding *SamplerTable( ) const;

        const std::vector<MetalUpdateDescItem<MetalBufferResource>>  &Buffers( ) const;
        const std::vector<MetalUpdateDescItem<MetalTextureResource>> &Textures( ) const;
        const std::vector<MetalUpdateDescItem<MetalSampler>>         &Samplers( ) const;

        [[nodiscard]] MetalRootSignature *RootSignature( ) const;

    protected:
        void BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource ) override;
        void BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource ) override;
        void BindSampler( const ResourceBindingSlot &slot, ISampler *sampler ) override;

    private:
        void UpdateDescriptorTable( const MetalBindingDesc &binding, MetalDescriptorTableBinding *table );
    };

} // namespace DenOfIz
