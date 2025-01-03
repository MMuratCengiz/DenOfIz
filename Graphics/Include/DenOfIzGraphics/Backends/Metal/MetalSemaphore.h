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

#include <DenOfIzGraphics/Backends/Interface/ISemaphore.h>
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
        void Notify( ) override;
        void WaitFor( id<MTLCommandBuffer> commandBuffer );
        void NotifyOnCommandBufferCompletion( const id<MTLCommandBuffer> &commandBuffer );

        [[nodiscard]] bool IsSignaled( ) const
        {
            return m_signaled;
        }
        [[nodiscard]] const id<MTLEvent> &GetFence( ) const
        {
            return m_fence;
        }
    };
} // namespace DenOfIz
