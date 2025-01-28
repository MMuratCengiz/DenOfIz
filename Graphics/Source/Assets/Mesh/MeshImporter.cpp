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

#include <DenOfIzGraphics/Assets/Mesh/MeshImporter.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glog/logging.h>
#include <unordered_map>

using namespace DenOfIz;

bool MeshImporter::ImportMesh( const InteropString &path, MeshStreamCallback *callback, const bool importTangents, const bool optimizeMesh, const uint32_t streamSize )
{
    if ( !callback )
    {
        LOG( ERROR ) << "Invalid callback provided to ImportMesh";
        return false;
    }

    Assimp::Importer importer;

    unsigned int flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipUVs;

    if ( importTangents )
    {
        flags |= aiProcess_CalcTangentSpace;
    }

    if ( optimizeMesh )
    {
        flags |= aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;
    }

    const aiScene *scene = importer.ReadFile( path.Get( ), flags );

    if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode )
    {
        LOG( ERROR ) << "Failed to load mesh: " << path.Get( ) << "\nError: " << importer.GetErrorString( );
        return false;
    }

    for ( int i = 0; i < scene->mNumMeshes; i++ )
    {
        ProcessAssimpMesh( scene->mMeshes[ i ], scene, callback, streamSize );
    }

    // Todo add reasonable return here
    return scene->mNumMeshes > 0;
}

void MeshImporter::ProcessNodeHierarchy( const aiNode *node, InteropArray<JointNode> &hierarchy, const std::unordered_map<std::string, uint32_t> &boneMap, int32_t parentIndex )
{
    const std::string nodeName = node->mName.C_Str( );
    const auto        it       = boneMap.find( nodeName );

    int32_t currentIndex = parentIndex;
    if ( it != boneMap.end( ) )
    {
        currentIndex = it->second;

        JointNode &jointNode  = hierarchy.GetElement( currentIndex );
        jointNode.JointIndex  = currentIndex;
        jointNode.ParentIndex = parentIndex;

        jointNode.ChildIndices = InteropArray<uint32_t>( node->mNumChildren );
    }

    for ( uint32_t i = 0; i < node->mNumChildren; i++ )
    {
        ProcessNodeHierarchy( node->mChildren[ i ], hierarchy, boneMap, currentIndex );

        if ( it != boneMap.end( ) )
        {
            if ( auto childIt = boneMap.find( node->mChildren[ i ]->mName.C_Str( ) ); childIt != boneMap.end( ) )
            {
                JointNode &jointNode = hierarchy.GetElement( currentIndex );
                jointNode.ChildIndices.SetElement( i, childIt->second );
            }
        }
    }
}

void MeshImporter::ProcessJoints( const aiMesh *aiMesh, const aiScene *scene, MeshStreamCallback *callback )
{
    if ( !aiMesh->HasBones( ) )
    {
        return;
    }

    std::unordered_map<std::string, uint32_t> boneMap;
    for ( uint32_t i = 0; i < aiMesh->mNumBones; i++ )
    {
        boneMap[ aiMesh->mBones[ i ]->mName.C_Str( ) ] = i;
    }

    // Process joints
    InteropArray<Joint>     joints( aiMesh->mNumBones );
    InteropArray<float>     inverseBindMatrices( aiMesh->mNumBones * 16 );
    InteropArray<JointNode> hierarchy( aiMesh->mNumBones );

    for ( uint32_t i = 0; i < aiMesh->mNumBones; i++ )
    {
        const aiBone *bone  = aiMesh->mBones[ i ];
        Joint        &joint = joints.GetElement( i );

        joint.Name = bone->mName.C_Str( );

        const aiMatrix4x4 &offsetMat    = bone->mOffsetMatrix;
        const uint32_t     matrixOffset = i * 16;

        float *m = joint.InverseBindMatrix.M;
        m[ 0 ]   = offsetMat.a1;
        m[ 1 ]   = offsetMat.a2;
        m[ 2 ]   = offsetMat.a3;
        m[ 3 ]   = offsetMat.a4;
        m[ 4 ]   = offsetMat.b1;
        m[ 5 ]   = offsetMat.b2;
        m[ 6 ]   = offsetMat.b3;
        m[ 7 ]   = offsetMat.b4;
        m[ 8 ]   = offsetMat.c1;
        m[ 9 ]   = offsetMat.c2;
        m[ 10 ]  = offsetMat.c3;
        m[ 11 ]  = offsetMat.c4;
        m[ 12 ]  = offsetMat.d1;
        m[ 13 ]  = offsetMat.d2;
        m[ 14 ]  = offsetMat.d3;
        m[ 15 ]  = offsetMat.d4;

        for ( int j = 0; j < 16; j++ )
        {
            inverseBindMatrices.SetElement( matrixOffset + j, m[ j ] );
        }
    }

    ProcessNodeHierarchy( scene->mRootNode, hierarchy, boneMap, -1 );

    callback->OnJointData( joints );
    callback->OnJointHierarchy( hierarchy );
    callback->OnInverseBindMatrices( inverseBindMatrices );
}

