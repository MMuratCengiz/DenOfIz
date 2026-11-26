/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/IFence.h>
#include <condition_variable>
#include <memory>
#include <mutex>
#include "MetalContext.h"

namespace DenOfIz
{

    class MetalFence final : public IFence
    {
        struct State
        {
            std::mutex              Mutex;
            std::condition_variable Condition;
            uint64_t                SubmittedValue = 0;
            uint64_t                CompletedValue = 0;
        };

        MetalContext          *m_context;
        std::shared_ptr<State> m_state;

    public:
        MetalFence( MetalContext *context );
        ~MetalFence( ) override;
        void Wait( ) override;
        void Reset( ) override;
        void Notify( );

        void NotifyOnCommandBufferCompletion( const id<MTLCommandBuffer> &commandBuffer );
    };

} // namespace DenOfIz
