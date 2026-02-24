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

#include "DenOfIzExamples/Bistro.h"
#include <filesystem>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <unordered_set>
#include "DenOfIzExamples/FileIO.h"
#include "DenOfIzExamples/GltfLoader.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"

#include <cgltf.h>

using namespace DenOfIz;

Bistro::Bistro( DenOfIz_LogicalDevice logicalDevice )
{
    spdlog::info( "Initializing Bistro scene..." );

    if ( !std::filesystem::exists( m_bistroRawPath ) )
    {
        spdlog::error( "Bistro GLTF file not found at: {}", m_bistroRawPath );
        return;
    }

    GltfLoader     gltfLoader;
    GltfLoadResult gltfResult = gltfLoader.Load( m_bistroRawPath, true );

    if ( !gltfResult.Success )
    {
        spdlog::error( "Failed to load Bistro GLTF: {}", gltfResult.ErrorMessage );
        return;
    }

    if ( gltfResult.Meshes.empty( ) )
    {
        spdlog::error( "No meshes found in Bistro GLTF" );
        return;
    }

    DenOfIz_BatchResourceCopy     batchCopy = DENOFIZ_NULL_HANDLE;
    DenOfIz_BatchResourceCopyDesc batchDesc{ };
    batchDesc.Device = logicalDevice;
    DenOfIz_BatchResourceCopy_Create( &batchDesc, &batchCopy );
    DenOfIz_BatchResourceCopy_Begin( batchCopy );

    cgltf_data   *gltfData    = nullptr;
    cgltf_options options     = { };
    cgltf_result  parseResult = cgltf_parse_file( &options, m_bistroRawPath.c_str( ), &gltfData );
    bool          hasGltfData = ( parseResult == cgltf_result_success );
    if ( hasGltfData )
    {
        cgltf_result loadResult = cgltf_load_buffers( &options, gltfData, m_bistroRawPath.c_str( ) );
        if ( loadResult != cgltf_result_success )
        {
            cgltf_free( gltfData );
            gltfData    = nullptr;
            hasGltfData = false;
        }
    }

    std::unordered_map<uint32_t, uint32_t> gltfMaterialToOurMaterial;

    auto loadTextureFromImage = [ &batchCopy, this ]( cgltf_image *image, const std::filesystem::path &gltfBasePath, const std::string &textureType ) -> uint32_t
    {
        if ( !image || !image->uri )
        {
            return INVALID_TEXTURE_HANDLE;
        }

        std::string texturePath = ( gltfBasePath / image->uri ).string( );
        std::string actualPath  = texturePath;

        if ( !std::filesystem::exists( actualPath ) )
        {
            std::filesystem::path pathObj( texturePath );
            std::string           extension = pathObj.extension( ).string( );

            std::vector<std::string> extensionsToTry = { ".dds", ".png", ".jpg", ".jpeg" };

            bool found = false;
            for ( const auto &ext : extensionsToTry )
            {
                std::string tryPath = pathObj.parent_path( ).string( ) + "/" + pathObj.stem( ).string( ) + ext;
                if ( std::filesystem::exists( tryPath ) )
                {
                    actualPath = tryPath;
                    found      = true;
                    break;
                }
            }

            if ( !found )
            {
                spdlog::warn( "[{}] Texture not found: {}", textureType, texturePath );
                return INVALID_TEXTURE_HANDLE;
            }
        }

        std::string cacheKey = actualPath + "_" + textureType;
        if ( m_texturePathToHandle.contains( cacheKey ) )
        {
            return m_texturePathToHandle[ cacheKey ];
        }

        DenOfIz_StringView texturePathView = DENOFIZ_STRING( actualPath.c_str( ) );
        DenOfIz_Texture    texture         = DenOfIz_BatchResourceCopy_CreateAndLoadTexture( batchCopy, texturePathView );
        if ( DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            DenOfIz_Format format = DENOFIZ_FORMAT_UNDEFINED;
            DenOfIz_TextureResource_GetFormat( texture, &format );

            const uint32_t handle = static_cast<uint32_t>( m_textureHandles.size( ) );
            m_textureHandles.push_back( texture );
            m_texturePathToHandle[ cacheKey ] = handle;
            return handle;
        }
        spdlog::error( "[{}] Failed to load texture: {}", textureType, actualPath );
        return INVALID_TEXTURE_HANDLE;
    };

    if ( hasGltfData && gltfData->materials_count > 0 )
    {
        std::filesystem::path gltfBasePath = std::filesystem::path( m_bistroRawPath ).parent_path( );

        for ( size_t i = 0; i < gltfData->materials_count; ++i )
        {
            cgltf_material *mat = &gltfData->materials[ i ];

            BistroMaterial bistroMaterial;
            bistroMaterial.MaterialRef                    = mat->name ? mat->name : "";
            bistroMaterial.AlbedoTextureHandle            = INVALID_TEXTURE_HANDLE;
            bistroMaterial.NormalTextureHandle            = INVALID_TEXTURE_HANDLE;
            bistroMaterial.MetallicRoughnessTextureHandle = INVALID_TEXTURE_HANDLE;
            bistroMaterial.EmissiveTextureHandle          = INVALID_TEXTURE_HANDLE;
            bistroMaterial.OcclusionTextureHandle         = INVALID_TEXTURE_HANDLE;

            if ( mat->has_pbr_metallic_roughness )
            {
                if ( mat->pbr_metallic_roughness.base_color_texture.texture && mat->pbr_metallic_roughness.base_color_texture.texture->image )
                {
                    bistroMaterial.AlbedoTextureHandle = loadTextureFromImage( mat->pbr_metallic_roughness.base_color_texture.texture->image, gltfBasePath, "Albedo" );
                }

                if ( mat->pbr_metallic_roughness.metallic_roughness_texture.texture && mat->pbr_metallic_roughness.metallic_roughness_texture.texture->image )
                {
                    bistroMaterial.MetallicRoughnessTextureHandle =
                        loadTextureFromImage( mat->pbr_metallic_roughness.metallic_roughness_texture.texture->image, gltfBasePath, "MetallicRoughness" );
                }
            }

            if ( mat->has_pbr_specular_glossiness )
            {
                if ( mat->pbr_specular_glossiness.diffuse_texture.texture && mat->pbr_specular_glossiness.diffuse_texture.texture->image )
                {
                    bistroMaterial.AlbedoTextureHandle = loadTextureFromImage( mat->pbr_specular_glossiness.diffuse_texture.texture->image, gltfBasePath, "Albedo" );
                }

                if ( mat->pbr_specular_glossiness.specular_glossiness_texture.texture && mat->pbr_specular_glossiness.specular_glossiness_texture.texture->image )
                {
                    bistroMaterial.MetallicRoughnessTextureHandle =
                        loadTextureFromImage( mat->pbr_specular_glossiness.specular_glossiness_texture.texture->image, gltfBasePath, "SpecularGlossiness" );
                }
            }

            if ( mat->normal_texture.texture && mat->normal_texture.texture->image )
            {
                bistroMaterial.NormalTextureHandle = loadTextureFromImage( mat->normal_texture.texture->image, gltfBasePath, "Normal" );
            }

            if ( mat->emissive_texture.texture && mat->emissive_texture.texture->image )
            {
                bistroMaterial.EmissiveTextureHandle = loadTextureFromImage( mat->emissive_texture.texture->image, gltfBasePath, "Emissive" );
            }

            if ( mat->occlusion_texture.texture && mat->occlusion_texture.texture->image )
            {
                bistroMaterial.OcclusionTextureHandle = loadTextureFromImage( mat->occlusion_texture.texture->image, gltfBasePath, "Occlusion" );
            }
            gltfMaterialToOurMaterial[ static_cast<uint32_t>( i ) ] = static_cast<uint32_t>( m_bistroMaterials.size( ) );
            m_bistroMaterials.push_back( std::move( bistroMaterial ) );
        }
    }
    else
    {
        BistroMaterial defaultMaterial;
        defaultMaterial.MaterialRef                    = "Default";
        defaultMaterial.AlbedoTextureHandle            = INVALID_TEXTURE_HANDLE;
        defaultMaterial.NormalTextureHandle            = INVALID_TEXTURE_HANDLE;
        defaultMaterial.MetallicRoughnessTextureHandle = INVALID_TEXTURE_HANDLE;
        defaultMaterial.EmissiveTextureHandle          = INVALID_TEXTURE_HANDLE;
        defaultMaterial.OcclusionTextureHandle         = INVALID_TEXTURE_HANDLE;
        m_bistroMaterials.push_back( std::move( defaultMaterial ) );
    }
    size_t totalNumVertices = 0;
    size_t totalNumIndices  = 0;

    for ( const auto &mesh : gltfResult.Meshes )
    {
        totalNumVertices += mesh.Vertices.size( );
        totalNumIndices += mesh.Indices.size( );
    }

    std::vector<BistroVertex> allVertices;
    std::vector<uint32_t>     allIndices;
    allVertices.reserve( totalNumVertices );
    allIndices.reserve( totalNumIndices );

    std::vector<size_t> meshVertexOffsets;
    std::vector<size_t> meshIndexOffsets;
    meshVertexOffsets.reserve( gltfResult.Meshes.size( ) );
    meshIndexOffsets.reserve( gltfResult.Meshes.size( ) );

    for ( const auto &mesh : gltfResult.Meshes )
    {
        meshVertexOffsets.push_back( allVertices.size( ) );
        meshIndexOffsets.push_back( allIndices.size( ) );

        for ( const auto &gltfVertex : mesh.Vertices )
        {
            BistroVertex bistroVertex;
            bistroVertex.Position  = { gltfVertex.Position.X, gltfVertex.Position.Y, gltfVertex.Position.Z, 1.0f };
            bistroVertex.Normal    = { gltfVertex.Normal.X, gltfVertex.Normal.Y, -gltfVertex.Normal.Z, 0.0f };
            bistroVertex.UV        = gltfVertex.TexCoord;
            bistroVertex.Tangent   = { gltfVertex.Tangent.X, gltfVertex.Tangent.Y, -gltfVertex.Tangent.Z, gltfVertex.Tangent.W };
            bistroVertex.Bitangent = { 0.0f, 0.0f, 0.0f, 0.0f };
            allVertices.push_back( bistroVertex );
        }

        for ( const auto &index : mesh.Indices )
        {
            allIndices.push_back( index );
        }
    }

    auto extractTransform = []( cgltf_node *node ) -> DenOfIz_Float4x4
    {
        float mat[ 16 ];
        cgltf_node_transform_world( node, mat );

        DenOfIz_Float4x4 result;
        result._11 = mat[ 0 ];
        result._12 = mat[ 4 ];
        result._13 = -mat[ 8 ];
        result._14 = mat[ 12 ];

        result._21 = mat[ 1 ];
        result._22 = mat[ 5 ];
        result._23 = -mat[ 9 ];
        result._24 = mat[ 13 ];

        result._31 = -mat[ 2 ];
        result._32 = -mat[ 6 ];
        result._33 = mat[ 10 ];
        result._34 = -mat[ 14 ];

        result._41 = mat[ 3 ];
        result._42 = mat[ 7 ];
        result._43 = -mat[ 11 ];
        result._44 = mat[ 15 ];

        return result;
    };

    if ( hasGltfData && gltfData->scenes_count > 0 )
    {
        cgltf_scene *scene = &gltfData->scenes[ 0 ];

        std::function<void( cgltf_node * )> processNode = [ & ]( cgltf_node *node )
        {
            if ( node->mesh )
            {
                cgltf_mesh *mesh      = node->mesh;
                size_t      meshIndex = mesh - gltfData->meshes;

                if ( meshIndex < gltfResult.Meshes.size( ) )
                {
                    BistroData bistroData;
                    bistroData.DrawData.VertexOffset = static_cast<uint32_t>( meshVertexOffsets[ meshIndex ] );
                    bistroData.DrawData.IndexOffset  = static_cast<uint32_t>( meshIndexOffsets[ meshIndex ] );
                    bistroData.DrawData.NumIndices   = static_cast<uint32_t>( gltfResult.Meshes[ meshIndex ].Indices.size( ) );
                    bistroData.DrawData.NumVertices  = static_cast<uint32_t>( gltfResult.Meshes[ meshIndex ].Vertices.size( ) );

                    uint32_t matIndex = gltfResult.Meshes[ meshIndex ].MaterialIndex;
                    if ( gltfMaterialToOurMaterial.contains( matIndex ) )
                    {
                        bistroData.MaterialIndex = gltfMaterialToOurMaterial[ matIndex ];
                    }
                    else
                    {
                        bistroData.MaterialIndex = 0;
                    }

                    bistroData.MaterialRef = m_bistroMaterials[ bistroData.MaterialIndex ].MaterialRef;

                    bistroData.Object.ObjectIndex = static_cast<uint32_t>( m_bistroData.size( ) );
                    bistroData.Object.Transform   = extractTransform( node );

                    bistroData.MinBounds = { 0, 0, 0 };
                    bistroData.MaxBounds = { 0, 0, 0 };

                    m_bistroData.push_back( bistroData );
                }
            }

            for ( size_t i = 0; i < node->children_count; ++i )
            {
                processNode( node->children[ i ] );
            }
        };

        for ( size_t i = 0; i < scene->nodes_count; ++i )
        {
            processNode( scene->nodes[ i ] );
        }
    }
    else
    {
        m_bistroData.resize( gltfResult.Meshes.size( ) );

        for ( size_t i = 0; i < gltfResult.Meshes.size( ); ++i )
        {
            const auto &mesh = gltfResult.Meshes[ i ];

            BistroData &bistroData           = m_bistroData[ i ];
            bistroData.DrawData.VertexOffset = static_cast<uint32_t>( meshVertexOffsets[ i ] );
            bistroData.DrawData.IndexOffset  = static_cast<uint32_t>( meshIndexOffsets[ i ] );
            bistroData.DrawData.NumIndices   = static_cast<uint32_t>( mesh.Indices.size( ) );
            bistroData.DrawData.NumVertices  = static_cast<uint32_t>( mesh.Vertices.size( ) );

            if ( gltfMaterialToOurMaterial.contains( mesh.MaterialIndex ) )
            {
                bistroData.MaterialIndex = gltfMaterialToOurMaterial[ mesh.MaterialIndex ];
            }
            else
            {
                bistroData.MaterialIndex = 0;
            }

            bistroData.MaterialRef = m_bistroMaterials[ bistroData.MaterialIndex ].MaterialRef;

            bistroData.Object.ObjectIndex = static_cast<uint32_t>( i );
            bistroData.Object.Transform   = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

            bistroData.MinBounds = { 0, 0, 0 };
            bistroData.MaxBounds = { 0, 0, 0 };
        }
    }

    std::vector<BistroMaterialData> materialDataBuffer( m_bistroMaterials.size( ) );
    for ( size_t i = 0; i < m_bistroMaterials.size( ); ++i )
    {
        const auto &material                                   = m_bistroMaterials[ i ];
        materialDataBuffer[ i ].AlbedoTextureHandle            = material.AlbedoTextureHandle;
        materialDataBuffer[ i ].NormalTextureHandle            = material.NormalTextureHandle;
        materialDataBuffer[ i ].MetallicRoughnessTextureHandle = material.MetallicRoughnessTextureHandle;
        materialDataBuffer[ i ].EmissiveTextureHandle          = material.EmissiveTextureHandle;
        materialDataBuffer[ i ].OcclusionTextureHandle         = material.OcclusionTextureHandle;
    }

    std::vector<BistroMeshInfo> meshInfoData( m_bistroData.size( ) );
    for ( size_t i = 0; i < m_bistroData.size( ); ++i )
    {
        const auto &bistroData = m_bistroData[ i ];

        meshInfoData[ i ].IndexOffsetBytes     = bistroData.DrawData.IndexOffset * sizeof( uint32_t );
        meshInfoData[ i ].AttributeStrideBytes = sizeof( BistroVertex );
        meshInfoData[ i ].MaterialInstanceId   = bistroData.MaterialIndex;

        uint32_t vertexBaseOffset                       = bistroData.DrawData.VertexOffset * sizeof( BistroVertex );
        meshInfoData[ i ].PositionAttributeOffsetBytes  = vertexBaseOffset + offsetof( BistroVertex, Position );
        meshInfoData[ i ].NormalAttributeOffsetBytes    = vertexBaseOffset + offsetof( BistroVertex, Normal );
        meshInfoData[ i ].UVAttributeOffsetBytes        = vertexBaseOffset + offsetof( BistroVertex, UV );
        meshInfoData[ i ].TangentAttributeOffsetBytes   = vertexBaseOffset + offsetof( BistroVertex, Tangent );
        meshInfoData[ i ].BitangentAttributeOffsetBytes = vertexBaseOffset + offsetof( BistroVertex, Bitangent );
    }

    DenOfIz_BufferDesc materialBufferDesc{ };
    materialBufferDesc.NumBytes                  = materialDataBuffer.size( ) * sizeof( BistroMaterialData );
    materialBufferDesc.StructureDesc.NumElements = materialDataBuffer.size( );
    materialBufferDesc.StructureDesc.Stride      = sizeof( BistroMaterialData );
    materialBufferDesc.StructureDesc.Offset      = 0;
    materialBufferDesc.Usage                     = DENOFIZ_BUFFER_USAGE_STORAGE_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    materialBufferDesc.DebugName                 = DENOFIZ_STRING( "MaterialBuffer" );
    materialBufferDesc.HeapType                  = DENOFIZ_HEAP_TYPE_GPU;
    DenOfIz_LogicalDevice_CreateBuffer( logicalDevice, &materialBufferDesc, &m_materialBuffer );

    DenOfIz_BufferDesc meshInfoBufferDesc{ };
    meshInfoBufferDesc.NumBytes                  = meshInfoData.size( ) * sizeof( BistroMeshInfo );
    meshInfoBufferDesc.StructureDesc.NumElements = meshInfoData.size( );
    meshInfoBufferDesc.StructureDesc.Stride      = sizeof( BistroMeshInfo );
    meshInfoBufferDesc.StructureDesc.Offset      = 0;
    meshInfoBufferDesc.Usage                     = DENOFIZ_BUFFER_USAGE_STORAGE_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    meshInfoBufferDesc.DebugName                 = DENOFIZ_STRING( "MeshInfoBuffer" );
    meshInfoBufferDesc.HeapType                  = DENOFIZ_HEAP_TYPE_GPU;
    DenOfIz_LogicalDevice_CreateBuffer( logicalDevice, &meshInfoBufferDesc, &m_meshInfoBuffer );

    DenOfIz_BufferDesc vbBufferDesc{ };
    vbBufferDesc.NumBytes  = allVertices.size( ) * sizeof( BistroVertex );
    vbBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_STORAGE_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT | DENOFIZ_BUFFER_USAGE_ACCELERATION_GEOMETRY_BIT;
    vbBufferDesc.DebugName = DENOFIZ_STRING( "BistroVertexBuffer" );
    vbBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    DenOfIz_LogicalDevice_CreateBuffer( logicalDevice, &vbBufferDesc, &m_vertexBuffer );

    DenOfIz_BufferDesc ibBufferDesc{ };
    ibBufferDesc.NumBytes  = allIndices.size( ) * sizeof( uint32_t );
    ibBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_STORAGE_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT | DENOFIZ_BUFFER_USAGE_ACCELERATION_GEOMETRY_BIT;
    ibBufferDesc.DebugName = DENOFIZ_STRING( "BistroIndexBuffer" );
    ibBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    DenOfIz_LogicalDevice_CreateBuffer( logicalDevice, &ibBufferDesc, &m_indexBuffer );

    DenOfIz_CopyToGpuBufferDesc vertexCopyDesc{ };
    vertexCopyDesc.DstBuffer        = m_vertexBuffer;
    vertexCopyDesc.DstBufferOffset  = 0;
    vertexCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( allVertices.data( ) );
    vertexCopyDesc.Data.NumElements = allVertices.size( ) * sizeof( BistroVertex );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &vertexCopyDesc );

    DenOfIz_CopyToGpuBufferDesc indexCopyDesc{ };
    indexCopyDesc.DstBuffer        = m_indexBuffer;
    indexCopyDesc.DstBufferOffset  = 0;
    indexCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( allIndices.data( ) );
    indexCopyDesc.Data.NumElements = allIndices.size( ) * sizeof( uint32_t );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &indexCopyDesc );

    DenOfIz_CopyToGpuBufferDesc materialCopy{ };
    materialCopy.DstBuffer        = m_materialBuffer;
    materialCopy.Data.Elements    = reinterpret_cast<const Byte *>( materialDataBuffer.data( ) );
    materialCopy.Data.NumElements = materialDataBuffer.size( ) * sizeof( BistroMaterialData );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &materialCopy );

    DenOfIz_CopyToGpuBufferDesc meshInfoCopy{ };
    meshInfoCopy.DstBuffer        = m_meshInfoBuffer;
    meshInfoCopy.Data.Elements    = reinterpret_cast<const Byte *>( meshInfoData.data( ) );
    meshInfoCopy.Data.NumElements = meshInfoData.size( ) * sizeof( BistroMeshInfo );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &meshInfoCopy );

    DenOfIz_BatchResourceCopy_Submit( batchCopy, DENOFIZ_NULL_HANDLE );
    DenOfIz_BatchResourceCopy_Destroy( batchCopy );

    if ( gltfData )
    {
        cgltf_free( gltfData );
    }

    spdlog::info( "Bistro scene loaded: {} meshes, {} materials, {} textures", m_bistroData.size( ), m_bistroMaterials.size( ), m_textureHandles.size( ) );
}

