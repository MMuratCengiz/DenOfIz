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

#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetReader.h"

#include "DenOfIzGraphics/Assets/Serde/Physics/PhysicsAsset.h"
#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetReaderHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;
MeshAssetReader::MeshAssetReader( const MeshAssetReaderDesc &desc ) : m_reader( desc.Reader ), m_desc( desc ), m_metadataRead( false )
{
    if ( !m_reader )
    {
        spdlog::critical( "BinaryReader cannot be null for MeshAssetReader" );
    }
}

MeshAssetReader::~MeshAssetReader( ) = default;

BoundingVolume MeshAssetReader::ReadBoundingVolume( ) const
{
    BoundingVolume bv;
    bv.Type = static_cast<BoundingVolumeType>( m_reader->ReadUInt32( ) );
    bv.Name = m_reader->ReadString( );
    switch ( bv.Type )
    {
    case BoundingVolumeType::Box:
        bv.Box.Min = m_reader->ReadFloat_3( );
        bv.Box.Max = m_reader->ReadFloat_3( );
        break;
    case BoundingVolumeType::Sphere:
        bv.Sphere.Center = m_reader->ReadFloat_3( );
        bv.Sphere.Radius = m_reader->ReadFloat( );
        break;
    case BoundingVolumeType::Capsule:
        bv.Capsule.Start  = m_reader->ReadFloat_3( );
        bv.Capsule.End    = m_reader->ReadFloat_3( );
        bv.Capsule.Radius = m_reader->ReadFloat( );
        break;
    case BoundingVolumeType::ConvexHull:
        bv.ConvexHull.VertexStream = AssetReaderHelpers::ReadAssetDataStream( m_reader );
        break;
    }
    return bv;
}

SubMeshData MeshAssetReader::ReadCompleteSubMeshData( ) const
{
    SubMeshData data;
    data.Name         = m_reader->ReadString( );
    data.Topology     = static_cast<PrimitiveTopology>( m_reader->ReadUInt32( ) );
    data.NumVertices  = m_reader->ReadUInt64( );
    data.VertexStream = AssetReaderHelpers::ReadAssetDataStream( m_reader );
    data.NumIndices   = m_reader->ReadUInt64( );
    data.IndexType    = static_cast<IndexType>( m_reader->ReadUInt32( ) );
    data.IndexStream  = AssetReaderHelpers::ReadAssetDataStream( m_reader );
    data.MinBounds    = m_reader->ReadFloat_3( );
    data.MaxBounds    = m_reader->ReadFloat_3( );
    data.MaterialRef  = AssetUri::Parse( m_reader->ReadString( ) );
    data.LODLevel     = m_reader->ReadUInt32( );

    const uint32_t bvCount = m_reader->ReadUInt32( );
    DZArenaArrayHelper<BoundingVolumeArray, BoundingVolume>::AllocateAndConstructArray( m_meshAsset->_Arena, data.BoundingVolumes, bvCount );
    for ( uint32_t j = 0; j < bvCount; ++j )
    {
        data.BoundingVolumes.Elements[ j ] = ReadBoundingVolume( );
    }
    return data;
}

MorphTarget MeshAssetReader::ReadCompleteMorphTargetData( ) const
{
    MorphTarget data;
    data.Name              = m_reader->ReadString( );
    data.VertexDeltaStream = AssetReaderHelpers::ReadAssetDataStream( m_reader );
    data.DefaultWeight     = m_reader->ReadFloat( );
    return data;
}

