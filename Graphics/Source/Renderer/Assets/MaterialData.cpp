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

#include <DenOfIzGraphics/Renderer/Assets/MaterialData.h>

using namespace DenOfIz;

MaterialData::MaterialData( const MaterialDesc &desc )
{

    m_sampler = desc.Device->CreateSampler( SamplerDesc{ } );
    if ( !desc.AlbedoTexture.empty( ) )
    {
        m_albedoTexture = desc.BatchCopy->CreateAndLoadTexture( desc.AlbedoTexture );
    }
    if ( !desc.NormalTexture.empty( ) )
    {
        m_normalTexture = desc.BatchCopy->CreateAndLoadTexture( desc.NormalTexture );
    }
    if ( !desc.HeightTexture.empty( ) )
    {
        m_heightTexture = desc.BatchCopy->CreateAndLoadTexture( desc.HeightTexture );
    }
    if ( !desc.MetallicTexture.empty( ) )
    {
        m_metallicTexture = desc.BatchCopy->CreateAndLoadTexture( desc.MetallicTexture );
    }
    if ( !desc.RoughnessTexture.empty( ) )
    {
        m_roughnessTexture = desc.BatchCopy->CreateAndLoadTexture( desc.RoughnessTexture );
    }
    if ( !desc.AoTexture.empty( ) )
    {
        m_aoTexture = desc.BatchCopy->CreateAndLoadTexture( desc.AoTexture );
    }
}

ISampler *MaterialData::Sampler( ) const
{
    return m_sampler.get( );
}

ITextureResource *MaterialData::AlbedoTexture( ) const
{
    return m_albedoTexture.get( );
}

ITextureResource *MaterialData::NormalTexture( ) const
{
    return m_normalTexture.get( );
}

ITextureResource *MaterialData::HeightTexture( ) const
{
    return m_heightTexture.get( );
}

ITextureResource *MaterialData::MetallicTexture( ) const
{
    return m_metallicTexture.get( );
}

ITextureResource *MaterialData::RoughnessTexture( ) const
{
    return m_roughnessTexture.get( );
}
ITextureResource *MaterialData::AoTexture( ) const
{
    return m_aoTexture.get( );
}
