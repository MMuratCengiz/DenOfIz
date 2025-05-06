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

#include <DenOfIzGraphics/Animation/LocalToModelJob.h>
#include <DenOfIzGraphics/Animation/Skeleton.h>
#include <glog/logging.h>

#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/span.h>

using ozz::make_span;
using ozz::span;

namespace DenOfIz
{
    class LocalToModelJob::Impl
    {
    public:
        static bool ValidateSetup( const LocalToModelJob &job )
        {
            if ( !job.setup )
            {
                LOG( ERROR ) << "LocalToModelJob: setup is null";
                return false;
            }

            return true;
        }
    };

    LocalToModelJob::LocalToModelJob( ) : m_impl( new Impl( ) ), setup( nullptr )
    {
    }

    LocalToModelJob::~LocalToModelJob( )
    {
        delete m_impl;
    }

    bool LocalToModelJob::Run( )
    {
        if ( !m_impl->ValidateSetup( *this ) )
        {
            return false;
        }

        auto &setupImpl    = *setup->m_impl;
        auto &skeletonImpl = *setup->GetSkeleton( )->m_impl;

        ozz::animation::LocalToModelJob ozzJob;
        ozzJob.skeleton = skeletonImpl.ozzSkeleton.get( );
        ozzJob.input    = make_span( setupImpl.localTransforms );
        ozzJob.output   = make_span( setupImpl.modelTransforms );

        if ( !ozzJob.Run( ) )
        {
            LOG( ERROR ) << "Failed to convert local to model space transforms";
            return false;
        }

        return true;
    }
} // namespace DenOfIz
