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
#include <DenOfIzGraphics/Animation/SamplingJob.h>
#include <DenOfIzGraphics/Animation/Skeleton.h>
#include <glog/logging.h>

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/span.h>

using ozz::make_span;
using ozz::span;

namespace DenOfIz
{
    class SamplingJob::Impl
    {
    public:
        static bool ValidateSetup( const SamplingJob &job )
        {
            if ( !job.animation )
            {
                LOG( ERROR ) << "SamplingJob: animation is null";
                return false;
            }

            if ( !job.setup )
            {
                LOG( ERROR ) << "SamplingJob: setup is null";
                return false;
            }

            if ( job.ratio < 0.0f || job.ratio > 1.0f )
            {
                LOG( WARNING ) << "SamplingJob: ratio " << job.ratio << " is outside [0,1] range, will be clamped";
            }

            return true;
        }
    };

    SamplingJob::SamplingJob( ) : m_impl( new Impl( ) ), animation( nullptr ), ratio( 0.0f ), setup( nullptr )
    {
    }

    SamplingJob::~SamplingJob( )
    {
        delete m_impl;
    }

    bool SamplingJob::Run( )
    {
        if ( !m_impl->ValidateSetup( *this ) )
        {
            return false;
        }

        const float clampedRatio = std::max( 0.0f, std::min( ratio, 1.0f ) );

        auto &animImpl     = *animation->m_impl;
        auto &setupImpl    = *setup->m_impl;
        auto &skeletonImpl = *setup->GetSkeleton( )->m_impl;

        ozz::animation::SamplingJob ozzJob;
        ozzJob.animation = animImpl.ozzAnimation.get( );
        ozzJob.ratio     = clampedRatio;
        ozzJob.output    = make_span( setupImpl.localTransforms );
        ozzJob.context   = setupImpl.context.get( );

        if ( !ozzJob.Run( ) )
        {
            LOG( ERROR ) << "Animation sampling failed for animation '" << animation->GetName( ).Get( ) << "'";
            for ( size_t i = 0; i < setupImpl.localTransforms.size( ); ++i )
            {
                setupImpl.localTransforms[ i ] = skeletonImpl.ozzSkeleton->joint_rest_poses( )[ i ];
            }

            return false;
        }

        return true;
    }
} // namespace DenOfIz
