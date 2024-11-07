#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12ShaderRecordData.h>
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

using namespace DenOfIz;

DX12ShaderRecordData::DX12ShaderRecordData( DX12Context *context, const ShaderRecordDataDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_layout = dynamic_cast<DX12ShaderRecordLayout *>( m_desc.Layout );
}

void DX12ShaderRecordData::Begin( )
{
}

void DX12ShaderRecordData::Cbv( uint32_t index, const IBufferResource *bufferResource )
{
}

void DX12ShaderRecordData::Cbv( uint32_t index, const InteropArray<Byte> &data )
{
}

void DX12ShaderRecordData::Srv( uint32_t index, const IBufferResource *textureResource )
{
}

void DX12ShaderRecordData::Srv( uint32_t index, const ITextureResource *textureResource )
{
}

void DX12ShaderRecordData::Srv( uint32_t index, const InteropArray<Byte> &data )
{
}

void DX12ShaderRecordData::Uav( uint32_t index, const IBufferResource *textureResource )
{
}

void DX12ShaderRecordData::Uav( uint32_t index, const ITextureResource *textureResource )
{
}

void DX12ShaderRecordData::Sampler( uint32_t index, const ISampler *sampler )
{
}
void DX12ShaderRecordData::End( )
{
}