uint32_t MeshAssetReader::VertexEntryNumBytes( ) const
{
    uint32_t    size       = 0;
    const auto &attributes = m_meshAsset->EnabledAttributes;
    const auto &config     = m_meshAsset->AttributeConfig;
    if ( attributes.Position )
    {
        size += 4 * sizeof( float );
    }
    if ( attributes.Normal )
    {
        size += 4 * sizeof( float );
    }
    if ( attributes.UV )
    {
        size += config.NumUVAttributes * 2 * sizeof( float );
    }
    if ( attributes.Color )
    {
        for ( size_t i = 0; i < config.ColorFormats.NumElements; ++i )
        {
            switch ( config.ColorFormats.Elements[ i ] )
            {
            case ColorFormat::RGBA:
                size += 4 * sizeof( float );
                break;
            case ColorFormat::RGB:
                size += 3 * sizeof( float );
                break;
            case ColorFormat::RG:
                size += 2 * sizeof( float );
                break;
            case ColorFormat::R:
                size += 1 * sizeof( float );
                break;
            }
        }
    }
    if ( attributes.Tangent )
    {
        size += 4 * sizeof( float );
    }
    if ( attributes.Bitangent )
    {
        size += 4 * sizeof( float );
    }
    if ( attributes.BlendIndices )
    {
        size += config.MaxBoneInfluences * sizeof( uint32_t );
    }
    if ( attributes.BlendWeights )
    {
        size += config.MaxBoneInfluences * sizeof( float );
    }
    return size;
}

uint32_t MeshAssetReader::MorphDeltaEntryNumBytes( ) const
{
    uint32_t    size       = 0;
    const auto &attributes = m_meshAsset->MorphTargetDeltaAttributes;
    if ( attributes.Position )
    {
        size += sizeof( Float_4 );
    }
    if ( attributes.Normal )
    {
        size += sizeof( Float_4 );
    }
    if ( attributes.Tangent )
    {
        size += sizeof( Float_4 );
    }
    return size;
}

MeshVertex MeshAssetReader::ReadSingleVertex( ) const
{
    MeshVertex  vertex;
    const auto &attributes = m_meshAsset->EnabledAttributes;
    const auto &config     = m_meshAsset->AttributeConfig;

    if ( attributes.Position )
    {
        vertex.Position = m_reader->ReadFloat_4( );
    }
    if ( attributes.Normal )
    {
        vertex.Normal = m_reader->ReadFloat_4( );
    }
    if ( attributes.UV )
    {
        DZArenaArrayHelper<Float_2Array, Float_2>::AllocateAndConstructArray( m_meshAsset->_Arena, vertex.UVs, config.NumUVAttributes );
        for ( uint32_t i = 0; i < config.NumUVAttributes; ++i )
        {
            vertex.UVs.Elements[ i ] = m_reader->ReadFloat_2( );
        }
    }
    if ( attributes.Color )
    {
        DZArenaArrayHelper<Float_4Array, Float_4>::AllocateAndConstructArray( m_meshAsset->_Arena, vertex.Colors, config.ColorFormats.NumElements );
        for ( size_t i = 0; i < config.ColorFormats.NumElements; ++i )
        {
            Float_4 colorRead = { 0.0f, 0.0f, 0.0f, 1.0f };
            switch ( config.ColorFormats.Elements[ i ] )
            {
            case ColorFormat::RGBA:
                {
                    colorRead = m_reader->ReadFloat_4( );
                }
                break;
            case ColorFormat::RGB:
                {
                    const Float_3 rgb = m_reader->ReadFloat_3( );
                    colorRead         = { rgb.X, rgb.Y, rgb.Z, 1.0f };
                }
                break;
            case ColorFormat::RG:
                {
                    const Float_2 rg = m_reader->ReadFloat_2( );
                    colorRead        = { rg.X, rg.Y, 0.0f, 1.0f };
                }
                break;
            case ColorFormat::R:
                {
                    const float r = m_reader->ReadFloat( );
                    colorRead     = { r, 0.0f, 0.0f, 1.0f };
                }
                break;
            }
            vertex.Colors.Elements[ i ] = colorRead;
        }
    }
    if ( attributes.Tangent )
    {
        vertex.Tangent = m_reader->ReadFloat_4( );
    }
    if ( attributes.Bitangent )
    {
        vertex.Bitangent = m_reader->ReadFloat_4( );
    }
    if ( attributes.BlendIndices )
    {
        vertex.BlendIndices = m_reader->ReadUInt32_4( );
    }
    if ( attributes.BlendWeights )
    {
        vertex.BoneWeights = m_reader->ReadFloat_4( );
    }
    return vertex;
}

