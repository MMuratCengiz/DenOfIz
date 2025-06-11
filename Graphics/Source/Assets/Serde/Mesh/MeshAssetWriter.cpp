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
#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include <unordered_map>

using namespace DenOfIz;

MeshAssetWriter::MeshAssetWriter( const MeshAssetWriterDesc &desc ) : m_writer( desc.Writer ), m_desc( desc ), m_state( State::Idle )
{
    if ( !m_writer )
    {
        spdlog::critical( "BinaryWriter cannot be null for MeshAssetWriter" );
    }
}

MeshAssetWriter::~MeshAssetWriter( ) = default;

void MeshAssetWriter::CalculateStrides( )
{
    m_vertexStride         = 0;
    const auto &attributes = m_meshAsset.EnabledAttributes;
    const auto &config     = m_meshAsset.AttributeConfig;
    if ( attributes.Position )
    {
        m_vertexStride += 4 * sizeof( float );
    }
    if ( attributes.Normal )
    {
        m_vertexStride += 4 * sizeof( float );
    }
    if ( attributes.UV )
    {
        m_vertexStride += config.NumUVAttributes * 2 * sizeof( float );
    }
    if ( attributes.Color )
    {
        for ( size_t i = 0; i < config.ColorFormats.NumElements; ++i )
        {
            switch ( config.ColorFormats.Elements[ i ] )
            {
            case ColorFormat::RGBA:
                m_vertexStride += 4 * sizeof( float );
                break;
            case ColorFormat::RGB:
                m_vertexStride += 3 * sizeof( float );
                break;
            case ColorFormat::RG:
                m_vertexStride += 2 * sizeof( float );
                break;
            case ColorFormat::R:
                m_vertexStride += 1 * sizeof( float );
                break;
            }
        }
    }
    if ( attributes.Tangent )
    {
        m_vertexStride += 4 * sizeof( float );
    }
    if ( attributes.Bitangent )
    {
        m_vertexStride += 4 * sizeof( float );
    }
    if ( attributes.BlendIndices )
    {
        m_vertexStride += config.MaxBoneInfluences * sizeof( uint32_t );
    }
    if ( attributes.BlendWeights )
    {
        m_vertexStride += config.MaxBoneInfluences * sizeof( float );
    }

    m_morphDeltaStride       = 0;
    const auto &deltaAttribs = m_meshAsset.MorphTargetDeltaAttributes;
    if ( deltaAttribs.Position )
    {
        m_morphDeltaStride += sizeof( Float_4 );
    }
    if ( deltaAttribs.Normal )
    {
        m_morphDeltaStride += sizeof( Float_4 );
    }
    if ( deltaAttribs.Tangent )
    {
        m_morphDeltaStride += sizeof( Float_4 );
    }
}

void MeshAssetWriter::WriteBoundingVolume( const BoundingVolume &bv ) const
{
    m_writer->WriteUInt32( static_cast<uint32_t>( bv.Type ) );
    m_writer->WriteString( bv.Name );
    switch ( bv.Type )
    {
    case BoundingVolumeType::Box:
        m_writer->WriteFloat_3( bv.Box.Min );
        m_writer->WriteFloat_3( bv.Box.Max );
        break;
    case BoundingVolumeType::Sphere:
        m_writer->WriteFloat_3( bv.Sphere.Center );
        m_writer->WriteFloat( bv.Sphere.Radius );
        break;
    case BoundingVolumeType::Capsule:
        m_writer->WriteFloat_3( bv.Capsule.Start );
        m_writer->WriteFloat_3( bv.Capsule.End );
        m_writer->WriteFloat( bv.Capsule.Radius );
        break;
    case BoundingVolumeType::ConvexHull:
        AssetWriterHelpers::WriteAssetDataStream( m_writer, bv.ConvexHull.VertexStream );
        break;
    }
}

