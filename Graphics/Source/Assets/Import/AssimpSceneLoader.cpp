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

#include "DenOfIzGraphicsInternal/Assets/Import/AssimpSceneLoader.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <ranges>
#include <set>
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAsset.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include "DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h"

using namespace DenOfIz;

AssimpSceneLoader::AssimpSceneLoader( ) : m_importer( std::make_unique<Assimp::Importer>( ) ), m_desc( { } )
{
}

AssimpSceneLoader::~AssimpSceneLoader( ) = default;

bool AssimpSceneLoader::LoadScene( const InteropString &filePath, const AssimpImportDesc &desc )
{
    m_desc = desc;
    ConfigureImportFlags( desc );
    spdlog::info( "Loading scene from: {}", filePath.Get( ) );
    m_scene = m_importer->ReadFile( FileIO::GetResourcePath( filePath ).Get( ), m_importFlags );

    if ( !m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode )
    {
        spdlog::error( "Failed to load scene: {}", m_importer->GetErrorString( ) );
        return false;
    }

    GatherSceneStatistics( );
    spdlog::info( "Scene loaded successfully. Stats: {} meshes, {} materials, {} textures, {} animations, {} bones", m_stats.TotalMeshes, m_stats.TotalMaterials,
                  m_stats.TotalTextures, m_stats.TotalAnimations, m_stats.TotalBones );
    return true;
}

const aiScene *AssimpSceneLoader::GetScene( ) const
{
    return m_scene;
}

const AssimpSceneStats &AssimpSceneLoader::GetStats( ) const
{
    return m_stats;
}

unsigned int AssimpSceneLoader::GetImportFlags( ) const
{
    return m_importFlags;
}

void AssimpSceneLoader::ConfigureImportFlags( const AssimpImportDesc &desc )
{
    m_importFlags = 0;

    m_importFlags |= aiProcess_ImproveCacheLocality;
    m_importFlags |= aiProcess_SortByPType;

    if ( desc.TriangulateMeshes )
    {
        m_importFlags |= aiProcess_Triangulate;
    }

    if ( desc.JoinIdenticalVertices )
    {
        m_importFlags |= aiProcess_JoinIdenticalVertices;
    }

    if ( desc.CalculateTangentSpace )
    {
        m_importFlags |= aiProcess_CalcTangentSpace;
    }

    if ( desc.LimitBoneWeights )
    {
        m_importFlags |= aiProcess_LimitBoneWeights;
        m_importer->SetPropertyInteger( AI_CONFIG_PP_LBW_MAX_WEIGHTS, desc.MaxBoneWeightsPerVertex );
    }

    if ( desc.ConvertToLeftHanded )
    {
        m_importFlags |= aiProcess_ConvertToLeftHanded;
    }

    if ( desc.RemoveRedundantMaterials )
    {
        m_importFlags |= aiProcess_RemoveRedundantMaterials;
    }

    if ( desc.GenerateNormals )
    {
        if ( desc.SmoothNormals )
        {
            m_importFlags |= aiProcess_GenSmoothNormals;
            m_importer->SetPropertyFloat( AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, desc.SmoothNormalsAngle );
        }
        else
        {
            m_importFlags |= aiProcess_GenNormals;
        }
    }

    if ( desc.PreTransformVertices )
    {
        m_importFlags |= aiProcess_PreTransformVertices;
    }
    else if ( desc.OptimizeGraph )
    {
        m_importFlags |= aiProcess_OptimizeGraph;
    }

    if ( desc.OptimizeMeshes )
    {
        m_importFlags |= aiProcess_OptimizeMeshes;
    }

    if ( desc.MergeMeshes )
    {
        m_importFlags |= aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
        if ( m_importFlags & aiProcess_PreTransformVertices )
        {
            spdlog::warn( "MergeMeshes and PreTransformVertices may conflict. Disabling PreTransformVertices." );
            m_importFlags &= ~aiProcess_PreTransformVertices;
        }
        if ( !( m_importFlags & aiProcess_PreTransformVertices ) )
        {
            m_importFlags |= aiProcess_OptimizeGraph;
        }
    }

    if ( desc.DropNormals )
    {
        m_importFlags |= aiProcess_DropNormals;
        m_importFlags &= ~( aiProcess_GenNormals | aiProcess_GenSmoothNormals );
    }
}

