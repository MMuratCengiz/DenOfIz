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

#include "DenOfIzGraphics/Assets/Import/OzzExporter.h"
#include "DenOfIzGraphicsInternal/Animation/OzzImpl.h"
#include "DenOfIzGraphicsInternal/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/io/archive.h>
#include <ozz/base/io/stream.h>

#include <cmath>
#include <cstring>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#define OZZ_EXPORTER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::OzzExporterImpl, handle )
#define OZZ_SKELETON_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::OzzSkeletonImpl, handle )

namespace DenOfIz
{
    struct ReferenceTransform
    {
        float translation[ 3 ];
        float rotation[ 4 ];
        float scale[ 3 ];
    };

    static void SanitizeTransform( float translation[ 3 ], float rotation[ 4 ], float scale[ 3 ] )
    {
        for ( int i = 0; i < 3; ++i )
        {
            if ( std::isnan( translation[ i ] ) || std::isinf( translation[ i ] ) )
            {
                translation[ i ] = 0.0f;
            }
        }

        bool rotNan = false;
        for ( int i = 0; i < 4; ++i )
        {
            if ( std::isnan( rotation[ i ] ) || std::isinf( rotation[ i ] ) )
            {
                rotNan = true;
                break;
            }
        }

        if ( rotNan )
        {
            rotation[ 0 ] = 0.0f;
            rotation[ 1 ] = 0.0f;
            rotation[ 2 ] = 0.0f;
            rotation[ 3 ] = 1.0f;
        }
        else
        {
            float len = std::sqrt( rotation[ 0 ] * rotation[ 0 ] + rotation[ 1 ] * rotation[ 1 ] +
                                   rotation[ 2 ] * rotation[ 2 ] + rotation[ 3 ] * rotation[ 3 ] );
            if ( len < 1e-6f )
            {
                rotation[ 0 ] = 0.0f;
                rotation[ 1 ] = 0.0f;
                rotation[ 2 ] = 0.0f;
                rotation[ 3 ] = 1.0f;
            }
            else if ( std::abs( len - 1.0f ) > 1e-4f )
            {
                float invLen  = 1.0f / len;
                rotation[ 0 ] *= invLen;
                rotation[ 1 ] *= invLen;
                rotation[ 2 ] *= invLen;
                rotation[ 3 ] *= invLen;
            }
        }

        for ( int i = 0; i < 3; ++i )
        {
            if ( std::isnan( scale[ i ] ) || std::isinf( scale[ i ] ) )
            {
                scale[ i ] = 1.0f;
            }
            else if ( std::abs( scale[ i ] ) < 1e-4f )
            {
                scale[ i ] = 1e-4f;
            }
        }
    }

    class OzzExporterImpl
    {
    public:
        cgltf_data                               *m_gltfData          = nullptr;
        ozz::unique_ptr<ozz::animation::Skeleton> m_ozzSkeleton;
        ozz::animation::Skeleton                 *m_externalSkeleton  = nullptr;
        std::string                               m_errorMessage;
        std::string                               m_resultSkeletonPath;
        std::vector<std::string>                  m_resultAnimationPaths;

        std::unordered_map<std::string, int32_t> m_jointNameToIndex;
        std::vector<std::string>                 m_jointNames;
        std::vector<int32_t>                     m_jointParents;

        std::unordered_map<std::string, ReferenceTransform> m_referenceTransforms;
        bool                                                m_sanitizeTransforms = false;

        ~OzzExporterImpl( )
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
            m_ozzSkeleton.reset( );
            m_externalSkeleton = nullptr;
            m_errorMessage.clear( );
            m_resultSkeletonPath.clear( );
            m_resultAnimationPaths.clear( );
            m_jointNameToIndex.clear( );
            m_jointNames.clear( );
            m_jointParents.clear( );
            m_referenceTransforms.clear( );
            m_sanitizeTransforms = false;
        }