void MeshAssetWriter::WriteSubMeshData( const SubMeshData &data ) const
{
    m_writer->WriteString( data.Name );
    m_writer->WriteUInt32( static_cast<uint32_t>( data.Topology ) );
    m_writer->WriteUInt64( data.NumVertices );
    AssetWriterHelpers::WriteAssetDataStream( m_writer, data.VertexStream );
    m_writer->WriteUInt64( data.NumIndices );
    m_writer->WriteUInt32( static_cast<uint32_t>( data.IndexType ) );
    AssetWriterHelpers::WriteAssetDataStream( m_writer, data.IndexStream );
    m_writer->WriteFloat_3( data.MinBounds );
    m_writer->WriteFloat_3( data.MaxBounds );
    m_writer->WriteString( data.MaterialRef.ToInteropString( ) );
    m_writer->WriteUInt32( data.LODLevel );
    m_writer->WriteUInt32( data.BoundingVolumes.NumElements );
    for ( size_t j = 0; j < data.BoundingVolumes.NumElements; ++j )
    {
        WriteBoundingVolume( data.BoundingVolumes.Elements[ j ] );
    }
}

void MeshAssetWriter::WriteMorphTargetData( const MorphTarget &data ) const
{
    m_writer->WriteString( data.Name );
    AssetWriterHelpers::WriteAssetDataStream( m_writer, data.VertexDeltaStream );
    m_writer->WriteFloat( data.DefaultWeight );
}

void MeshAssetWriter::WriteHeader( const uint64_t totalNumBytes )
{
    m_streamStartLocation = m_writer->Position( );

    m_writer->WriteUInt64( m_meshAsset.Magic );
    m_writer->WriteUInt32( m_meshAsset.Version );
    m_writer->WriteUInt64( totalNumBytes ); // NumBytes, Will be written later at finalize!!
    m_writer->WriteString( m_meshAsset.Uri.ToInteropString( ) );

    WriteTopLevelMetadata( );
    WriteMetadataArrays( );
}

void MeshAssetWriter::WriteTopLevelMetadata( )
{
    m_writer->WriteString( m_meshAsset.Name );
    m_writer->WriteUInt32( m_meshAsset.NumLODs );

    uint32_t enabledFlags = 0;
    if ( m_meshAsset.EnabledAttributes.Position )
    {
        enabledFlags |= 1 << 0;
    }
    if ( m_meshAsset.EnabledAttributes.Normal )
    {
        enabledFlags |= 1 << 1;
    }
    if ( m_meshAsset.EnabledAttributes.UV )
    {
        enabledFlags |= 1 << 2;
    }
    if ( m_meshAsset.EnabledAttributes.Color )
    {
        enabledFlags |= 1 << 3;
    }
    if ( m_meshAsset.EnabledAttributes.Tangent )
    {
        enabledFlags |= 1 << 4;
    }
    if ( m_meshAsset.EnabledAttributes.Bitangent )
    {
        enabledFlags |= 1 << 5;
    }
    if ( m_meshAsset.EnabledAttributes.BlendIndices )
    {
        enabledFlags |= 1 << 6;
    }
    if ( m_meshAsset.EnabledAttributes.BlendWeights )
    {
        enabledFlags |= 1 << 7;
    }
    m_writer->WriteUInt32( enabledFlags );
    m_writer->WriteUInt32( m_meshAsset.AttributeConfig.NumPositionComponents );
    m_writer->WriteUInt32( m_meshAsset.AttributeConfig.NumUVAttributes );
    m_writer->WriteUInt32( m_meshAsset.AttributeConfig.UVChannels.NumElements );
    for ( size_t i = 0; i < m_meshAsset.AttributeConfig.UVChannels.NumElements; ++i )
    {
        const auto &channel = m_meshAsset.AttributeConfig.UVChannels.Elements[ i ];
        m_writer->WriteString( channel.SemanticName );
        m_writer->WriteUInt32( channel.Index );
    }
    m_writer->WriteUInt32( m_meshAsset.AttributeConfig.ColorFormats.NumElements );
    for ( size_t i = 0; i < m_meshAsset.AttributeConfig.ColorFormats.NumElements; ++i )
    {
        m_writer->WriteUInt32( static_cast<uint32_t>( m_meshAsset.AttributeConfig.ColorFormats.Elements[ i ] ) );
    }
    m_writer->WriteUInt32( m_meshAsset.AttributeConfig.MaxBoneInfluences );

    uint32_t morphFlags = 0;
    if ( m_meshAsset.MorphTargetDeltaAttributes.Position )
    {
        morphFlags |= 1 << 0;
    }
    if ( m_meshAsset.MorphTargetDeltaAttributes.Normal )
    {
        morphFlags |= 1 << 1;
    }
    if ( m_meshAsset.MorphTargetDeltaAttributes.Tangent )
    {
        morphFlags |= 1 << 2;
    }
    m_writer->WriteUInt32( morphFlags );
    m_writer->WriteUInt32( m_meshAsset.AnimationRefs.NumElements );
    for ( size_t i = 0; i < m_meshAsset.AnimationRefs.NumElements; ++i )
    {
        m_writer->WriteString( m_meshAsset.AnimationRefs.Elements[ i ].ToInteropString( ) );
    }
    m_writer->WriteString( m_meshAsset.SkeletonRef.ToInteropString( ) );
}

