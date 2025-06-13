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

#include "DenOfIzGraphicsInternal/Assets/Import/AssimpAnimationProcessor.h"
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

AssimpAnimationProcessor::AssimpAnimationProcessor( )  = default;
AssimpAnimationProcessor::~AssimpAnimationProcessor( ) = default;

ImporterResultCode AssimpAnimationProcessor::PreprocessAnimations( const AssimpImportContext &context, AnimationProcessingStats &stats )
{
    m_stats = AnimationProcessingStats{ };
    if ( !context.Scene->HasAnimations( ) )
    {
        spdlog::info( "No animations found in the scene" );
        stats = m_stats;
        return ImporterResultCode::Success;
    }

    m_stats.TotalAnimations = context.Scene->mNumAnimations;
    for ( unsigned int i = 0; i < context.Scene->mNumAnimations; ++i )
    {
        if ( const aiAnimation *anim = context.Scene->mAnimations[ i ] )
        {
            CountAnimationData( anim, m_stats );
        }
    }
    m_stats.RequiredArenaSize = CalculateAnimationMemoryRequirements( m_stats );
    stats                     = m_stats;
    spdlog::info( "Animation preprocessing complete: {} animations, {} tracks, {} total keys, {} bytes required", m_stats.TotalAnimations, m_stats.TotalTracks,
                  m_stats.TotalPositionKeys + m_stats.TotalRotationKeys + m_stats.TotalScaleKeys + m_stats.TotalMorphKeys, m_stats.RequiredArenaSize );
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpAnimationProcessor::ProcessAllAnimations( AssimpImportContext &context ) const
{
    if ( !context.Scene->HasAnimations( ) )
    {
        return ImporterResultCode::Success;
    }

    DZArenaArrayHelper<AssetUriArray, AssetUri>::AllocateAndConstructArray( *context.MainArena, context.MeshAsset.AnimationRefs, context.Scene->mNumAnimations );
    for ( unsigned int i = 0; i < context.Scene->mNumAnimations; ++i )
    {
        AssetUri animUri;
        if ( const ImporterResultCode result = ProcessAnimation( context, context.Scene->mAnimations[ i ], animUri ); result != ImporterResultCode::Success )
        {
            spdlog::error( "Failed to process animation {}", i );
            return result;
        }

        context.MeshAsset.AnimationRefs.Elements[ i ] = animUri;
    }

    spdlog::info( "Processed {} animations successfully", context.Scene->mNumAnimations );
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpAnimationProcessor::ProcessAnimation( AssimpImportContext &context, const aiAnimation *animation, AssetUri &outAssetUri ) const
{
    const std::string animNameStr = animation->mName.C_Str( );
    InteropString     animName    = AssetPathUtilities::SanitizeAssetName( animNameStr.c_str( ) );
    if ( animName.IsEmpty( ) )
    {
        animName = InteropString( "Animation_" ).Append( std::to_string( context.CreatedAssets.size( ) ).c_str( ) );
    }

    spdlog::info( "Processing animation: {} Duration: {} ticks, TicksPerSec: {}", animName.Get( ), animation->mDuration, animation->mTicksPerSecond );

    AnimationAsset animAsset;
    animAsset._Arena.EnsureCapacity( m_stats.RequiredArenaSize / m_stats.TotalAnimations * 2 ); // Estimate per animation
    animAsset.Name        = animName;
    animAsset.SkeletonRef = context.SkeletonAssetUri;

    AnimationClip clip;
    clip.Name                   = animName;
    const double ticksPerSecond = animation->mTicksPerSecond > 0 ? animation->mTicksPerSecond : 24.0;
    clip.Duration               = static_cast<float>( animation->mDuration / ticksPerSecond );

    if ( const ImporterResultCode result = BuildAnimationClip( context, animation, clip ); result != ImporterResultCode::Success )
    {
        return result;
    }

    DZArenaArrayHelper<AnimationClipArray, AnimationClip>::AllocateAndConstructArray( animAsset._Arena, animAsset.Animations, 1 );
    animAsset.Animations.Elements[ 0 ] = clip;
    return WriteAnimationAsset( context, animAsset, outAssetUri );
}

void AssimpAnimationProcessor::CountAnimationData( const aiAnimation *animation, AnimationProcessingStats &stats ) const
{
    stats.TotalTracks += animation->mNumChannels;
    for ( unsigned int c = 0; c < animation->mNumChannels; ++c )
    {
        if ( const aiNodeAnim *nodeAnim = animation->mChannels[ c ] )
        {
            stats.TotalPositionKeys += nodeAnim->mNumPositionKeys;
            stats.TotalRotationKeys += nodeAnim->mNumRotationKeys;
            stats.TotalScaleKeys += nodeAnim->mNumScalingKeys;
        }
    }

    stats.TotalTracks += animation->mNumMorphMeshChannels;
    for ( unsigned int m = 0; m < animation->mNumMorphMeshChannels; ++m )
    {
        if ( const aiMeshMorphAnim *morphAnim = animation->mMorphMeshChannels[ m ] )
        {
            stats.TotalMorphKeys += morphAnim->mNumKeys;
        }
    }
}

size_t AssimpAnimationProcessor::CalculateAnimationMemoryRequirements( const AnimationProcessingStats &stats ) const
{
    size_t required = 0;

    required += stats.TotalAnimations * sizeof( AnimationAsset );
    required += stats.TotalAnimations * sizeof( AnimationClip );
    required += stats.TotalTracks * sizeof( JointAnimTrack );
    required += stats.TotalTracks * sizeof( MorphAnimTrack );
    required += stats.TotalPositionKeys * sizeof( PositionKey );
    required += stats.TotalRotationKeys * sizeof( RotationKey );
    required += stats.TotalScaleKeys * sizeof( ScaleKey );
    required += stats.TotalMorphKeys * sizeof( MorphKeyframe );
    required += stats.TotalTracks * 64;

    // Add 30% buffer for misc allocations
    return static_cast<size_t>( required * 1.3 );
}

ImporterResultCode AssimpAnimationProcessor::BuildAnimationClip( const AssimpImportContext &context, const aiAnimation *animation, AnimationClip &clip ) const
{
    const double ticksPerSecond = animation->mTicksPerSecond > 0 ? animation->mTicksPerSecond : 24.0;
    if ( animation->mNumChannels > 0 )
    {
        spdlog::info( "Processing {} joint animation channels", animation->mNumChannels );

        DZArenaArrayHelper<JointAnimTrackArray, JointAnimTrack>::AllocateAndConstructArray( *context.MainArena, clip.Tracks, animation->mNumChannels );

        for ( unsigned int i = 0; i < animation->mNumChannels; ++i )
        {
            const aiNodeAnim *nodeAnim = animation->mChannels[ i ];
            JointAnimTrack   &track    = clip.Tracks.Elements[ i ];

            ProcessJointAnimationTrack( context, nodeAnim, track, ticksPerSecond );
        }
    }
    if ( animation->mNumMorphMeshChannels > 0 )
    {
        spdlog::info( "Processing {} morph target animation channels", animation->mNumMorphMeshChannels );

        DZArenaArrayHelper<MorphAnimTrackArray, MorphAnimTrack>::AllocateAndConstructArray( *context.MainArena, clip.MorphTracks, animation->mNumMorphMeshChannels );

        for ( unsigned int i = 0; i < animation->mNumMorphMeshChannels; ++i )
        {
            const aiMeshMorphAnim *morphAnim = animation->mMorphMeshChannels[ i ];
            MorphAnimTrack        &track     = clip.MorphTracks.Elements[ i ];

            ProcessMorphAnimationTrack( context, morphAnim, track, ticksPerSecond );
        }
    }

    return ImporterResultCode::Success;
}

void AssimpAnimationProcessor::ProcessJointAnimationTrack( const AssimpImportContext &context, const aiNodeAnim *nodeAnim, JointAnimTrack &track,
                                                           const double ticksPerSecond ) const
{
    track.JointName = nodeAnim->mNodeName.C_Str( );
    if ( !context.BoneNameToIndexMap.contains( track.JointName.Get( ) ) )
    {
        spdlog::warn( "Animation channel '{}' does not correspond to a known skeleton joint", track.JointName.Get( ) );
    }

    if ( nodeAnim->mNumPositionKeys > 0 )
    {
        DZArenaArrayHelper<PositionKeyArray, PositionKey>::AllocateAndConstructArray( *context.MainArena, track.PositionKeys, nodeAnim->mNumPositionKeys );

        for ( unsigned int k = 0; k < nodeAnim->mNumPositionKeys; ++k )
        {
            PositionKey &key = track.PositionKeys.Elements[ k ];
            key.Timestamp    = static_cast<float>( nodeAnim->mPositionKeys[ k ].mTime / ticksPerSecond );
            key.Value        = ConvertVector3( nodeAnim->mPositionKeys[ k ].mValue, context.Desc.ScaleFactor );
        }
    }
    if ( nodeAnim->mNumRotationKeys > 0 )
    {
        DZArenaArrayHelper<RotationKeyArray, RotationKey>::AllocateAndConstructArray( *context.MainArena, track.RotationKeys, nodeAnim->mNumRotationKeys );

        for ( unsigned int k = 0; k < nodeAnim->mNumRotationKeys; ++k )
        {
            RotationKey &key = track.RotationKeys.Elements[ k ];
            key.Timestamp    = static_cast<float>( nodeAnim->mRotationKeys[ k ].mTime / ticksPerSecond );
            key.Value        = ConvertQuaternion( nodeAnim->mRotationKeys[ k ].mValue );
        }
    }

    if ( nodeAnim->mNumScalingKeys > 0 )
    {
        DZArenaArrayHelper<ScaleKeyArray, ScaleKey>::AllocateAndConstructArray( *context.MainArena, track.ScaleKeys, nodeAnim->mNumScalingKeys );

        for ( unsigned int k = 0; k < nodeAnim->mNumScalingKeys; ++k )
        {
            ScaleKey &key = track.ScaleKeys.Elements[ k ];
            key.Timestamp = static_cast<float>( nodeAnim->mScalingKeys[ k ].mTime / ticksPerSecond );
            key.Value     = ConvertVector3( nodeAnim->mScalingKeys[ k ].mValue, 1.0f ); // Don't scale the scale values
        }
    }
}

void AssimpAnimationProcessor::ProcessMorphAnimationTrack( const AssimpImportContext &context, const aiMeshMorphAnim *morphAnim, MorphAnimTrack &track,
                                                           const double ticksPerSecond ) const
{
    track.Name = morphAnim->mName.C_Str( );
    DZArenaArrayHelper<MorphKeyframeArray, MorphKeyframe>::AllocateAndConstructArray( *context.MainArena, track.Keyframes, morphAnim->mNumKeys );
    for ( unsigned int k = 0; k < morphAnim->mNumKeys; ++k )
    {
        MorphKeyframe        &keyframe = track.Keyframes.Elements[ k ];
        const aiMeshMorphKey &aiKey    = morphAnim->mKeys[ k ];
        keyframe.Timestamp             = static_cast<float>( aiKey.mTime / ticksPerSecond );
        keyframe.Weight                = aiKey.mNumValuesAndWeights > 0 ? static_cast<float>( aiKey.mWeights[ 0 ] ) : 0.0f;
    }
}

ImporterResultCode AssimpAnimationProcessor::WriteAnimationAsset( AssimpImportContext &context, AnimationAsset &animationAsset, AssetUri &outAssetUri ) const
{
    const InteropString assetFilename   = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, animationAsset.Name, "Animation", AnimationAsset::Extension( ) );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( InteropString( context.TargetDirectory ).Append( "/" ).Append( assetFilename.Get( ) ) );

    outAssetUri        = AssetUri::Create( assetFilename );
    animationAsset.Uri = outAssetUri;

    spdlog::info( "Writing Animation asset to: {}", targetAssetPath.Get( ) );

    BinaryWriter         writer( targetAssetPath );
    AnimationAssetWriter assetWriter( { &writer } );
    assetWriter.Write( animationAsset );

    context.CreatedAssets.push_back( outAssetUri );

    return ImporterResultCode::Success;
}

Float_4 AssimpAnimationProcessor::ConvertQuaternion( const aiQuaternion &quat ) const
{
    return { quat.x, quat.y, quat.z, quat.w };
}

Float_3 AssimpAnimationProcessor::ConvertVector3( const aiVector3D &vec, const float scaleFactor ) const
{
    return { vec.x * scaleFactor, vec.y * scaleFactor, vec.z * scaleFactor };
}