void AssimpSceneLoader::GatherSceneStatistics( )
{
    m_stats = AssimpSceneStats{ };
    m_uniqueBoneNames.clear( );
    m_nodeChildrenCount.clear( );

    if ( !m_scene )
    {
        return;
    }

    m_stats.TotalMeshes     = m_scene->mNumMeshes;
    m_stats.TotalMaterials  = m_scene->mNumMaterials;
    m_stats.TotalTextures   = m_scene->mNumTextures;
    m_stats.TotalAnimations = m_scene->mNumAnimations;

    std::set<unsigned int> usedMeshIndices;
    CountNodeHierarchy( m_scene->mRootNode );

    for ( unsigned int i = 0; i < m_scene->mNumMeshes; ++i )
    {
        if ( const aiMesh *mesh = m_scene->mMeshes[ i ] )
        {
            m_stats.TotalVertices += mesh->mNumVertices;
            m_stats.TotalIndices += mesh->mNumFaces * 3; // Assuming triangulated

            m_stats.MaxUVChannels    = std::max( m_stats.MaxUVChannels, mesh->GetNumUVChannels( ) );
            m_stats.MaxColorChannels = std::max( m_stats.MaxColorChannels, mesh->GetNumColorChannels( ) );
            if ( mesh->HasBones( ) )
            {
                for ( unsigned int b = 0; b < mesh->mNumBones; ++b )
                {
                    if ( const aiBone *bone = mesh->mBones[ b ] )
                    {
                        m_uniqueBoneNames[ bone->mName.C_Str( ) ] = true;
                    }
                }
            }
        }
    }

    m_stats.TotalBones = static_cast<uint32_t>( m_uniqueBoneNames.size( ) );
    if ( !m_uniqueBoneNames.empty( ) )
    {
        CountBoneHierarchy( m_scene->mRootNode );
    }
    for ( unsigned int i = 0; i < m_scene->mNumAnimations; ++i )
    {
        if ( const aiAnimation *anim = m_scene->mAnimations[ i ] )
        {
            for ( unsigned int c = 0; c < anim->mNumChannels; ++c )
            {
                if ( const aiNodeAnim *nodeAnim = anim->mChannels[ c ] )
                {
                    m_stats.TotalAnimationKeys += nodeAnim->mNumPositionKeys;
                    m_stats.TotalAnimationKeys += nodeAnim->mNumRotationKeys;
                    m_stats.TotalAnimationKeys += nodeAnim->mNumScalingKeys;
                }
            }

            for ( unsigned int m = 0; m < anim->mNumMorphMeshChannels; ++m )
            {
                if ( const aiMeshMorphAnim *morphAnim = anim->mMorphMeshChannels[ m ] )
                {
                    m_stats.TotalAnimationKeys += morphAnim->mNumKeys;
                }
            }
        }
    }

    for ( const auto &count : m_nodeChildrenCount | std::views::values )
    {
        m_stats.MaxChildrenPerJoint = std::max( m_stats.MaxChildrenPerJoint, count );
    }

    m_stats.EstimatedArenaSize = 0;
    m_stats.EstimatedArenaSize += m_stats.TotalVertices * sizeof( MeshVertex );
    m_stats.EstimatedArenaSize += m_stats.TotalIndices * sizeof( uint32_t );
    m_stats.EstimatedArenaSize += m_stats.TotalMeshes * sizeof( SubMeshData );
    m_stats.EstimatedArenaSize += m_stats.TotalJoints * sizeof( Joint );
    m_stats.EstimatedArenaSize += m_stats.TotalJoints * m_stats.MaxChildrenPerJoint * sizeof( uint32_t );
    m_stats.EstimatedArenaSize += m_stats.TotalAnimationKeys * sizeof( PositionKey );
    m_stats.EstimatedArenaSize += m_stats.TotalAnimationKeys * sizeof( RotationKey );
    m_stats.EstimatedArenaSize += m_stats.TotalAnimationKeys * sizeof( ScaleKey );
    m_stats.EstimatedArenaSize     = static_cast<size_t>( m_stats.EstimatedArenaSize * 1.2 );
    m_stats.EstimatedAssetsCreated = 1; // Mesh asset
    if ( m_desc.ImportMaterials )
    {
        m_stats.EstimatedAssetsCreated += m_stats.TotalMaterials;
    }
    if ( m_desc.ImportTextures )
    {
        m_stats.EstimatedAssetsCreated += m_stats.TotalTextures;
    }
    if ( m_desc.ImportAnimations )
    {
        m_stats.EstimatedAssetsCreated += m_stats.TotalAnimations;
    }
    if ( m_desc.ImportSkeletons && m_stats.TotalBones > 0 )
    {
        m_stats.EstimatedAssetsCreated += 1;
    }
}

void AssimpSceneLoader::CountNodeHierarchy( const aiNode *node )
{
    if ( !node )
    {
        return;
    }
    for ( unsigned int i = 0; i < node->mNumMeshes; ++i )
    {
        if ( node->mMeshes[ i ] < m_scene->mNumMeshes )
        {
            m_stats.TotalUniqueMeshes++;
        }
    }
    if ( m_uniqueBoneNames.contains( node->mName.C_Str( ) ) )
    {
        uint32_t boneChildren = 0;
        for ( unsigned int i = 0; i < node->mNumChildren; ++i )
        {
            if ( m_uniqueBoneNames.contains( node->mChildren[ i ]->mName.C_Str( ) ) )
            {
                boneChildren++;
            }
        }
        m_nodeChildrenCount[ node ] = boneChildren;
    }
    for ( unsigned int i = 0; i < node->mNumChildren; ++i )
    {
        CountNodeHierarchy( node->mChildren[ i ] );
    }
}

void AssimpSceneLoader::CountBoneHierarchy( const aiNode *node )
{
    if ( !node )
    {
        return;
    }

    if ( m_uniqueBoneNames.contains( node->mName.C_Str( ) ) )
    {
        m_stats.TotalJoints++;
    }

    for ( unsigned int i = 0; i < node->mNumChildren; ++i )
    {
        CountBoneHierarchy( node->mChildren[ i ] );
    }
}