MorphTargetDelta MeshAssetReader::ReadSingleMorphTargetDelta( ) const
{
    MorphTargetDelta delta{ };
    const auto      &attributes = m_meshAsset->MorphTargetDeltaAttributes;
    if ( attributes.Position )
    {
        delta.Position = m_reader->ReadFloat_4( );
    }
    if ( attributes.Normal )
    {
        delta.Normal = m_reader->ReadFloat_4( );
    }
    if ( attributes.Tangent )
    {
        delta.Tangent = m_reader->ReadFloat_4( );
    }
    return delta;
}

MeshAsset *MeshAssetReader::Read( )
{
    if ( m_metadataRead )
    {
        spdlog::warn( "ReadMetadata called more than once." );
        m_reader->Seek( m_dataBlockStartOffset );
        return m_meshAsset;
    }
    m_meshAsset        = new MeshAsset( );
    m_meshAsset->Magic = m_reader->ReadUInt64( );
    if ( m_meshAsset->Magic != MeshAsset{ }.Magic )
    {
        spdlog::critical( "Invalid MeshAsset magic number." );
    }
    m_meshAsset->Version = m_reader->ReadUInt32( );
    if ( m_meshAsset->Version > MeshAsset::Latest )
    {
        spdlog::warn( "MeshAsset version mismatch." );
    }
    m_meshAsset->NumBytes = m_reader->ReadUInt64( );
    m_meshAsset->Uri      = AssetUri::Parse( m_reader->ReadString( ) );
    m_meshAsset->_Arena.EnsureCapacity( m_meshAsset->NumBytes );

    m_meshAsset->Name                           = m_reader->ReadString( );
    m_meshAsset->NumLODs                        = m_reader->ReadUInt32( );
    const uint32_t enabledFlags                 = m_reader->ReadUInt32( );
    m_meshAsset->EnabledAttributes.Position     = enabledFlags & 1 << 0;
    m_meshAsset->EnabledAttributes.Normal       = enabledFlags & 1 << 1;
    m_meshAsset->EnabledAttributes.UV           = enabledFlags & 1 << 2;
    m_meshAsset->EnabledAttributes.Color        = enabledFlags & 1 << 3;
    m_meshAsset->EnabledAttributes.Tangent      = enabledFlags & 1 << 4;
    m_meshAsset->EnabledAttributes.Bitangent    = enabledFlags & 1 << 5;
    m_meshAsset->EnabledAttributes.BlendIndices = enabledFlags & 1 << 6;
    m_meshAsset->EnabledAttributes.BlendWeights = enabledFlags & 1 << 7;

    m_meshAsset->AttributeConfig.NumPositionComponents = m_reader->ReadUInt32( );
    m_meshAsset->AttributeConfig.NumUVAttributes       = m_reader->ReadUInt32( );
    const uint32_t uvChanCount                         = m_reader->ReadUInt32( );
    DZArenaArrayHelper<UVChannelArray, UVChannel>::AllocateAndConstructArray( m_meshAsset->_Arena, m_meshAsset->AttributeConfig.UVChannels, uvChanCount );
    for ( size_t i = 0; i < uvChanCount; ++i )
    {
        auto &channel        = m_meshAsset->AttributeConfig.UVChannels.Elements[ i ];
        channel.SemanticName = m_reader->ReadString( );
        channel.Index        = m_reader->ReadUInt32( );
    }
    const uint32_t colorFmtCount = m_reader->ReadUInt32( );
    DZArenaArrayHelper<ColorFormatArray, ColorFormat>::AllocateAndConstructArray( m_meshAsset->_Arena, m_meshAsset->AttributeConfig.ColorFormats, colorFmtCount );
    for ( size_t i = 0; i < colorFmtCount; ++i )
    {
        m_meshAsset->AttributeConfig.ColorFormats.Elements[ i ] = static_cast<ColorFormat>( m_reader->ReadUInt32( ) );
    }
    m_meshAsset->AttributeConfig.MaxBoneInfluences = m_reader->ReadUInt32( );

    const uint32_t morphFlags                        = m_reader->ReadUInt32( );
    m_meshAsset->MorphTargetDeltaAttributes.Position = morphFlags & 1 << 0;
    m_meshAsset->MorphTargetDeltaAttributes.Normal   = morphFlags & 1 << 1;
    m_meshAsset->MorphTargetDeltaAttributes.Tangent  = morphFlags & 1 << 2;

    const uint32_t numAnimationRefs = m_reader->ReadUInt32( );
    DZArenaArrayHelper<AssetUriArray, AssetUri>::AllocateAndConstructArray( m_meshAsset->_Arena, m_meshAsset->AnimationRefs, numAnimationRefs );
    for ( uint32_t i = 0; i < numAnimationRefs; ++i )
    {
        m_meshAsset->AnimationRefs.Elements[ i ] = AssetUri::Parse( m_reader->ReadString( ) );
    }
    m_meshAsset->SkeletonRef = AssetUri::Parse( m_reader->ReadString( ) );

    const uint32_t numSubMeshes = m_reader->ReadUInt32( );
    DZArenaArrayHelper<SubMeshDataArray, SubMeshData>::AllocateAndConstructArray( m_meshAsset->_Arena, m_meshAsset->SubMeshes, numSubMeshes );
    for ( uint32_t i = 0; i < numSubMeshes; ++i )
    {
        m_meshAsset->SubMeshes.Elements[ i ] = ReadCompleteSubMeshData( );
    }

    const uint32_t numMorphTargets = m_reader->ReadUInt32( );
    DZArenaArrayHelper<MorphTargetArray, MorphTarget>::AllocateAndConstructArray( m_meshAsset->_Arena, m_meshAsset->MorphTargets, numMorphTargets );
    for ( uint32_t i = 0; i < numMorphTargets; ++i )
    {
        m_meshAsset->MorphTargets.Elements[ i ] = ReadCompleteMorphTargetData( );
    }

    m_meshAsset->UserProperties = AssetReaderHelpers::ReadUserProperties( &m_meshAsset->_Arena, m_reader );
    m_dataBlockStartOffset      = m_reader->Position( );
    m_metadataRead              = true;
    m_reader->Seek( m_dataBlockStartOffset );

    return m_meshAsset;
}

