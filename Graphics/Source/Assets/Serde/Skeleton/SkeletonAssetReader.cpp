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

#include "DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetReader.h"
#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetReader.h"
#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetReaderHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

SkeletonAssetReader::SkeletonAssetReader( const SkeletonAssetReaderDesc &desc ) : m_reader( desc.Reader )
{
    if ( !m_reader )
    {
        spdlog::critical( "BinaryReader cannot be null for SkeletonAssetReader" );
    }
}

SkeletonAssetReader::~SkeletonAssetReader( ) = default;

SkeletonAsset *SkeletonAssetReader::Read( )
{
    m_skeletonAsset = new SkeletonAsset( );

    m_skeletonAsset->Magic = m_reader->ReadUInt64( );
    if ( m_skeletonAsset->Magic != SkeletonAsset{ }.Magic )
    {
        spdlog::critical( "Invalid SkeletonAsset magic number." );
    }

    m_skeletonAsset->Version = m_reader->ReadUInt32( );
    if ( m_skeletonAsset->Version > SkeletonAsset::Latest )
    {
        spdlog::warn( "SkeletonAsset version mismatch." );
    }

    m_skeletonAsset->NumBytes = m_reader->ReadUInt64( );
    m_skeletonAsset->Uri      = AssetUri::Parse( m_reader->ReadString( ) );
    m_skeletonAsset->_Arena.EnsureCapacity( m_skeletonAsset->NumBytes );

    m_skeletonAsset->Name = m_reader->ReadString( );

    const uint32_t numJoints = m_reader->ReadUInt32( );
    DZArenaArrayHelper<JointArray, Joint>::AllocateAndConstructArray( m_skeletonAsset->_Arena, m_skeletonAsset->Joints, numJoints );

    for ( uint32_t i = 0; i < numJoints; ++i )
    {
        Joint &joint = m_skeletonAsset->Joints.Elements[ i ];

        joint.Name              = m_reader->ReadString( );
        joint.InverseBindMatrix = m_reader->ReadFloat_4x4( );
        joint.LocalTranslation  = m_reader->ReadFloat_3( );
        joint.LocalRotationQuat = m_reader->ReadFloat_4( );
        joint.LocalScale        = m_reader->ReadFloat_3( );
        joint.Index             = m_reader->ReadUInt32( );
        joint.ParentIndex       = m_reader->ReadInt32( );

        const uint32_t numChildren = m_reader->ReadUInt32( );
        joint.ChildIndices         = UInt32Array::Create( numChildren );

        for ( uint32_t j = 0; j < numChildren; ++j )
        {
            joint.ChildIndices.Elements[ j ] = m_reader->ReadUInt32( );
        }
    }

    return m_skeletonAsset;
}