Bistro::~Bistro( )
{
    for ( auto &texture : m_textureHandles )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            DenOfIz_TextureResource_Destroy( texture );
        }
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_materialBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_materialBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_vertexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_vertexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_indexBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_indexBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_meshInfoBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_meshInfoBuffer );
    }
}

DenOfIz_Buffer Bistro::GetMaterialBuffer( ) const
{
    return m_materialBuffer;
}

DenOfIz_Buffer Bistro::GetVertexBuffer( ) const
{
    return m_vertexBuffer;
}

DenOfIz_Buffer Bistro::GetIndexBuffer( ) const
{
    return m_indexBuffer;
}

DenOfIz_Buffer Bistro::GetMeshInfoBuffer( ) const
{
    return m_meshInfoBuffer;
}

const std::vector<BistroData> &Bistro::GetBistroData( ) const
{
    return m_bistroData;
}

size_t Bistro::GetNumSubmeshes( ) const
{
    return m_bistroData.size( );
}

size_t Bistro::GetNumMaterials( ) const
{
    return m_bistroMaterials.size( );
}

DenOfIz_TextureArray Bistro::GetTextures( )
{
    DenOfIz_TextureArray result{ };
    result.NumElements = static_cast<uint32_t>( m_textureHandles.size( ) );
    result.Elements    = m_textureHandles.data( );
    return result;
}
