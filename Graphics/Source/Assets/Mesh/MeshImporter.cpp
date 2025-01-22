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
    // Only process the first mesh for now
    if ( scene->mNumMeshes > 0 )
    {
        ProcessAssimpMesh( scene->mMeshes[ 0 ], scene, callback, streamSize );
        return true;
    }

    return false;
}

void MeshImporter::ProcessAssimpMesh( const aiMesh *aiMesh, const aiScene *scene, MeshStreamCallback *callback, uint32_t streamSize )
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

    InteropArray<SubMesh> subMeshes( 1 );
    SubMesh              &subMesh = subMeshes.GetElement( 0 );
    subMesh.BaseVertex            = 0;
    subMesh.BaseIndex             = 0;
    subMesh.NumVertices           = totalVertices;
    subMesh.NumIndices            = totalIndices;
    subMesh.MaterialIndex         = aiMesh->mMaterialIndex;

    callback->OnComplete( subMeshes );
}
