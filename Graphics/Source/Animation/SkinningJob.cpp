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

#include <DenOfIzGraphics/Animation/Skeleton.h>
#include <DenOfIzGraphics/Animation/SkinningJob.h>
#include <glog/logging.h>

#include <ozz/animation/runtime/skinning_job.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/base/span.h>

using ozz::make_span;
using ozz::span;

namespace DenOfIz
{
    class SkinningJob::Impl
    {
    public:
        // Temporary buffers for conversion between engine and ozz formats
        ozz::vector<ozz::math::Float4x4>   ozzJointInverseBindPoses;
        ozz::vector<uint16_t>              ozzJointIndices;
        ozz::vector<ozz::math::Float3>     ozzInPositions;
        ozz::vector<ozz::math::Float3>     ozzInNormals;
        ozz::vector<ozz::math::Float3>     ozzInTangents;
        ozz::vector<ozz::math::SimdFloat4> ozzInWeights;
        ozz::vector<ozz::math::Float3>     ozzOutPositions;
        ozz::vector<ozz::math::Float3>     ozzOutNormals;
        ozz::vector<ozz::math::Float3>     ozzOutTangents;

        static ozz::math::Float3 ToOzzFloat3( const Float_3 &v )
        {
            return ozz::math::Float3( v.X, v.Y, v.Z );
        }

        static Float_3 FromOzzFloat3( const ozz::math::Float3 &v )
        {
            return Float_3{ v.x, v.y, v.z };
        }

        static bool ValidateSetup( const SkinningJob &job )
        {
            if ( !job.setup )
            {
                LOG( ERROR ) << "SkinningJob: setup is null";
                return false;
            }

            if ( job.jointIndices.NumElements( ) == 0 )
            {
                LOG( ERROR ) << "SkinningJob: No joint indices provided";
                return false;
            }

            if ( job.jointInverseBindPoses.NumElements( ) != job.jointIndices.NumElements( ) )
            {
                LOG( ERROR ) << "SkinningJob: Joint indices and inverse bind poses count mismatch";
                return false;
            }

            if ( job.in_positions.NumElements( ) == 0 )
            {
                LOG( ERROR ) << "SkinningJob: No input positions provided";
                return false;
            }

            if ( job.influences != Influences::One && job.influences != Influences::Two && job.influences != Influences::Four && job.influences != Influences::Dynamic )
            {
                LOG( ERROR ) << "SkinningJob: Invalid number of influences";
                return false;
            }

            if ( job.influences != Influences::One && job.in_weights.NumElements( ) == 0 )
            {
                LOG( ERROR ) << "SkinningJob: Weights are required for multiple influences";
                return false;
            }

            return true;
        }
    };

    SkinningJob::SkinningJob( ) : m_impl( new Impl( ) ), setup( nullptr ), influences( Influences::Four )
    {
    }

    SkinningJob::~SkinningJob( )
    {
        delete m_impl;
    }

