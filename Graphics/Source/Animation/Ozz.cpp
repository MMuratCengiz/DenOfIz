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

#include "DenOfIzGraphicsInternal/Animation/OzzImpl.h"

#include <cstddef>

#include <ozz/animation/offline/raw_track.h>
#include <ozz/animation/offline/track_builder.h>
#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/ik_aim_job.h>
#include <ozz/animation/runtime/ik_two_bone_job.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/track_sampling_job.h>
#include <ozz/animation/runtime/track_triggering_job.h>
#include <ozz/base/io/archive.h>
#include <ozz/base/io/stream.h>
#include <ozz/base/maths/simd_quaternion.h>
#include <ozz/base/maths/vec_float.h>
#include <ozz/base/span.h>
#include <ozz/geometry/runtime/skinning_job.h>

#include <algorithm>
#include <ranges>

#include "DenOfIzGraphicsInternal/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

// Include last due to DirectXMath
#include "DenOfIzGraphicsInternal/Utilities/InteropMathConverter.h"

namespace DenOfIz
{
    OzzSkeletonImpl::OzzSkeletonImpl( DenOfIz_StringView skeletonFilePath )
    {
        LoadSkeleton( skeletonFilePath );
    }

    OzzSkeletonImpl::OzzSkeletonImpl( DenOfIz_BinaryContainer skeletonData )
    {
        LoadSkeletonFromBinaryContainer( skeletonData );
    }

    OzzSkeletonImpl::~OzzSkeletonImpl( )
    {
        for ( const auto context : contexts )
        {
            delete context;
        }
        contexts.clear( );
    }

    void OzzSkeletonImpl::LoadSkeleton( DenOfIz_StringView skeletonFilePath )
    {
        std::string filePath( skeletonFilePath.Chars, skeletonFilePath.NumChars );
        std::string resolvedPath = FileIO::GetResourcePath( skeletonFilePath );

        ozz::io::File file( resolvedPath.c_str( ), "rb" );
        if ( !file.opened( ) )
        {
            spdlog::error( "Failed to open skeleton file: {}", resolvedPath );
            return;
        }

        ozz::io::IArchive archive( &file );
        if ( !archive.TestTag<ozz::animation::Skeleton>( ) )
        {
            spdlog::error( "Invalid skeleton file format: {}", resolvedPath );
            return;
        }

        skeleton = ozz::make_unique<ozz::animation::Skeleton>( );
        archive >> *skeleton;

        if ( skeleton->num_joints( ) == 0 )
        {
            spdlog::error( "Skeleton has no joints: {}", resolvedPath );
            skeleton.reset( );
            return;
        }

        m_valid = true;
        spdlog::info( "Loaded ozz skeleton with {} joints from: {}", skeleton->num_joints( ), resolvedPath );
    }

    void OzzSkeletonImpl::LoadSkeletonFromBinaryContainer( DenOfIz_BinaryContainer skeletonData )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeletonData ) )
        {
            spdlog::error( "Invalid skeleton binary container" );
            return;
        }

        DenOfIz_ByteArrayView dataView = DenOfIz_BinaryContainer_GetData( skeletonData );
        if ( dataView.Elements == nullptr || dataView.NumElements == 0 )
        {
            spdlog::error( "Empty skeleton binary container" );
            return;
        }

        ozz::io::MemoryStream stream;
        stream.Write( dataView.Elements, dataView.NumElements );
        stream.Seek( 0, ozz::io::Stream::kSet );

        ozz::io::IArchive archive( &stream );
        if ( !archive.TestTag<ozz::animation::Skeleton>( ) )
        {
            spdlog::error( "Invalid skeleton binary container format" );
            return;
        }

        skeleton = ozz::make_unique<ozz::animation::Skeleton>( );
        archive >> *skeleton;

        if ( skeleton->num_joints( ) == 0 )
        {
            spdlog::error( "Skeleton binary container has no joints" );
            skeleton.reset( );
            return;
        }

        m_valid = true;
        spdlog::info( "Loaded ozz skeleton with {} joints from binary container", skeleton->num_joints( ) );
    }

    bool OzzSkeletonImpl::LoadAnimation( DenOfIz_StringView animationFilePath, InternalContext *context ) const
    {
        if ( !skeleton )
        {
            spdlog::error( "Cannot load animation: skeleton not loaded" );
            return false;
        }

        std::string filePath( animationFilePath.Chars, animationFilePath.NumChars );
        std::string resolvedPath = FileIO::GetResourcePath( animationFilePath );

        ozz::io::File file( resolvedPath.c_str( ), "rb" );
        if ( !file.opened( ) )
        {
            spdlog::error( "Failed to open animation file: {}", resolvedPath );
            return false;
        }

        ozz::io::IArchive archive( &file );
        if ( !archive.TestTag<ozz::animation::Animation>( ) )
        {
            spdlog::error( "Invalid animation file format: {}", resolvedPath );
            return false;
        }

        context->animation = ozz::make_unique<ozz::animation::Animation>( );
        archive >> *context->animation;

        if ( context->animation->num_tracks( ) == 0 )
        {
            spdlog::error( "Animation has no tracks: {}", resolvedPath );
            context->animation.reset( );
            return false;
        }

        context->animationName = context->animation->name( );

        spdlog::info( "Loaded ozz animation with {} tracks, duration: {}s from: {}", context->animation->num_tracks( ), context->animation->duration( ), resolvedPath );
        return true;
    }

    bool OzzSkeletonImpl::LoadAnimationFromBinaryContainer( DenOfIz_BinaryContainer animationData, InternalContext *context ) const
    {
        if ( !skeleton )
        {
            spdlog::error( "Cannot load animation: skeleton not loaded" );
            return false;
        }

        if ( !DENOFIZ_HANDLE_IS_VALID( animationData ) )
        {
            spdlog::error( "Invalid animation binary container" );
            return false;
        }

        DenOfIz_ByteArrayView dataView = DenOfIz_BinaryContainer_GetData( animationData );
        if ( dataView.Elements == nullptr || dataView.NumElements == 0 )
        {
            spdlog::error( "Empty animation binary container" );
            return false;
        }

        ozz::io::MemoryStream stream;
        stream.Write( dataView.Elements, dataView.NumElements );
        stream.Seek( 0, ozz::io::Stream::kSet );

        ozz::io::IArchive archive( &stream );
        if ( !archive.TestTag<ozz::animation::Animation>( ) )
        {
            spdlog::error( "Invalid animation binary container format" );
            return false;
        }

        context->animation = ozz::make_unique<ozz::animation::Animation>( );
        archive >> *context->animation;

        if ( context->animation->num_tracks( ) == 0 )
        {
            spdlog::error( "Animation binary container has no tracks" );
            context->animation.reset( );
            return false;
        }

        context->animationName = context->animation->name( );

        spdlog::info( "Loaded ozz animation with {} tracks, duration: {}s from binary container", context->animation->num_tracks( ), context->animation->duration( ) );
        return true;
    }

    static ozz::math::SimdFloat4 ToOzzSimdFloat4( const DenOfIz_Float3 &v )
    {
        return ozz::math::simd_float4::Load3PtrU( &v.X );
    }

    static DenOfIz_Float4 FromOzzSimdQuaternion( const ozz::math::SimdQuaternion &q )
    {
        DenOfIz_Float4 result;
        float          values[ 4 ];
        ozz::math::StorePtrU( q.xyzw, values );
        result.X = values[ 0 ];
        result.Y = values[ 1 ];
        result.Z = values[ 2 ];
        result.W = values[ 3 ];
        return result;
    }

    static ozz::math::Float4x4 ToOzzFloat4x4( const DenOfIz_Float4x4 &m )
    {
        ozz::math::Float4x4 result;
        float               col0[ 4 ] = { m._11, m._12, -m._13, m._14 };
        float               col1[ 4 ] = { m._21, m._22, -m._23, m._24 };
        float               col2[ 4 ] = { -m._31, -m._32, m._33, -m._34 };
        float               col3[ 4 ] = { m._41, m._42, -m._43, m._44 };
        result.cols[ 0 ]              = ozz::math::simd_float4::LoadPtrU( col0 );
        result.cols[ 1 ]              = ozz::math::simd_float4::LoadPtrU( col1 );
        result.cols[ 2 ]              = ozz::math::simd_float4::LoadPtrU( col2 );
        result.cols[ 3 ]              = ozz::math::simd_float4::LoadPtrU( col3 );
        return result;
    }

    static DenOfIz_Float4x4 FromOzzFloat4x4( const ozz::math::Float4x4 &m )
    {
        DenOfIz_Float4x4 result;
        float            col0[ 4 ], col1[ 4 ], col2[ 4 ], col3[ 4 ];

        ozz::math::StorePtrU( m.cols[ 0 ], col0 );
        ozz::math::StorePtrU( m.cols[ 1 ], col1 );
        ozz::math::StorePtrU( m.cols[ 2 ], col2 );
        ozz::math::StorePtrU( m.cols[ 3 ], col3 );

        result._11 = col0[ 0 ];
        result._12 = col0[ 1 ];
        result._13 = -col0[ 2 ];
        result._14 = col0[ 3 ];
        result._21 = col1[ 0 ];
        result._22 = col1[ 1 ];
        result._23 = -col1[ 2 ];
        result._24 = col1[ 3 ];
        result._31 = -col2[ 0 ];
        result._32 = -col2[ 1 ];
        result._33 = col2[ 2 ];
        result._34 = -col2[ 3 ];
        result._41 = col3[ 0 ];
        result._42 = col3[ 1 ];
        result._43 = -col3[ 2 ];
        result._44 = col3[ 3 ];

        return result;
    }
} // namespace DenOfIz

