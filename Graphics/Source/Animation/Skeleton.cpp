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
#include <glog/logging.h>

#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/containers/vector.h>
#include <unordered_map>

namespace DenOfIz
{
    class Skeleton::Impl
    {
    public:
        ozz::unique_ptr<ozz::animation::Skeleton> ozzSkeleton;
        std::unordered_map<std::string, int>      jointNameToIndex;

        static ozz::math::Transform GetJointLocalTransform( const Joint &joint )
        {
            ozz::math::Transform transform;
            transform.translation = ozz::math::Float3( joint.LocalTranslation.X, joint.LocalTranslation.Y, -joint.LocalTranslation.Z );
            transform.rotation    = ozz::math::Quaternion( -joint.LocalRotationQuat.X, -joint.LocalRotationQuat.Y, joint.LocalRotationQuat.Z, joint.LocalRotationQuat.W );
            transform.scale       = ozz::math::Float3( joint.LocalScale.X, joint.LocalScale.Y, joint.LocalScale.Z );
            return transform;
        }
    };

    Skeleton::Skeleton( const SkeletonAsset &skeletonAsset ) : m_impl( new Impl( ) )
    {
        m_impl->ozzSkeleton = ozz::make_unique<ozz::animation::Skeleton>( );

        const auto  &joints    = skeletonAsset.Joints;
        const size_t numJoints = joints.NumElements( );

        ozz::vector<int16_t> parents( numJoints );

        ozz::animation::offline::RawSkeleton rawSkeleton;
        for ( size_t i = 0; i < numJoints; ++i )
        {
            const Joint &joint = joints.GetElement( i );
            parents[ i ]       = joint.ParentIndex;

            if ( joint.ParentIndex == -1 )
            {
                ozz::animation::offline::RawSkeleton::Joint rootJoint;
                rootJoint.name      = joint.Name.Get( );
                rootJoint.transform = Impl::GetJointLocalTransform( joint );

                rawSkeleton.roots.push_back( rootJoint );
            }
        }

        for ( size_t i = 0; i < numJoints; ++i )
        {
            if ( parents[ i ] < 0 )
            {
                continue;
            }

            std::function<ozz::animation::offline::RawSkeleton::Joint *( int32_t, ozz::vector<ozz::animation::offline::RawSkeleton::Joint> & )> FindJoint;
            FindJoint = [ & ]( const int32_t targetIdx, ozz::vector<ozz::animation::offline::RawSkeleton::Joint> &children ) -> ozz::animation::offline::RawSkeleton::Joint *
            {
                for ( auto &child : children )
                {
                    if ( child.name == joints.GetElement( targetIdx ).Name.Get( ) )
                    {
                        return &child;
                    }

                    if ( const auto foundInChildren = FindJoint( targetIdx, child.children ) )
                    {
                        return foundInChildren;
                    }
                }
                return nullptr;
            };

            ozz::animation::offline::RawSkeleton::Joint *parentJoint = nullptr;
            for ( auto &root : rawSkeleton.roots )
            {
                if ( root.name == joints.GetElement( parents[ i ] ).Name.Get( ) )
                {
                    parentJoint = &root;
                    break;
                }

                parentJoint = FindJoint( parents[ i ], root.children );
                if ( parentJoint )
                {
                    break;
                }
            }

            if ( parentJoint )
            {
                const Joint &joint = joints.GetElement( i );

                ozz::animation::offline::RawSkeleton::Joint childJoint;
                childJoint.name      = joint.Name.Get( );
                childJoint.transform = Impl::GetJointLocalTransform( joint );

                parentJoint->children.push_back( childJoint );
            }
        }

        ozz::animation::offline::SkeletonBuilder builder;
        if ( auto builtSkeleton = builder( rawSkeleton ) )
        {
            m_impl->ozzSkeleton = std::move( builtSkeleton );
            for ( int i = 0; i < m_impl->ozzSkeleton->num_joints( ); ++i )
            {
                m_impl->jointNameToIndex[ m_impl->ozzSkeleton->joint_names( )[ i ] ] = i;
            }

            LOG( INFO ) << "Successfully created skeleton with " << m_impl->ozzSkeleton->num_joints( ) << " joints";
        }
        else
        {
            LOG( ERROR ) << "Failed to build ozz skeleton";
        }
    }

    Skeleton::~Skeleton( )
    {
        delete m_impl;
    }

    int Skeleton::GetNumJoints( ) const
    {
        return m_impl->ozzSkeleton->num_joints( );
    }

    bool Skeleton::HasJoint( const InteropString &jointName ) const
    {
        return m_impl->jointNameToIndex.contains( jointName.Get( ) );
    }

    int Skeleton::GetJointIndex( const InteropString &jointName ) const
    {
        const auto it = m_impl->jointNameToIndex.find( jointName.Get( ) );
        return it != m_impl->jointNameToIndex.end( ) ? it->second : -1;
    }

    InteropString Skeleton::GetJointName( const int jointIndex ) const
    {
        if ( jointIndex >= 0 && jointIndex < m_impl->ozzSkeleton->num_joints( ) )
        {
            return InteropString( m_impl->ozzSkeleton->joint_names( )[ jointIndex ] );
        }
        return InteropString( "" );
    }
} // namespace DenOfIz