void MeshAssetWriter::WriteMetadataArrays( )
{
    m_writer->WriteUInt32( m_meshAsset.SubMeshes.NumElements );
    for ( size_t i = 0; i < m_meshAsset.SubMeshes.NumElements; ++i )
    {
        WriteSubMeshData( m_meshAsset.SubMeshes.Elements[ i ] );
    }

    m_writer->WriteUInt32( m_meshAsset.MorphTargets.NumElements );
    for ( size_t i = 0; i < m_meshAsset.MorphTargets.NumElements; ++i )
    {
        WriteMorphTargetData( m_meshAsset.MorphTargets.Elements[ i ] );
    }

    AssetWriterHelpers::WriteProperties( m_writer, m_meshAsset.UserProperties );
}

void MeshAssetWriter::WriteVertexInternal( const MeshVertex &vertex ) const
{
    const auto &attributes = m_meshAsset.EnabledAttributes;
    const auto &config     = m_meshAsset.AttributeConfig;

    if ( attributes.Position )
    {
        m_writer->WriteFloat_4( vertex.Position );
    }
    if ( attributes.Normal )
    {
        m_writer->WriteFloat_4( vertex.Normal );
    }
    if ( attributes.UV )
    {
        for ( size_t i = 0; i < config.NumUVAttributes; ++i )
        {
            if ( i < vertex.UVs.NumElements )
            {
                m_writer->WriteFloat_2( vertex.UVs.Elements[ i ] );
            }
            else
            {
                m_writer->WriteFloat( 0.0f );
                m_writer->WriteFloat( 0.0f );
            }
        }
    }
    if ( attributes.Color )
    {
        for ( size_t i = 0; i < config.ColorFormats.NumElements; ++i )
        {
            Float_4 colorToWrite = { 0.0f, 0.0f, 0.0f, 1.0f };
            if ( i < vertex.Colors.NumElements )
            {
                colorToWrite = vertex.Colors.Elements[ i ];
            }
            switch ( config.ColorFormats.Elements[ i ] )
            {
            case ColorFormat::RGBA:
                m_writer->WriteFloat_4( colorToWrite );
                break;
            case ColorFormat::RGB:
                m_writer->WriteFloat_3( { colorToWrite.X, colorToWrite.Y, colorToWrite.Z } );
                break;
            case ColorFormat::RG:
                m_writer->WriteFloat_2( { colorToWrite.X, colorToWrite.Y } );
                break;
            case ColorFormat::R:
                m_writer->WriteFloat( colorToWrite.X );
                break;
            }
        }
    }
    if ( attributes.Tangent )
    {
        m_writer->WriteFloat_4( vertex.Tangent );
    }
    if ( attributes.Bitangent )
    {
        m_writer->WriteFloat_4( vertex.Bitangent );
    }
    if ( attributes.BlendIndices )
    {
        m_writer->WriteUInt32_4( vertex.BlendIndices );
    }
    if ( attributes.BlendWeights )
    {
        m_writer->WriteFloat_4( vertex.BoneWeights );
    }
}