    bool SkinningJob::Run( )
    {
        if ( !m_impl->ValidateSetup( *this ) )
        {
            return false;
        }

        auto &setupImpl = *static_cast<AnimationSetup::Impl *>( setup->m_impl );

        const size_t numVertices = in_positions.NumElements( );
        out_positions.Resize( numVertices );

        if ( in_normals.NumElements( ) > 0 )
        {
            out_normals.Resize( numVertices );
        }

        if ( in_tangents.NumElements( ) > 0 )
        {
            out_tangents.Resize( numVertices );
        }

        m_impl->ozzJointIndices.resize( jointIndices.NumElements( ) );
        for ( size_t i = 0; i < jointIndices.NumElements( ); ++i )
        {
            m_impl->ozzJointIndices[ i ] = static_cast<uint16_t>( jointIndices.GetElement( i ) );
        }

        m_impl->ozzJointInverseBindPoses.resize( jointInverseBindPoses.NumElements( ) );
        for ( size_t i = 0; i < jointInverseBindPoses.NumElements( ); ++i )
        {
            const Float_4x4     &ibp    = jointInverseBindPoses.GetElement( i );
            ozz::math::Float4x4 &ozzIbp = m_impl->ozzJointInverseBindPoses[ i ];

            ozzIbp.cols[ 0 ] = ozz::math::simd_float4::Load( ibp._11, ibp._21, ibp._31, ibp._41 );
            ozzIbp.cols[ 1 ] = ozz::math::simd_float4::Load( ibp._12, ibp._22, ibp._32, ibp._42 );
            ozzIbp.cols[ 2 ] = ozz::math::simd_float4::Load( ibp._13, ibp._23, ibp._33, ibp._43 );
            ozzIbp.cols[ 3 ] = ozz::math::simd_float4::Load( ibp._14, ibp._24, ibp._34, ibp._44 );
        }

        m_impl->ozzInPositions.resize( numVertices );
        for ( size_t i = 0; i < numVertices; ++i )
        {
            m_impl->ozzInPositions[ i ] = m_impl->ToOzzFloat3( in_positions.GetElement( i ) );
        }

        m_impl->ozzOutPositions.resize( numVertices );

        const bool hasNormals = in_normals.NumElements( ) > 0;
        if ( hasNormals )
        {
            m_impl->ozzInNormals.resize( numVertices );
            m_impl->ozzOutNormals.resize( numVertices );
            for ( size_t i = 0; i < numVertices; ++i )
            {
                m_impl->ozzInNormals[ i ] = m_impl->ToOzzFloat3( in_normals.GetElement( i ) );
            }
        }

        const bool hasTangents = in_tangents.NumElements( ) > 0;
        if ( hasTangents )
        {
            m_impl->ozzInTangents.resize( numVertices );
            m_impl->ozzOutTangents.resize( numVertices );
            for ( size_t i = 0; i < numVertices; ++i )
            {
                m_impl->ozzInTangents[ i ] = m_impl->ToOzzFloat3( in_tangents.GetElement( i ) );
            }
        }

        if ( influences != Influences::One )
        {
            m_impl->ozzInWeights.resize( numVertices );

            for ( size_t i = 0; i < numVertices; ++i )
            {
                const Float_4 &w          = in_weights.GetElement( i );
                m_impl->ozzInWeights[ i ] = ozz::math::simd_float4::Load( w.X, w.Y, w.Z, w.W );
            }
        }

        // Create ozz skinning job
        ozz::animation::SkinningJob ozzJob;
        ozzJob.joint_matrices           = make_span( setupImpl.modelTransforms );
        ozzJob.joint_indices            = make_span( m_impl->ozzJointIndices );
        ozzJob.joint_inverse_bind_poses = make_span( m_impl->ozzJointInverseBindPoses );
        ozzJob.in.positions             = make_span( m_impl->ozzInPositions );
        if ( hasNormals )
        {
            ozzJob.in.normals = make_span( m_impl->ozzInNormals );
        }
        if ( hasTangents )
        {
            ozzJob.in.tangents = make_span( m_impl->ozzInTangents );
        }
        ozzJob.out.positions = make_span( m_impl->ozzOutPositions );
        if ( hasNormals )
        {
            ozzJob.out.normals = make_span( m_impl->ozzOutNormals );
        }
        if ( hasTangents )
        {
            ozzJob.out.tangents = make_span( m_impl->ozzOutTangents );
        }

        switch ( influences )
        {
        case Influences::One:
            ozzJob.influences = ozz::animation::SkinningJob::Influences::kOne;
            break;
        case Influences::Two:
            ozzJob.influences = ozz::animation::SkinningJob::Influences::kTwo;
            ozzJob.in.weights = make_span( m_impl->ozzInWeights );
            break;
        case Influences::Four:
            ozzJob.influences = ozz::animation::SkinningJob::Influences::kFour;
            ozzJob.in.weights = make_span( m_impl->ozzInWeights );
            break;
        case Influences::Dynamic:
            ozzJob.influences = ozz::animation::SkinningJob::Influences::kDynamic;
            ozzJob.in.weights = make_span( m_impl->ozzInWeights );
            break;
        }

        if ( !ozzJob.Run( ) )
        {
            LOG( ERROR ) << "Skinning failed";
            return false;
        }

        for ( size_t i = 0; i < numVertices; ++i )
        {
            out_positions.GetElement( i ) = m_impl->FromOzzFloat3( m_impl->ozzOutPositions[ i ] );
        }

        if ( hasNormals )
        {
            for ( size_t i = 0; i < numVertices; ++i )
            {
                out_normals.GetElement( i ) = m_impl->FromOzzFloat3( m_impl->ozzOutNormals[ i ] );
            }
        }

        if ( hasTangents )
        {
            for ( size_t i = 0; i < numVertices; ++i )
            {
                out_tangents.GetElement( i ) = m_impl->FromOzzFloat3( m_impl->ozzOutTangents[ i ] );
            }
        }

        return true;
    }
} // namespace DenOfIz
