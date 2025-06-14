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

#include "DenOfIzGraphics/Animation/AnimationStateManager.h"

#include <array>
#include <ranges>
#include "DenOfIzGraphicsInternal/Utilities/InteropMathConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

AnimationStateManager::AnimationStateManager( const AnimationStateManagerDesc &desc )
{
    if ( !desc.Skeleton )
    {
        spdlog::error( "Skeleton is required for AnimationStateManager" );
        return;
    }

    m_ozzAnimation = new OzzAnimation( desc.Skeleton );
    m_modelTransforms.resize( m_ozzAnimation->GetNumJoints( ) );
    m_blendSourceTransforms.resize( m_ozzAnimation->GetNumJoints( ) );
    m_blendTargetTransforms.resize( m_ozzAnimation->GetNumJoints( ) );
}

AnimationStateManager::~AnimationStateManager( )
{
    for ( auto &state : m_animations | std::views::values )
    {
        if ( state.Context )
        {
            m_ozzAnimation->DestroyContext( state.Context );
            state.Context = nullptr;
        }
    }

    delete m_ozzAnimation;
}

void AnimationStateManager::AddAnimation( const AnimationAsset &animationAsset )
{
    for ( size_t i = 0; i < animationAsset.Animations.NumElements; ++i )
    {
        const AnimationClip &clip     = animationAsset.Animations.Elements[ i ];
        InteropString        animName = clip.Name;

        if ( animName.NumChars( ) == 0 )
        {
            animName = InteropString( ( "Animation_" + std::to_string( i ) ).c_str( ) );
        }

        OzzContext *context = m_ozzAnimation->NewContext( );
        m_ozzAnimation->LoadAnimation( &animationAsset, context );

        AnimationState state;
        state.Name    = animName;
        state.Context = context;
        state.Loop    = true;
        state.Playing = false;

        m_animations[ animName.Get( ) ] = std::move( state );

        spdlog::info( "Added animation ' {} ' with duration {} s", animName.Get( ), clip.Duration );
    }

    if ( m_currentAnimation.NumChars( ) > 0 && !m_animations.empty( ) )
    {
        m_currentAnimation = InteropString( m_animations.begin( )->first.c_str( ) );
        spdlog::info( "Set default animation to ' {} '", m_currentAnimation.Get( ) );
    }
}

void AnimationStateManager::Play( const InteropString &animationName, const bool loop )
{
    if ( !HasAnimation( animationName ) )
    {
        spdlog::error( "Animation ' {} ' not found", animationName.Get( ) );
        return;
    }

    m_blendingState.InProgress = false;

    auto &prevAnim   = m_animations[ m_currentAnimation.Get( ) ];
    prevAnim.Playing = false;

    m_currentAnimation = animationName;
    auto &newAnim      = m_animations[ m_currentAnimation.Get( ) ];

    newAnim.Loop        = loop;
    newAnim.Playing     = true;
    newAnim.CurrentTime = 0.0f;

    spdlog::info( "Playing animation ' {} '{}", animationName.Get( ), ( loop ? " (looping)" : "" ) );
}

void AnimationStateManager::BlendTo( const InteropString &animationName, const float blendTime )
{
    if ( !HasAnimation( animationName ) )
    {
        spdlog::error( "Animation ' {} ' not found", animationName.Get( ) );
        return;
    }

    if ( m_currentAnimation.Get( ) == animationName.Get( ) )
    {
        return;
    }

    m_blendingState.SourceAnimation  = m_currentAnimation;
    m_blendingState.TargetAnimation  = animationName;
    m_blendingState.BlendTime        = blendTime;
    m_blendingState.CurrentBlendTime = 0.0f;
    m_blendingState.InProgress       = true;

    auto &targetAnim       = m_animations[ animationName.Get( ) ];
    targetAnim.Weight      = 0.0f;
    targetAnim.Playing     = true;
    targetAnim.CurrentTime = 0.0f;

    spdlog::info( "Blending from ' {} ' to ' {} ' over {} s", m_currentAnimation.Get( ), animationName.Get( ), blendTime );
}

void AnimationStateManager::Stop( )
{
    if ( m_currentAnimation.NumChars( ) > 0 )
    {
        auto &anim       = m_animations[ m_currentAnimation.Get( ) ];
        anim.Playing     = false;
        anim.CurrentTime = 0.0f;
    }

    m_blendingState.InProgress = false;
}

void AnimationStateManager::Pause( )
{
    if ( m_currentAnimation.NumChars( ) > 0 )
    {
        auto &anim   = m_animations[ m_currentAnimation.Get( ) ];
        anim.Playing = false;
    }
}

void AnimationStateManager::Resume( )
{
    if ( m_currentAnimation.NumChars( ) > 0 )
    {
        auto &anim   = m_animations[ m_currentAnimation.Get( ) ];
        anim.Playing = true;
    }
}

