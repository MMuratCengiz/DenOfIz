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

#pragma once

#include <deque>
#include <functional>
#include <mutex>
#include <vector>

namespace DenOfIz
{
    class DeferredDestroyQueue;

    //! Base of the objects a logical device hands to the application. It only carries the queue the object is released through, an object
    //! that was never attached to one (created internally, or by a backend without deferred destruction) is deleted on the spot.
    class IDeviceObject
    {
        friend class DeferredDestroyQueue;
        DeferredDestroyQueue *m_destroyQueue = nullptr;

    public:
        virtual ~IDeviceObject( ) = default;

        [[nodiscard]] DeferredDestroyQueue *DestroyQueue( ) const
        {
            return m_destroyQueue;
        }
    };

    //! Keeps destroyed objects alive until the GPU can no longer be using them.
    //!
    //! Destroying an object only queues it. At the next frame boundary the queued objects are tied to everything that has been submitted to
    //! the GPU so far, and they are deleted once that work has finished. This covers work that is already executing as well as command lists
    //! that were recorded earlier in the frame and are submitted before the frame boundary.
    class DeferredDestroyQueue
    {
    public:
        //! Returns true once the GPU work it was captured for has finished. Must also return true when the device was lost.
        using Completion = std::function<bool( )>;
        //! Captures everything submitted to any queue of the device up to this call.
        using CaptureCompletion = std::function<Completion( )>;

    private:
        struct Batch
        {
            std::vector<IDeviceObject *> Objects;
            Completion                   IsComplete;
        };

        CaptureCompletion            m_captureCompletion;
        std::mutex                   m_mutex;
        std::vector<IDeviceObject *> m_open;
        std::deque<Batch>            m_closed;

    public:
        explicit DeferredDestroyQueue( CaptureCompletion captureCompletion );
        ~DeferredDestroyQueue( );

        DeferredDestroyQueue( const DeferredDestroyQueue & )            = delete;
        DeferredDestroyQueue &operator=( const DeferredDestroyQueue & ) = delete;

        //! Objects destroyed through Destroy( ) after this call are released by this queue. A null queue leaves the object as it is.
        static void Attach( DeferredDestroyQueue *queue, IDeviceObject *object );
        //! Queues the object when it is attached to a queue, deletes it right away otherwise. Safe to call from any thread.
        static void Destroy( IDeviceObject *object );

        //! Frame boundary: nothing that is still being recorded references the objects destroyed so far. Deletes what the GPU is done with.
        void EndFrame( );
        //! Deletes the objects of earlier frames the GPU is done with, without declaring a frame boundary.
        void Collect( );
        //! Deletes everything right now. The caller guarantees that the GPU is idle and that nothing recorded references the objects.
        void ReleaseAll( );

    private:
        void TakeCompleted( std::vector<IDeviceObject *> &outObjects );
    };
} // namespace DenOfIz
