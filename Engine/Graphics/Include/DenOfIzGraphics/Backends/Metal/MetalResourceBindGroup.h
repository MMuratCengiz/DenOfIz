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
#include "MetalBufferResource.h"
#include "MetalContext.h"
#include "MetalRootSignature.h"
#include "MetalTextureResource.h"

namespace DenOfIz
{
    template <typename T>
    struct MetalUpdateDescItem
    {
        std::string      Name;
        T               *Resource;
        uint32_t         Location;
        MTLRenderStages  ShaderStages;
        MTLResourceUsage Usage;
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

        bool                                     m_bindBuffer;
        NSMutableArray<MTLArgumentDescriptor *> *m_argumentDescriptors;
        id<MTLBuffer>                            m_argumentBuffer;
        id<MTLArgumentEncoder>                   m_argumentEncoder;
        bool                                     m_bindHeap;
        id<MTLHeap>                              m_heap;
        std::vector<uint64_t>                    m_descriptorTable;
        BitSet<ShaderStage>                      m_shaderStages;

    public:
        MetalResourceBindGroup( MetalContext *context, ResourceBindGroupDesc desc );
        void Update( const UpdateDesc &desc ) override;

        const std::vector<MetalUpdateDescItem<MetalBufferResource>>  &Buffers( ) const;
        const std::vector<MetalUpdateDescItem<MetalTextureResource>> &Textures( ) const;
        const std::vector<MetalUpdateDescItem<MetalSampler>>         &Samplers( ) const;

        [[nodiscard]] bool                 BindBuffer( ) const;
        [[nodiscard]] const id<MTLBuffer> &ArgumentBuffer( ) const;
        [[nodiscard]] bool                 BindHeap( ) const;
        [[nodiscard]] const id<MTLHeap>   &Heap( ) const;

    protected:
        void BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource ) override;
        void BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource ) override;
        void BindSampler( const ResourceBindingSlot &slot, ISampler *sampler ) override;

    private:
        MTLArgumentDescriptor *CreateArgumentDescriptor( const ResourceBindingSlot &slot );
        id<MTLBuffer>          CreateEntryBuffer( bool readonlyHeap );
        void                   SetGpuAddress( uint32_t index, uint64_t address );

        template <typename T>
        void PushUpdateDesc( const ResourceBindingSlot &slot, MTLResourceUsage usage, T *resource, std::vector<MetalUpdateDescItem<T>> &items )
        {
            const MetalBindingDesc &metalBinding = m_rootSignature->FindMetalBinding( slot );

            MetalUpdateDescItem<T> item{ };
            item.Usage        = usage;
            item.Resource     = resource;
            item.ShaderStages = metalBinding.Stages;
            item.Location     = metalBinding.Parent.Reflection.LocationHint;
            item.Name         = metalBinding.Parent.Name;

            items.push_back( item );
        }
    };

} // namespace DenOfIz