void MeshAssetWriter::WriteMorphTargetDeltaInternal( const MorphTargetDelta &delta ) const
{
    const auto &attributes = m_meshAsset.MorphTargetDeltaAttributes;
    if ( attributes.Position )
    {
        m_writer->WriteFloat_4( delta.Position );
    }
    if ( attributes.Normal )
    {
        m_writer->WriteFloat_4( delta.Normal );
    }
    if ( attributes.Tangent )
    {
        m_writer->WriteFloat_4( delta.Tangent );
    }
}

void MeshAssetWriter::Write( const MeshAsset &meshAssetData )
{
    if ( m_state != State::Idle )
    {
        spdlog::critical( "WriteMetadata can only be called once at the beginning." );
    }

    m_meshAsset                = meshAssetData;
    m_expectedSubMeshCount     = m_meshAsset.SubMeshes.NumElements;
    m_expectedMorphTargetCount = m_meshAsset.MorphTargets.NumElements;
    CalculateStrides( );

    m_state                   = State::ReadyToWriteData;
    m_currentSubMeshIndex     = 0;
    m_currentMorphTargetIndex = 0;
    m_writtenSubMeshCount     = 0;
    m_writtenMorphTargetCount = 0;
    m_numVertices             = 0;
    m_numIndices              = 0;
    m_numDeltas               = 0;

    WriteHeader( 0 ); // Do not write num bytes just yet since we don't know data required for the streams
}

void MeshAssetWriter::AddVertex( const MeshVertex &vertex )
{
    if ( m_state != State::ReadyToWriteData && m_state != State::WritingVertices )
    {
        spdlog::critical( "AddVertex called at invalid state {}", static_cast<int>( m_state ) );
    }
    if ( m_currentSubMeshIndex >= m_expectedSubMeshCount )
    {
        spdlog::critical( "AddVertex called after all SubMeshes should have been written." );
    }

    SubMeshData &currentSubMesh = m_meshAsset.SubMeshes.Elements[ m_currentSubMeshIndex ];
    if ( m_numVertices == 0 )
    {
        m_state                            = State::WritingVertices;
        currentSubMesh.VertexStream.Offset = m_writer->Position( );
    }

    WriteVertexInternal( vertex );
    m_numVertices++;

    if ( m_numVertices == currentSubMesh.NumVertices )
    {
        currentSubMesh.VertexStream.NumBytes = m_numVertices * m_vertexStride;
        m_state                              = State::ExpectingIndices;
        m_numIndices                         = 0;
        if ( currentSubMesh.NumIndices == 0 )
        {
            m_state          = State::ExpectingHulls;
            m_currentBVIndex = 0;

            bool hasHulls = false;
            for ( size_t i = 0; i < currentSubMesh.BoundingVolumes.NumElements; ++i )
            {
                if ( currentSubMesh.BoundingVolumes.Elements[ i ].Type == BoundingVolumeType::ConvexHull )
                {
                    hasHulls = true;
                    break;
                }
            }
            if ( !hasHulls )
            {
                m_writtenSubMeshCount++;
                m_state               = m_writtenSubMeshCount < m_expectedSubMeshCount ? State::ReadyToWriteData : State::ExpectingMorphTarget;
                m_currentSubMeshIndex = m_writtenSubMeshCount;
                m_numVertices         = 0;
                m_numIndices          = 0;
            }
        }
    }
}