        ozz::animation::Skeleton *GetSkeleton( )
        {
            if ( m_externalSkeleton )
            {
                return m_externalSkeleton;
            }
            return m_ozzSkeleton.get( );
        }

        bool ParseGltf( const std::string &filePath )
        {
            std::string resolvedPath = FileIO::GetResourcePath( DenOfIz_StringView{ filePath.c_str( ), filePath.size( ) } );

            cgltf_options options = { };
            cgltf_result  result  = cgltf_parse_file( &options, resolvedPath.c_str( ), &m_gltfData );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to parse GLTF file: " + resolvedPath;
                return false;
            }

            result = cgltf_load_buffers( &options, m_gltfData, resolvedPath.c_str( ) );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to load GLTF buffers";
                cgltf_free( m_gltfData );
                m_gltfData = nullptr;
                return false;
            }

            return true;
        }

        bool ParseGltfFromMemory( const void *data, size_t size, const std::string &basePath )
        {
            cgltf_options options = { };
            cgltf_result  result  = cgltf_parse( &options, data, size, &m_gltfData );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to parse GLTF data from memory";
                return false;
            }

            std::string resolvedBasePath = basePath.empty( ) ? "." : basePath;
            result                       = cgltf_load_buffers( &options, m_gltfData, resolvedBasePath.c_str( ) );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to load GLTF buffers from memory";
                cgltf_free( m_gltfData );
                m_gltfData = nullptr;
                return false;
            }

            return true;
        }

        void BuildReferenceTransformMap( cgltf_data *refData )
        {
            m_referenceTransforms.clear( );

            if ( !refData || refData->skins_count == 0 )
            {
                return;
            }

            cgltf_skin *skin = &refData->skins[ 0 ];
            for ( size_t j = 0; j < skin->joints_count; ++j )
            {
                cgltf_node *node = skin->joints[ j ];
                if ( !node || !node->name )
                {
                    continue;
                }

                ReferenceTransform ref = { };
                ref.translation[ 0 ] = 0.0f;
                ref.translation[ 1 ] = 0.0f;
                ref.translation[ 2 ] = 0.0f;
                ref.rotation[ 0 ]    = 0.0f;
                ref.rotation[ 1 ]    = 0.0f;
                ref.rotation[ 2 ]    = 0.0f;
                ref.rotation[ 3 ]    = 1.0f;
                ref.scale[ 0 ]       = 1.0f;
                ref.scale[ 1 ]       = 1.0f;
                ref.scale[ 2 ]       = 1.0f;

                if ( node->has_translation )
                {
                    ref.translation[ 0 ] = node->translation[ 0 ];
                    ref.translation[ 1 ] = node->translation[ 1 ];
                    ref.translation[ 2 ] = node->translation[ 2 ];
                }
                if ( node->has_rotation )
                {
                    ref.rotation[ 0 ] = node->rotation[ 0 ];
                    ref.rotation[ 1 ] = node->rotation[ 1 ];
                    ref.rotation[ 2 ] = node->rotation[ 2 ];
                    ref.rotation[ 3 ] = node->rotation[ 3 ];
                }
                if ( node->has_scale )
                {
                    ref.scale[ 0 ] = node->scale[ 0 ];
                    ref.scale[ 1 ] = node->scale[ 1 ];
                    ref.scale[ 2 ] = node->scale[ 2 ];
                }

                m_referenceTransforms[ node->name ] = ref;
            }

            spdlog::info( "OzzExporter: Loaded {} reference transforms", m_referenceTransforms.size( ) );
        }

        bool LoadReferenceTransforms( const std::string &filePath )
        {
            std::string resolvedPath = FileIO::GetResourcePath( DenOfIz_StringView{ filePath.c_str( ), filePath.size( ) } );

            cgltf_data   *refData = nullptr;
            cgltf_options options = { };
            cgltf_result  result  = cgltf_parse_file( &options, resolvedPath.c_str( ), &refData );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to parse reference GLTF file: " + resolvedPath;
                return false;
            }

            result = cgltf_load_buffers( &options, refData, resolvedPath.c_str( ) );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to load reference GLTF buffers";
                cgltf_free( refData );
                return false;
            }

            BuildReferenceTransformMap( refData );
            cgltf_free( refData );
            return true;
        }

        bool LoadReferenceTransformsFromMemory( const void *data, size_t size, const std::string &basePath )
        {
            cgltf_data   *refData = nullptr;
            cgltf_options options = { };
            cgltf_result  result  = cgltf_parse( &options, data, size, &refData );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to parse reference GLTF data from memory";
                return false;
            }

            std::string resolvedBasePath = basePath.empty( ) ? "." : basePath;
            result                       = cgltf_load_buffers( &options, refData, resolvedBasePath.c_str( ) );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to load reference GLTF buffers from memory";
                cgltf_free( refData );
                return false;
            }

            BuildReferenceTransformMap( refData );
            cgltf_free( refData );
            return true;
        }

        void BuildSkeletonHierarchy( )
        {
            m_jointNameToIndex.clear( );
            m_jointNames.clear( );
            m_jointParents.clear( );

            if ( !m_gltfData || m_gltfData->skins_count == 0 )
            {
                return;
            }

            cgltf_skin *skin = &m_gltfData->skins[ 0 ];

            std::unordered_map<cgltf_node *, int32_t> nodeToIndex;
            nodeToIndex.reserve( skin->joints_count );

            for ( size_t j = 0; j < skin->joints_count; ++j )
            {
                cgltf_node *jointNode           = skin->joints[ j ];
                std::string jointName           = jointNode->name ? jointNode->name : "joint_" + std::to_string( j );
                m_jointNameToIndex[ jointName ] = static_cast<int32_t>( j );
                m_jointNames.push_back( jointName );
                nodeToIndex[ jointNode ]        = static_cast<int32_t>( j );
            }

            m_jointParents.resize( skin->joints_count, -1 );
            for ( size_t j = 0; j < skin->joints_count; ++j )
            {
                cgltf_node *jointNode = skin->joints[ j ];
                if ( jointNode->parent )
                {
                    auto it = nodeToIndex.find( jointNode->parent );
                    if ( it != nodeToIndex.end( ) )
                    {
                        m_jointParents[ j ] = it->second;
                    }
                }
            }
        }

        bool WriteOzzSkeleton( const std::string &outputPath )
        {
            if ( m_jointNames.empty( ) )
            {
                m_errorMessage = "No skeleton data to export";
                return false;
            }

            ozz::animation::offline::RawSkeleton                                       rawSkeleton;
            std::unordered_map<int32_t, ozz::animation::offline::RawSkeleton::Joint *> jointMap;

            cgltf_skin *skin = &m_gltfData->skins[ 0 ];

            for ( size_t i = 0; i < m_jointNames.size( ); ++i )
            {
                int32_t parentIdx = m_jointParents[ i ];

                ozz::animation::offline::RawSkeleton::Joint joint;
                joint.name = m_jointNames[ i ];

                cgltf_node *node = skin->joints[ i ];
                if ( node )
                {
                    float translation[ 3 ] = { 0.0f, 0.0f, 0.0f };
                    float rotation[ 4 ]    = { 0.0f, 0.0f, 0.0f, 1.0f };
                    float scale[ 3 ]       = { 1.0f, 1.0f, 1.0f };

                    if ( node->has_translation )
                    {
                        translation[ 0 ] = node->translation[ 0 ];
                        translation[ 1 ] = node->translation[ 1 ];
                        translation[ 2 ] = node->translation[ 2 ];
                    }
                    if ( node->has_rotation )
                    {
                        rotation[ 0 ] = node->rotation[ 0 ];
                        rotation[ 1 ] = node->rotation[ 1 ];
                        rotation[ 2 ] = node->rotation[ 2 ];
                        rotation[ 3 ] = node->rotation[ 3 ];
                    }
                    if ( node->has_scale )
                    {
                        scale[ 0 ] = node->scale[ 0 ];
                        scale[ 1 ] = node->scale[ 1 ];
                        scale[ 2 ] = node->scale[ 2 ];
                    }

                    if ( !m_referenceTransforms.empty( ) )
                    {
                        auto refIt = m_referenceTransforms.find( m_jointNames[ i ] );
                        if ( refIt != m_referenceTransforms.end( ) )
                        {
                            std::memcpy( translation, refIt->second.translation, sizeof( float ) * 3 );
                            std::memcpy( rotation, refIt->second.rotation, sizeof( float ) * 4 );
                            std::memcpy( scale, refIt->second.scale, sizeof( float ) * 3 );
                        }
                        else
                        {
                            spdlog::warn( "OzzExporter: Joint '{}' not found in reference GLTF, using source transform", m_jointNames[ i ] );
                        }
                    }

                    if ( m_sanitizeTransforms )
                    {
                        SanitizeTransform( translation, rotation, scale );
                    }

                    joint.transform.translation = ozz::math::Float3( translation[ 0 ], translation[ 1 ], translation[ 2 ] );
                    joint.transform.rotation    = ozz::math::Quaternion( rotation[ 0 ], rotation[ 1 ], rotation[ 2 ], rotation[ 3 ] );
                    joint.transform.scale       = ozz::math::Float3( scale[ 0 ], scale[ 1 ], scale[ 2 ] );
                }

                if ( parentIdx < 0 )
                {
                    rawSkeleton.roots.push_back( joint );
                    jointMap[ static_cast<int32_t>( i ) ] = &rawSkeleton.roots.back( );
                }
                else
                {
                    auto parentIt = jointMap.find( parentIdx );
                    if ( parentIt != jointMap.end( ) )
                    {
                        parentIt->second->children.push_back( joint );
                        jointMap[ static_cast<int32_t>( i ) ] = &parentIt->second->children.back( );
                    }
                    else
                    {
                        rawSkeleton.roots.push_back( joint );
                        jointMap[ static_cast<int32_t>( i ) ] = &rawSkeleton.roots.back( );
                    }
                }
            }

            ozz::animation::offline::SkeletonBuilder builder;
            m_ozzSkeleton = builder( rawSkeleton );

            if ( !m_ozzSkeleton )
            {
                m_errorMessage = "Failed to build ozz skeleton";
                return false;
            }

            ozz::io::File file( outputPath.c_str( ), "wb" );
            if ( !file.opened( ) )
            {
                m_errorMessage = "Failed to open file for writing: " + outputPath;
                return false;
            }

            ozz::io::OArchive archive( &file );
            archive << *GetSkeleton( );

            spdlog::info( "OzzExporter: Wrote skeleton to {}", outputPath );
            return true;
        }

        bool WriteOzzAnimation( const std::string &outputPath, uint32_t animIndex )
        {
            if ( !m_gltfData || animIndex >= m_gltfData->animations_count )
            {
                m_errorMessage = "Animation index out of range";
                return false;
            }

            ozz::animation::Skeleton *skeleton = GetSkeleton( );
            if ( !skeleton )
            {
                m_errorMessage = "No skeleton available for animation export";
                return false;
            }

            cgltf_animation *gltfAnim = &m_gltfData->animations[ animIndex ];

            float maxTime = 0.0f;
            for ( size_t c = 0; c < gltfAnim->channels_count; ++c )
            {
                cgltf_accessor *input = gltfAnim->channels[ c ].sampler->input;
                if ( input->has_max && input->max[ 0 ] > maxTime )
                {
                    maxTime = input->max[ 0 ];
                }
            }

            ozz::animation::offline::RawAnimation rawAnimation;
            rawAnimation.duration = maxTime > 0.0f ? maxTime : 1.0f;
            rawAnimation.tracks.resize( skeleton->num_joints( ) );

            std::unordered_map<std::string, int> jointNameToOzzIndex;
            for ( int i = 0; i < skeleton->num_joints( ); ++i )
            {
                jointNameToOzzIndex[ skeleton->joint_names( )[ i ] ] = i;
            }

            std::vector<float> times;
            std::vector<float> values;

            for ( size_t c = 0; c < gltfAnim->channels_count; ++c )
            {
                cgltf_animation_channel *channel = &gltfAnim->channels[ c ];
                cgltf_node              *target  = channel->target_node;

                if ( !target || !target->name )
                {
                    continue;
                }

                std::string jointName = target->name;
                auto        it        = jointNameToOzzIndex.find( jointName );
                if ( it == jointNameToOzzIndex.end( ) )
                {
                    continue;
                }

                int                                                jointIndex = it->second;
                ozz::animation::offline::RawAnimation::JointTrack &track      = rawAnimation.tracks[ jointIndex ];

                cgltf_animation_sampler *sampler       = channel->sampler;
                cgltf_accessor          *input         = sampler->input;
                cgltf_accessor          *output        = sampler->output;
                size_t                   numKeyframes  = input->count;
                size_t                   numComponents = cgltf_num_components( output->type );

                times.resize( numKeyframes );
                cgltf_accessor_unpack_floats( input, times.data( ), numKeyframes );

                values.resize( numKeyframes * numComponents );
                cgltf_accessor_unpack_floats( output, values.data( ), numKeyframes * numComponents );

                if ( channel->target_path == cgltf_animation_path_type_translation )
                {
                    track.translations.reserve( numKeyframes );
                    for ( size_t k = 0; k < numKeyframes; ++k )
                    {
                        ozz::animation::offline::RawAnimation::TranslationKey transKey;
                        transKey.time  = times[ k ] / rawAnimation.duration;
                        transKey.value = ozz::math::Float3( values[ k * 3 ], values[ k * 3 + 1 ], values[ k * 3 + 2 ] );
                        track.translations.push_back( transKey );
                    }
                }
                else if ( channel->target_path == cgltf_animation_path_type_rotation )
                {
                    track.rotations.reserve( numKeyframes );
                    for ( size_t k = 0; k < numKeyframes; ++k )
                    {
                        ozz::animation::offline::RawAnimation::RotationKey rotKey;
                        rotKey.time  = times[ k ] / rawAnimation.duration;
                        rotKey.value = ozz::math::Quaternion( values[ k * 4 ], values[ k * 4 + 1 ], values[ k * 4 + 2 ], values[ k * 4 + 3 ] );
                        track.rotations.push_back( rotKey );
                    }
                }
                else if ( channel->target_path == cgltf_animation_path_type_scale )
                {
                    track.scales.reserve( numKeyframes );
                    for ( size_t k = 0; k < numKeyframes; ++k )
                    {
                        ozz::animation::offline::RawAnimation::ScaleKey scaleKey;
                        scaleKey.time  = times[ k ] / rawAnimation.duration;
                        scaleKey.value = ozz::math::Float3( values[ k * 3 ], values[ k * 3 + 1 ], values[ k * 3 + 2 ] );
                        track.scales.push_back( scaleKey );
                    }
                }
            }

            ozz::animation::offline::AnimationBuilder builder;
            auto                                      builtAnim = builder( rawAnimation );

            if ( !builtAnim )
            {
                m_errorMessage = "Failed to build ozz animation";
                return false;
            }

            ozz::io::File file( outputPath.c_str( ), "wb" );
            if ( !file.opened( ) )
            {
                m_errorMessage = "Failed to open file for writing: " + outputPath;
                return false;
            }

            ozz::io::OArchive archive( &file );
            archive << *builtAnim;

            spdlog::info( "OzzExporter: Wrote animation to {}", outputPath );
            return true;
        }
    };
} // namespace DenOfIz

