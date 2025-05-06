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

#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/maths/vec_float.h>
#include <ozz/base/span.h>

#include <DenOfIzGraphics/Animation/AnimationSetup.h>
#include <DenOfIzGraphics/Animation/Skeleton.h>
#include <glog/logging.h>

#include <DenOfIzGraphics/Utilities/InteropMathConverter.h>

namespace DenOfIz
{
    class AnimationSetup::Impl
    {
    public:
        Skeleton                                             *skeleton;
        ozz::vector<ozz::math::SoaTransform>                  localTransforms;
        ozz::vector<ozz::math::Float4x4>                      modelTransforms;
        std::unique_ptr<ozz::animation::SamplingJob::Context> context;

        static Float_3 FromOzzTranslation( const ozz::math::Float3 &translation )
        {
            return Float_3{ translation.x, translation.y, -translation.z };
        }

        static Float_4 FromOzzRotation( const ozz::math::Quaternion &rotation )
        {
            return Float_4{ -rotation.x, -rotation.y, rotation.z, rotation.w };
        }

        static Float_3 FromOzzScale( const ozz::math::Float3 &scale )
        {
            return Float_3{ scale.x, scale.y, scale.z };
        }
    };

    AnimationSetup::AnimationSetup( Skeleton *skeleton ) : m_impl( new Impl( ) )
    {
        m_impl->skeleton                             = skeleton;
        const ozz::animation::Skeleton *ozzSkeleton  = m_impl->skeleton->m_impl->ozzSkeleton.get( );
        const size_t                    numSoaJoints = ozzSkeleton->num_soa_joints( );
        m_impl->localTransforms.resize( numSoaJoints );

        for ( auto &transform : m_impl->localTransforms )
        {
            transform = ozz::math::SoaTransform::identity( );
        }

        m_impl->modelTransforms.resize( ozzSkeleton->num_joints( ) );
        m_impl->context = std::make_unique<ozz::animation::SamplingJob::Context>( );
        m_impl->context->Resize( ozzSkeleton->num_joints( ) );

        LOG( INFO ) << "Created animation setup for skeleton with " << ozzSkeleton->num_joints( ) << " joints";
    }

    AnimationSetup::~AnimationSetup( )
    {
        delete m_impl;
    }

    Skeleton *AnimationSetup::GetSkeleton( ) const
    {
        return m_impl->skeleton;
    }

    InteropArray<Float_4x4> AnimationSetup::GetLocalTransforms( )
    {
        InteropArray<Float_4x4> result;

        // Not implemented yet - we need to convert from SoA (Structure of Arrays) to AoS (Array of Structures)
        // This would be a complex conversion

        return result;
    }

    InteropArray<Float_4x4> AnimationSetup::GetModelTransforms( ) const
    {
        using namespace DirectX;

        InteropArray<Float_4x4> result;
        const size_t            numTransforms = m_impl->modelTransforms.size( );
        result.Resize( numTransforms );

        static const XMMATRIX correctionMatrix = XMMatrixRotationX( XM_PIDIV2 );

        for ( size_t i = 0; i < numTransforms; ++i )
        {
            const ozz::math::Float4x4 &ozzMat = m_impl->modelTransforms[ i ];
            Float_4x4                 &out    = result.GetElement( i );

            ozz::math::Float3     ozzTranslation;
            ozz::math::Quaternion ozzQuat;
            ozz::math::Float3     ozzScale;

            // Decomposing gives us a sure way of building a matrix due to differences in matrix layout
            if ( ozz::math::ToAffine( ozzMat, &ozzTranslation, &ozzQuat, &ozzScale ) )
            {
                const Float_3 translation = Impl::FromOzzTranslation( ozzTranslation );
                const Float_4 rotation    = Impl::FromOzzRotation( ozzQuat );
                const Float_3 scale       = Impl::FromOzzScale( ozzScale );

                // clang-format off
                XMMATRIX xmOut = XMMatrixAffineTransformation(
                    XMVectorSet(scale.X, scale.Y, scale.Z, 1.0f),
                    XMVectorZero(),
                    XMVectorSet(rotation.X, rotation.Y, rotation.Z, rotation.W),
                    XMVectorSet(translation.X, translation.Y, translation.Z, 1.0f)
                );
                // clang-format on

                xmOut = XMMatrixMultiply( xmOut, correctionMatrix );
                out   = InteropMathConverter::Float_4X4FromXMMATRIX( xmOut );
            }
        }

        return result;
    }

    int AnimationSetup::GetNumJoints( ) const
    {
        return m_impl->skeleton->GetNumJoints( );
    }
} // namespace DenOfIz
