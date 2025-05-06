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

#include <DenOfIzGraphics/Animation/IKAimJob.h>
#include <DenOfIzGraphics/Animation/Skeleton.h>
#include <glog/logging.h>

#include <ozz/animation/runtime/ik_aim_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/maths/vec_float.h>

namespace DenOfIz
{
    class IKAimJob::Impl
    {
    public:
        static ozz::math::Float3 ToOzzFloat3( const Float_3 &v )
        {
            return ozz::math::Float3( v.X, v.Y, v.Z );
        }

        static bool ValidateSetup( const IKAimJob &job )
        {
            if ( !job.setup )
            {
                LOG( ERROR ) << "IKAimJob: setup is null";
                return false;
            }

            auto &setupImpl    = *job.setup->m_impl;
            auto &skeletonImpl = *job.setup->GetSkeleton( )->m_impl;

            if ( job.jointIndex < 0 || job.jointIndex >= skeletonImpl.ozzSkeleton->num_joints( ) )
            {
                LOG( ERROR ) << "IKAimJob: joint index out of range";
                return false;
            }

            // Additional validations can be added as needed

            return true;
        }
    };

    IKAimJob::IKAimJob( ) :
        m_impl( new Impl( ) ), setup( nullptr ), jointIndex( -1 ), target{ 0.0f, 0.0f, 0.0f }, forward{ 0.0f, 0.0f, 1.0f }, up{ 0.0f, 1.0f, 0.0f }, maxAngle( 3.14159f ),
        weight( 1.0f ), alignedToModel( false ), twistAxis{ 0.0f, 0.0f, 0.0f }, poleVector{ 0.0f, 1.0f, 0.0f }
    {
    }

    IKAimJob::~IKAimJob( )
    {
        delete m_impl;
    }

    bool IKAimJob::Run( )
    {
        if ( !m_impl->ValidateSetup( *this ) )
        {
            return false;
        }

        auto &setupImpl    = *setup->m_impl;
        auto &skeletonImpl = *setup->GetSkeleton( )->m_impl;

        ozz::animation::IKAimJob ozzJob;
        ozzJob.skeleton         = skeletonImpl.ozzSkeleton.get( );
        ozzJob.joint            = static_cast<ozz::animation::IKAimJob::JointId>( jointIndex );
        ozzJob.local_transforms = ozz::make_span( setupImpl.localTransforms );
        ozzJob.target           = m_impl->ToOzzFloat3( target );
        ozzJob.forward          = m_impl->ToOzzFloat3( forward );
        ozzJob.up               = m_impl->ToOzzFloat3( up );
        ozzJob.max_angle        = maxAngle;
        ozzJob.weight           = weight;
        ozzJob.aligned_to_model = alignedToModel;
        ozzJob.twist_axis       = m_impl->ToOzzFloat3( twistAxis );
        ozzJob.pole_vector      = m_impl->ToOzzFloat3( poleVector );

        if ( !ozzJob.Run( ) )
        {
            LOG( ERROR ) << "IK Aim job failed";
            return false;
        }

        return true;
    }
} // namespace DenOfIz
