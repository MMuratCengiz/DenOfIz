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

#include "DenOfIzGraphicsInternal/Assets/Import/AssimpSkeletonProcessor.h"
#include <DirectXMath.h>
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/InteropMathConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

AssimpSkeletonProcessor::AssimpSkeletonProcessor( )  = default;
AssimpSkeletonProcessor::~AssimpSkeletonProcessor( ) = default;

ImporterResultCode AssimpSkeletonProcessor::PreprocessSkeleton( AssimpImportContext &context, SkeletonBuildStats &stats )
{
    m_stats = SkeletonBuildStats{ };
    CollectBonesFromMeshes( context );

    if ( context.BoneNameToIndexMap.empty( ) )
    {
        spdlog::info( "No bones found in the scene" );
        return ImporterResultCode::Success;
    }

    CountBonesInHierarchy( context.Scene->mRootNode, context.BoneNameToIndexMap, m_stats.TotalJoints );

    std::unordered_map<const aiNode *, uint32_t> childrenCount;
    CalculateMaxChildren( context.Scene->mRootNode, context.BoneNameToIndexMap, childrenCount, m_stats.MaxChildrenPerJoint );

    m_stats.RequiredArenaSize = sizeof( SkeletonAsset );
    m_stats.RequiredArenaSize += m_stats.TotalJoints * sizeof( Joint );
    m_stats.RequiredArenaSize += m_stats.TotalJoints * m_stats.MaxChildrenPerJoint * sizeof( uint32_t );
    m_stats.RequiredArenaSize += m_stats.TotalJoints * 64;

    m_stats.RequiredArenaSize = static_cast<size_t>( m_stats.RequiredArenaSize * 1.2 );
    stats                     = m_stats;

    spdlog::info( "Skeleton preprocessing complete: {} joints, max {} children per joint, {} bytes required", m_stats.TotalJoints, m_stats.MaxChildrenPerJoint,
                  m_stats.RequiredArenaSize );
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpSkeletonProcessor::BuildSkeleton( AssimpImportContext &context, SkeletonAsset &skeletonAsset )
{
    if ( context.BoneNameToIndexMap.empty( ) )
    {
        return ImporterResultCode::Success;
    }

    skeletonAsset._Arena.EnsureCapacity( m_stats.RequiredArenaSize );
    DZArenaArrayHelper<JointArray, Joint>::AllocateAndConstructArray( skeletonAsset._Arena, skeletonAsset.Joints, m_stats.TotalJoints );

    std::vector<std::vector<uint32_t>> childrenIndices( m_stats.TotalJoints );
    for ( auto &children : childrenIndices )
    {
        children.reserve( m_stats.MaxChildrenPerJoint );
    }

    uint32_t jointIndex = 0;
    if ( const ImporterResultCode result = ProcessNodeHierarchy( context, context.Scene->mRootNode, skeletonAsset, -1, jointIndex, childrenIndices );
         result != ImporterResultCode::Success )
    {
        spdlog::error( "Failed to build skeleton hierarchy" );
        return result;
    }

    FinalizeJointChildren( skeletonAsset, childrenIndices );
    spdlog::info( "Built skeleton with {} joints", jointIndex );
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpSkeletonProcessor::WriteSkeletonAsset( AssimpImportContext &context, SkeletonAsset &skeletonAsset ) const
{
    const InteropString assetFilename   = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, skeletonAsset.Name, "Skeleton", SkeletonAsset::Extension( ) );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( InteropString( context.TargetDirectory ).Append( "/" ).Append( assetFilename.Get( ) ) );

    context.SkeletonAssetUri = AssetUri::Create( assetFilename );
    skeletonAsset.Uri        = context.SkeletonAssetUri;

    spdlog::info( "Writing Skeleton asset to: {}", targetAssetPath.Get( ) );

    BinaryWriter              writer( targetAssetPath );
    const SkeletonAssetWriter assetWriter( { &writer } );
    assetWriter.Write( skeletonAsset );

    context.CreatedAssets.push_back( context.SkeletonAssetUri );
    return ImporterResultCode::Success;
}

void AssimpSkeletonProcessor::CollectBonesFromMeshes( AssimpImportContext &context ) const
{
    context.BoneNameToIndexMap.clear( );
    context.BoneNameToInverseBindMatrixMap.clear( );

    for ( uint32_t i = 0; i < context.Scene->mNumMeshes; ++i )
    {
        const aiMesh *mesh = context.Scene->mMeshes[ i ];
        if ( mesh && mesh->HasBones( ) )
        {
            for ( unsigned int b = 0; b < mesh->mNumBones; ++b )
            {
                const aiBone *bone     = mesh->mBones[ b ];
                std::string   boneName = bone->mName.C_Str( );
                if ( !context.BoneNameToInverseBindMatrixMap.contains( boneName ) )
                {
                    context.BoneNameToInverseBindMatrixMap[ boneName ] = bone->mOffsetMatrix;
                }
                if ( !context.BoneNameToIndexMap.contains( boneName ) )
                {
                    context.BoneNameToIndexMap[ boneName ] = -1;
                }
            }
        }
    }

    spdlog::info( "Collected {} unique bones from meshes", context.BoneNameToIndexMap.size( ) );
}

void AssimpSkeletonProcessor::CountBonesInHierarchy( const aiNode *node, const std::unordered_map<std::string, uint32_t> &boneMap, uint32_t &numJoints )
{
    if ( !node )
    {
        return;
    }

    const std::string nodeName = node->mName.C_Str( );
    if ( boneMap.contains( nodeName ) )
    {
        numJoints++;
    }

    for ( unsigned int i = 0; i < node->mNumChildren; ++i )
    {
        CountBonesInHierarchy( node->mChildren[ i ], boneMap, numJoints );
    }
}

void AssimpSkeletonProcessor::CalculateMaxChildren( const aiNode *node, const std::unordered_map<std::string, uint32_t> &boneMap,
                                                    std::unordered_map<const aiNode *, uint32_t> &childrenCount, uint32_t &maxChildren )
{
    if ( !node )
    {
        return;
    }

    const std::string nodeName = node->mName.C_Str( );
    if ( boneMap.contains( nodeName ) )
    {
        uint32_t boneChildren = 0;
        for ( unsigned int i = 0; i < node->mNumChildren; ++i )
        {
            const std::string childName = node->mChildren[ i ]->mName.C_Str( );
            if ( boneMap.contains( childName ) )
            {
                boneChildren++;
            }
        }
        childrenCount[ node ] = boneChildren;
        maxChildren           = std::max( maxChildren, boneChildren );
    }

    for ( unsigned int i = 0; i < node->mNumChildren; ++i )
    {
        CalculateMaxChildren( node->mChildren[ i ], boneMap, childrenCount, maxChildren );
    }
}

ImporterResultCode AssimpSkeletonProcessor::ProcessNodeHierarchy( AssimpImportContext &context, const aiNode *node, SkeletonAsset &skeletonAsset, const int32_t parentJointIndex,
                                                                  uint32_t &jointIndex, std::vector<std::vector<uint32_t>> &childrenIndices )
{
    if ( !node )
    {
        return ImporterResultCode::Success;
    }

    const std::string nodeNameStr       = node->mName.C_Str( );
    int32_t           currentJointIndex = parentJointIndex;

    if ( context.BoneNameToIndexMap.contains( nodeNameStr ) )
    {
        bool alreadyAdded = false;
        for ( uint32_t j = 0; j < jointIndex; ++j )
        {
            if ( skeletonAsset.Joints.Elements[ j ].Name.Equals( nodeNameStr.c_str( ) ) )
            {
                currentJointIndex = skeletonAsset.Joints.Elements[ j ].Index;
                alreadyAdded      = true;
                break;
            }
        }

        if ( !alreadyAdded )
        {
            if ( jointIndex >= m_stats.TotalJoints )
            {
                spdlog::error( "Joint index {} exceeds total joints {}", jointIndex, m_stats.TotalJoints );
                return ImporterResultCode::ImportFailed;
            }

            currentJointIndex = jointIndex;
            jointIndex++;

            Joint &joint      = skeletonAsset.Joints.Elements[ currentJointIndex ];
            joint.Name        = nodeNameStr.c_str( );
            joint.Index       = currentJointIndex;
            joint.ParentIndex = parentJointIndex;

            const aiMatrix4x4 localMatrix = node->mTransformation;
            aiVector3D        translation, scale;
            aiQuaternion      rotation;
            localMatrix.Decompose( scale, rotation, translation );

            const float scaleFactor = context.Desc.ScaleFactor;
            joint.LocalTranslation  = { translation.x * scaleFactor, translation.y * scaleFactor, translation.z * scaleFactor };
            joint.LocalRotationQuat = { rotation.x, rotation.y, rotation.z, rotation.w };
            joint.LocalScale        = { scale.x, scale.y, scale.z };

            if ( context.BoneNameToInverseBindMatrixMap.contains( nodeNameStr ) )
            {
                const aiMatrix4x4 scaledMatrix = context.BoneNameToInverseBindMatrixMap[ nodeNameStr ];
                joint.InverseBindMatrix        = ConvertMatrix( scaledMatrix );
                joint.InverseBindMatrix._41 *= context.Desc.ScaleFactor;
                joint.InverseBindMatrix._42 *= context.Desc.ScaleFactor;
                joint.InverseBindMatrix._43 *= context.Desc.ScaleFactor;
            }
            else
            {
                joint.InverseBindMatrix     = Float_4x4{ };
                joint.InverseBindMatrix._11 = 1.0f;
                joint.InverseBindMatrix._22 = 1.0f;
                joint.InverseBindMatrix._33 = 1.0f;
                joint.InverseBindMatrix._44 = 1.0f;
            }

            if ( parentJointIndex >= 0 && parentJointIndex < static_cast<int32_t>( m_stats.TotalJoints ) )
            {
                childrenIndices[ parentJointIndex ].push_back( currentJointIndex );
            }

            context.BoneNameToIndexMap[ nodeNameStr ]         = currentJointIndex;
            context.IndexToAssimpNodeMap[ currentJointIndex ] = node;
        }
    }

    for ( unsigned int i = 0; i < node->mNumChildren; ++i )
    {
        if ( const ImporterResultCode childResult = ProcessNodeHierarchy( context, node->mChildren[ i ], skeletonAsset, currentJointIndex, jointIndex, childrenIndices );
             childResult != ImporterResultCode::Success )
        {
            return childResult;
        }
    }

    return ImporterResultCode::Success;
}

void AssimpSkeletonProcessor::FinalizeJointChildren( SkeletonAsset &skeletonAsset, const std::vector<std::vector<uint32_t>> &childrenIndices ) const
{
    for ( uint32_t i = 0; i < skeletonAsset.Joints.NumElements; ++i )
    {
        Joint                       &joint    = skeletonAsset.Joints.Elements[ i ];
        const std::vector<uint32_t> &children = childrenIndices[ i ];

        if ( !children.empty( ) )
        {
            DZArenaArrayHelper<UInt32Array, uint32_t>::AllocateAndConstructArray( skeletonAsset._Arena, joint.ChildIndices, children.size( ) );

            for ( size_t c = 0; c < children.size( ); ++c )
            {
                joint.ChildIndices.Elements[ c ] = children[ c ];
            }
        }
    }
}

Float_4x4 AssimpSkeletonProcessor::ConvertMatrix( const aiMatrix4x4 &matrix ) const
{
    aiVector3f   translation;
    aiQuaternion rotation;
    aiVector3f   scale;
    matrix.Decompose( scale, rotation, translation );

    const DirectX::XMMATRIX matrixXM = DirectX::XMMatrixAffineTransformation( DirectX::XMVectorSet( scale.x, scale.y, scale.z, 1.0f ), DirectX::XMVectorZero( ),
                                                                              DirectX::XMVectorSet( rotation.x, rotation.y, rotation.z, rotation.w ),
                                                                              DirectX::XMVectorSet( translation.x, translation.y, translation.z, 1.0f ) );

    return InteropMathConverter::Float_4X4FromXMMATRIX( matrixXM );
}

Float_4 AssimpSkeletonProcessor::ConvertQuaternion( const aiQuaternion &quat ) const
{
    return { quat.x, quat.y, quat.z, quat.w };
}

Float_3 AssimpSkeletonProcessor::ConvertVector3( const aiVector3D &vec, const float scaleFactor ) const
{
    return { vec.x * scaleFactor, vec.y * scaleFactor, vec.z * scaleFactor };
}
