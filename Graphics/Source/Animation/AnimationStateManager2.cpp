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

#include <DenOfIzGraphics/Animation/Animation.h>
#include <DenOfIzGraphics/Animation/AnimationSetup.h>
#include <DenOfIzGraphics/Animation/AnimationStateManager2.h>
#include <DenOfIzGraphics/Animation/AnimationUtils.h>
#include <DenOfIzGraphics/Animation/BlendingJob.h>
#include <DenOfIzGraphics/Animation/LocalToModelJob.h>
#include <DenOfIzGraphics/Animation/SamplingJob.h>
#include <DenOfIzGraphics/Animation/Skeleton.h>
#include <glog/logging.h>

#include <string>
#include <unordered_map>

#include "DenOfIzGraphics/Animation/AnimationSystem.h"

namespace DenOfIz
{
    struct AnimState
    {
        InteropString   Name;
        Animation      *OzzAnimation  = nullptr;
        AnimationSetup *Setup         = nullptr;
        float           PlaybackSpeed = 1.0f;
        float           CurrentTime   = 0.0f;
        float           Weight        = 1.0f;
        bool            Loop          = true;
        bool            Playing       = false;
    };

    struct BlendState
    {
        InteropString SourceAnimation;
        InteropString TargetAnimation;
        float         BlendTime        = 0.5f;
        float         CurrentBlendTime = 0.0f;
        bool          InProgress       = false;
    };

    class AnimationStateManager2::Impl
    {
    public:
        AnimationSystem animSystem;
        Skeleton       *skeleton    = nullptr;
        AnimationSetup *outputSetup = nullptr;

        std::unordered_map<std::string, AnimState> animations;
        InteropString                              currentAnimation;
        BlendState                                 blendingState;

        static void SampleAnimation( AnimState &state )
        {
            if ( !state.OzzAnimation || !state.Playing )
            {
                return;
            }

            const float duration = state.OzzAnimation->GetDuration( );
            if ( duration <= 0.0f )
            {
                return;
            }

            float ratio = state.CurrentTime / duration;
            ratio       = std::max( 0.0f, std::min( ratio, 1.0f ) );

            SamplingJob samplingJob;
            samplingJob.animation = state.OzzAnimation;
            samplingJob.ratio     = ratio;
            samplingJob.setup     = state.Setup;

            if ( !samplingJob.Run( ) )
            {
                LOG( ERROR ) << "SamplingJob failed for animation '" << state.Name.Get( ) << "'";
            }
        }

        void UpdateAnimationsAndBlending( float deltaTime )
        {
            for ( auto &state : animations | std::views::values )
            {
                if ( state.Playing )
                {
                    const float duration = state.OzzAnimation->GetDuration( );

                    state.CurrentTime += deltaTime * state.PlaybackSpeed;
                    if ( state.CurrentTime > duration )
                    {
                        if ( state.Loop )
                        {
                            state.CurrentTime = fmodf( state.CurrentTime, duration );
                        }
                        else
                        {
                            state.CurrentTime = duration;
                            state.Playing     = false;
                        }
                    }
                    SampleAnimation( state );
                }
            }

            if ( blendingState.InProgress )
            {
                blendingState.CurrentBlendTime += deltaTime;

                const float blendFactor = blendingState.CurrentBlendTime / blendingState.BlendTime;
                if ( blendFactor >= 1.0f )
                {
                    blendingState.InProgress = false;
                    currentAnimation         = blendingState.TargetAnimation;

                    for ( auto &[ name, state ] : animations )
                    {
                        state.Weight  = name == currentAnimation.Get( ) ? 1.0f : 0.0f;
                        state.Playing = name == currentAnimation.Get( );
                    }

                    LocalToModelJob localToModelJob;
                    localToModelJob.setup = animations[ currentAnimation.Get( ) ].Setup;
                    localToModelJob.Run( );

                    outputSetup = animations[ currentAnimation.Get( ) ].Setup;
                }
                else
                {
                    auto &sourceState = animations[ blendingState.SourceAnimation.Get( ) ];
                    auto &targetState = animations[ blendingState.TargetAnimation.Get( ) ];

                    sourceState.Weight = 1.0f - blendFactor;
                    targetState.Weight = blendFactor;

                    InteropArray<BlendingLayer> blendLayers;
                    blendLayers.Resize( 2 );

                    blendLayers.GetElement( 0 ).setup  = sourceState.Setup;
                    blendLayers.GetElement( 0 ).weight = sourceState.Weight;

                    blendLayers.GetElement( 1 ).setup  = targetState.Setup;
                    blendLayers.GetElement( 1 ).weight = targetState.Weight;

                    BlendingJob blendingJob;
                    blendingJob.layers    = blendLayers;
                    blendingJob.threshold = 0.1f;
                    blendingJob.output    = outputSetup;

                    if ( blendingJob.Run( ) )
                    {
                        LocalToModelJob localToModelJob;
                        localToModelJob.setup = outputSetup;
                        localToModelJob.Run( );
                    }
                    else
                    {
                        LOG( ERROR ) << "BlendingJob failed";
                    }
                }
            }
            else if ( !currentAnimation.Get( )[ 0 ] == '\0' )
            {
                LocalToModelJob localToModelJob;
                localToModelJob.setup = animations[ currentAnimation.Get( ) ].Setup;

                if ( localToModelJob.Run( ) )
                {
                    outputSetup = animations[ currentAnimation.Get( ) ].Setup;
                }
                else
                {
                    LOG( ERROR ) << "LocalToModelJob failed for animation '" << currentAnimation.Get( ) << "'";
                }
            }
        }
    };

