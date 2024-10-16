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

    m_sampler = std::unique_ptr<ISampler>( desc.Device->CreateSampler( SamplerDesc{ } ) );
    if ( !desc.AlbedoTexture.Str( ).empty( ) )
    {
        m_albedoTexture = std::unique_ptr<ITextureResource>( desc.BatchCopy->CreateAndLoadTexture( desc.AlbedoTexture.Str( ) ) );
    }
    if ( !desc.NormalTexture.Str( ).empty( ) )
    {
        m_normalTexture = std::unique_ptr<ITextureResource>( desc.BatchCopy->CreateAndLoadTexture( desc.NormalTexture.Str( ) ) );
    }
    if ( !desc.HeightTexture.Str( ).empty( ) )
    {
        m_heightTexture = std::unique_ptr<ITextureResource>( desc.BatchCopy->CreateAndLoadTexture( desc.HeightTexture.Str( ) ) );
    }
    if ( !desc.MetallicTexture.Str( ).empty( ) )
    {
        m_metallicTexture = std::unique_ptr<ITextureResource>( desc.BatchCopy->CreateAndLoadTexture( desc.MetallicTexture.Str( ) ) );
    }
    if ( !desc.RoughnessTexture.Str( ).empty( ) )
    {
        m_roughnessTexture = std::unique_ptr<ITextureResource>( desc.BatchCopy->CreateAndLoadTexture( desc.RoughnessTexture.Str( ) ) );
    }
    if ( !desc.AoTexture.Str( ).empty( ) )
    {
        m_aoTexture = std::unique_ptr<ITextureResource>( desc.BatchCopy->CreateAndLoadTexture( desc.AoTexture.Str( ) ) );
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