extern "C"
{

    DenOfIz_OzzSkeleton DenOfIz_Ozz_CreateSkeleton( DenOfIz_StringView skeletonFilePath )
    {
        DenOfIz::OzzSkeletonImpl *impl = new DenOfIz::OzzSkeletonImpl( skeletonFilePath );
        return DENOFIZ_TO_HANDLE( impl );
    }

    DenOfIz_OzzSkeleton DenOfIz_Ozz_CreateSkeletonFromBinaryContainer( DenOfIz_BinaryContainer skeletonData )
    {
        DenOfIz::OzzSkeletonImpl *impl = new DenOfIz::OzzSkeletonImpl( skeletonData );
        return DENOFIZ_TO_HANDLE( impl );
    }

    void DenOfIz_Ozz_DestroySkeleton( DenOfIz_OzzSkeleton skeleton )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) )
        {
            return;
        }
        DenOfIz::OzzSkeletonImpl *impl = OZZ_SKELETON_IMPL( skeleton );
        delete impl;
    }

    bool DenOfIz_Ozz_IsSkeletonValid( DenOfIz_OzzSkeleton skeleton )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) )
        {
            return false;
        }
        DenOfIz::OzzSkeletonImpl *impl = OZZ_SKELETON_IMPL( skeleton );
        return impl->m_valid && impl->skeleton;
    }

    DenOfIz_OzzContext DenOfIz_Ozz_NewContext( DenOfIz_OzzSkeleton skeleton )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        DenOfIz::OzzSkeletonImpl *impl    = OZZ_SKELETON_IMPL( skeleton );
        DenOfIz::InternalContext *context = new DenOfIz::InternalContext( );

        if ( impl->skeleton )
        {
            context->samplingContext = ozz::make_unique<ozz::animation::SamplingJob::Context>( );
            context->samplingContext->Resize( impl->skeleton->num_joints( ) );
            context->localTransforms.resize( impl->skeleton->num_soa_joints( ) );
            context->modelTransforms.resize( impl->skeleton->num_joints( ) );
        }

        impl->contexts.push_back( context );
        return DENOFIZ_TO_HANDLE( context );
    }

    void DenOfIz_Ozz_DestroyContext( DenOfIz_OzzSkeleton skeleton, DenOfIz_OzzContext context )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) || !DENOFIZ_HANDLE_IS_VALID( context ) )
        {
            return;
        }

        DenOfIz::OzzSkeletonImpl *impl            = OZZ_SKELETON_IMPL( skeleton );
        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( context );
        auto                      it              = std::ranges::find( impl->contexts, internalContext );

        if ( it != impl->contexts.end( ) )
        {
            impl->contexts.erase( it );
            delete internalContext;
        }
    }

    bool DenOfIz_Ozz_LoadAnimation( DenOfIz_OzzSkeleton skeleton, DenOfIz_StringView animationFilePath, DenOfIz_OzzContext context )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) || !DENOFIZ_HANDLE_IS_VALID( context ) )
        {
            spdlog::error( "Invalid skeleton or context handle" );
            return false;
        }

        DenOfIz::OzzSkeletonImpl *impl            = OZZ_SKELETON_IMPL( skeleton );
        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( context );
        return impl->LoadAnimation( animationFilePath, internalContext );
    }

    bool DenOfIz_Ozz_LoadAnimationFromBinaryContainer( DenOfIz_OzzSkeleton skeleton, DenOfIz_BinaryContainer animationData, DenOfIz_OzzContext context )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) || !DENOFIZ_HANDLE_IS_VALID( context ) )
        {
            spdlog::error( "Invalid skeleton or context handle" );
            return false;
        }

        DenOfIz::OzzSkeletonImpl *impl            = OZZ_SKELETON_IMPL( skeleton );
        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( context );
        return impl->LoadAnimationFromBinaryContainer( animationData, internalContext );
    }

    void DenOfIz_Ozz_UnloadAnimation( DenOfIz_OzzContext context )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( context ) )
        {
            return;
        }

        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( context );
        internalContext->animation.reset( );
        internalContext->animationName.clear( );
    }

    void DenOfIz_Ozz_LoadTrackFloat( const DenOfIz_FloatArray *keys, const float duration, DenOfIz_OzzContext context )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( context ) || keys == nullptr || keys->NumElements == 0 )
        {
            spdlog::error( "Invalid context or empty keys" );
            return;
        }

        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( context );

        ozz::animation::offline::RawFloatTrack rawTrack;
        rawTrack.keyframes.resize( keys->NumElements );

        const float step = duration / ( keys->NumElements - 1 );
        for ( size_t i = 0; i < keys->NumElements; ++i )
        {
            auto &keyframe         = rawTrack.keyframes[ i ];
            keyframe.ratio         = i * step;
            keyframe.value         = keys->Elements[ i ];
            keyframe.interpolation = ozz::animation::offline::RawTrackInterpolation::kLinear;
        }

        constexpr ozz::animation::offline::TrackBuilder builder;
        if ( auto track = builder( rawTrack ) )
        {
            internalContext->floatTracks.push_back( std::move( track ) );
        }
    }

    void DenOfIz_Ozz_LoadTrackFloat2( const DenOfIz_Float2Array *keys, const DenOfIz_FloatArray *timestamps, DenOfIz_OzzContext context )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( context ) || keys == nullptr || timestamps == nullptr || keys->NumElements == 0 || timestamps->NumElements != keys->NumElements )
        {
            spdlog::error( "Invalid context, empty keys, or mismatched timestamps" );
            return;
        }

        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( context );

        ozz::animation::offline::RawFloat2Track rawTrack;
        rawTrack.keyframes.resize( keys->NumElements );

        for ( size_t i = 0; i < keys->NumElements; ++i )
        {
            const DenOfIz_Float2 &key      = keys->Elements[ i ];
            auto                 &keyframe = rawTrack.keyframes[ i ];
            keyframe.ratio                 = timestamps->Elements[ i ];
            keyframe.value                 = ozz::math::Float2( key.X, key.Y );
            keyframe.interpolation         = ozz::animation::offline::RawTrackInterpolation::kLinear;
        }

        constexpr ozz::animation::offline::TrackBuilder builder;
        if ( auto track = builder( rawTrack ) )
        {
            internalContext->float2Tracks.push_back( std::move( track ) );
        }
    }

    void DenOfIz_Ozz_LoadTrackFloat3( const DenOfIz_Float3Array *keys, const DenOfIz_FloatArray *timestamps, DenOfIz_OzzContext context )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( context ) || keys == nullptr || timestamps == nullptr || keys->NumElements == 0 || timestamps->NumElements != keys->NumElements )
        {
            spdlog::error( "Invalid context, empty keys, or mismatched timestamps" );
            return;
        }

        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( context );

        ozz::animation::offline::RawFloat3Track rawTrack;
        rawTrack.keyframes.resize( keys->NumElements );

        for ( size_t i = 0; i < keys->NumElements; ++i )
        {
            const DenOfIz_Float3 &key      = keys->Elements[ i ];
            auto                 &keyframe = rawTrack.keyframes[ i ];
            keyframe.ratio                 = timestamps->Elements[ i ];
            keyframe.value                 = ozz::math::Float3( key.X, key.Y, key.Z );
            keyframe.interpolation         = ozz::animation::offline::RawTrackInterpolation::kLinear;
        }

        constexpr ozz::animation::offline::TrackBuilder builder;
        if ( auto track = builder( rawTrack ) )
        {
            internalContext->float3Tracks.push_back( std::move( track ) );
        }
    }

    void DenOfIz_Ozz_LoadTrackFloat4( const DenOfIz_Float4Array *keys, const DenOfIz_FloatArray *timestamps, DenOfIz_OzzContext context )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( context ) || keys == nullptr || timestamps == nullptr || keys->NumElements == 0 || timestamps->NumElements != keys->NumElements )
        {
            spdlog::error( "Invalid context, empty keys, or mismatched timestamps" );
            return;
        }

        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( context );

        ozz::animation::offline::RawFloat4Track rawTrack;
        rawTrack.keyframes.resize( keys->NumElements );

        for ( size_t i = 0; i < keys->NumElements; ++i )
        {
            const DenOfIz_Float4 &key      = keys->Elements[ i ];
            auto                 &keyframe = rawTrack.keyframes[ i ];
            keyframe.ratio                 = timestamps->Elements[ i ];
            keyframe.value                 = ozz::math::Float4( key.X, key.Y, key.Z, key.W );
            keyframe.interpolation         = ozz::animation::offline::RawTrackInterpolation::kLinear;
        }

        constexpr ozz::animation::offline::TrackBuilder builder;
        if ( auto track = builder( rawTrack ) )
        {
            internalContext->float4Tracks.push_back( std::move( track ) );
        }
    }

    bool DenOfIz_Ozz_RunSamplingJob( DenOfIz_OzzSkeleton skeleton, const DenOfIz_SamplingJobDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) || desc == nullptr || !DENOFIZ_HANDLE_IS_VALID( desc->Context ) )
        {
            spdlog::error( "Invalid sampling job parameters" );
            return false;
        }

        DenOfIz::OzzSkeletonImpl *impl            = OZZ_SKELETON_IMPL( skeleton );
        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( desc->Context );

        if ( !internalContext->animation || !internalContext->samplingContext )
        {
            spdlog::error( "No animation loaded in context or sampling context not initialized" );
            return false;
        }

        if ( desc->OutTransforms.Elements == nullptr )
        {
            spdlog::error( "desc->OutTransforms.Elements is null" );
            return false;
        }

        if ( desc->OutTransforms.NumElements != (size_t)impl->skeleton->num_joints( ) )
        {
            spdlog::error( "desc->OutTransforms has incorrect number of elements, use GetNumJoints()" );
            return false;
        }

        const float ratio = ozz::math::Clamp( 0.f, desc->Ratio, 1.f );
        for ( size_t i = 0; i < internalContext->localTransforms.size( ); ++i )
        {
            internalContext->localTransforms[ i ] = impl->skeleton->joint_rest_poses( )[ i ];
        }

        ozz::animation::SamplingJob samplingJob;
        samplingJob.animation = internalContext->animation.get( );
        samplingJob.context   = internalContext->samplingContext.get( );
        samplingJob.ratio     = ratio;
        samplingJob.output    = ozz::make_span( internalContext->localTransforms );

        if ( !samplingJob.Run( ) )
        {
            spdlog::error( "Animation sampling failed" );
            return false;
        }

        ozz::animation::LocalToModelJob ltmJob;
        ltmJob.skeleton = impl->skeleton.get( );
        ltmJob.input    = ozz::make_span( internalContext->localTransforms );
        ltmJob.output   = ozz::make_span( internalContext->modelTransforms );

        if ( !ltmJob.Run( ) )
        {
            spdlog::error( "Local to model transformation failed" );
            return false;
        }

        for ( size_t i = 0; i < internalContext->modelTransforms.size( ); ++i )
        {
            desc->OutTransforms.Elements[ i ] = DenOfIz::FromOzzFloat4x4( internalContext->modelTransforms[ i ] );
        }
        return true;
    }

    bool DenOfIz_Ozz_RunSamplingJobLocal( DenOfIz_OzzSkeleton skeleton, const DenOfIz_SamplingJobLocalDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) || desc == nullptr || !DENOFIZ_HANDLE_IS_VALID( desc->Context ) )
        {
            spdlog::error( "Invalid sampling job local parameters" );
            return false;
        }

        DenOfIz::OzzSkeletonImpl *impl            = OZZ_SKELETON_IMPL( skeleton );
        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( desc->Context );

        if ( !internalContext->animation || !internalContext->samplingContext )
        {
            spdlog::error( "No animation loaded in context or sampling context not initialized" );
            return false;
        }

        if ( desc->OutTransforms.Elements == nullptr )
        {
            spdlog::error( "desc->OutTransforms.Elements is null" );
            return false;
        }

        if ( desc->OutTransforms.NumElements != (size_t)impl->skeleton->num_joints( ) )
        {
            spdlog::error( "desc->OutTransforms has incorrect number of elements, use GetNumJoints()" );
            return false;
        }

        const float ratio = ozz::math::Clamp( 0.f, desc->Ratio, 1.f );
        for ( size_t i = 0; i < internalContext->localTransforms.size( ); ++i )
        {
            internalContext->localTransforms[ i ] = impl->skeleton->joint_rest_poses( )[ i ];
        }

        ozz::animation::SamplingJob samplingJob;
        samplingJob.animation = internalContext->animation.get( );
        samplingJob.context   = internalContext->samplingContext.get( );
        samplingJob.ratio     = ratio;
        samplingJob.output    = ozz::make_span( internalContext->localTransforms );

        if ( !samplingJob.Run( ) )
        {
            spdlog::error( "Animation sampling failed" );
            return false;
        }

        const int numJoints = impl->skeleton->num_joints( );
        for ( int i = 0; i < numJoints; ++i )
        {
            const int soaIdx = i / 4;
            const int lane   = i % 4;

            const auto &soaTransform = internalContext->localTransforms[ soaIdx ];

            float tx[ 4 ], ty[ 4 ], tz[ 4 ];
            ozz::math::StorePtrU( soaTransform.translation.x, tx );
            ozz::math::StorePtrU( soaTransform.translation.y, ty );
            ozz::math::StorePtrU( soaTransform.translation.z, tz );

            float rx[ 4 ], ry[ 4 ], rz[ 4 ], rw[ 4 ];
            ozz::math::StorePtrU( soaTransform.rotation.x, rx );
            ozz::math::StorePtrU( soaTransform.rotation.y, ry );
            ozz::math::StorePtrU( soaTransform.rotation.z, rz );
            ozz::math::StorePtrU( soaTransform.rotation.w, rw );

            float sx[ 4 ], sy[ 4 ], sz[ 4 ];
            ozz::math::StorePtrU( soaTransform.scale.x, sx );
            ozz::math::StorePtrU( soaTransform.scale.y, sy );
            ozz::math::StorePtrU( soaTransform.scale.z, sz );

            desc->OutTransforms.Elements[ i ].Translation.X = tx[ lane ];
            desc->OutTransforms.Elements[ i ].Translation.Y = ty[ lane ];
            desc->OutTransforms.Elements[ i ].Translation.Z = tz[ lane ];

            desc->OutTransforms.Elements[ i ].Rotation.X = rx[ lane ];
            desc->OutTransforms.Elements[ i ].Rotation.Y = ry[ lane ];
            desc->OutTransforms.Elements[ i ].Rotation.Z = rz[ lane ];
            desc->OutTransforms.Elements[ i ].Rotation.W = rw[ lane ];

            desc->OutTransforms.Elements[ i ].Scale.X = sx[ lane ];
            desc->OutTransforms.Elements[ i ].Scale.Y = sy[ lane ];
            desc->OutTransforms.Elements[ i ].Scale.Z = sz[ lane ];
        }

        return true;
    }

    bool DenOfIz_Ozz_RunBlendingJob( DenOfIz_OzzSkeleton skeleton, const DenOfIz_BlendingJobDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) || desc == nullptr || !DENOFIZ_HANDLE_IS_VALID( desc->Context ) || desc->Layers.NumElements == 0 )
        {
            spdlog::error( "Invalid blending job parameters" );
            return false;
        }

        DenOfIz::OzzSkeletonImpl *impl            = OZZ_SKELETON_IMPL( skeleton );
        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( desc->Context );

        const size_t                                      numLayers = desc->Layers.NumElements;
        std::vector<ozz::animation::BlendingJob::Layer>   ozzLayers( numLayers );
        std::vector<ozz::vector<ozz::math::SoaTransform>> layerTransforms( numLayers );

        const size_t numSoaJoints = impl->skeleton->num_soa_joints( );
        for ( size_t i = 0; i < numLayers; ++i )
        {
            const auto &layer = desc->Layers.Elements[ i ];
            if ( layer.Transforms.NumElements == 0 )
            {
                spdlog::error( "Invalid transforms in layer {}", i );
                return false;
            }

            layerTransforms[ i ].resize( numSoaJoints );

            for ( size_t j = 0; j < numSoaJoints; ++j )
            {
                layerTransforms[ i ][ j ] = impl->skeleton->joint_rest_poses( )[ j ];
            }

            ozzLayers[ i ].transform = ozz::make_span( layerTransforms[ i ] );
            ozzLayers[ i ].weight    = layer.Weight;
        }

        ozz::vector<ozz::math::SoaTransform> output( numSoaJoints );

        ozz::animation::BlendingJob blendingJob;
        blendingJob.threshold = desc->Threshold;
        blendingJob.rest_pose = impl->skeleton->joint_rest_poses( );
        blendingJob.layers    = ozz::make_span( ozzLayers );
        blendingJob.output    = ozz::make_span( output );

        if ( !blendingJob.Run( ) )
        {
            spdlog::error( "Blending job failed" );
            return false;
        }

        ozz::animation::LocalToModelJob ltmJob;
        ltmJob.skeleton = impl->skeleton.get( );
        ltmJob.input    = ozz::make_span( output );
        ltmJob.output   = ozz::make_span( internalContext->modelTransforms );

        if ( !ltmJob.Run( ) )
        {
            spdlog::error( "Local to model transformation failed after blending" );
            return false;
        }

        for ( size_t i = 0; i < internalContext->modelTransforms.size( ); ++i )
        {
            desc->OutTransforms.Elements[ i ] = DenOfIz::FromOzzFloat4x4( internalContext->modelTransforms[ i ] );
        }

        return true;
    }

    bool DenOfIz_Ozz_RunLocalToModelJob( DenOfIz_OzzSkeleton skeleton, const DenOfIz_LocalToModelJobDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) || desc == nullptr || !DENOFIZ_HANDLE_IS_VALID( desc->Context ) )
        {
            spdlog::error( "Invalid local to model job parameters" );
            return false;
        }

        DenOfIz::OzzSkeletonImpl *impl            = OZZ_SKELETON_IMPL( skeleton );
        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( desc->Context );

        ozz::animation::LocalToModelJob ltmJob;
        ltmJob.skeleton = impl->skeleton.get( );
        ltmJob.input    = impl->skeleton->joint_rest_poses( );
        ltmJob.output   = ozz::make_span( internalContext->modelTransforms );

        if ( !ltmJob.Run( ) )
        {
            spdlog::error( "Local to model transformation failed" );
            return false;
        }

        for ( size_t i = 0; i < internalContext->modelTransforms.size( ); ++i )
        {
            desc->OutTransforms.Elements[ i ] = DenOfIz::FromOzzFloat4x4( internalContext->modelTransforms[ i ] );
        }

        return true;
    }

    bool DenOfIz_Ozz_RunLocalToModelFromTRS( DenOfIz_OzzSkeleton skeleton, const DenOfIz_LocalToModelFromTRSDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) || desc == nullptr )
        {
            spdlog::error( "Invalid local to model from TRS job parameters" );
            return false;
        }

        DenOfIz::OzzSkeletonImpl *impl = OZZ_SKELETON_IMPL( skeleton );

        if ( !impl->skeleton )
        {
            spdlog::error( "Skeleton not initialized" );
            return false;
        }

        const int numJoints = impl->skeleton->num_joints( );

        if ( desc->LocalTransforms.Elements == nullptr || desc->LocalTransforms.NumElements != (size_t)numJoints )
        {
            spdlog::error( "desc->LocalTransforms has incorrect number of elements" );
            return false;
        }

        if ( desc->OutTransforms.Elements == nullptr || desc->OutTransforms.NumElements != (size_t)numJoints )
        {
            spdlog::error( "desc->OutTransforms has incorrect number of elements" );
            return false;
        }

        const int numSoaJoints = impl->skeleton->num_soa_joints( );
        ozz::vector<ozz::math::SoaTransform> localTransforms( numSoaJoints );

        for ( int soaIdx = 0; soaIdx < numSoaJoints; ++soaIdx )
        {
            float tx[ 4 ] = { 0, 0, 0, 0 };
            float ty[ 4 ] = { 0, 0, 0, 0 };
            float tz[ 4 ] = { 0, 0, 0, 0 };
            float rx[ 4 ] = { 0, 0, 0, 0 };
            float ry[ 4 ] = { 0, 0, 0, 0 };
            float rz[ 4 ] = { 0, 0, 0, 0 };
            float rw[ 4 ] = { 1, 1, 1, 1 };
            float sx[ 4 ] = { 1, 1, 1, 1 };
            float sy[ 4 ] = { 1, 1, 1, 1 };
            float sz[ 4 ] = { 1, 1, 1, 1 };

            for ( int lane = 0; lane < 4; ++lane )
            {
                const int jointIdx = soaIdx * 4 + lane;
                if ( jointIdx < numJoints )
                {
                    const auto &transform = desc->LocalTransforms.Elements[ jointIdx ];
                    tx[ lane ]            = transform.Translation.X;
                    ty[ lane ]            = transform.Translation.Y;
                    tz[ lane ]            = transform.Translation.Z;
                    rx[ lane ]            = transform.Rotation.X;
                    ry[ lane ]            = transform.Rotation.Y;
                    rz[ lane ]            = transform.Rotation.Z;
                    rw[ lane ]            = transform.Rotation.W;
                    sx[ lane ]            = transform.Scale.X;
                    sy[ lane ]            = transform.Scale.Y;
                    sz[ lane ]            = transform.Scale.Z;
                }
            }

            localTransforms[ soaIdx ].translation.x = ozz::math::simd_float4::LoadPtrU( tx );
            localTransforms[ soaIdx ].translation.y = ozz::math::simd_float4::LoadPtrU( ty );
            localTransforms[ soaIdx ].translation.z = ozz::math::simd_float4::LoadPtrU( tz );
            localTransforms[ soaIdx ].rotation.x    = ozz::math::simd_float4::LoadPtrU( rx );
            localTransforms[ soaIdx ].rotation.y    = ozz::math::simd_float4::LoadPtrU( ry );
            localTransforms[ soaIdx ].rotation.z    = ozz::math::simd_float4::LoadPtrU( rz );
            localTransforms[ soaIdx ].rotation.w    = ozz::math::simd_float4::LoadPtrU( rw );
            localTransforms[ soaIdx ].scale.x       = ozz::math::simd_float4::LoadPtrU( sx );
            localTransforms[ soaIdx ].scale.y       = ozz::math::simd_float4::LoadPtrU( sy );
            localTransforms[ soaIdx ].scale.z       = ozz::math::simd_float4::LoadPtrU( sz );
        }

        ozz::vector<ozz::math::Float4x4> modelTransforms( numJoints );

        ozz::animation::LocalToModelJob ltmJob;
        ltmJob.skeleton = impl->skeleton.get( );
        ltmJob.input    = ozz::make_span( localTransforms );
        ltmJob.output   = ozz::make_span( modelTransforms );

        if ( !ltmJob.Run( ) )
        {
            spdlog::error( "Local to model transformation failed" );
            return false;
        }

        for ( int i = 0; i < numJoints; ++i )
        {
            desc->OutTransforms.Elements[ i ] = DenOfIz::FromOzzFloat4x4( modelTransforms[ i ] );
        }

        return true;
    }

    bool DenOfIz_Ozz_RunSkinningJob( const DenOfIz_SkinningJobDesc *desc )
    {
        if ( desc == nullptr || !DENOFIZ_HANDLE_IS_VALID( desc->Context ) || desc->JointTransforms.NumElements == 0 || desc->Vertices.NumElements == 0 ||
             desc->Weights.NumElements == 0 || desc->Indices.NumElements == 0 || desc->InfluenceCount <= 0 )
        {
            spdlog::error( "Invalid skinning job parameters" );
            return false;
        }

        const size_t                     numJoints = desc->JointTransforms.NumElements;
        ozz::vector<ozz::math::Float4x4> jointMatrices( numJoints );

        for ( size_t i = 0; i < numJoints; ++i )
        {
            jointMatrices[ i ] = DenOfIz::ToOzzFloat4x4( desc->JointTransforms.Elements[ i ] );
        }

        const size_t numVertices = desc->Vertices.NumElements;

        ozz::geometry::SkinningJob skinningJob;
        skinningJob.vertex_count     = (int)desc->Vertices.NumElements;
        skinningJob.influences_count = desc->InfluenceCount;
        skinningJob.joint_matrices   = ozz::make_span( jointMatrices );

        skinningJob.in_positions  = ozz::span( desc->Vertices.Elements, numVertices * 3 );
        skinningJob.joint_weights = ozz::span( desc->Weights.Elements, numVertices * desc->InfluenceCount );
        skinningJob.joint_indices = ozz::span( desc->Indices.Elements, numVertices * desc->InfluenceCount );

        if ( desc->OutVertices.Elements == nullptr || desc->OutVertices.NumElements < numVertices * 3 )
        {
            spdlog::error( "desc->OutVertices.Elements is null or too small" );
            return false;
        }
        skinningJob.out_positions = ozz::span( desc->OutVertices.Elements, numVertices * 3 );

        if ( desc->OutNormals.Elements == nullptr || desc->OutNormals.NumElements < numVertices * 3 )
        {
            spdlog::error( "desc->OutNormals.Elements is null or too small" );
            return false;
        }

        if ( !skinningJob.Run( ) )
        {
            spdlog::error( "Skinning job failed" );
            return false;
        }

        return true;
    }

    DenOfIz_IkTwoBoneJobResult DenOfIz_Ozz_RunIkTwoBoneJob( const DenOfIz_IkTwoBoneJobDesc *desc )
    {
        DenOfIz_IkTwoBoneJobResult result;
        result.Success = false;
        result.Reached = false;

        if ( desc == nullptr )
        {
            spdlog::error( "Invalid IK two bone job parameters" );
            return result;
        }

        const ozz::math::Float4x4 startMatrix = DenOfIz::ToOzzFloat4x4( desc->StartJointMatrix );
        const ozz::math::Float4x4 midMatrix   = DenOfIz::ToOzzFloat4x4( desc->MidJointMatrix );
        const ozz::math::Float4x4 endMatrix   = DenOfIz::ToOzzFloat4x4( desc->EndJointMatrix );

        ozz::animation::IKTwoBoneJob ozzJob;

        ozzJob.target      = DenOfIz::ToOzzSimdFloat4( desc->Target );
        ozzJob.pole_vector = DenOfIz::ToOzzSimdFloat4( desc->PoleVector );
        ozzJob.mid_axis    = DenOfIz::ToOzzSimdFloat4( desc->MidAxis );
        ozzJob.twist_angle = desc->TwistAngle;
        ozzJob.soften      = desc->Soften;
        ozzJob.weight      = desc->Weight;

        ozzJob.start_joint = &startMatrix;
        ozzJob.mid_joint   = &midMatrix;
        ozzJob.end_joint   = &endMatrix;

        ozz::math::SimdQuaternion startCorrection;
        ozz::math::SimdQuaternion midCorrection;
        bool                      targetReached = false;

        ozzJob.start_joint_correction = &startCorrection;
        ozzJob.mid_joint_correction   = &midCorrection;
        ozzJob.reached                = &targetReached;

        if ( !ozzJob.Validate( ) )
        {
            spdlog::error( "IKTwoBoneJob: Validation failed" );
            return result;
        }

        if ( !ozzJob.Run( ) )
        {
            spdlog::error( "IKTwoBoneJob: Execution failed" );
            return result;
        }

        result.Success              = true;
        result.StartJointCorrection = DenOfIz::FromOzzSimdQuaternion( startCorrection );
        result.MidJointCorrection   = DenOfIz::FromOzzSimdQuaternion( midCorrection );
        result.Reached              = targetReached;

        return result;
    }

    DenOfIz_IkAimJobResult DenOfIz_Ozz_RunIkAimJob( DenOfIz_OzzSkeleton skeleton, const DenOfIz_IkAimJobDesc *desc )
    {
        DenOfIz_IkAimJobResult result;
        result.Success = false;

        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) || desc == nullptr || !DENOFIZ_HANDLE_IS_VALID( desc->Context ) || desc->JointIndex < 0 )
        {
            spdlog::error( "Invalid IK aim job parameters" );
            return result;
        }

        DenOfIz::OzzSkeletonImpl *impl = OZZ_SKELETON_IMPL( skeleton );

        if ( !impl->skeleton || desc->JointIndex >= impl->skeleton->num_joints( ) )
        {
            spdlog::error( "Invalid joint index or skeleton not initialized" );
            return result;
        }

        ozz::animation::IKAimJob ozzJob;
        ozzJob.up      = DenOfIz::ToOzzSimdFloat4( desc->Up );
        ozzJob.forward = DenOfIz::ToOzzSimdFloat4( desc->Forward );
        ozzJob.target  = DenOfIz::ToOzzSimdFloat4( desc->Target );
        ozzJob.weight  = desc->Weight;

        const ozz::math::Float4x4 jointMatrix = ozz::math::Float4x4::identity( );
        ozzJob.joint                          = &jointMatrix;

        ozz::math::SimdQuaternion jointCorrection;
        ozzJob.joint_correction = &jointCorrection;

        if ( !ozzJob.Validate( ) )
        {
            spdlog::error( "IKAimJob: Validation failed" );
            return result;
        }

        if ( !ozzJob.Run( ) )
        {
            spdlog::error( "IKAimJob: Execution failed" );
            return result;
        }

        result.Success         = true;
        result.JointCorrection = DenOfIz::FromOzzSimdQuaternion( jointCorrection );
        return result;
    }

    DenOfIz_TrackSamplingResult DenOfIz_Ozz_RunTrackSamplingJob( const DenOfIz_TrackSamplingJobDesc *desc )
    {
        DenOfIz_TrackSamplingResult result;
        result.Success = false;

        if ( desc == nullptr || !DENOFIZ_HANDLE_IS_VALID( desc->Context ) || desc->TrackIndex < 0 )
        {
            spdlog::error( "Invalid track sampling job parameters" );
            return result;
        }

        result.Type = desc->Type;

        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( desc->Context );
        const float               ratio           = ozz::math::Clamp( 0.f, desc->Ratio, 1.f );
        bool                      success         = false;

        switch ( desc->Type )
        {
        case DenOfIz_TrackSamplingResultType_Float:
            {
                if ( desc->TrackIndex >= (int)internalContext->floatTracks.size( ) )
                {
                    spdlog::error( "Float track index out of range" );
                    return result;
                }
                ozz::animation::FloatTrackSamplingJob job;
                job.track  = internalContext->floatTracks[ desc->TrackIndex ].get( );
                job.ratio  = ratio;
                job.result = &result.FloatValue;
                success    = job.Run( );
                break;
            }

        case DenOfIz_TrackSamplingResultType_Float2:
            {
                if ( desc->TrackIndex >= (int)internalContext->float2Tracks.size( ) )
                {
                    spdlog::error( "Float2 track index out of range" );
                    return result;
                }

                ozz::animation::Float2TrackSamplingJob job;
                job.track = internalContext->float2Tracks[ desc->TrackIndex ].get( );
                job.ratio = ratio;
                ozz::math::Float2 float2Value;
                job.result = &float2Value;
                success    = job.Run( );
                if ( success )
                {
                    result.Float2Value.X = float2Value.x;
                    result.Float2Value.Y = float2Value.y;
                }
                break;
            }

        case DenOfIz_TrackSamplingResultType_Float3:
            {
                if ( desc->TrackIndex >= (int)internalContext->float3Tracks.size( ) )
                {
                    spdlog::error( "Float3 track index out of range" );
                    return result;
                }

                ozz::animation::Float3TrackSamplingJob job;
                job.track = internalContext->float3Tracks[ desc->TrackIndex ].get( );
                job.ratio = ratio;
                ozz::math::Float3 float3Value;
                job.result = &float3Value;
                success    = job.Run( );
                if ( success )
                {
                    result.Float3Value.X = float3Value.x;
                    result.Float3Value.Y = float3Value.y;
                    result.Float3Value.Z = float3Value.z;
                }
                break;
            }

        case DenOfIz_TrackSamplingResultType_Float4:
            {
                if ( desc->TrackIndex >= (int)internalContext->float4Tracks.size( ) )
                {
                    spdlog::error( "Float4 track index out of range" );
                    return result;
                }

                ozz::animation::Float4TrackSamplingJob job;
                job.track = internalContext->float4Tracks[ desc->TrackIndex ].get( );
                job.ratio = ratio;
                ozz::math::Float4 float4Value;
                job.result = &float4Value;
                success    = job.Run( );
                if ( success )
                {
                    result.Float4Value.X = float4Value.x;
                    result.Float4Value.Y = float4Value.y;
                    result.Float4Value.Z = float4Value.z;
                    result.Float4Value.W = float4Value.w;
                }
                break;
            }

        case DenOfIz_TrackSamplingResultType_Quaternion:
            {
                if ( desc->TrackIndex >= (int)internalContext->quaternionTracks.size( ) )
                {
                    spdlog::error( "Quaternion track index out of range" );
                    return result;
                }

                ozz::animation::QuaternionTrackSamplingJob job;
                job.track = internalContext->quaternionTracks[ desc->TrackIndex ].get( );
                job.ratio = ratio;
                ozz::math::Quaternion quaternionResult;
                job.result = &quaternionResult;
                success    = job.Run( );
                if ( success )
                {
                    result.QuaternionValue.X = quaternionResult.x;
                    result.QuaternionValue.Y = quaternionResult.y;
                    result.QuaternionValue.Z = quaternionResult.z;
                    result.QuaternionValue.W = quaternionResult.w;
                }
                break;
            }
        default:
            spdlog::error( "Unsupported track value type" );
            return result;
        }

        if ( !success )
        {
            spdlog::error( "Track sampling failed" );
            return result;
        }

        result.Success = true;
        return result;
    }

    DenOfIz_TrackTriggeringResult DenOfIz_Ozz_RunTrackTriggeringJob( DenOfIz_OzzSkeleton skeleton, const DenOfIz_TrackTriggeringJobDesc *desc )
    {
        DenOfIz_TrackTriggeringResult result;
        result.Success = false;

        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) || desc == nullptr || !DENOFIZ_HANDLE_IS_VALID( desc->Context ) || desc->TrackIndex < 0 )
        {
            spdlog::error( "Invalid track triggering job parameters" );
            return result;
        }

        DenOfIz::OzzSkeletonImpl *impl            = OZZ_SKELETON_IMPL( skeleton );
        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( desc->Context );

        if ( desc->TrackIndex >= (int)internalContext->floatTracks.size( ) )
        {
            spdlog::error( "Track index out of range" );
            return result;
        }

        ozz::animation::TrackTriggeringJob job;
        job.track     = internalContext->floatTracks[ desc->TrackIndex ].get( );
        job.from      = desc->PreviousRatio;
        job.to        = desc->Ratio;
        job.threshold = 0.5f;
        job.iterator  = nullptr;

        if ( !job.Run( ) )
        {
            spdlog::error( "Track triggering failed" );
            return result;
        }

        size_t edgeCount = 0;
        auto   it        = job.iterator;
        while ( it && *it != job.end( ) )
        {
            ++edgeCount;
            ++it;
        }

        std::vector<DenOfIz_TrackTriggeringEdge> &edges = impl->m_trackTriggeringEdges.emplace_back( edgeCount );
        result.Edges.Elements                           = edges.data( );
        result.Edges.NumElements                        = edges.size( );
        edgeCount                                       = 0;
        it                                              = job.iterator;
        while ( it && *it != job.end( ) )
        {
            const auto &edge          = *it;
            edges[ edgeCount ].Ratio  = edge->ratio;
            edges[ edgeCount ].Rising = edge->rising;
            ++edgeCount;
            ++it;
        }

        delete job.iterator;
        result.Success = true;
        return result;
    }

    DenOfIz_StringViewArray DenOfIz_Ozz_GetJointNames( DenOfIz_OzzSkeleton skeleton )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) )
        {
            DenOfIz_StringViewArray empty;
            empty.Elements    = nullptr;
            empty.NumElements = 0;
            return empty;
        }

        DenOfIz::OzzSkeletonImpl *impl = OZZ_SKELETON_IMPL( skeleton );

        if ( !impl->skeleton )
        {
            spdlog::error( "Skeleton not initialized" );
            DenOfIz_StringViewArray empty;
            empty.Elements    = nullptr;
            empty.NumElements = 0;
            return empty;
        }

        if ( impl->m_jointNames.empty( ) )
        {
            const int numJoints = impl->skeleton->num_joints( );
            impl->m_jointNames.resize( numJoints );
            impl->m_jointNamesView.resize( numJoints );
            for ( int i = 0; i < numJoints; ++i )
            {
                impl->m_jointNames[ i ]              = impl->skeleton->joint_names( )[ i ];
                impl->m_jointNamesView[ i ].Chars    = impl->m_jointNames[ i ].c_str( );
                impl->m_jointNamesView[ i ].NumChars = (uint32_t)impl->m_jointNames[ i ].length( );
            }
        }

        DenOfIz_StringViewArray result;
        result.Elements    = impl->m_jointNamesView.data( );
        result.NumElements = impl->m_jointNamesView.size( );
        return result;
    }

    int DenOfIz_Ozz_GetNumSoaJoints( DenOfIz_OzzSkeleton skeleton )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) )
        {
            return 0;
        }

        DenOfIz::OzzSkeletonImpl *impl = OZZ_SKELETON_IMPL( skeleton );
        return impl->skeleton ? impl->skeleton->num_soa_joints( ) : 0;
    }

    int DenOfIz_Ozz_GetNumJoints( DenOfIz_OzzSkeleton skeleton )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) )
        {
            return 0;
        }

        DenOfIz::OzzSkeletonImpl *impl = OZZ_SKELETON_IMPL( skeleton );
        return impl->skeleton ? impl->skeleton->num_joints( ) : 0;
    }

    DenOfIz_Int32Array DenOfIz_Ozz_GetJointParents( DenOfIz_OzzSkeleton skeleton )
    {
        DenOfIz_Int32Array empty;
        empty.Elements    = nullptr;
        empty.NumElements = 0;

        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) )
        {
            return empty;
        }

        DenOfIz::OzzSkeletonImpl *impl = OZZ_SKELETON_IMPL( skeleton );
        if ( !impl->skeleton )
        {
            return empty;
        }

        if ( impl->m_jointParents.empty( ) )
        {
            const auto &parents   = impl->skeleton->joint_parents( );
            const int   numJoints = impl->skeleton->num_joints( );
            impl->m_jointParents.resize( numJoints );
            for ( int i = 0; i < numJoints; ++i )
            {
                impl->m_jointParents[ i ] = parents[ i ];
            }
        }

        DenOfIz_Int32Array result;
        result.Elements    = impl->m_jointParents.data( );
        result.NumElements = impl->m_jointParents.size( );
        return result;
    }

    int32_t DenOfIz_Ozz_FindJoint( DenOfIz_OzzSkeleton skeleton, DenOfIz_StringView name )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) )
        {
            return -1;
        }

        DenOfIz::OzzSkeletonImpl *impl = OZZ_SKELETON_IMPL( skeleton );
        if ( !impl->skeleton )
        {
            return -1;
        }

        DenOfIz_Ozz_GetJointNames( skeleton );

        std::string searchName( name.Chars, name.NumChars );
        for ( size_t i = 0; i < impl->m_jointNames.size( ); ++i )
        {
            if ( impl->m_jointNames[ i ] == searchName )
            {
                return (int32_t)i;
            }
        }

        return -1;
    }

    DenOfIz_OzzJointArray DenOfIz_Ozz_GetJoints( DenOfIz_OzzSkeleton skeleton )
    {
        DenOfIz_OzzJointArray empty;
        empty.Elements    = nullptr;
        empty.NumElements = 0;

        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) )
        {
            return empty;
        }

        DenOfIz::OzzSkeletonImpl *impl = OZZ_SKELETON_IMPL( skeleton );
        if ( !impl->skeleton )
        {
            return empty;
        }

        if ( impl->m_joints.empty( ) )
        {
            DenOfIz_Ozz_GetJointNames( skeleton );
            DenOfIz_Ozz_GetJointParents( skeleton );
            DenOfIz_Ozz_GetRestPoses( skeleton );

            const int numJoints = impl->skeleton->num_joints( );
            impl->m_joints.resize( numJoints );
            for ( int i = 0; i < numJoints; ++i )
            {
                impl->m_joints[ i ].Name        = impl->m_jointNamesView[ i ];
                impl->m_joints[ i ].Index       = i;
                impl->m_joints[ i ].ParentIndex = impl->m_jointParents[ i ];
                impl->m_joints[ i ].RestPose    = impl->m_restPoses[ i ];
            }
        }

        DenOfIz_OzzJointArray result;
        result.Elements    = impl->m_joints.data( );
        result.NumElements = impl->m_joints.size( );
        return result;
    }

    DenOfIz_OzzJointTransformArray DenOfIz_Ozz_GetRestPoses( DenOfIz_OzzSkeleton skeleton )
    {
        DenOfIz_OzzJointTransformArray empty;
        empty.Elements    = nullptr;
        empty.NumElements = 0;

        if ( !DENOFIZ_HANDLE_IS_VALID( skeleton ) )
        {
            return empty;
        }

        DenOfIz::OzzSkeletonImpl *impl = OZZ_SKELETON_IMPL( skeleton );
        if ( !impl->skeleton )
        {
            return empty;
        }

        if ( impl->m_restPoses.empty( ) )
        {
            const auto &restPoses = impl->skeleton->joint_rest_poses( );
            const int   numJoints = impl->skeleton->num_joints( );
            impl->m_restPoses.resize( numJoints );

            for ( int i = 0; i < numJoints; ++i )
            {
                const int soaIdx = i / 4;
                const int lane   = i % 4;

                const auto &soaTransform = restPoses[ soaIdx ];

                float tx[ 4 ], ty[ 4 ], tz[ 4 ];
                ozz::math::StorePtrU( soaTransform.translation.x, tx );
                ozz::math::StorePtrU( soaTransform.translation.y, ty );
                ozz::math::StorePtrU( soaTransform.translation.z, tz );

                float rx[ 4 ], ry[ 4 ], rz[ 4 ], rw[ 4 ];
                ozz::math::StorePtrU( soaTransform.rotation.x, rx );
                ozz::math::StorePtrU( soaTransform.rotation.y, ry );
                ozz::math::StorePtrU( soaTransform.rotation.z, rz );
                ozz::math::StorePtrU( soaTransform.rotation.w, rw );

                float sx[ 4 ], sy[ 4 ], sz[ 4 ];
                ozz::math::StorePtrU( soaTransform.scale.x, sx );
                ozz::math::StorePtrU( soaTransform.scale.y, sy );
                ozz::math::StorePtrU( soaTransform.scale.z, sz );

                impl->m_restPoses[ i ].Translation.X = tx[ lane ];
                impl->m_restPoses[ i ].Translation.Y = ty[ lane ];
                impl->m_restPoses[ i ].Translation.Z = tz[ lane ];

                impl->m_restPoses[ i ].Rotation.X = rx[ lane ];
                impl->m_restPoses[ i ].Rotation.Y = ry[ lane ];
                impl->m_restPoses[ i ].Rotation.Z = rz[ lane ];
                impl->m_restPoses[ i ].Rotation.W = rw[ lane ];

                impl->m_restPoses[ i ].Scale.X = sx[ lane ];
                impl->m_restPoses[ i ].Scale.Y = sy[ lane ];
                impl->m_restPoses[ i ].Scale.Z = sz[ lane ];
            }
        }

        DenOfIz_OzzJointTransformArray result;
        result.Elements    = impl->m_restPoses.data( );
        result.NumElements = impl->m_restPoses.size( );
        return result;
    }

    float DenOfIz_Ozz_GetAnimationDuration( DenOfIz_OzzContext context )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( context ) )
        {
            return 0.0f;
        }

        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( context );
        return internalContext->animation ? internalContext->animation->duration( ) : 0.0f;
    }

    DenOfIz_StringView DenOfIz_Ozz_GetAnimationName( DenOfIz_OzzContext context )
    {
        DenOfIz_StringView empty;
        empty.Chars    = nullptr;
        empty.NumChars = 0;

        if ( !DENOFIZ_HANDLE_IS_VALID( context ) )
        {
            return empty;
        }

        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( context );
        if ( !internalContext->animation )
        {
            return empty;
        }

        DenOfIz_StringView result;
        result.Chars    = internalContext->animationName.c_str( );
        result.NumChars = (uint32_t)internalContext->animationName.length( );
        return result;
    }

    int DenOfIz_Ozz_GetAnimationNumTracks( DenOfIz_OzzContext context )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( context ) )
        {
            return 0;
        }

        DenOfIz::InternalContext *internalContext = OZZ_CONTEXT_IMPL( context );
        return internalContext->animation ? internalContext->animation->num_tracks( ) : 0;
    }
}
