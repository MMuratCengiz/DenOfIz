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

#include <DenOfIzGraphicsInternal/Backends/Interface/IFence.h>
#include <condition_variable>
#include <memory>
#include <mutex>
#include "MetalContext.h"

namespace DenOfIz
{

    //! Timeline-style fence: every submission that signals this fence gets a monotonically increasing value,
    //! Wait() blocks until the GPU has completed the most recently submitted value (matches DX12Fence semantics).
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
        std::shared_ptr<State> m_state; // shared with completion handlers so they outlive the fence safely

    public:
        MetalFence( MetalContext *context );
        ~MetalFence( ) override;
        void Wait( ) override;
        void Reset( ) override;
        void Notify( );

        void NotifyOnCommandBufferCompletion( const id<MTLCommandBuffer> &commandBuffer );
    };

} // namespace DenOfIz