void MeshAssetWriter::AddIndex16( const uint16_t index )
{
    if ( m_state != State::ExpectingIndices && m_state != State::WritingIndices )
    {
        spdlog::critical( "AddIndex16 called at invalid state {}", static_cast<int>( m_state ) );
    }
    SubMeshData &currentSubMesh = m_meshAsset.SubMeshes.Elements[ m_currentSubMeshIndex ];
    if ( currentSubMesh.IndexType != IndexType::Uint16 )
    {
        spdlog::warn( "Adding uint16 index to SubMesh {} expecting Uint32.", m_currentSubMeshIndex );
    }

    if ( m_state == State::ExpectingIndices )
    {
        currentSubMesh.IndexStream.Offset = m_writer->Position( );
        m_state                           = State::WritingIndices;
    }

    m_writer->WriteUInt16( index );
    m_numIndices++;

    if ( m_numIndices == currentSubMesh.NumIndices )
    {
        currentSubMesh.IndexStream.NumBytes = m_numIndices * sizeof( uint16_t );
        m_state                             = State::ExpectingHulls;
        m_currentBVIndex                    = 0;

        bool hasHulls = false;
        for ( size_t i = 0; i < currentSubMesh.BoundingVolumes.NumElements; ++i )
        {
            if ( currentSubMesh.BoundingVolumes.Elements[ i ].Type == BoundingVolumeType::ConvexHull )
            {
                hasHulls = true;
                break;
            }
        }
        if ( !hasHulls )
        {
            m_writtenSubMeshCount++;
            m_state               = m_writtenSubMeshCount < m_expectedSubMeshCount ? State::ReadyToWriteData : State::ExpectingMorphTarget;
            m_currentSubMeshIndex = m_writtenSubMeshCount;
            m_numVertices         = 0;
            m_numIndices          = 0;
        }
    }
}

void MeshAssetWriter::AddIndex32( const uint32_t index )
{
    if ( m_state != State::ExpectingIndices && m_state != State::WritingIndices )
    {
        spdlog::critical( "AddIndex32 called at invalid state {}", static_cast<int>( m_state ) );
    }
    SubMeshData &currentSubMesh = m_meshAsset.SubMeshes.Elements[ m_currentSubMeshIndex ];
    if ( currentSubMesh.IndexType != IndexType::Uint32 )
    {
        spdlog::warn( "Adding uint32 index to SubMesh {} expecting Uint16.", m_currentSubMeshIndex );
    }

    if ( m_state == State::ExpectingIndices )
    {
        currentSubMesh.IndexStream.Offset = m_writer->Position( );
        m_state                           = State::WritingIndices;
    }

    m_writer->WriteUInt32( index );
    m_numIndices++;

    if ( m_numIndices == currentSubMesh.NumIndices )
    {
        currentSubMesh.IndexStream.NumBytes = m_numIndices * sizeof( uint32_t );
        m_state                             = State::ExpectingHulls;
        m_currentBVIndex                    = 0;

        bool hasHulls = false;
        for ( size_t i = 0; i < currentSubMesh.BoundingVolumes.NumElements; ++i )
        {
            if ( currentSubMesh.BoundingVolumes.Elements[ i ].Type == BoundingVolumeType::ConvexHull )
            {
                hasHulls = true;
                break;
            }
        }
        if ( !hasHulls )
        {
            m_writtenSubMeshCount++;
            m_state               = m_writtenSubMeshCount < m_expectedSubMeshCount ? State::ReadyToWriteData : State::ExpectingMorphTarget;
            m_currentSubMeshIndex = m_writtenSubMeshCount;
            m_numVertices         = 0;
            m_numIndices          = 0;
        }
    }
}

