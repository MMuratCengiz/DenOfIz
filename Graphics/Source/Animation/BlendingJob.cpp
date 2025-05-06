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

#include <DenOfIzGraphics/Animation/BlendingJob.h>
#include <DenOfIzGraphics/Animation/Skeleton.h>
#include <glog/logging.h>

#include <ozz/animation/runtime/blending_job.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/span.h>

using ozz::make_span;
using ozz::span;

namespace DenOfIz
{
    class BlendingJob::Impl
    {
    public:
        ozz::vector<ozz::animation::BlendingJob::Layer> ozzLayers;

        static bool ValidateSetup( const BlendingJob &job )
        {
            if ( job.layers.NumElements( ) == 0 )
            {
                LOG( ERROR ) << "BlendingJob: No layers provided";
                return false;
            }

            if ( !job.output )
            {
                LOG( ERROR ) << "BlendingJob: output setup is null";
                return false;
            }

            const Skeleton *baseSkeleton = job.output->GetSkeleton( );
            for ( size_t i = 0; i < job.layers.NumElements( ); ++i )
            {
                const BlendingLayer &layer = job.layers.GetElement( i );

                if ( !layer.setup )
                {
                    LOG( ERROR ) << "BlendingJob: layer " << i << " has null setup";
                    return false;
                }

                if ( layer.setup->GetSkeleton( ) != baseSkeleton )
                {
                    LOG( ERROR ) << "BlendingJob: layer " << i << " uses a different skeleton";
                    return false;
                }
            }

            return true;
        }
    };

    BlendingJob::BlendingJob( ) : m_impl( new Impl( ) ), threshold( 0.1f ), output( nullptr )
    {
    }

    BlendingJob::~BlendingJob( )
    {
        delete m_impl;
    }

    bool BlendingJob::Run( )
    {
        if ( !m_impl->ValidateSetup( *this ) )
        {
            return false;
        }

        m_impl->ozzLayers.clear( );
        m_impl->ozzLayers.resize( layers.NumElements( ) );

        auto &outputImpl   = *output->m_impl;
        auto &skeletonImpl = *output->GetSkeleton( )->m_impl;

        ozz::animation::BlendingJob ozzJob;
        ozzJob.threshold = threshold;

        for ( size_t i = 0; i < layers.NumElements( ); ++i )
        {
            const BlendingLayer                &srcLayer = layers.GetElement( i );
            ozz::animation::BlendingJob::Layer &ozzLayer = m_impl->ozzLayers[ i ];

            auto &setupImpl = *srcLayer.setup->m_impl;

            ozzLayer.transform = make_span( setupImpl.localTransforms );
            ozzLayer.weight    = srcLayer.weight;
        }

        ozzJob.layers    = make_span( m_impl->ozzLayers );
        ozzJob.rest_pose = skeletonImpl.ozzSkeleton->joint_rest_poses( );
        ozzJob.output    = make_span( outputImpl.localTransforms );

        if ( !ozzJob.Run( ) )
        {
            LOG( ERROR ) << "Animation blending failed";
            for ( size_t i = 0; i < outputImpl.localTransforms.size( ); ++i )
            {
                outputImpl.localTransforms[ i ] = skeletonImpl.ozzSkeleton->joint_rest_poses( )[ i ];
            }

            return false;
        }

        return true;
    }
} // namespace DenOfIz
