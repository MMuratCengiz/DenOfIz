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

SkeletonAsset SkeletonAssetReader::Read( )
{
    m_skeletonAsset = SkeletonAsset( );

    m_skeletonAsset.Magic = m_reader->ReadUInt64( );
    if ( m_skeletonAsset.Magic != SkeletonAsset{ }.Magic )
    {
        LOG( FATAL ) << "Invalid SkeletonAsset magic number.";
    }

    m_skeletonAsset.Version = m_reader->ReadUInt32( );
    if ( m_skeletonAsset.Version > SkeletonAsset::Latest )
    {
        LOG( WARNING ) << "SkeletonAsset version mismatch.";
    }

    m_skeletonAsset.NumBytes = m_reader->ReadUInt64( );
    m_skeletonAsset.Uri      = AssetUri::Parse( m_reader->ReadString( ) );

    m_skeletonAsset.Name = m_reader->ReadString( );

    const uint32_t numJoints = m_reader->ReadUInt32( );
    m_skeletonAsset.Joints.Resize( numJoints );

    for ( uint32_t i = 0; i < numJoints; ++i )
    {
        Joint &joint = m_skeletonAsset.Joints.GetElement( i );

        joint.Name              = m_reader->ReadString( );
        joint.InverseBindMatrix = m_reader->ReadFloat_4x4( );
        joint.LocalTranslation  = m_reader->ReadFloat_3( );
        joint.LocalRotationQuat = m_reader->ReadFloat_4( );
        joint.LocalScale        = m_reader->ReadFloat_3( );
        joint.Index             = m_reader->ReadUInt32( );
        joint.ParentIndex       = m_reader->ReadInt32( );

        const uint32_t numChildren = m_reader->ReadUInt32( );
        joint.ChildIndices.Resize( numChildren );

        for ( uint32_t j = 0; j < numChildren; ++j )
        {
            joint.ChildIndices.SetElement( j, m_reader->ReadUInt32( ) );
        }
    }

    return m_skeletonAsset;
}