void MeshAssetReader::LoadStreamToMemory( LoadToMemoryDesc &desc ) const
{
    if ( !m_metadataRead )
    {
        spdlog::critical( "ReadMetadata must be called first." );
    }
    if ( !desc.Memory.Elements )
    {
        spdlog::critical( "Destination memory array cannot be null." );
        return;
    }
    if ( desc.Stream.NumBytes == 0 )
    {
        return;
    }

    m_reader->Seek( desc.Stream.Offset );
    if ( desc.Memory.NumElements < desc.DstMemoryOffset + desc.Stream.NumBytes )
    {
        const auto newMemory    = static_cast<uint8_t *>( std::realloc( desc.Memory.Elements, desc.DstMemoryOffset + desc.Stream.NumBytes ) );
        desc.Memory.Elements    = newMemory;
        desc.Memory.NumElements = desc.DstMemoryOffset + desc.Stream.NumBytes;
    }
    uint64_t bytesCopied = 0;
    while ( bytesCopied < desc.Stream.NumBytes )
    {
        constexpr size_t chunkSize         = 65536;
        const uint64_t   bytesToRead       = std::min<uint32_t>( chunkSize, desc.Stream.NumBytes - bytesCopied );
        const int        bytesActuallyRead = m_reader->Read( desc.Memory, static_cast<uint32_t>( desc.DstMemoryOffset + bytesCopied ), static_cast<uint32_t>( bytesToRead ) );
        if ( bytesActuallyRead != static_cast<int>( bytesToRead ) )
        {
            spdlog::critical( "Failed to read expected chunk size from mesh asset stream into memory." );
        }
        bytesCopied += bytesActuallyRead;
    }
}

size_t MeshAssetReader::NumVertices( const AssetDataStream &stream ) const
{
    return stream.NumBytes / VertexEntryNumBytes( );
}

size_t MeshAssetReader::NumIndices16( const AssetDataStream &stream ) const
{
    return stream.NumBytes / sizeof( uint16_t );
}

size_t MeshAssetReader::NumIndices32( const AssetDataStream &stream ) const
{
    return stream.NumBytes / sizeof( uint32_t );
}

size_t MeshAssetReader::NumMorphTargets( const AssetDataStream &stream ) const
{
    return stream.NumBytes / MorphDeltaEntryNumBytes( );
}