    AnimationStateManager2::AnimationStateManager2( const AnimationStateManagerDesc2 &desc ) : m_impl( new Impl( ) )
    {
        if ( !desc.Skeleton )
        {
            LOG( ERROR ) << "Skeleton is required for AnimationStateManager2";
            return;
        }

        m_impl->skeleton    = m_impl->animSystem.CreateSkeleton( *desc.Skeleton );
        m_impl->outputSetup = m_impl->animSystem.CreateAnimationSetup( m_impl->skeleton );

        LOG( INFO ) << "AnimationStateManager2 created with " << m_impl->skeleton->GetNumJoints( ) << " joints";
    }

    AnimationStateManager2::~AnimationStateManager2( )
    {
        for ( const auto &state : m_impl->animations | std::views::values )
        {
            if ( state.Setup )
            {
                m_impl->animSystem.ReleaseAnimationSetup( state.Setup );
            }
        }

        for ( const auto &state : m_impl->animations | std::views::values )
        {
            if ( state.OzzAnimation )
            {
                m_impl->animSystem.ReleaseAnimation( state.OzzAnimation );
            }
        }

        if ( m_impl->outputSetup )
        {
            bool isUsedByAnimation = false;
            for ( const auto &state : m_impl->animations | std::views::values )
            {
                if ( state.Setup == m_impl->outputSetup )
                {
                    isUsedByAnimation = true;
                    break;
                }
            }

            if ( !isUsedByAnimation )
            {
                m_impl->animSystem.ReleaseAnimationSetup( m_impl->outputSetup );
            }
        }

        if ( m_impl->skeleton )
        {
            m_impl->animSystem.ReleaseSkeleton( m_impl->skeleton );
        }

        delete m_impl;
    }

    void AnimationStateManager2::AddAnimation( const AnimationAsset &animationAsset ) const
    {
        for ( size_t i = 0; i < animationAsset.Animations.NumElements( ); ++i )
        {
            const AnimationClip &clip     = animationAsset.Animations.GetElement( i );
            std::string          animName = clip.Name.Get( );

            if ( animName.empty( ) )
            {
                animName = "Animation_" + std::to_string( i );
            }

            Animation *animation = m_impl->animSystem.CreateAnimation( animationAsset, m_impl->skeleton );
            if ( !animation )
            {
                LOG( ERROR ) << "Failed to create animation from clip";
                continue;
            }

            AnimationSetup *setup = m_impl->animSystem.CreateAnimationSetup( m_impl->skeleton );
            if ( !setup )
            {
                LOG( ERROR ) << "Failed to create animation setup";
                m_impl->animSystem.ReleaseAnimation( animation );
                continue;
            }

            AnimState state;
            state.Name                     = InteropString( animName.c_str( ) );
            state.OzzAnimation             = animation;
            state.Setup                    = setup;
            state.Loop                     = true;
            state.Playing                  = false;
            m_impl->animations[ animName ] = std::move( state );
            LOG( INFO ) << "Added animation '" << animName << "' with duration " << clip.Duration << "s";
        }

        if ( m_impl->currentAnimation.Get( )[ 0 ] == '\0' && !m_impl->animations.empty( ) )
        {
            m_impl->currentAnimation = m_impl->animations.begin( )->second.Name;
            LOG( INFO ) << "Set default animation to '" << m_impl->currentAnimation.Get( ) << "'";
        }
    }

