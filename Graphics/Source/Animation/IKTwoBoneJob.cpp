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

#include <ozz/animation/runtime/ik_two_bone_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/maths/quaternion.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/simd_quaternion.h>

#include <DenOfIzGraphics/Animation/IKTwoBoneJob.h>
#include <DenOfIzGraphics/Animation/Skeleton.h>
#include <glog/logging.h>

namespace DenOfIz
{
    class IKTwoBoneJob::Impl
    {
    public:
        static ozz::math::SimdFloat4 ToOzzSimdFloat4( const Float_3 &v )
        {
            return ozz::math::simd_float4::Load3PtrU( &v.X );
        }

        static Float_4 FromOzzSimdQuaternion( const ozz::math::SimdQuaternion &q )
        {
            Float_4 result;
            float   values[ 4 ];
            ozz::math::StorePtrU( q.xyzw, values );
            result.X = values[ 0 ];
            result.Y = values[ 1 ];
            result.Z = values[ 2 ];
            result.W = values[ 3 ];
            return result;
        }

        static ozz::math::Float4x4 ToOzzFloat4x4( const Float_4x4 &m )
        {
            ozz::math::Float4x4 result;
            result.cols[ 0 ] = ozz::math::simd_float4::LoadXPtrU( &m._11 );
            result.cols[ 1 ] = ozz::math::simd_float4::LoadXPtrU( &m._21 );
            result.cols[ 2 ] = ozz::math::simd_float4::LoadXPtrU( &m._31 );
            result.cols[ 3 ] = ozz::math::simd_float4::LoadXPtrU( &m._41 );
            return result;
        }
    };

    IKTwoBoneJob::IKTwoBoneJob( const IKTwoBoneJobDesc &desc ) : m_desc( desc ), m_impl( new Impl( ) )
    {
    }

    IKTwoBoneJob::~IKTwoBoneJob( )
    {
        delete m_impl;
    }

    bool IKTwoBoneJob::Run( )
    {
        const ozz::math::Float4x4 startMatrix = m_impl->ToOzzFloat4x4( m_desc.StartJointMatrix );
        const ozz::math::Float4x4 midMatrix   = m_impl->ToOzzFloat4x4( m_desc.MidJointMatrix );
        const ozz::math::Float4x4 endMatrix   = m_impl->ToOzzFloat4x4( m_desc.EndJointMatrix );

        ozz::animation::IKTwoBoneJob ozzJob;

        ozzJob.target      = m_impl->ToOzzSimdFloat4( m_desc.Target );
        ozzJob.pole_vector = m_impl->ToOzzSimdFloat4( m_desc.PoleVector );
        ozzJob.mid_axis    = m_impl->ToOzzSimdFloat4( m_desc.MidAxis );
        ozzJob.twist_angle = m_desc.TwistAngle;
        ozzJob.soften      = m_desc.Soften;
        ozzJob.weight      = m_desc.Weight;

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
            LOG( ERROR ) << "IKTwoBoneJob: Validation failed";
            return false;
        }

        if ( !ozzJob.Run( ) )
        {
            LOG( ERROR ) << "IKTwoBoneJob: Execution failed";
            return false;
        }

        m_startJointCorrection = m_impl->FromOzzSimdQuaternion( startCorrection );
        m_midJointCorrection   = m_impl->FromOzzSimdQuaternion( midCorrection );
        m_reached              = targetReached;

        return true;
    }

    bool IKTwoBoneJob::GetReached( ) const
    {
        return m_reached;
    }

    Float_4 IKTwoBoneJob::GetStartJointCorrection( ) const
    {
        return m_startJointCorrection;
    }

    Float_4 IKTwoBoneJob::GetMidJointCorrection( ) const
    {
        return m_midJointCorrection;
    }
} // namespace DenOfIz
