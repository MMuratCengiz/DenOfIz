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

    class DeferredDestroyQueue
    {
    public:
        using Completion        = std::function<bool( )>;
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

        static void Attach( DeferredDestroyQueue *queue, IDeviceObject *object );
        static void Destroy( IDeviceObject *object );

        void EndFrame( );
        void Collect( );
        void ReleaseAll( );

    private:
        void TakeCompleted( std::vector<IDeviceObject *> &outObjects );
    };
} // namespace DenOfIz