void AnimationStateManager::Update( const float deltaTime )
{
    if ( m_currentAnimation.NumChars( ) == 0 || !m_animations[ m_currentAnimation.Get( ) ].Playing )
    {
        return;
    }

    UpdateBlending( deltaTime );

    if ( auto &anim = m_animations[ m_currentAnimation.Get( ) ]; anim.Playing )
    {
        const float duration = OzzAnimation::GetAnimationDuration( anim.Context );
        if ( duration <= 0.0f )
        {
            return;
        }

        anim.CurrentTime += deltaTime * anim.PlaybackSpeed;
        if ( anim.CurrentTime > duration )
        {
            if ( anim.Loop )
            {
                anim.CurrentTime = fmodf( anim.CurrentTime, duration );
            }
            else
            {
                anim.CurrentTime = duration;
                anim.Playing     = false;
            }
        }

        Float_4x4Array  transforms{ m_modelTransforms.data( ), m_modelTransforms.size( ) };
        SamplingJobDesc samplingDesc;
        samplingDesc.Context       = anim.Context;
        samplingDesc.Ratio         = anim.CurrentTime / duration;
        samplingDesc.OutTransforms = &transforms;

        if ( !m_ozzAnimation->RunSamplingJob( samplingDesc ) )
        {
            spdlog::error( "Failed to sample animation ' {} '", anim.Name.Get( ) );
        }
    }
}

bool AnimationStateManager::HasAnimation( const InteropString &animationName ) const
{
    return m_animations.contains( animationName.Get( ) );
}

Float_4x4Array AnimationStateManager::GetModelSpaceTransforms( )
{
    if ( m_modelTransforms.empty( ) )
    {
        return { nullptr, 0 };
    }

    return { m_modelTransforms.data( ), m_modelTransforms.size( ) };
}

void AnimationStateManager::UpdateBlending( const float deltaTime )
{
    if ( !m_blendingState.InProgress )
    {
        return;
    }

    m_blendingState.CurrentBlendTime += deltaTime;

    const float blendFactor = m_blendingState.CurrentBlendTime / m_blendingState.BlendTime;
    if ( blendFactor >= 1.0f )
    {
        m_blendingState.InProgress = false;
        m_currentAnimation         = m_blendingState.TargetAnimation;

        for ( auto &[ name, anim ] : m_animations )
        {
            anim.Weight  = name == m_currentAnimation.Get( ) ? 1.0f : 0.0f;
            anim.Playing = name == m_currentAnimation.Get( );
        }
        return;
    }

    auto &sourceAnim = m_animations[ m_blendingState.SourceAnimation.Get( ) ];
    auto &targetAnim = m_animations[ m_blendingState.TargetAnimation.Get( ) ];

    sourceAnim.Weight = 1.0f - blendFactor;
    targetAnim.Weight = blendFactor;

    Float_4x4Array  blendSourceTransforms{ m_blendSourceTransforms.data( ), m_blendSourceTransforms.size( ) };
    SamplingJobDesc sourceSamplingDesc;
    sourceSamplingDesc.Context       = sourceAnim.Context;
    sourceSamplingDesc.Ratio         = sourceAnim.CurrentTime / OzzAnimation::GetAnimationDuration( sourceAnim.Context );
    sourceSamplingDesc.OutTransforms = &blendSourceTransforms;

    Float_4x4Array  blendTargetTransforms{ m_blendTargetTransforms.data( ), m_blendTargetTransforms.size( ) };
    SamplingJobDesc targetSamplingDesc;
    targetSamplingDesc.Context       = targetAnim.Context;
    targetSamplingDesc.Ratio         = targetAnim.CurrentTime / OzzAnimation::GetAnimationDuration( targetAnim.Context );
    targetSamplingDesc.OutTransforms = &blendTargetTransforms;

    const bool sourceSuccess = m_ozzAnimation->RunSamplingJob( sourceSamplingDesc );
    const bool targetSuccess = m_ozzAnimation->RunSamplingJob( targetSamplingDesc );
    if ( !sourceSuccess || !targetSuccess )
    {
        spdlog::error( "Failed to sample animations for blending" );
        return;
    }

    BlendingJobDesc blendingDesc;
    blendingDesc.Context   = sourceAnim.Context; // Can use either context
    blendingDesc.Threshold = 0.1f;

    BlendingJobLayerDesc sourceLayer;
    sourceLayer.Transforms = blendSourceTransforms;
    sourceLayer.Weight     = sourceAnim.Weight;

    BlendingJobLayerDesc targetLayer;
    targetLayer.Transforms = blendTargetTransforms;
    targetLayer.Weight     = targetAnim.Weight;

    std::array<BlendingJobLayerDesc, 2> layers{ };
    layers[ 0 ]                     = sourceLayer;
    layers[ 1 ]                     = targetLayer;
    blendingDesc.Layers.Elements    = layers.data( );
    blendingDesc.Layers.NumElements = layers.size( );
    Float_4x4Array transforms{ m_modelTransforms.data( ), m_modelTransforms.size( ) };
    blendingDesc.OutTransforms = &transforms;
    if ( !m_ozzAnimation->RunBlendingJob( blendingDesc ) )
    {
        spdlog::error( "Failed to blend animations" );
    }
}

const InteropString &AnimationStateManager::GetCurrentAnimationName( ) const
{
    return m_currentAnimation;
}

int AnimationStateManager::GetNumJoints( ) const
{
    return m_ozzAnimation ? m_ozzAnimation->GetNumJoints( ) : 0;
}
