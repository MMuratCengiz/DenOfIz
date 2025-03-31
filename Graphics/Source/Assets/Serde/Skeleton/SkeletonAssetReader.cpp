/**/

#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetReader.h>

using namespace DenOfIz;

/**/

#include <DenOfIzGraphics/Assets/Serde/Common/AssetReaderHelpers.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetReader.h>

using namespace DenOfIz;

SkeletonAssetReader::SkeletonAssetReader( const SkeletonAssetReaderDesc &desc ) : m_reader( desc.Reader )
{
    if ( !m_reader )
    {
        LOG( FATAL ) << "BinaryReader cannot be null for SkeletonAssetReader";
    }
}

SkeletonAssetReader::~SkeletonAssetReader( ) = default;

SkeletonAsset SkeletonAssetReader::ReadSkeletonAsset( )
{
    m_SkeletonAsset = SkeletonAsset( );

    m_SkeletonAsset.Magic = m_reader->ReadUInt64( );
    if ( m_SkeletonAsset.Magic != SkeletonAsset{ }.Magic )
    {
        LOG( FATAL ) << "Invalid SkeletonAsset magic number.";
    }

    m_SkeletonAsset.Version = m_reader->ReadUInt32( );
    if ( m_SkeletonAsset.Version > SkeletonAsset::Latest )
    {
        LOG( WARNING ) << "SkeletonAsset version mismatch.";
    }

    m_SkeletonAsset.NumBytes = m_reader->ReadUInt64( );
    m_SkeletonAsset.Uri      = AssetUri::Parse( m_reader->ReadString( ) );

    m_SkeletonAsset.Name = m_reader->ReadString( );

    const uint32_t numJoints = m_reader->ReadUInt32( );
    m_SkeletonAsset.Joints.Resize( numJoints );

    for ( uint32_t i = 0; i < numJoints; ++i )
    {
        Joint &joint = m_SkeletonAsset.Joints.GetElement( i );

        joint.Name              = m_reader->ReadString( );
        joint.InverseBindMatrix = m_reader->ReadFloat_4x4( );
        joint.LocalTransform    = m_reader->ReadFloat_4x4( );
        joint.GlobalTransform   = m_reader->ReadFloat_4x4( );
        joint.Index             = m_reader->ReadUInt32( );
        joint.ParentIndex       = m_reader->ReadInt32( );

        const uint32_t numChildren = m_reader->ReadUInt32( );
        joint.ChildIndices.Resize( numChildren );

        for ( uint32_t j = 0; j < numChildren; ++j )
        {
            joint.ChildIndices.SetElement( j, m_reader->ReadUInt32( ) );
        }
    }

    return m_SkeletonAsset;
}
