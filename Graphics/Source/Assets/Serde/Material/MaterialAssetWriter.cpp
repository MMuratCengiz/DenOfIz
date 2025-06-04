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

#include "DenOfIzGraphics/Assets/Serde/Common/AssetWriterHelpers.h"
#include "DenOfIzGraphics/Assets/Serde/Material/MaterialAssetWriter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MaterialAssetWriter::MaterialAssetWriter( const MaterialAssetWriterDesc &desc ) : m_writer( desc.Writer )
{
    if ( !m_writer )
    {
        LOG( FATAL ) << "BinaryWriter cannot be null for MaterialAssetWriter";
    }
}

MaterialAssetWriter::~MaterialAssetWriter( ) = default;

void MaterialAssetWriter::Write( const MaterialAsset &materialAsset ) const
{
    m_writer->WriteUInt64( materialAsset.Magic );
    m_writer->WriteUInt32( materialAsset.Version );
    m_writer->WriteUInt64( materialAsset.NumBytes );
    m_writer->WriteString( materialAsset.Uri.ToInteropString( ) );

    m_writer->WriteString( materialAsset.Name );
    m_writer->WriteString( materialAsset.ShaderRef );

    m_writer->WriteString( materialAsset.AlbedoMapRef.ToInteropString( ) );
    m_writer->WriteString( materialAsset.NormalMapRef.ToInteropString( ) );
    m_writer->WriteString( materialAsset.MetallicRoughnessMapRef.ToInteropString( ) );
    m_writer->WriteString( materialAsset.EmissiveMapRef.ToInteropString( ) );
    m_writer->WriteString( materialAsset.OcclusionMapRef.ToInteropString( ) );

    m_writer->WriteFloat_4( materialAsset.BaseColorFactor );
    m_writer->WriteFloat( materialAsset.MetallicFactor );
    m_writer->WriteFloat( materialAsset.RoughnessFactor );
    m_writer->WriteFloat_3( materialAsset.EmissiveFactor );

    m_writer->WriteByte( materialAsset.AlphaBlend ? 1 : 0 );
    m_writer->WriteByte( materialAsset.DoubleSided ? 1 : 0 );

    AssetWriterHelpers::WriteProperties( m_writer, materialAsset.Properties );
    m_writer->Flush( );
}
