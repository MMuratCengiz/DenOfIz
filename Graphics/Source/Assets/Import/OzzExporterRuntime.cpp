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

#include "DenOfIzGraphics/Assets/Import/OzzExporterRuntime.h"
#include "DenOfIzGraphicsInternal/Animation/OzzImpl.h"
#include "DenOfIzGraphicsInternal/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphicsInternal/Assets/Stream/BinaryContainerImpl.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

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
#include <string>
#include <unordered_map>
#include <vector>

#define OZZ_RUNTIME_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::OzzExporterRuntimeImpl, handle )
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

    class OzzExporterRuntimeImpl
    {
    public:
        cgltf_data                               *m_gltfData = nullptr;
        ozz::unique_ptr<ozz::animation::Skeleton> m_ozzSkeleton;
        std::string                               m_errorMessage;

        std::unordered_map<std::string, int32_t> m_jointNameToIndex;
        std::vector<std::string>                 m_jointNames;
        std::vector<int32_t>                     m_jointParents;

        std::vector<std::string>         m_animationNames;
        std::vector<DenOfIz_StringView>  m_animationNamesView;

        std::unordered_map<std::string, ReferenceTransform> m_referenceTransforms;
        bool                                                m_sanitizeTransforms = false;

        ~OzzExporterRuntimeImpl( )
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
            m_errorMessage.clear( );
            m_jointNameToIndex.clear( );
            m_jointNames.clear( );
            m_jointParents.clear( );
            m_animationNames.clear( );
            m_animationNamesView.clear( );
            m_referenceTransforms.clear( );
            m_sanitizeTransforms = false;
        }

        bool ParseGltf( const std::string &filePath )
        {
            Reset( );

            std::string resolvedPath = FileIO::GetResourcePath( DenOfIz_StringView{ filePath.c_str( ), filePath.size( ) } );

            cgltf_options options = { };
            cgltf_result  result  = cgltf_parse_file( &options, resolvedPath.c_str( ), &m_gltfData );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to parse GLTF file: " + resolvedPath;
                spdlog::error( "{}", m_errorMessage );
                return false;
            }

            result = cgltf_load_buffers( &options, m_gltfData, resolvedPath.c_str( ) );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to load GLTF buffers";
                spdlog::error( "{}", m_errorMessage );
                cgltf_free( m_gltfData );
                m_gltfData = nullptr;
                return false;
            }

            BuildSkeletonHierarchy( );
            CacheAnimationNames( );
            return true;
        }

        bool ParseGltfFromMemory( const void *data, size_t size, const std::string &basePath )
        {
            Reset( );

            cgltf_options options = { };
            cgltf_result  result  = cgltf_parse( &options, data, size, &m_gltfData );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to parse GLTF data from memory";
                spdlog::error( "{}", m_errorMessage );
                return false;
            }

            std::string resolvedBasePath = basePath.empty( ) ? "." : basePath;
            result                       = cgltf_load_buffers( &options, m_gltfData, resolvedBasePath.c_str( ) );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to load GLTF buffers from memory";
                spdlog::error( "{}", m_errorMessage );
                cgltf_free( m_gltfData );
                m_gltfData = nullptr;
                return false;
            }

            BuildSkeletonHierarchy( );
            CacheAnimationNames( );
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

        void CacheAnimationNames( )
        {
            m_animationNames.clear( );
            m_animationNamesView.clear( );

            if ( !m_gltfData )
            {
                return;
            }

            for ( size_t i = 0; i < m_gltfData->animations_count; ++i )
            {
                cgltf_animation *anim     = &m_gltfData->animations[ i ];
                std::string      animName = anim->name ? anim->name : "anim_" + std::to_string( i );
                m_animationNames.push_back( animName );
            }

            m_animationNamesView.resize( m_animationNames.size( ) );
            for ( size_t i = 0; i < m_animationNames.size( ); ++i )
            {
                m_animationNamesView[ i ].Chars    = m_animationNames[ i ].c_str( );
                m_animationNamesView[ i ].NumChars = m_animationNames[ i ].size( );
            }
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

            spdlog::info( "OzzExporterRuntime: Loaded {} reference transforms", m_referenceTransforms.size( ) );
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
                spdlog::error( "{}", m_errorMessage );
                return false;
            }

            result = cgltf_load_buffers( &options, refData, resolvedPath.c_str( ) );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to load reference GLTF buffers";
                spdlog::error( "{}", m_errorMessage );
                cgltf_free( refData );
                return false;
            }

            BuildReferenceTransformMap( refData );
            cgltf_free( refData );
            m_ozzSkeleton.reset( );
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
                spdlog::error( "{}", m_errorMessage );
                return false;
            }

            std::string resolvedBasePath = basePath.empty( ) ? "." : basePath;
            result                       = cgltf_load_buffers( &options, refData, resolvedBasePath.c_str( ) );
            if ( result != cgltf_result_success )
            {
                m_errorMessage = "Failed to load reference GLTF buffers from memory";
                spdlog::error( "{}", m_errorMessage );
                cgltf_free( refData );
                return false;
            }

            BuildReferenceTransformMap( refData );
            cgltf_free( refData );
            m_ozzSkeleton.reset( );
            return true;
        }

        bool BuildOzzSkeleton( )
        {
            if ( m_jointNames.empty( ) )
            {
                m_errorMessage = "No skeleton data available";
                return false;
            }

            if ( m_ozzSkeleton )
            {
                return true;
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
                            spdlog::warn( "OzzExporterRuntime: Joint '{}' not found in reference GLTF, using source transform", m_jointNames[ i ] );
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

            spdlog::info( "OzzExporterRuntime: Built skeleton with {} joints", m_ozzSkeleton->num_joints( ) );
            return true;
        }

        DenOfIz_BinaryContainer SerializeSkeletonToMemory( )
        {
            if ( !BuildOzzSkeleton( ) )
            {
                return DENOFIZ_NULL_HANDLE;
            }

            auto *container = new BinaryContainerImpl( );

            ozz::io::MemoryStream stream;
            ozz::io::OArchive     archive( &stream );
            archive << *m_ozzSkeleton;

            size_t dataSize = stream.Size( );
            stream.Seek( 0, ozz::io::Stream::kSet );
            std::vector<char> buffer( dataSize );
            stream.Read( buffer.data( ), dataSize );
            container->Stream( )->write( buffer.data( ), static_cast<std::streamsize>( dataSize ) );

            return DENOFIZ_TO_HANDLE( container );
        }

        DenOfIz_BinaryContainer SerializeAnimationToMemory( uint32_t animIndex )
        {
            if ( !m_gltfData || animIndex >= m_gltfData->animations_count )
            {
                m_errorMessage = "Animation index out of range";
                spdlog::error( "{}", m_errorMessage );
                return DENOFIZ_NULL_HANDLE;
            }

            if ( !BuildOzzSkeleton( ) )
            {
                m_errorMessage = "Failed to build skeleton for animation";
                spdlog::error( "{}", m_errorMessage );
                return DENOFIZ_NULL_HANDLE;
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
            rawAnimation.tracks.resize( m_ozzSkeleton->num_joints( ) );

            std::unordered_map<std::string, int> jointNameToOzzIndex;
            for ( int i = 0; i < m_ozzSkeleton->num_joints( ); ++i )
            {
                jointNameToOzzIndex[ m_ozzSkeleton->joint_names( )[ i ] ] = i;
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
                spdlog::error( "{}", m_errorMessage );
                return DENOFIZ_NULL_HANDLE;
            }

            auto *container = new BinaryContainerImpl( );

            ozz::io::MemoryStream stream;
            ozz::io::OArchive     archive( &stream );
            archive << *builtAnim;

            size_t dataSize = stream.Size( );
            stream.Seek( 0, ozz::io::Stream::kSet );
            std::vector<char> buffer( dataSize );
            stream.Read( buffer.data( ), dataSize );
            container->Stream( )->write( buffer.data( ), static_cast<std::streamsize>( dataSize ) );

            spdlog::info( "OzzExporterRuntime: Built animation '{}' with {} tracks, duration: {}s",
                          m_animationNames[ animIndex ], builtAnim->num_tracks( ), builtAnim->duration( ) );

            return DENOFIZ_TO_HANDLE( container );
        }

        int32_t FindAnimationIndexByName( const std::string &name ) const
        {
            for ( size_t i = 0; i < m_animationNames.size( ); ++i )
            {
                if ( m_animationNames[ i ] == name )
                {
                    return static_cast<int32_t>( i );
                }
            }
            return -1;
        }

        DenOfIz_BinaryContainer SerializeAnimationForSkeleton( ozz::animation::Skeleton *skeleton, uint32_t animIndex )
        {
            if ( !m_gltfData || animIndex >= m_gltfData->animations_count )
            {
                m_errorMessage = "Animation index out of range";
                spdlog::error( "{}", m_errorMessage );
                return DENOFIZ_NULL_HANDLE;
            }

            if ( !skeleton )
            {
                m_errorMessage = "No skeleton provided for animation export";
                spdlog::error( "{}", m_errorMessage );
                return DENOFIZ_NULL_HANDLE;
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
                spdlog::error( "{}", m_errorMessage );
                return DENOFIZ_NULL_HANDLE;
            }

            auto *container = new BinaryContainerImpl( );

            ozz::io::MemoryStream stream;
            ozz::io::OArchive     archive( &stream );
            archive << *builtAnim;

            size_t dataSize = stream.Size( );
            stream.Seek( 0, ozz::io::Stream::kSet );
            std::vector<char> buffer( dataSize );
            stream.Read( buffer.data( ), dataSize );
            container->Stream( )->write( buffer.data( ), static_cast<std::streamsize>( dataSize ) );

            spdlog::info( "OzzExporterRuntime: Built animation '{}' for external skeleton with {} tracks, duration: {}s",
                          m_animationNames[ animIndex ], builtAnim->num_tracks( ), builtAnim->duration( ) );

            return DENOFIZ_TO_HANDLE( container );
        }
    };
} // namespace DenOfIz

