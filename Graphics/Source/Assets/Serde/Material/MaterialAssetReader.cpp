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

#include "DenOfIzGraphics/Assets/Serde/Material/MaterialAssetReader.h"
#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetReaderHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MaterialAssetReader::MaterialAssetReader( const MaterialAssetReaderDesc &desc ) : m_reader( desc.Reader )
{
    if ( !m_reader )
    {
        spdlog::critical( "BinaryReader cannot be null for MaterialAssetReader" );
    }
}

MaterialAssetReader::~MaterialAssetReader( ) = default;

MaterialAsset* MaterialAssetReader::Read( )
{
    m_materialAsset       = new MaterialAsset( );
    m_materialAsset->Magic = m_reader->ReadUInt64( );
    if ( m_materialAsset->Magic != MaterialAsset{ }.Magic )
    {
        spdlog::critical( "Invalid MaterialAsset magic number." );
    }

    m_materialAsset->Version = m_reader->ReadUInt32( );
    if ( m_materialAsset->Version > MaterialAsset::Latest )
    {
        spdlog::warn( "MaterialAsset version mismatch." );
    }

    m_materialAsset->NumBytes = m_reader->ReadUInt64( );
    m_materialAsset->Uri      = AssetUri::Parse( m_reader->ReadString( ) );

    m_materialAsset->Name      = m_reader->ReadString( );
    m_materialAsset->ShaderRef = m_reader->ReadString( );

    m_materialAsset->AlbedoMapRef            = AssetUri::Parse( m_reader->ReadString( ) );
    m_materialAsset->NormalMapRef            = AssetUri::Parse( m_reader->ReadString( ) );
    m_materialAsset->MetallicRoughnessMapRef = AssetUri::Parse( m_reader->ReadString( ) );
    m_materialAsset->EmissiveMapRef          = AssetUri::Parse( m_reader->ReadString( ) );
    m_materialAsset->OcclusionMapRef         = AssetUri::Parse( m_reader->ReadString( ) );

    m_materialAsset->BaseColorFactor = m_reader->ReadFloat_4( );
    m_materialAsset->MetallicFactor  = m_reader->ReadFloat( );
    m_materialAsset->RoughnessFactor = m_reader->ReadFloat( );
    m_materialAsset->EmissiveFactor  = m_reader->ReadFloat_3( );

    m_materialAsset->AlphaBlend  = m_reader->ReadByte( ) != 0;
    m_materialAsset->DoubleSided = m_reader->ReadByte( ) != 0;
    m_materialAsset->Properties  = AssetReaderHelpers::ReadUserProperties( &m_materialAsset->_Arena, m_reader );

    return m_materialAsset;
}
