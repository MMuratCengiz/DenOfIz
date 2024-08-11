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

void MetalResourceBindGroup::BindTexture( const std::string &name, ITextureResource *resource )
{
    uint32_t slot = m_rootSignature->FindBinding( name ).Slot;
    m_textures.push_back( MetalUpdateDescItem<MetalTextureResource>{
        .Name     = name,
        .Resource = static_cast<MetalTextureResource *>( resource ),
        .Slot     = slot,
    } );
}

void MetalResourceBindGroup::BindBuffer( const std::string &name, IBufferResource *resource )
{
    uint32_t slot = m_rootSignature->FindBinding( name ).Slot;
    m_buffers.push_back( MetalUpdateDescItem<MetalBufferResource>{
        .Name     = name,
        .Resource = static_cast<MetalBufferResource *>( resource ),
        .Slot     = slot,
    } );
}

void MetalResourceBindGroup::BindSampler( const std::string &name, ISampler *sampler )
{
    uint32_t slot = m_rootSignature->FindBinding( name ).Slot;
    m_samplers.push_back( MetalUpdateDescItem<MetalSampler>{
        .Name     = name,
        .Resource = static_cast<MetalSampler *>( sampler ),
        .Slot     = slot,
    } );
}
