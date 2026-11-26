/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/ISemaphore.h>
#include "MetalContext.h"

namespace DenOfIz
{

    class MetalSemaphore final : public ISemaphore
    {
    private:
        constexpr static uint64_t MAX_FENCE_VALUE = 1000000;

        MetalContext     *m_context;
        id<MTLEvent>      m_fence;
        uint64_t          m_fenceValue = 0;
        std::atomic<bool> m_signaled   = false;

    public:
        MetalSemaphore( MetalContext *context );
        ~MetalSemaphore( ) override = default;
        void               Notify( ) override;
        bool               IsCompleted( ) const override;
        [[nodiscard]] bool IsSignaled( ) const;
        void               ResetSignaled( );
        void               WaitFor( id<MTLCommandBuffer> commandBuffer );
        void               NotifyOnCommandBufferCompletion( const id<MTLCommandBuffer> &commandBuffer );
    };
} // namespace DenOfIz
