/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <deque>
#include <functional>
#include <mutex>
#include <vector>

namespace DenOfIz
{
    class DeferredDestroyQueue;

    // An object never attached to a queue (created internally, or by a backend without deferred destruction) is deleted on the spot.
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

    // Destroyed objects are tied to everything submitted to the GPU by the next frame boundary, not by the Destroy call, so command lists
    // recorded earlier in the frame and submitted before the boundary are covered too.
    class DeferredDestroyQueue
    {
    public:
        // Must also return true when the device was lost.
        using Completion = std::function<bool( )>;
        // Captures everything submitted to any queue of the device up to this call.
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

        // A null queue leaves the object as it is.
        static void Attach( DeferredDestroyQueue *queue, IDeviceObject *object );
        // Safe to call from any thread.
        static void Destroy( IDeviceObject *object );

        // Frame boundary: nothing that is still being recorded references the objects destroyed so far.
        void EndFrame( );
        // Deletes what earlier frames' GPU work is done with, without declaring a frame boundary.
        void Collect( );
        // The caller guarantees that the GPU is idle and that nothing recorded references the objects.
        void ReleaseAll( );

    private:
        void TakeCompleted( std::vector<IDeviceObject *> &outObjects );
    };
} // namespace DenOfIz