void MeshImporter::ProcessAnimations( const aiScene *scene, MeshStreamCallback *callback )
{
    if ( !scene->HasAnimations( ) )
        return;

    InteropArray<Animation> animations( scene->mNumAnimations );

    for ( uint32_t i = 0; i < scene->mNumAnimations; i++ )
    {
        const aiAnimation *anim      = scene->mAnimations[ i ];
        Animation         &animation = animations.GetElement( i );

        animation.Name           = anim->mName.C_Str( );
        animation.Duration       = anim->mDuration;
        animation.TicksPerSecond = anim->mTicksPerSecond;

        InteropArray<AnimationChannel> channels( anim->mNumChannels );

        for ( uint32_t j = 0; j < anim->mNumChannels; j++ )
        {
            const aiNodeAnim *nodeAnim = anim->mChannels[ j ];
            AnimationChannel &channel  = channels.GetElement( j );

            channel.BoneName = nodeAnim->mNodeName.C_Str( );

            const uint32_t numKeys = std::max( { nodeAnim->mNumPositionKeys, nodeAnim->mNumRotationKeys, nodeAnim->mNumScalingKeys } );

            InteropArray<AnimationKey> keys( numKeys );

            for ( uint32_t k = 0; k < numKeys; k++ )
            {
                AnimationKey &key = keys.GetElement( k );

                if ( k < nodeAnim->mNumPositionKeys )
                {
                    const aiVectorKey &posKey = nodeAnim->mPositionKeys[ k ];
                    key.Time                  = posKey.mTime;
                    key.Position              = { posKey.mValue.x, posKey.mValue.y, posKey.mValue.z };
                }

                if ( k < nodeAnim->mNumRotationKeys )
                {
                    const aiQuatKey &rotKey = nodeAnim->mRotationKeys[ k ];
                    key.Rotation            = { rotKey.mValue.x, rotKey.mValue.y, rotKey.mValue.z, rotKey.mValue.w };
                }

                if ( k < nodeAnim->mNumScalingKeys )
                {
                    const aiVectorKey &scaleKey = nodeAnim->mScalingKeys[ k ];
                    key.Scale                   = { scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z };
                }
            }

            channel.Keys = keys;
        }

        animation.Channels = channels;
    }

    callback->OnAnimationData( animations );
}