void MeshAssetWriter::AddConvexHullData( const uint32_t boundingVolumeIndex, const ByteArrayView &vertexData )
{
    if ( m_state != State::ExpectingHulls && m_state != State::WritingHulls )
    {
        spdlog::critical( "AddConvexHullData called at invalid state {}", static_cast<int>( m_state ) );
    }
    SubMeshData &currentSubMesh = m_meshAsset.SubMeshes.Elements[ m_currentSubMeshIndex ];
    if ( boundingVolumeIndex >= currentSubMesh.BoundingVolumes.NumElements ||
         currentSubMesh.BoundingVolumes.Elements[ boundingVolumeIndex ].Type != BoundingVolumeType::ConvexHull )
    {
        spdlog::critical( "Invalid boundingVolumeIndex or not a ConvexHull type." );
    }

    if ( m_state == State::ExpectingHulls )
    {
        m_state = State::WritingHulls;
    }

    BoundingVolume &bv                = currentSubMesh.BoundingVolumes.Elements[ boundingVolumeIndex ];
    bv.ConvexHull.VertexStream.Offset = m_writer->Position( );
    m_writer->WriteBytes( vertexData );
    bv.ConvexHull.VertexStream.NumBytes = vertexData.NumElements;

    m_currentBVIndex++;

    uint32_t expectedHullCount = 0;
    for ( size_t i = 0; i < currentSubMesh.BoundingVolumes.NumElements; ++i )
    {
        if ( currentSubMesh.BoundingVolumes.Elements[ i ].Type == BoundingVolumeType::ConvexHull )
        {
            expectedHullCount++;
        }
    }

    if ( m_currentBVIndex == expectedHullCount )
    {
        m_writtenSubMeshCount++;
        m_state               = m_writtenSubMeshCount < m_expectedSubMeshCount ? State::ReadyToWriteData : State::ExpectingMorphTarget;
        m_currentSubMeshIndex = m_writtenSubMeshCount;
        m_numVertices         = 0;
        m_numIndices          = 0;
    }
}

void MeshAssetWriter::AddMorphTargetDelta( const MorphTargetDelta &delta )
{
    if ( m_state != State::ExpectingMorphTarget && m_state != State::WritingDeltas )
    {
        spdlog::critical( "AddMorphTargetDelta called at invalid state {}", static_cast<int>( m_state ) );
    }
    if ( m_currentMorphTargetIndex >= m_expectedMorphTargetCount )
    {
        spdlog::critical( "AddMorphTargetDelta called after all MorphTargets should have been written." );
    }

    MorphTarget &currentMorph = m_meshAsset.MorphTargets.Elements[ m_currentMorphTargetIndex ];

    if ( m_numDeltas == 0 )
    {
        m_state                               = State::WritingDeltas;
        currentMorph.VertexDeltaStream.Offset = m_writer->Position( );
    }

    WriteMorphTargetDeltaInternal( delta );
    m_numDeltas++;

    if ( m_numDeltas == m_meshAsset.SubMeshes.Elements[ 0 ].NumVertices )
    {
        currentMorph.VertexDeltaStream.NumBytes = m_numDeltas * m_morphDeltaStride;
        m_writtenMorphTargetCount++;
        m_numDeltas               = 0;
        m_currentMorphTargetIndex = m_writtenMorphTargetCount;
        m_state                   = m_writtenMorphTargetCount < m_expectedMorphTargetCount ? State::ExpectingMorphTarget : State::DataWritten;
    }
}

void MeshAssetWriter::FinalizeAsset( )
{
    if ( m_state != State::DataWritten && m_state != State::ExpectingMorphTarget && m_state != State::SubMeshEnded )
    {
        if ( !( m_state == State::ReadyToWriteData && m_expectedSubMeshCount == 0 && m_expectedMorphTargetCount == 0 ) )
        {
            spdlog::critical( "FinalizeAsset called in invalid state {} . Ensure all expected data was added.", static_cast<int>( m_state ) );
        }
    }
    if ( m_writtenSubMeshCount != m_expectedSubMeshCount )
    {
        spdlog::critical( "FinalizeAsset called but not all SubMeshes were written/ended." );
    }
    if ( m_writtenMorphTargetCount != m_expectedMorphTargetCount )
    {
        spdlog::critical( "FinalizeAsset called but not all MorphTargets were written/ended." );
    }

    const uint64_t currentPos = m_writer->Position( );
    m_writer->Seek( m_streamStartLocation );
    WriteHeader( currentPos ); // Rewrite headers again with populated NumBytes for the MeshAsset and all the AssetDataSteams within
    m_writer->Seek( currentPos );

    m_writer->Flush( );
    m_state = State::Finalized;
}
