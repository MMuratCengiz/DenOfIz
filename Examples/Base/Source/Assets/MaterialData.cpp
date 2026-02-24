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

#include "DenOfIzExamples/Assets/MaterialData.h"

using namespace DenOfIz;

MaterialData::MaterialData( const MaterialDesc &desc )
{
    DenOfIz_SamplerDesc samplerDesc{ };
    DenOfIz_LogicalDevice_CreateSampler( desc.Device, &samplerDesc, &m_sampler );

    if ( !desc.AlbedoTexture.empty( ) )
    {
        const DenOfIz_StringView texturePath( desc.AlbedoTexture.c_str( ), static_cast<uint32_t>( desc.AlbedoTexture.length( ) ) );
        m_albedoTexture = DenOfIz_BatchResourceCopy_CreateAndLoadTexture( desc.BatchCopy, texturePath );
    }
    if ( !desc.NormalTexture.empty( ) )
    {
        const DenOfIz_StringView texturePath( desc.NormalTexture.c_str( ), static_cast<uint32_t>( desc.NormalTexture.length( ) ) );
        m_normalTexture = DenOfIz_BatchResourceCopy_CreateAndLoadTexture( desc.BatchCopy, texturePath );
    }
    if ( !desc.HeightTexture.empty( ) )
    {
        const DenOfIz_StringView texturePath( desc.HeightTexture.c_str( ), static_cast<uint32_t>( desc.HeightTexture.length( ) ) );
        m_heightTexture = DenOfIz_BatchResourceCopy_CreateAndLoadTexture( desc.BatchCopy, texturePath );
    }
    if ( !desc.MetallicTexture.empty( ) )
    {
        const DenOfIz_StringView texturePath( desc.MetallicTexture.c_str( ), static_cast<uint32_t>( desc.MetallicTexture.length( ) ) );
        m_metallicTexture = DenOfIz_BatchResourceCopy_CreateAndLoadTexture( desc.BatchCopy, texturePath );
    }
    if ( !desc.RoughnessTexture.empty( ) )
    {
        const DenOfIz_StringView texturePath( desc.RoughnessTexture.c_str( ), static_cast<uint32_t>( desc.RoughnessTexture.length( ) ) );
        m_roughnessTexture = DenOfIz_BatchResourceCopy_CreateAndLoadTexture( desc.BatchCopy, texturePath );
    }
    if ( !desc.AoTexture.empty( ) )
    {
        const DenOfIz_StringView texturePath( desc.AoTexture.c_str( ), static_cast<uint32_t>( desc.AoTexture.length( ) ) );
        m_aoTexture = DenOfIz_BatchResourceCopy_CreateAndLoadTexture( desc.BatchCopy, texturePath );
    }
}

MaterialData::~MaterialData( )
{
    if ( DENOFIZ_HANDLE_IS_VALID( m_sampler ) )
    {
        DenOfIz_Sampler_Destroy( m_sampler );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_albedoTexture ) )
    {
        DenOfIz_TextureResource_Destroy( m_albedoTexture );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_normalTexture ) )
    {
        DenOfIz_TextureResource_Destroy( m_normalTexture );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_heightTexture ) )
    {
        DenOfIz_TextureResource_Destroy( m_heightTexture );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_metallicTexture ) )
    {
        DenOfIz_TextureResource_Destroy( m_metallicTexture );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_roughnessTexture ) )
    {
        DenOfIz_TextureResource_Destroy( m_roughnessTexture );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_aoTexture ) )
    {
        DenOfIz_TextureResource_Destroy( m_aoTexture );
    }
}

DenOfIz_Sampler MaterialData::Sampler( ) const
{
    return m_sampler;
}

DenOfIz_Texture MaterialData::AlbedoTexture( ) const
{
    return m_albedoTexture;
}

DenOfIz_Texture MaterialData::NormalTexture( ) const
{
    return m_normalTexture;
}

DenOfIz_Texture MaterialData::HeightTexture( ) const
{
    return m_heightTexture;
}

DenOfIz_Texture MaterialData::MetallicTexture( ) const
{
    return m_metallicTexture;
}

DenOfIz_Texture MaterialData::RoughnessTexture( ) const
{
    return m_roughnessTexture;
}

DenOfIz_Texture MaterialData::AoTexture( ) const
{
    return m_aoTexture;
}
