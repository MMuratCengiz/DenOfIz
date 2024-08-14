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

#include <DenOfIzGraphics/Backends/Metal/MetalResourceBindGroup.h>

using namespace DenOfIz;

MetalResourceBindGroup::MetalResourceBindGroup( MetalContext *context, ResourceBindGroupDesc desc ) : IResourceBindGroup( desc ), m_context( context )
{
    m_context = context;
    m_rootSignature = static_cast<MetalRootSignature *>( desc.RootSignature );
}

void MetalResourceBindGroup::Update( const UpdateDesc &desc )
{
    m_updateDesc = desc;

    m_buffers.clear( );
    m_textures.clear( );
    m_samplers.clear( );

    IResourceBindGroup::Update( desc );
}

const std::vector<MetalUpdateDescItem<MetalBufferResource>> &MetalResourceBindGroup::Buffers( ) const
{
    return m_buffers;
}

const std::vector<MetalUpdateDescItem<MetalTextureResource>> &MetalResourceBindGroup::Textures( ) const
{
    return m_textures;
}

const std::vector<MetalUpdateDescItem<MetalSampler>> &MetalResourceBindGroup::Samplers( ) const
{
    return m_samplers;
}

void MetalResourceBindGroup::BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource )
{
    uint32_t location = m_rootSignature->FindMetalBinding( slot ).Location;
    m_textures.push_back( MetalUpdateDescItem<MetalTextureResource>{
        .Resource = static_cast<MetalTextureResource *>( resource ),
        .Location = location,
    } );
}

void MetalResourceBindGroup::BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource )
{
    uint32_t location = m_rootSignature->FindMetalBinding( slot ).Location;
    m_buffers.push_back( MetalUpdateDescItem<MetalBufferResource>{
        .Resource = static_cast<MetalBufferResource *>( resource ),
        .Location = location,
    } );
}

void MetalResourceBindGroup::BindSampler( const ResourceBindingSlot &slot, ISampler *sampler )
{
    uint32_t location = m_rootSignature->FindMetalBinding( slot ).Location;
    m_samplers.push_back( MetalUpdateDescItem<MetalSampler>{
        .Resource = static_cast<MetalSampler *>( sampler ),
        .Location = location,
    } );
}