size_t MeshAssetReader::NumConvexHulls( const AssetDataStream &stream ) const
{
    return stream.NumBytes /* TODO */;
}

void MeshAssetReader::ReadVertices( const AssetDataStream &stream, const MeshVertexArray &result ) const
{
    if ( !m_metadataRead )
    {
        spdlog::critical( "ReadMetadata must be called first." );
    }
    const uint32_t vertexSize = VertexEntryNumBytes( );
    if ( vertexSize == 0 || stream.NumBytes == 0 )
    {
        return;
    }
    const uint64_t numVertices = NumVertices( stream );
    if ( result.NumElements < numVertices )
    {
        spdlog::critical( "Destination memory array is too small, allocate at least NumVertices( ) amount of elements" );
    }
    m_reader->Seek( stream.Offset );
    for ( uint64_t i = 0; i < numVertices; ++i )
    {
        result.Elements[ i ] = ReadSingleVertex( );
    }
}

void MeshAssetReader::ReadIndices16( const AssetDataStream &stream, const UInt16Array &result ) const
{
    if ( !m_metadataRead )
    {
        spdlog::critical( "ReadMetadata must be called first." );
    }
    constexpr uint32_t indexSize = sizeof( uint16_t );
    if ( stream.NumBytes == 0 )
    {
        return;
    }
    const uint64_t numIndices = stream.NumBytes / indexSize;
    if ( result.NumElements < numIndices )
    {
        spdlog::critical( "Destination memory array is too small, allocate at least NumIndices16( ) amount of elements" );
    }
    m_reader->Seek( stream.Offset );
    for ( uint64_t i = 0; i < numIndices; ++i )
    {
        result.Elements[ i ] = m_reader->ReadUInt16( );
    }
}

void MeshAssetReader::ReadIndices32( const AssetDataStream &stream, const UInt32Array &result ) const
{
    if ( !m_metadataRead )
    {
        spdlog::critical( "ReadMetadata must be called first." );
    }
    constexpr uint32_t indexSize = sizeof( uint32_t );
    if ( stream.NumBytes == 0 )
    {
        return;
    }
    const uint64_t numIndices = stream.NumBytes / indexSize;
    if ( stream.NumBytes % indexSize != 0 )
    {
        spdlog::warn( "Index stream size warning for stream with offset {}", stream.Offset );
    }
    if ( result.NumElements < numIndices )
    {
        spdlog::critical( "Destination memory array is too small, allocate at least NumIndices32( ) amount of elements" );
        return;
    }
    m_reader->Seek( stream.Offset );
    for ( uint64_t i = 0; i < numIndices; ++i )
    {
        result.Elements[ i ] = m_reader->ReadUInt32( );
    }
}

void MeshAssetReader::ReadMorphTargetDeltas( const AssetDataStream &stream, const MorphTargetDeltaArray &result ) const
{
    if ( !m_metadataRead )
    {
        spdlog::critical( "ReadMetadata must be called first." );
    }
    const uint32_t deltaSize = MorphDeltaEntryNumBytes( );
    if ( deltaSize == 0 || stream.NumBytes == 0 )
    {
        return;
    }
    const uint64_t numDeltas = NumMorphTargets( stream );
    if ( result.NumElements < numDeltas )
    {
        spdlog::critical( "Destination memory array is too small, allocate at least NumMorphTargets( ) amount of data" );
    }
    m_reader->Seek( stream.Offset );
    for ( uint64_t i = 0; i < numDeltas; ++i )
    {
        result.Elements[ i ] = ReadSingleMorphTargetDelta( );
    }
}

void MeshAssetReader::ReadConvexHullData( const AssetDataStream &stream, ByteArray &result ) const
{
    if ( !m_metadataRead )
    {
        spdlog::critical( "ReadMetadata must be called first." );
    }
    if ( result.NumElements < stream.NumBytes )
    {
        spdlog::critical( "Destination memory array is too small, allocate at least stream.NumBytes amount of bytes" /*TODO explain size requirements*/ );
    }
    if ( stream.NumBytes == 0 )
    {
        return;
    }
    m_reader->Seek( stream.Offset );
    result = m_reader->ReadBytes( static_cast<uint32_t>( stream.NumBytes ) );
}
