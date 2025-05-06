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
#include <DenOfIzGraphics/Animation/AnimationSystem.h>
#include <DenOfIzGraphics/Animation/Skeleton.h>
#include <glog/logging.h>

#include <unordered_set>

namespace DenOfIz
{
    class AnimationSystem::Impl
    {
    public:
        std::unordered_set<Skeleton *>       skeletons;
        std::unordered_set<Animation *>      animations;
        std::unordered_set<AnimationSetup *> setups;
    };

    AnimationSystem::AnimationSystem( ) : m_impl( new Impl( ) )
    {
    }

    AnimationSystem::~AnimationSystem( )
    {
        for ( const auto setup : m_impl->setups )
        {
            delete setup;
        }

        for ( const auto animation : m_impl->animations )
        {
            delete animation;
        }

        for ( const auto skeleton : m_impl->skeletons )
        {
            delete skeleton;
        }

        delete m_impl;
    }

    Skeleton *AnimationSystem::CreateSkeleton( const SkeletonAsset &skeletonAsset ) const
    {
        auto *skeleton = new Skeleton( skeletonAsset );
        m_impl->skeletons.insert( skeleton );
        return skeleton;
    }

    Animation *AnimationSystem::CreateAnimation( const AnimationAsset &animationAsset, Skeleton *skeleton ) const
    {
        if ( !m_impl->skeletons.contains( skeleton ) )
        {
            LOG( ERROR ) << "AnimationSystem::CreateAnimation: skeleton not created by this system";
            return nullptr;
        }

        auto *animation = new Animation( animationAsset, skeleton );
        m_impl->animations.insert( animation );
        return animation;
    }

    AnimationSetup *AnimationSystem::CreateAnimationSetup( Skeleton *skeleton ) const
    {
        if ( !m_impl->skeletons.contains( skeleton ) )
        {
            LOG( ERROR ) << "AnimationSystem::CreateAnimationSetup: skeleton not created by this system";
            return nullptr;
        }

        auto *setup = new AnimationSetup( skeleton );
        m_impl->setups.insert( setup );
        return setup;
    }

    void AnimationSystem::ReleaseSkeleton( Skeleton *skeleton ) const
    {
        const auto it = m_impl->skeletons.find( skeleton );
        if ( it != m_impl->skeletons.end( ) )
        {
            m_impl->skeletons.erase( it );
            delete skeleton;
        }
    }

    void AnimationSystem::ReleaseAnimation( Animation *animation ) const
    {
        const auto it = m_impl->animations.find( animation );
        if ( it != m_impl->animations.end( ) )
        {
            m_impl->animations.erase( it );
            delete animation;
        }
    }

    void AnimationSystem::ReleaseAnimationSetup( AnimationSetup *setup ) const
    {
        const auto it = m_impl->setups.find( setup );
        if ( it != m_impl->setups.end( ) )
        {
            m_impl->setups.erase( it );
            delete setup;
        }
    }
} // namespace DenOfIz
