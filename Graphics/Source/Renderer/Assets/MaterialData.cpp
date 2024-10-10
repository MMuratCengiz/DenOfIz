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

void MaterialData::AttachSampler( std::unique_ptr<ISampler> sampler )
{
    this->m_sampler = std::move( sampler );
}

void MaterialData::AttachAlbedoData( std::unique_ptr<ITextureResource> texture )
{
    this->m_albedoTexture = std::move( texture );
}

void MaterialData::AttachNormalData( std::unique_ptr<ITextureResource> texture )
{
    this->m_normalTexture = std::move( texture );
}

void MaterialData::AttachHeightData( std::unique_ptr<ITextureResource> texture )
{
    this->m_heightTexture = std::move( texture );
}

void MaterialData::AttachMetallicData( std::unique_ptr<ITextureResource> texture )
{
    this->m_metallicTexture = std::move( texture );
}

void MaterialData::AttachRoughnessData( std::unique_ptr<ITextureResource> texture )
{
    this->m_roughnessTexture = std::move( texture );
}

void MaterialData::AttachAoData( std::unique_ptr<ITextureResource> texture )
{
    this->m_aoTexture = std::move( texture );
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
