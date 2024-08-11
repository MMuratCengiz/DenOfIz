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
}

const std::vector<UpdateDescItem<IBufferResource>> &MetalResourceBindGroup::Buffers( ) const
{
    return m_updateDesc.Buffers;
}

const std::vector<UpdateDescItem<ITextureResource>> &MetalResourceBindGroup::Textures( ) const
{
    return m_updateDesc.Textures;
}

const std::vector<UpdateDescItem<ISampler>> &MetalResourceBindGroup::Samplers( ) const
{
    return m_updateDesc.Samplers;
}

void MetalResourceBindGroup::BindTexture( const std::string &name, ITextureResource *resource )
{
}
void MetalResourceBindGroup::BindBuffer( const std::string &name, IBufferResource *resource )
{
}
void MetalResourceBindGroup::BindSampler( const std::string &name, ISampler *sampler )
{
}