void MeshImporter::ProcessAssimpMesh( const aiMesh *aiMesh, const aiScene *scene, MeshStreamCallback *callback, const uint32_t streamSize )
{
    const uint32_t totalVertices     = aiMesh->mNumVertices;
    uint32_t       processedVertices = 0;

    const MeshBufferSizes sizes = CalculateBufferSizes( scene );
    callback->OnBegin( sizes );

    while ( processedVertices < totalVertices )
    {
        const uint32_t           verticesThisIteration = std::min( streamSize, totalVertices - processedVertices );
        InteropArray<MeshVertex> vertices( verticesThisIteration );

        for ( uint32_t i = 0; i < verticesThisIteration; i++ )
        {
            const uint32_t vertexIndex = processedVertices + i;
            MeshVertex    &vertex      = vertices.GetElement( i );

            vertex.Position.X = aiMesh->mVertices[ vertexIndex ].x;
            vertex.Position.Y = aiMesh->mVertices[ vertexIndex ].y;
            vertex.Position.Z = aiMesh->mVertices[ vertexIndex ].z;

            if ( aiMesh->mNormals )
            {
                vertex.Normal.X = aiMesh->mNormals[ vertexIndex ].x;
                vertex.Normal.Y = aiMesh->mNormals[ vertexIndex ].y;
                vertex.Normal.Z = aiMesh->mNormals[ vertexIndex ].z;
            }

            if ( aiMesh->mTextureCoords[ 0 ] )
            {
                vertex.TexCoord.X = aiMesh->mTextureCoords[ 0 ][ vertexIndex ].x;
                vertex.TexCoord.Y = aiMesh->mTextureCoords[ 0 ][ vertexIndex ].y;
            }

            if ( aiMesh->mTangents )
            {
                vertex.Tangent.X = aiMesh->mTangents[ vertexIndex ].x;
                vertex.Tangent.Y = aiMesh->mTangents[ vertexIndex ].y;
                vertex.Tangent.Z = aiMesh->mTangents[ vertexIndex ].z;
            }

            if ( aiMesh->mBitangents )
            {
                vertex.Bitangent.X = aiMesh->mBitangents[ vertexIndex ].x;
                vertex.Bitangent.Y = aiMesh->mBitangents[ vertexIndex ].y;
                vertex.Bitangent.Z = aiMesh->mBitangents[ vertexIndex ].z;
            }

            if ( aiMesh->HasVertexColors( 0 ) )
            {
                vertex.Color.X = aiMesh->mColors[ 0 ][ vertexIndex ].r;
                vertex.Color.Y = aiMesh->mColors[ 0 ][ vertexIndex ].g;
                vertex.Color.Z = aiMesh->mColors[ 0 ][ vertexIndex ].b;
                vertex.Color.W = aiMesh->mColors[ 0 ][ vertexIndex ].a;
            }
            else
            {
                vertex.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
            }

            if ( aiMesh->HasBones( ) )
            {
                vertex.BoneIndices = { 0, 0, 0, 0 };
                vertex.BoneWeights = { 0.0f, 0.0f, 0.0f, 0.0f };

                uint32_t weightCount = 0;

                for ( uint32_t b = 0; b < aiMesh->mNumBones && weightCount < 4; b++ )
                {
                    const aiBone *bone = aiMesh->mBones[ b ];

                    for ( uint32_t w = 0; w < bone->mNumWeights; w++ )
                    {
                        if ( const aiVertexWeight &weight = bone->mWeights[ w ]; weight.mVertexId == vertexIndex && weightCount < 4 )
                        {
                            switch ( weightCount )
                            {
                            case 0:
                                vertex.BoneIndices.X = b;
                                vertex.BoneWeights.X = weight.mWeight;
                                break;
                            case 1:
                                vertex.BoneIndices.Y = b;
                                vertex.BoneWeights.Y = weight.mWeight;
                                break;
                            case 2:
                                vertex.BoneIndices.Z = b;
                                vertex.BoneWeights.Z = weight.mWeight;
                                break;
                            case 3:
                                vertex.BoneIndices.W = b;
                                vertex.BoneWeights.W = weight.mWeight;
                                break;
                            }
                            weightCount++;
                        }
                    }
                }

                if ( const float sum = vertex.BoneWeights.X + vertex.BoneWeights.Y + vertex.BoneWeights.Z + vertex.BoneWeights.W; sum > 0.0f )
                {
                    const float invSum = 1.0f / sum;
                    vertex.BoneWeights.X *= invSum;
                    vertex.BoneWeights.Y *= invSum;
                    vertex.BoneWeights.Z *= invSum;
                    vertex.BoneWeights.W *= invSum;
                }
            }
        }

        callback->OnVertexData( vertices, processedVertices );
        processedVertices += verticesThisIteration;
    }

    const uint32_t totalIndices     = aiMesh->mNumFaces * 3;
    uint32_t       processedIndices = 0;

    while ( processedIndices < totalIndices )
    {
        const uint32_t         indicesThisIteration = std::min( streamSize, totalIndices - processedIndices );
        InteropArray<uint32_t> indices( indicesThisIteration );

        for ( uint32_t i = 0; i < indicesThisIteration; i++ )
        {
            const uint32_t faceIndex   = ( processedIndices + i ) / 3;
            const uint32_t indexInFace = ( processedIndices + i ) % 3;

            const aiFace &face = aiMesh->mFaces[ faceIndex ];
            indices.SetElement( i, face.mIndices[ indexInFace ] );
        }

        callback->OnIndexData( indices, processedIndices );
        processedIndices += indicesThisIteration;
    }

    if ( aiMesh->HasBones( ) )
    {
        ProcessJoints( aiMesh, scene, callback );
    }
    ProcessAnimations( scene, callback );

    InteropArray<SubMesh> subMeshes( 1 );
    SubMesh              &subMesh = subMeshes.GetElement( 0 );
    subMesh.BaseVertex            = 0;
    subMesh.BaseIndex             = 0;
    subMesh.NumVertices           = totalVertices;
    subMesh.NumIndices            = totalIndices;
    subMesh.MaterialIndex         = aiMesh->mMaterialIndex;

    callback->OnComplete( subMeshes );
}

MeshBufferSizes MeshImporter::CalculateBufferSizes( const aiScene *scene )
{
    MeshBufferSizes sizes{ };
    sizes.TotalVertices = 0;
    sizes.TotalIndices  = 0;
    sizes.NumSubMeshes  = scene->mNumMeshes;
    sizes.NumJoints     = 0;
    sizes.NumAnimations = scene->mNumAnimations;

    for ( uint32_t i = 0; i < scene->mNumMeshes; i++ )
    {
        const aiMesh *mesh = scene->mMeshes[ i ];
        sizes.TotalVertices += mesh->mNumVertices;
        sizes.TotalIndices += mesh->mNumFaces * 3;
        sizes.NumJoints += mesh->mNumBones;
    }

    return sizes;
}