extern "C"
{

    DenOfIz_OzzExporter DenOfIz_OzzExporter_Create( )
    {
        auto *impl = new DenOfIz::OzzExporterImpl( );
        return DENOFIZ_TO_HANDLE( impl );
    }

    void DenOfIz_OzzExporter_Destroy( DenOfIz_OzzExporter exporter )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( exporter ) )
        {
            return;
        }
        delete OZZ_EXPORTER_IMPL( exporter );
    }

    bool DenOfIz_OzzExporter_ValidateGltf( DenOfIz_OzzExporter exporter, DenOfIz_StringView filePath )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( exporter ) )
        {
            return false;
        }
        return DenOfIz::FileIO::FileExists( filePath );
    }

    bool DenOfIz_OzzExporter_ValidateGltfFromMemory( DenOfIz_OzzExporter exporter, DenOfIz_ByteArrayView gltfData )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( exporter ) || gltfData.Elements == nullptr || gltfData.NumElements == 0 )
        {
            return false;
        }

        cgltf_data   *data    = nullptr;
        cgltf_options options = { };
        cgltf_result  result  = cgltf_parse( &options, gltfData.Elements, gltfData.NumElements, &data );
        if ( result != cgltf_result_success )
        {
            return false;
        }
        cgltf_free( data );
        return true;
    }

    DenOfIz_OzzExportResult DenOfIz_OzzExporter_Export( DenOfIz_OzzExporter exporter, const DenOfIz_OzzExportDesc *desc )
    {
        DenOfIz_OzzExportResult result = { };
        result.ResultCode              = DENOFIZ_OZZ_EXPORT_SUCCESS;

        if ( !DENOFIZ_HANDLE_IS_VALID( exporter ) || desc == NULL )
        {
            result.ResultCode = DENOFIZ_OZZ_EXPORT_INVALID_PARAMETERS;
            return result;
        }

        DenOfIz::OzzExporterImpl *impl = OZZ_EXPORTER_IMPL( exporter );
        impl->Reset( );

        std::string outputDir( desc->OutputDirectory.Chars, desc->OutputDirectory.NumChars );
        std::string prefix( desc->AssetNamePrefix.Chars, desc->AssetNamePrefix.NumChars );

        bool fromMemory = desc->GltfSourceData.Elements != nullptr && desc->GltfSourceData.NumElements > 0;

        if ( fromMemory )
        {
            std::string basePath = desc->GltfSourceBasePath.Chars ? std::string( desc->GltfSourceBasePath.Chars, desc->GltfSourceBasePath.NumChars ) : "";
            if ( !impl->ParseGltfFromMemory( desc->GltfSourceData.Elements, desc->GltfSourceData.NumElements, basePath ) )
            {
                result.ResultCode            = DENOFIZ_OZZ_EXPORT_PARSE_ERROR;
                char *errMsg                 = strdup( impl->m_errorMessage.c_str( ) );
                result.ErrorMessage.Chars    = errMsg;
                result.ErrorMessage.NumChars = impl->m_errorMessage.size( );
                return result;
            }
        }
        else
        {
            std::string sourcePathStr( desc->GltfSourcePath.Chars, desc->GltfSourcePath.NumChars );

            if ( !DenOfIz::FileIO::FileExists( desc->GltfSourcePath ) )
            {
                result.ResultCode            = DENOFIZ_OZZ_EXPORT_FILE_NOT_FOUND;
                impl->m_errorMessage         = "GLTF file not found: " + sourcePathStr;
                char *errMsg                 = strdup( impl->m_errorMessage.c_str( ) );
                result.ErrorMessage.Chars    = errMsg;
                result.ErrorMessage.NumChars = impl->m_errorMessage.size( );
                return result;
            }

            if ( !impl->ParseGltf( sourcePathStr ) )
            {
                result.ResultCode            = DENOFIZ_OZZ_EXPORT_PARSE_ERROR;
                char *errMsg                 = strdup( impl->m_errorMessage.c_str( ) );
                result.ErrorMessage.Chars    = errMsg;
                result.ErrorMessage.NumChars = impl->m_errorMessage.size( );
                return result;
            }
        }

        impl->m_sanitizeTransforms = desc->SanitizeTransforms;

        bool hasReferenceFromFile   = desc->ReferenceGltfPath.Chars != nullptr && desc->ReferenceGltfPath.NumChars > 0;
        bool hasReferenceFromMemory = desc->ReferenceGltfData.Elements != nullptr && desc->ReferenceGltfData.NumElements > 0;

        if ( hasReferenceFromFile )
        {
            std::string refPath( desc->ReferenceGltfPath.Chars, desc->ReferenceGltfPath.NumChars );
            if ( !impl->LoadReferenceTransforms( refPath ) )
            {
                spdlog::warn( "OzzExporter: Failed to load reference GLTF: {}", impl->m_errorMessage );
            }
        }
        else if ( hasReferenceFromMemory )
        {
            std::string refBasePath = desc->ReferenceGltfBasePath.Chars
                                          ? std::string( desc->ReferenceGltfBasePath.Chars, desc->ReferenceGltfBasePath.NumChars )
                                          : "";
            if ( !impl->LoadReferenceTransformsFromMemory( desc->ReferenceGltfData.Elements, desc->ReferenceGltfData.NumElements, refBasePath ) )
            {
                spdlog::warn( "OzzExporter: Failed to load reference GLTF from memory: {}", impl->m_errorMessage );
            }
        }

        bool hasExternalSkeleton = DENOFIZ_HANDLE_IS_VALID( desc->ExternalSkeleton );

        if ( hasExternalSkeleton )
        {
            DenOfIz::OzzSkeletonImpl *animImpl = OZZ_SKELETON_IMPL( desc->ExternalSkeleton );
            if ( !animImpl || !animImpl->skeleton )
            {
                result.ResultCode            = DENOFIZ_OZZ_EXPORT_NO_SKELETON;
                impl->m_errorMessage         = "External OzzSkeleton has no skeleton loaded";
                char *errMsg                 = strdup( impl->m_errorMessage.c_str( ) );
                result.ErrorMessage.Chars    = errMsg;
                result.ErrorMessage.NumChars = impl->m_errorMessage.size( );
                return result;
            }
            impl->m_externalSkeleton = animImpl->skeleton.get( );
        }
        else
        {
            impl->BuildSkeletonHierarchy( );

            if ( impl->m_jointNames.empty( ) )
            {
                result.ResultCode            = DENOFIZ_OZZ_EXPORT_NO_SKELETON;
                impl->m_errorMessage         = "No skeleton found in GLTF file";
                char *errMsg                 = strdup( impl->m_errorMessage.c_str( ) );
                result.ErrorMessage.Chars    = errMsg;
                result.ErrorMessage.NumChars = impl->m_errorMessage.size( );
                return result;
            }
        }

        std::string resolvedOutputDir = DenOfIz::FileIO::GetResourcePath( desc->OutputDirectory );
        if ( !DenOfIz::FileIO::FileExists( desc->OutputDirectory ) )
        {
            DenOfIz::FileIO::CreateDirectories( desc->OutputDirectory );
        }
        resolvedOutputDir = DenOfIz::FileIO::GetAbsolutePath( desc->OutputDirectory );

        std::string baseName;
        if ( !prefix.empty( ) )
        {
            baseName = prefix;
        }
        else if ( !fromMemory && desc->GltfSourcePath.Chars )
        {
            baseName = std::filesystem::path( std::string( desc->GltfSourcePath.Chars, desc->GltfSourcePath.NumChars ) ).stem( ).string( );
        }
        else
        {
            baseName = "export";
        }

        if ( desc->ExportSkeleton && !hasExternalSkeleton )
        {
            std::string skelPath = ( std::filesystem::path( resolvedOutputDir ) / ( baseName + ".ozzskel" ) ).string( );
            if ( impl->WriteOzzSkeleton( skelPath ) )
            {
                impl->m_resultSkeletonPath = skelPath;
            }
            else
            {
                result.ResultCode            = DENOFIZ_OZZ_EXPORT_WRITE_FAILED;
                char *errMsg                 = strdup( impl->m_errorMessage.c_str( ) );
                result.ErrorMessage.Chars    = errMsg;
                result.ErrorMessage.NumChars = impl->m_errorMessage.size( );
                return result;
            }
        }

        if ( desc->ExportAnimations && impl->m_gltfData->animations_count > 0 )
        {
            for ( uint32_t i = 0; i < impl->m_gltfData->animations_count; ++i )
            {
                cgltf_animation *anim     = &impl->m_gltfData->animations[ i ];
                std::string      animName = anim->name ? anim->name : "anim_" + std::to_string( i );
                std::string      animPath = ( std::filesystem::path( resolvedOutputDir ) / ( animName + ".ozzanim" ) ).string( );

                if ( impl->WriteOzzAnimation( animPath, i ) )
                {
                    impl->m_resultAnimationPaths.push_back( animPath );
                }
            }
        }

        if ( !impl->m_resultSkeletonPath.empty( ) )
        {
            char *skelPathCopy               = strdup( impl->m_resultSkeletonPath.c_str( ) );
            result.SkeletonFilePath.Chars    = skelPathCopy;
            result.SkeletonFilePath.NumChars = impl->m_resultSkeletonPath.size( );
        }

        if ( !impl->m_resultAnimationPaths.empty( ) )
        {
            result.AnimationFilePaths.NumElements = impl->m_resultAnimationPaths.size( );
            DenOfIz_StringView *animPaths         = static_cast<DenOfIz_StringView *>( malloc( sizeof( DenOfIz_StringView ) * result.AnimationFilePaths.NumElements ) );
            for ( size_t i = 0; i < result.AnimationFilePaths.NumElements; ++i )
            {
                char *animPathCopy      = strdup( impl->m_resultAnimationPaths[ i ].c_str( ) );
                animPaths[ i ].Chars    = animPathCopy;
                animPaths[ i ].NumChars = impl->m_resultAnimationPaths[ i ].size( );
            }
            result.AnimationFilePaths.Elements = animPaths;
        }
        return result;
    }

    void DenOfIz_OzzExportResult_Destroy( DenOfIz_OzzExportResult *result )
    {
        if ( result == NULL )
        {
            return;
        }
        if ( result->ErrorMessage.Chars != NULL )
        {
            free( const_cast<char *>( result->ErrorMessage.Chars ) );
            result->ErrorMessage.Chars    = NULL;
            result->ErrorMessage.NumChars = 0;
        }
        if ( result->SkeletonFilePath.Chars != NULL )
        {
            free( const_cast<char *>( result->SkeletonFilePath.Chars ) );
            result->SkeletonFilePath.Chars    = NULL;
            result->SkeletonFilePath.NumChars = 0;
        }
        if ( result->AnimationFilePaths.Elements != NULL )
        {
            for ( size_t i = 0; i < result->AnimationFilePaths.NumElements; ++i )
            {
                if ( result->AnimationFilePaths.Elements[ i ].Chars != NULL )
                {
                    free( const_cast<char *>( result->AnimationFilePaths.Elements[ i ].Chars ) );
                }
            }
            free( result->AnimationFilePaths.Elements );
            result->AnimationFilePaths.Elements    = NULL;
            result->AnimationFilePaths.NumElements = 0;
        }
    }
}