    void AnimationStateManager2::Play( const InteropString &animationName, const bool loop ) const
    {
        if ( !HasAnimation( animationName ) )
        {
            LOG( ERROR ) << "Animation '" << animationName.Get( ) << "' not found";
            return;
        }

        m_impl->blendingState.InProgress = false;
        if ( m_impl->currentAnimation.Get( )[ 0 ] != '\0' )
        {
            auto &prevAnim   = m_impl->animations[ m_impl->currentAnimation.Get( ) ];
            prevAnim.Playing = false;
        }

        m_impl->currentAnimation = animationName;
        auto &newAnim            = m_impl->animations[ animationName.Get( ) ];

        newAnim.Loop        = loop;
        newAnim.Playing     = true;
        newAnim.CurrentTime = 0.0f;
        newAnim.Weight      = 1.0f;

        m_impl->outputSetup = newAnim.Setup;

        LOG( INFO ) << "Playing animation '" << animationName.Get( ) << "'" << ( loop ? " (looping)" : "" );
    }

    void AnimationStateManager2::BlendTo( const InteropString &animationName, const float blendTime ) const
    {
        if ( !HasAnimation( animationName ) )
        {
            LOG( ERROR ) << "Animation '" << animationName.Get( ) << "' not found";
            return;
        }

        if ( strcmp( m_impl->currentAnimation.Get( ), animationName.Get( ) ) == 0 )
        {
            return;
        }

        m_impl->blendingState.SourceAnimation  = m_impl->currentAnimation;
        m_impl->blendingState.TargetAnimation  = animationName;
        m_impl->blendingState.BlendTime        = blendTime;
        m_impl->blendingState.CurrentBlendTime = 0.0f;
        m_impl->blendingState.InProgress       = true;

        auto &targetAnim       = m_impl->animations[ animationName.Get( ) ];
        targetAnim.Weight      = 0.0f;
        targetAnim.Playing     = true;
        targetAnim.CurrentTime = 0.0f;

        LOG( INFO ) << "Blending from '" << m_impl->currentAnimation.Get( ) << "' to '" << animationName.Get( ) << "' over " << blendTime << "s";
    }

    void AnimationStateManager2::Stop( ) const
    {
        if ( m_impl->currentAnimation.Get( )[ 0 ] != '\0' )
        {
            auto &anim       = m_impl->animations[ m_impl->currentAnimation.Get( ) ];
            anim.Playing     = false;
            anim.CurrentTime = 0.0f;
        }

        m_impl->blendingState.InProgress = false;
    }

    void AnimationStateManager2::Pause( ) const
    {
        if ( m_impl->currentAnimation.Get( )[ 0 ] != '\0' )
        {
            auto &anim   = m_impl->animations[ m_impl->currentAnimation.Get( ) ];
            anim.Playing = false;
        }
    }

    void AnimationStateManager2::Resume( ) const
    {
        if ( m_impl->currentAnimation.Get( )[ 0 ] != '\0' )
        {
            auto &anim   = m_impl->animations[ m_impl->currentAnimation.Get( ) ];
            anim.Playing = true;
        }
    }

    void AnimationStateManager2::Update( const float deltaTime ) const
    {
        m_impl->UpdateAnimationsAndBlending( deltaTime );
    }

    bool AnimationStateManager2::HasAnimation( const InteropString &animationName ) const
    {
        return m_impl->animations.contains( animationName.Get( ) );
    }

    void AnimationStateManager2::GetModelSpaceTransforms( InteropArray<Float_4x4> &outTransforms ) const
    {
        if ( !m_impl->outputSetup )
        {
            outTransforms.Clear( );
            return;
        }

        outTransforms = m_impl->outputSetup->GetModelTransforms( );
        AnimationUtils::ApplyCoordinateSystemCorrection( outTransforms );
    }

    const InteropString &AnimationStateManager2::GetCurrentAnimationName( ) const
    {
        return m_impl->currentAnimation;
    }

    int AnimationStateManager2::GetNumJoints( ) const
    {
        return m_impl->skeleton ? m_impl->skeleton->GetNumJoints( ) : 0;
    }
} // namespace DenOfIz
