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

#include <DenOfIzGraphics/Animation/MotionBlendingJob.h>
#include <glog/logging.h>

#include <ozz/animation/runtime/motion_blending_job.h>
#include <ozz/base/maths/quaternion.h>
#include <ozz/base/maths/vec_float.h>

namespace DenOfIz
{
    class MotionBlendingJob::Impl
    {
    public:
        ozz::vector<ozz::animation::MotionBlendingJob::Spec> ozzSpecs;

        static ozz::math::Float3 ToOzzFloat3( const Float_3 &v )
        {
            return ozz::math::Float3( v.X, v.Y, v.Z );
        }

        static ozz::math::Quaternion ToOzzQuaternion( const Float_4 &q )
        {
            return ozz::math::Quaternion( q.X, q.Y, q.Z, q.W );
        }

        static Float_3 FromOzzFloat3( const ozz::math::Float3 &v )
        {
            return Float_3{ v.x, v.y, v.z };
        }

        static Float_4 FromOzzQuaternion( const ozz::math::Quaternion &q )
        {
            return Float_4{ q.x, q.y, q.z, q.w };
        }

        static bool ValidateSetup( const MotionBlendingJob &job )
        {
            if ( job.inputs.NumElements( ) == 0 )
            {
                LOG( ERROR ) << "MotionBlendingJob: No inputs provided";
                return false;
            }

            // Additional validations can be added as needed

            return true;
        }
    };

    MotionBlendingJob::MotionBlendingJob( ) : m_impl( new Impl( ) ), threshold( 0.1f ), outTranslation{ 0.0f, 0.0f, 0.0f }, outRotation{ 0.0f, 0.0f, 0.0f, 1.0f }
    {
    }

    MotionBlendingJob::~MotionBlendingJob( )
    {
        delete m_impl;
    }

    bool MotionBlendingJob::Run( )
    {
        if ( !m_impl->ValidateSetup( *this ) )
        {
            return false;
        }

        const size_t numInputs = inputs.NumElements( );
        m_impl->ozzSpecs.resize( numInputs );

        for ( size_t i = 0; i < numInputs; ++i )
        {
            const MotionBlendSpec                   &spec    = inputs.GetElement( i );
            ozz::animation::MotionBlendingJob::Spec &ozzSpec = m_impl->ozzSpecs[ i ];

            ozzSpec.weight      = spec.weight;
            ozzSpec.translation = m_impl->ToOzzFloat3( spec.translation );
            ozzSpec.rotation    = m_impl->ToOzzQuaternion( spec.rotation );
        }

        ozz::animation::MotionBlendingJob ozzJob;
        ozzJob.inputs    = make_span( m_impl->ozzSpecs );
        ozzJob.threshold = threshold;

        ozz::math::Float3     blendedTranslation;
        ozz::math::Quaternion blendedRotation;

        ozzJob.output_translation = &blendedTranslation;
        ozzJob.output_rotation    = &blendedRotation;

        if ( !ozzJob.Run( ) )
        {
            LOG( ERROR ) << "Motion blending failed";
            return false;
        }

        outTranslation = m_impl->FromOzzFloat3( blendedTranslation );
        outRotation    = m_impl->FromOzzQuaternion( blendedRotation );
        return true;
    }
} // namespace DenOfIz
