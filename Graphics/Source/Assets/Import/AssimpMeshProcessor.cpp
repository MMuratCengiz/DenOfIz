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

#include "DenOfIzGraphicsInternal/Assets/Import/AssimpMeshProcessor.h"
#include <algorithm>
#include <limits>
#include <ranges>
#include <set>
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

AssimpMeshProcessor::AssimpMeshProcessor( )  = default;
AssimpMeshProcessor::~AssimpMeshProcessor( ) = default;

ImporterResultCode AssimpMeshProcessor::CollectMeshes( AssimpImportContext &context )
{
    m_meshesToProcess.clear( );
    m_subMeshData.clear( );

    m_meshesToProcess.reserve( context.Scene->mNumMeshes );
    m_subMeshData.reserve( context.Scene->mNumMeshes );

    std::set<unsigned int> processedMeshIndices;

    CollectMeshesFromNode( context, context.Scene->mRootNode, m_meshesToProcess, processedMeshIndices );
    spdlog::info( "Collected {} unique meshes from {} total meshes", m_meshesToProcess.size( ), context.Scene->mNumMeshes );

    if ( m_meshesToProcess.empty( ) )
    {
        return ImporterResultCode::Success;
    }

    const aiMesh *firstMesh = m_meshesToProcess[ 0 ];
    DetermineVertexAttributes( firstMesh, context.MeshAsset.EnabledAttributes, context.MeshAsset.AttributeConfig, context.Desc );

    if ( !m_subMeshData.empty( ) )
    {
        DZArenaArrayHelper<SubMeshDataArray, SubMeshData>::AllocateAndConstructArray( *context.MainArena, context.MeshAsset.SubMeshes, m_subMeshData.size( ) );

        for ( size_t i = 0; i < m_subMeshData.size( ); ++i )
        {
            context.MeshAsset.SubMeshes.Elements[ i ] = m_subMeshData[ i ];
        }
    }

    context.MeshAsset.NumLODs = 1; // TODO: LOD support
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpMeshProcessor::ProcessAllMeshes( AssimpImportContext &context, MeshAssetWriter &meshWriter )
{
    context.CurrentSubMeshIndex = 0;
    for ( const aiMesh *mesh : m_meshesToProcess )
    {
        if ( const ImporterResultCode result = ProcessSingleMesh( context, mesh, meshWriter ); result != ImporterResultCode::Success )
        {
            spdlog::error( "Failed to process mesh: {}", mesh->mName.C_Str( ) );
            return result;
        }

        m_stats.ProcessedMeshes++;
    }

    spdlog::info( "Processed {} meshes with {} vertices and {} indices total", m_stats.ProcessedMeshes, m_stats.ProcessedVertices, m_stats.ProcessedIndices );
    return ImporterResultCode::Success;
}

const MeshProcessingStats &AssimpMeshProcessor::GetStats( ) const
{
    return m_stats;
}

ImporterResultCode AssimpMeshProcessor::ProcessSingleMesh( AssimpImportContext &context, const aiMesh *mesh, MeshAssetWriter &assetWriter )
{
    if ( !mesh->HasFaces( ) || !mesh->HasPositions( ) )
    {
        return ImporterResultCode::Success;
    }

    const uint32_t submeshIndex = context.CurrentSubMeshIndex;
    if ( submeshIndex >= context.MeshAsset.SubMeshes.NumElements )
    {
        spdlog::error( "Invalid submesh index {}", submeshIndex );
        return ImporterResultCode::InvalidParameters;
    }

    spdlog::info( "Processing mesh: {} (SubMesh {} with {} vertices and {} indices)", mesh->mName.C_Str( ), submeshIndex, mesh->mNumVertices, mesh->mNumFaces * 3 );

    const VertexEnabledAttributes &attributes      = context.MeshAsset.EnabledAttributes;
    const VertexAttributeConfig   &attributeConfig = context.MeshAsset.AttributeConfig;

    std::vector<std::vector<std::pair<int, float>>> boneInfluences;
    if ( attributes.BlendIndices && mesh->HasBones( ) )
    {
        PrepareBoneInfluences( context, mesh, boneInfluences );
    }

    for ( unsigned int i = 0; i < mesh->mNumVertices; ++i )
    {
        MeshVertex vertex{ };
        if ( attributes.Position )
        {
            vertex.Position = ConvertPosition( mesh->mVertices[ i ], context.Desc.ScaleFactor );
        }

        if ( attributes.Normal && mesh->HasNormals( ) )
        {
            vertex.Normal = ConvertNormal( mesh->mNormals[ i ] );
        }
        if ( attributes.Tangent && mesh->HasTangentsAndBitangents( ) )
        {
            vertex.Tangent = ConvertTangent( mesh->mTangents[ i ] );
        }
        if ( attributes.Bitangent && mesh->HasTangentsAndBitangents( ) )
        {
            vertex.Bitangent = ConvertTangent( mesh->mBitangents[ i ] );
        }

        DZArenaArrayHelper<Float_2Array, Float_2>::AllocateAndConstructArray( *context.MainArena, vertex.UVs, attributeConfig.NumUVAttributes );
        for ( uint32_t uvChan = 0; uvChan < attributeConfig.NumUVAttributes; ++uvChan )
        {
            if ( mesh->HasTextureCoords( uvChan ) )
            {
                vertex.UVs.Elements[ uvChan ] = ConvertUV( mesh->mTextureCoords[ uvChan ][ i ] );
            }
            else
            {
                vertex.UVs.Elements[ uvChan ] = { 0.0f, 0.0f };
            }
        }

        DZArenaArrayHelper<Float_4Array, Float_4>::AllocateAndConstructArray( *context.MainArena, vertex.Colors, attributeConfig.ColorFormats.NumElements );
        for ( uint32_t colChan = 0; colChan < attributeConfig.ColorFormats.NumElements; ++colChan )
        {
            if ( mesh->HasVertexColors( colChan ) )
            {
                vertex.Colors.Elements[ colChan ] = ConvertColor( mesh->mColors[ colChan ][ i ] );
            }
            else
            {
                vertex.Colors.Elements[ colChan ] = { 1.0f, 1.0f, 1.0f, 1.0f };
            }
        }

        if ( attributes.BlendIndices && !boneInfluences.empty( ) )
        {
            ApplyBoneInfluencesToVertex( vertex, boneInfluences[ i ] );
        }

        assetWriter.AddVertex( vertex );
        m_stats.ProcessedVertices++;
    }

    for ( unsigned int i = 0; i < mesh->mNumFaces; ++i )
    {
        const aiFace &face = mesh->mFaces[ i ];
        if ( face.mNumIndices == 3 ) // Triangulated
        {
            assetWriter.AddIndex32( face.mIndices[ 0 ] );
            assetWriter.AddIndex32( face.mIndices[ 1 ] );
            assetWriter.AddIndex32( face.mIndices[ 2 ] );
            m_stats.ProcessedIndices += 3;
        }
    }

    context.CurrentSubMeshIndex++;
    return ImporterResultCode::Success;
}

void AssimpMeshProcessor::CollectMeshesFromNode( AssimpImportContext &context, const aiNode *node, std::vector<const aiMesh *> &uniqueMeshes,
                                                 std::set<unsigned int> &processedIndices )
{
    if ( !node )
    {
        return;
    }
    for ( unsigned int i = 0; i < node->mNumMeshes; ++i )
    {
        unsigned int meshIndex = node->mMeshes[ i ];
        if ( processedIndices.contains( meshIndex ) )
            continue;

        const aiMesh *mesh = context.Scene->mMeshes[ meshIndex ];
        if ( mesh && mesh->HasFaces( ) && mesh->HasPositions( ) )
        {
            uniqueMeshes.push_back( mesh );
            processedIndices.insert( meshIndex );

            SubMeshData subMesh;
            subMesh.Name = mesh->mName.C_Str( );
            if ( subMesh.Name.IsEmpty( ) )
            {
                subMesh.Name = InteropString( "SubMesh_" ).Append( std::to_string( m_subMeshData.size( ) ).c_str( ) );
            }

            subMesh.NumVertices = mesh->mNumVertices;
            subMesh.NumIndices  = mesh->mNumFaces * 3;
            subMesh.Topology    = PrimitiveTopology::Triangle;
            subMesh.IndexType   = IndexType::Uint32;

            CalculateMeshBounds( mesh, context.Desc.ScaleFactor, subMesh.MinBounds, subMesh.MaxBounds );
            subMesh.LODLevel                    = 0;
            subMesh.BoundingVolumes.Elements    = nullptr;
            subMesh.BoundingVolumes.NumElements = 0;
            if ( context.Desc.ImportMaterials && mesh->mMaterialIndex < context.Scene->mNumMaterials )
            {
                const aiMaterial *material = context.Scene->mMaterials[ mesh->mMaterialIndex ];
                std::string       matName  = material->GetName( ).C_Str( );
                if ( context.MaterialNameToAssetUriMap.contains( matName ) )
                {
                    subMesh.MaterialRef = context.MaterialNameToAssetUriMap[ matName ];
                }
            }
            m_subMeshData.push_back( subMesh );
        }
    }

    for ( unsigned int i = 0; i < node->mNumChildren; ++i )
    {
        CollectMeshesFromNode( context, node->mChildren[ i ], uniqueMeshes, processedIndices );
    }
}

void AssimpMeshProcessor::DetermineVertexAttributes( const aiMesh *mesh, VertexEnabledAttributes &attributes, VertexAttributeConfig &config, const AssimpImportDesc &desc ) const
{
    attributes.Position     = mesh->HasPositions( );
    attributes.Normal       = desc.GenerateNormals || mesh->HasNormals( );
    attributes.Tangent      = desc.CalculateTangentSpace || mesh->HasTangentsAndBitangents( );
    attributes.Bitangent    = desc.CalculateTangentSpace || mesh->HasTangentsAndBitangents( );
    attributes.UV           = mesh->GetNumUVChannels( ) > 0;
    attributes.Color        = mesh->HasVertexColors( 0 );
    attributes.BlendIndices = mesh->HasBones( );
    attributes.BlendWeights = mesh->HasBones( );

    config.NumPositionComponents = 4;
    config.NumUVAttributes       = mesh->GetNumUVChannels( );
    config.MaxBoneInfluences     = desc.MaxBoneWeightsPerVertex;
}

void AssimpMeshProcessor::CalculateMeshBounds( const aiMesh *mesh, const float scaleFactor, Float_3 &outMin, Float_3 &outMax ) const
{
    if ( !mesh || !mesh->HasPositions( ) || mesh->mNumVertices == 0 )
    {
        outMin = outMax = { 0, 0, 0 };
        return;
    }

    outMin = { std::numeric_limits<float>::max( ), std::numeric_limits<float>::max( ), std::numeric_limits<float>::max( ) };
    outMax = { std::numeric_limits<float>::lowest( ), std::numeric_limits<float>::lowest( ), std::numeric_limits<float>::lowest( ) };

    for ( unsigned int i = 0; i < mesh->mNumVertices; ++i )
    {
        const aiVector3D &pos = mesh->mVertices[ i ];
        outMin.X              = std::min( outMin.X, pos.x * scaleFactor );
        outMin.Y              = std::min( outMin.Y, pos.y * scaleFactor );
        outMin.Z              = std::min( outMin.Z, pos.z * scaleFactor );
        outMax.X              = std::max( outMax.X, pos.x * scaleFactor );
        outMax.Y              = std::max( outMax.Y, pos.y * scaleFactor );
        outMax.Z              = std::max( outMax.Z, pos.z * scaleFactor );
    }
}

void AssimpMeshProcessor::PrepareBoneInfluences( AssimpImportContext &context, const aiMesh *mesh, std::vector<std::vector<std::pair<int, float>>> &boneInfluences ) const
{
    boneInfluences.resize( mesh->mNumVertices );

    for ( unsigned int b = 0; b < mesh->mNumBones; ++b )
    {
        const aiBone *bone     = mesh->mBones[ b ];
        std::string   boneName = bone->mName.C_Str( );

        if ( !context.BoneNameToIndexMap.contains( boneName ) )
        {
            spdlog::warn( "Bone '{}' not found in skeleton", boneName );
            continue;
        }

        int boneIndex = context.BoneNameToIndexMap[ boneName ];
        if ( boneIndex < 0 )
        {
            spdlog::warn( "Bone '{}' has invalid index {}", boneName, boneIndex );
            continue;
        }

        for ( unsigned int w = 0; w < bone->mNumWeights; ++w )
        {
            const aiVertexWeight &weight = bone->mWeights[ w ];
            if ( weight.mVertexId < mesh->mNumVertices )
            {
                boneInfluences[ weight.mVertexId ].emplace_back( boneIndex, weight.mWeight );
            }
        }
    }

    for ( auto &influences : boneInfluences )
    {
        std::ranges::sort( influences, []( const auto &a, const auto &b ) { return a.second > b.second; } );
        if ( influences.size( ) > context.MeshAsset.AttributeConfig.MaxBoneInfluences )
        {
            influences.resize( context.MeshAsset.AttributeConfig.MaxBoneInfluences );
        }

        float totalWeight = 0.0f;
        for ( const auto &weight : influences | std::views::values )
        {
            totalWeight += weight;
        }
        if ( totalWeight > 1e-6f )
        {
            for ( auto &weight : influences | std::views::values )
            {
                weight /= totalWeight;
            }
        }
    }
}

void AssimpMeshProcessor::ApplyBoneInfluencesToVertex( MeshVertex &vertex, const std::vector<std::pair<int, float>> &influences ) const
{
    vertex.BlendIndices = { 0, 0, 0, 0 };
    vertex.BoneWeights  = { 0.0f, 0.0f, 0.0f, 0.0f };

    for ( size_t i = 0; i < influences.size( ) && i < 4; ++i )
    {
        const auto &[ boneIdx, weight ] = influences[ i ];
        switch ( i )
        {
        case 0:
            vertex.BlendIndices.X = boneIdx;
            vertex.BoneWeights.X  = weight;
            break;
        case 1:
            vertex.BlendIndices.Y = boneIdx;
            vertex.BoneWeights.Y  = weight;
            break;
        case 2:
            vertex.BlendIndices.Z = boneIdx;
            vertex.BoneWeights.Z  = weight;
            break;
        case 3:
            vertex.BlendIndices.W = boneIdx;
            vertex.BoneWeights.W  = weight;
            break;
        default:
            break;
        }
    }
}

Float_4 AssimpMeshProcessor::ConvertPosition( const aiVector3D &pos, const float scaleFactor ) const
{
    return { pos.x * scaleFactor, pos.y * scaleFactor, pos.z * scaleFactor, 1.0f };
}

Float_4 AssimpMeshProcessor::ConvertNormal( const aiVector3D &normal ) const
{
    return { normal.x, normal.y, normal.z, 0.0f };
}

Float_4 AssimpMeshProcessor::ConvertTangent( const aiVector3D &tangent ) const
{
    return { tangent.x, tangent.y, tangent.z, 1.0f };
}

Float_2 AssimpMeshProcessor::ConvertUV( const aiVector3D &uv ) const
{
    return { uv.x, uv.y };
}

Float_4 AssimpMeshProcessor::ConvertColor( const aiColor4D &color ) const
{
    return { color.r, color.g, color.b, color.a };
}
