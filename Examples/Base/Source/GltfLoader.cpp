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

#include "DenOfIzExamples/GltfLoader.h"
#include "DenOfIzExamples/FileIO.h"

#include <cgltf.h>
#include <cstring>

namespace DenOfIz
{
    class GltfLoaderImpl
    {
    public:
        cgltf_data *m_gltfData = nullptr;
        std::string m_errorMessage;

        ~GltfLoaderImpl( )
        {
            if ( m_gltfData )
            {
                cgltf_free( m_gltfData );
                m_gltfData = nullptr;
            }
        }

        void Reset( )
        {
            if ( m_gltfData )
            {
                cgltf_free( m_gltfData );
                m_gltfData = nullptr;
            }
            m_errorMessage.clear( );
        }

        bool ParseFile( const std::string &filePath )
        {
            cgltf_options options = { };
            cgltf_result  result  = cgltf_parse_file( &options, filePath.c_str( ), &m_gltfData );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to parse GLTF file: " + filePath;
                return false;
            }

            result = cgltf_load_buffers( &options, m_gltfData, filePath.c_str( ) );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to load GLTF buffers";
                cgltf_free( m_gltfData );
                m_gltfData = nullptr;
                return false;
            }

            return true;
        }

        void LoadMeshes( std::vector<GltfMesh> &meshes, bool convertToLH )
        {
            if ( !m_gltfData || m_gltfData->meshes_count == 0 )
            {
                return;
            }

            for ( size_t meshIdx = 0; meshIdx < m_gltfData->meshes_count; ++meshIdx )
            {
                cgltf_mesh *mesh = &m_gltfData->meshes[ meshIdx ];

                GltfMesh gltfMesh;
                gltfMesh.Name = mesh->name ? mesh->name : "";

                for ( size_t primIdx = 0; primIdx < mesh->primitives_count; ++primIdx )
                {
                    cgltf_primitive *primitive = &mesh->primitives[ primIdx ];

                    if ( primIdx == 0 && primitive->material )
                    {
                        gltfMesh.MaterialIndex = static_cast<uint32_t>( primitive->material - m_gltfData->materials );
                    }

                    size_t       vertexCount    = 0;
                    const float *positions      = nullptr;
                    const float *normals        = nullptr;
                    const float *texcoords      = nullptr;
                    const float *tangents       = nullptr;
                    const void  *joints         = nullptr;
                    const float *weights        = nullptr;
                    bool         jointsAreShort = false;

                    for ( size_t a = 0; a < primitive->attributes_count; ++a )
                    {
                        cgltf_attribute *attr     = &primitive->attributes[ a ];
                        cgltf_accessor  *accessor = attr->data;

                        if ( attr->type == cgltf_attribute_type_position )
                        {
                            vertexCount         = accessor->count;
                            const uint8_t *base = cgltf_buffer_view_data( accessor->buffer_view );
                            positions           = reinterpret_cast<const float *>( base + accessor->offset );
                        }
                        else if ( attr->type == cgltf_attribute_type_normal )
                        {
                            const uint8_t *base = cgltf_buffer_view_data( accessor->buffer_view );
                            normals             = reinterpret_cast<const float *>( base + accessor->offset );
                        }
                        else if ( attr->type == cgltf_attribute_type_texcoord )
                        {
                            const uint8_t *base = cgltf_buffer_view_data( accessor->buffer_view );
                            texcoords           = reinterpret_cast<const float *>( base + accessor->offset );
                        }
                        else if ( attr->type == cgltf_attribute_type_tangent )
                        {
                            const uint8_t *base = cgltf_buffer_view_data( accessor->buffer_view );
                            tangents            = reinterpret_cast<const float *>( base + accessor->offset );
                        }
                        else if ( attr->type == cgltf_attribute_type_joints )
                        {
                            const uint8_t *base = cgltf_buffer_view_data( accessor->buffer_view );
                            joints              = base + accessor->offset;
                            jointsAreShort      = ( accessor->component_type == cgltf_component_type_r_16u );
                        }
                        else if ( attr->type == cgltf_attribute_type_weights )
                        {
                            const uint8_t *base = cgltf_buffer_view_data( accessor->buffer_view );
                            weights             = reinterpret_cast<const float *>( base + accessor->offset );
                        }
                    }

                    size_t baseVertex = gltfMesh.Vertices.size( );
                    gltfMesh.Vertices.resize( baseVertex + vertexCount );

                    for ( size_t v = 0; v < vertexCount; ++v )
                    {
                        GltfVertex &sv = gltfMesh.Vertices[ baseVertex + v ];

                        if ( positions )
                        {
                            sv.Position.X = positions[ v * 3 + 0 ];
                            sv.Position.Y = positions[ v * 3 + 1 ];
                            sv.Position.Z = positions[ v * 3 + 2 ];
                            if ( convertToLH )
                            {
                                sv.Position.Z = -sv.Position.Z;
                            }
                        }

                        if ( normals )
                        {
                            sv.Normal.X = normals[ v * 3 + 0 ];
                            sv.Normal.Y = normals[ v * 3 + 1 ];
                            sv.Normal.Z = normals[ v * 3 + 2 ];
                            if ( convertToLH )
                            {
                                sv.Normal.Z = -sv.Normal.Z;
                            }
                        }

                        if ( texcoords )
                        {
                            sv.TexCoord.X = texcoords[ v * 2 + 0 ];
                            sv.TexCoord.Y = texcoords[ v * 2 + 1 ];
                        }

                        if ( tangents )
                        {
                            sv.Tangent.X = tangents[ v * 4 + 0 ];
                            sv.Tangent.Y = tangents[ v * 4 + 1 ];
                            sv.Tangent.Z = tangents[ v * 4 + 2 ];
                            sv.Tangent.W = tangents[ v * 4 + 3 ];
                            if ( convertToLH )
                            {
                                sv.Tangent.Z = -sv.Tangent.Z;
                                sv.Tangent.W = -sv.Tangent.W;
                            }
                        }

                        if ( joints )
                        {
                            if ( jointsAreShort )
                            {
                                const uint16_t *j = static_cast<const uint16_t *>( joints ) + v * 4;
                                sv.BlendIndices   = { j[ 0 ], j[ 1 ], j[ 2 ], j[ 3 ] };
                            }
                            else
                            {
                                const uint8_t *j = static_cast<const uint8_t *>( joints ) + v * 4;
                                sv.BlendIndices  = { j[ 0 ], j[ 1 ], j[ 2 ], j[ 3 ] };
                            }
                        }
                        else
                        {
                            sv.BlendIndices = { 0, 0, 0, 0 };
                        }

                        if ( weights )
                        {
                            sv.BoneWeights.X = weights[ v * 4 + 0 ];
                            sv.BoneWeights.Y = weights[ v * 4 + 1 ];
                            sv.BoneWeights.Z = weights[ v * 4 + 2 ];
                            sv.BoneWeights.W = weights[ v * 4 + 3 ];

                            float total = sv.BoneWeights.X + sv.BoneWeights.Y + sv.BoneWeights.Z + sv.BoneWeights.W;
                            if ( total > 1e-6f )
                            {
                                float inv = 1.0f / total;
                                sv.BoneWeights.X *= inv;
                                sv.BoneWeights.Y *= inv;
                                sv.BoneWeights.Z *= inv;
                                sv.BoneWeights.W *= inv;
                            }
                        }
                        else
                        {
                            sv.BoneWeights = { 1.0f, 0.0f, 0.0f, 0.0f };
                        }
                    }

                    if ( primitive->indices )
                    {
                        cgltf_accessor *indexAccessor = primitive->indices;
                        size_t          baseIndex     = gltfMesh.Indices.size( );
                        size_t          indexCount    = indexAccessor->count;
                        gltfMesh.Indices.resize( baseIndex + indexCount );

                        for ( size_t i = 0; i < indexCount; ++i )
                        {
                            gltfMesh.Indices[ baseIndex + i ] = static_cast<uint32_t>( cgltf_accessor_read_index( indexAccessor, i ) + baseVertex );
                        }

                        if ( convertToLH )
                        {
                            for ( size_t i = baseIndex; i + 2 < gltfMesh.Indices.size( ); i += 3 )
                            {
                                std::swap( gltfMesh.Indices[ i + 1 ], gltfMesh.Indices[ i + 2 ] );
                            }
                        }
                    }
                }

                meshes.push_back( std::move( gltfMesh ) );
            }
        }

        void LoadSkins( std::vector<GltfSkin> &skins, bool convertToLH )
        {
            if ( !m_gltfData || m_gltfData->skins_count == 0 )
            {
                return;
            }

            for ( size_t skinIdx = 0; skinIdx < m_gltfData->skins_count; ++skinIdx )
            {
                cgltf_skin *skin = &m_gltfData->skins[ skinIdx ];

                GltfSkin gltfSkin;
                gltfSkin.Name = skin->name ? skin->name : "";

                if ( skin->inverse_bind_matrices )
                {
                    gltfSkin.InverseBindMatrices.resize( skin->joints_count );
                    for ( size_t j = 0; j < skin->joints_count; ++j )
                    {
                        float mat[ 16 ];
                        cgltf_accessor_read_float( skin->inverse_bind_matrices, j, mat, 16 );

                        DenOfIz_Float4x4 &ibm = gltfSkin.InverseBindMatrices[ j ];

                        if ( convertToLH )
                        {
                            ibm._11 = mat[ 0 ];
                            ibm._12 = mat[ 1 ];
                            ibm._13 = -mat[ 2 ];
                            ibm._14 = mat[ 3 ];
                            ibm._21 = mat[ 4 ];
                            ibm._22 = mat[ 5 ];
                            ibm._23 = -mat[ 6 ];
                            ibm._24 = mat[ 7 ];
                            ibm._31 = -mat[ 8 ];
                            ibm._32 = -mat[ 9 ];
                            ibm._33 = mat[ 10 ];
                            ibm._34 = -mat[ 11 ];
                            ibm._41 = mat[ 12 ];
                            ibm._42 = mat[ 13 ];
                            ibm._43 = -mat[ 14 ];
                            ibm._44 = mat[ 15 ];
                        }
                        else
                        {
                            memcpy( &ibm, mat, sizeof( float ) * 16 );
                        }
                    }
                }

                gltfSkin.JointNames.resize( skin->joints_count );
                gltfSkin.JointParents.resize( skin->joints_count, -1 );

                for ( size_t j = 0; j < skin->joints_count; ++j )
                {
                    cgltf_node *jointNode    = skin->joints[ j ];
                    gltfSkin.JointNames[ j ] = jointNode->name ? jointNode->name : "";

                    if ( jointNode->parent )
                    {
                        for ( size_t k = 0; k < skin->joints_count; ++k )
                        {
                            if ( skin->joints[ k ] == jointNode->parent )
                            {
                                gltfSkin.JointParents[ j ] = static_cast<int32_t>( k );
                                break;
                            }
                        }
                    }
                }

                skins.push_back( std::move( gltfSkin ) );
            }
        }
    };

    GltfLoader::GltfLoader( ) : m_impl( std::make_unique<GltfLoaderImpl>( ) )
    {
    }

    GltfLoader::~GltfLoader( ) = default;

    GltfLoadResult GltfLoader::Load( const std::string &filePath, bool convertToLeftHanded )
    {
        GltfLoadResult result;
        m_impl->Reset( );

        DenOfIz_StringView pathView{ filePath.c_str( ), filePath.size( ) };
        if ( !FileIO::FileExists( pathView ) )
        {
            result.Success      = false;
            result.ErrorMessage = "File not found: " + filePath;
            return result;
        }

        if ( !m_impl->ParseFile( filePath ) )
        {
            result.Success      = false;
            result.ErrorMessage = m_impl->m_errorMessage;
            return result;
        }

        m_impl->LoadMeshes( result.Meshes, convertToLeftHanded );
        m_impl->LoadSkins( result.Skins, convertToLeftHanded );

        result.Success = true;
        return result;
    }
} // namespace DenOfIz
