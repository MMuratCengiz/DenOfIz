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

#include <DenOfIzGraphics/Animation/IKTwoBoneJob.h>
#include <DenOfIzGraphics/Animation/Skeleton.h>
#include <glog/logging.h>

#include <ozz/animation/runtime/ik_two_bone_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/maths/quaternion.h>
#include <ozz/base/maths/vec_float.h>

namespace DenOfIz
{
    class IKTwoBoneJob::Impl
    {
    public:
        static ozz::math::Float3 ToOzzFloat3( const Float_3 &v )
        {
            return ozz::math::Float3( v.X, v.Y, v.Z );
        }

        static ozz::math::Quaternion ToOzzQuaternion( const Float_4 &q )
        {
            return ozz::math::Quaternion( q.X, q.Y, q.Z, q.W );
        }

        static bool ValidateSetup( const IKTwoBoneJob &job )
        {
            if ( !job.setup )
            {
                LOG( ERROR ) << "IKTwoBoneJob: setup is null";
                return false;
            }

            auto &setupImpl    = *job.setup->m_impl;
            auto &skeletonImpl = *job.setup->GetSkeleton( )->m_impl;

            const int numJoints = skeletonImpl.ozzSkeleton->num_joints( );

            if ( job.startJointIndex < 0 || job.startJointIndex >= numJoints )
            {
                LOG( ERROR ) << "IKTwoBoneJob: start joint index out of range";
                return false;
            }

            if ( job.midJointIndex < 0 || job.midJointIndex >= numJoints )
            {
                LOG( ERROR ) << "IKTwoBoneJob: mid joint index out of range";
                return false;
            }

            if ( job.endJointIndex < 0 || job.endJointIndex >= numJoints )
            {
                LOG( ERROR ) << "IKTwoBoneJob: end joint index out of range";
                return false;
            }

            const ozz::animation::Skeleton *ozzSkeleton = skeletonImpl.ozzSkeleton.get( );

            if ( ozzSkeleton->joint_parent( job.midJointIndex ) != job.startJointIndex )
            {
                LOG( ERROR ) << "IKTwoBoneJob: mid joint is not a child of start joint";
                return false;
            }

            if ( ozzSkeleton->joint_parent( job.endJointIndex ) != job.midJointIndex )
            {
                LOG( ERROR ) << "IKTwoBoneJob: end joint is not a child of mid joint";
                return false;
            }

            if ( job.softenDistance > 1.0f )
            {
                LOG( ERROR ) << "IKTwoBoneJob: soften distance must be less than or equal to 1.0";
                return false;
            }

            return true;
        }
    };

    IKTwoBoneJob::IKTwoBoneJob( ) :
        m_impl( new Impl( ) ), setup( nullptr ), startJointIndex( -1 ), midJointIndex( -1 ), endJointIndex( -1 ), target{ 0.0f, 0.0f, 0.0f },
        targetRotation{ 0.0f, 0.0f, 0.0f, 1.0f }, poleVector{ 0.0f, 1.0f, 0.0f }, weight( 1.0f ), twistAngle( 0.0f ), soften( false ), softenDistance( 0.8f ),
        allowStretching( false ), usePoleVector( false ), useTargetRotation( false )
    {
    }

    IKTwoBoneJob::~IKTwoBoneJob( )
    {
        delete m_impl;
    }

    bool IKTwoBoneJob::Run( )
    {
        if ( !m_impl->ValidateSetup( *this ) )
        {
            return false;
        }

        auto &setupImpl    = *setup->m_impl;
        auto &skeletonImpl = *setup->GetSkeleton( )->m_impl;

        ozz::animation::IKTwoBoneJob ozzJob;

        ozzJob.skeleton         = skeletonImpl.ozzSkeleton.get( );
        ozzJob.local_transforms = make_span( setupImpl.localTransforms );

        ozzJob.start_joint     = static_cast<ozz::animation::IKTwoBoneJob::JointId>( startJointIndex );
        ozzJob.mid_joint       = static_cast<ozz::animation::IKTwoBoneJob::JointId>( midJointIndex );
        ozzJob.end_joint       = static_cast<ozz::animation::IKTwoBoneJob::JointId>( endJointIndex );
        ozzJob.target          = m_impl->ToOzzFloat3( target );
        ozzJob.weight          = weight;
        ozzJob.twist_angle     = twistAngle;
        ozzJob.soften          = soften;
        ozzJob.soften_distance = softenDistance;
        ozzJob.stretching      = allowStretching;

        if ( usePoleVector )
        {
            ozzJob.pole_vector = m_impl->ToOzzFloat3( poleVector );
        }

        if ( useTargetRotation )
        {
            ozzJob.target_rotation = m_impl->ToOzzQuaternion( targetRotation );
        }

        if ( !ozzJob.Run( ) )
        {
            LOG( ERROR ) << "IK Two Bone job failed";
            return false;
        }

        return true;
    }
} // namespace DenOfIz