extern "C"
{

    DenOfIz_OzzExporterRuntime DenOfIz_OzzExporterRuntime_Create( )
    {
        auto *impl = new DenOfIz::OzzExporterRuntimeImpl( );
        return DENOFIZ_TO_HANDLE( impl );
    }

    void DenOfIz_OzzExporterRuntime_Destroy( DenOfIz_OzzExporterRuntime runtime )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( runtime ) )
        {
            return;
        }
        delete OZZ_RUNTIME_IMPL( runtime );
    }

    bool DenOfIz_OzzExporterRuntime_Load( DenOfIz_OzzExporterRuntime runtime, DenOfIz_StringView gltfFilePath )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( runtime ) )
        {
            return false;
        }

        DenOfIz::OzzExporterRuntimeImpl *impl = OZZ_RUNTIME_IMPL( runtime );
        std::string                      filePath( gltfFilePath.Chars, gltfFilePath.NumChars );
        return impl->ParseGltf( filePath );
    }

    bool DenOfIz_OzzExporterRuntime_LoadFromMemory( DenOfIz_OzzExporterRuntime runtime, DenOfIz_ByteArrayView gltfBytes, DenOfIz_StringView basePath )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( runtime ) || gltfBytes.Elements == nullptr || gltfBytes.NumElements == 0 )
        {
            return false;
        }

        DenOfIz::OzzExporterRuntimeImpl *impl         = OZZ_RUNTIME_IMPL( runtime );
        std::string                      basePathStr  = basePath.Chars ? std::string( basePath.Chars, basePath.NumChars ) : "";
        return impl->ParseGltfFromMemory( gltfBytes.Elements, gltfBytes.NumElements, basePathStr );
    }

    bool DenOfIz_OzzExporterRuntime_LoadReference( DenOfIz_OzzExporterRuntime runtime, DenOfIz_StringView referenceGltfPath )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( runtime ) || referenceGltfPath.Chars == nullptr || referenceGltfPath.NumChars == 0 )
        {
            return false;
        }

        DenOfIz::OzzExporterRuntimeImpl *impl = OZZ_RUNTIME_IMPL( runtime );
        std::string                      filePath( referenceGltfPath.Chars, referenceGltfPath.NumChars );
        return impl->LoadReferenceTransforms( filePath );
    }

    bool DenOfIz_OzzExporterRuntime_LoadReferenceFromMemory( DenOfIz_OzzExporterRuntime runtime, DenOfIz_ByteArrayView gltfBytes, DenOfIz_StringView basePath )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( runtime ) || gltfBytes.Elements == nullptr || gltfBytes.NumElements == 0 )
        {
            return false;
        }

        DenOfIz::OzzExporterRuntimeImpl *impl        = OZZ_RUNTIME_IMPL( runtime );
        std::string                      basePathStr = basePath.Chars ? std::string( basePath.Chars, basePath.NumChars ) : "";
        return impl->LoadReferenceTransformsFromMemory( gltfBytes.Elements, gltfBytes.NumElements, basePathStr );
    }

    void DenOfIz_OzzExporterRuntime_SetSanitizeTransforms( DenOfIz_OzzExporterRuntime runtime, bool sanitize )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( runtime ) )
        {
            return;
        }

        DenOfIz::OzzExporterRuntimeImpl *impl = OZZ_RUNTIME_IMPL( runtime );
        if ( impl->m_sanitizeTransforms != sanitize )
        {
            impl->m_sanitizeTransforms = sanitize;
            impl->m_ozzSkeleton.reset( );
        }
    }

    DenOfIz_OzzGltfDesc DenOfIz_OzzExporterRuntime_GetDesc( DenOfIz_OzzExporterRuntime runtime )
    {
        DenOfIz_OzzGltfDesc info = { };

        if ( !DENOFIZ_HANDLE_IS_VALID( runtime ) )
        {
            return info;
        }

        DenOfIz::OzzExporterRuntimeImpl *impl = OZZ_RUNTIME_IMPL( runtime );

        info.HasSkeleton   = !impl->m_jointNames.empty( );
        info.NumAnimations = static_cast<uint32_t>( impl->m_animationNames.size( ) );

        if ( !impl->m_animationNamesView.empty( ) )
        {
            info.AnimationNames.Elements    = impl->m_animationNamesView.data( );
            info.AnimationNames.NumElements = impl->m_animationNamesView.size( );
        }

        return info;
    }

    DenOfIz_BinaryContainer DenOfIz_OzzExporterRuntime_BuildSkeleton( DenOfIz_OzzExporterRuntime runtime )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( runtime ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        DenOfIz::OzzExporterRuntimeImpl *impl = OZZ_RUNTIME_IMPL( runtime );
        return impl->SerializeSkeletonToMemory( );
    }

    DenOfIz_BinaryContainer DenOfIz_OzzExporterRuntime_BuildAnimation( DenOfIz_OzzExporterRuntime runtime, DenOfIz_OzzSkeleton ozzAnimation, uint32_t animationIndex )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( runtime ) || !DENOFIZ_HANDLE_IS_VALID( ozzAnimation ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        DenOfIz::OzzExporterRuntimeImpl *impl     = OZZ_RUNTIME_IMPL( runtime );
        DenOfIz::OzzSkeletonImpl       *animImpl = OZZ_SKELETON_IMPL( ozzAnimation );

        if ( !animImpl->skeleton )
        {
            spdlog::error( "OzzExporterRuntime: OzzSkeleton has no skeleton loaded" );
            return DENOFIZ_NULL_HANDLE;
        }

        return impl->SerializeAnimationForSkeleton( animImpl->skeleton.get( ), animationIndex );
    }

    DenOfIz_BinaryContainer DenOfIz_OzzExporterRuntime_BuildAnimationByName( DenOfIz_OzzExporterRuntime runtime, DenOfIz_OzzSkeleton ozzAnimation, DenOfIz_StringView animationName )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( runtime ) || !DENOFIZ_HANDLE_IS_VALID( ozzAnimation ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        DenOfIz::OzzExporterRuntimeImpl *impl     = OZZ_RUNTIME_IMPL( runtime );
        DenOfIz::OzzSkeletonImpl       *animImpl = OZZ_SKELETON_IMPL( ozzAnimation );
        std::string                      name( animationName.Chars, animationName.NumChars );

        int32_t index = impl->FindAnimationIndexByName( name );
        if ( index < 0 )
        {
            spdlog::error( "OzzExporterRuntime: Animation '{}' not found", name );
            return DENOFIZ_NULL_HANDLE;
        }

        if ( !animImpl->skeleton )
        {
            spdlog::error( "OzzExporterRuntime: OzzSkeleton has no skeleton loaded" );
            return DENOFIZ_NULL_HANDLE;
        }

        return impl->SerializeAnimationForSkeleton( animImpl->skeleton.get( ), static_cast<uint32_t>( index ) );
    }
}
