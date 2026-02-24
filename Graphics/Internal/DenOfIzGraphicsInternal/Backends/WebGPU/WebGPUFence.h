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
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "WebGPUContext.h"

namespace DenOfIz
{
    class WebGPUFence final : public IFence
    {
        WebGPUContext          *m_context;
        std::atomic<bool>       m_signaled{ true };
        std::mutex              m_mutex;
        std::condition_variable m_cv;
        bool                    m_callbackRegistered{ false };

    public:
        explicit WebGPUFence( WebGPUContext *context );
        ~WebGPUFence( ) override;

        void Wait( ) override;
        void Reset( ) override;

        void        PrepareForSignal( );
#if DZ_WEBGPU_USE_DAWN_API
        static void OnWorkDone( WGPUQueueWorkDoneStatus status, void *userdata1, void *userdata2 );
#else
        static void OnWorkDone( WGPUQueueWorkDoneStatus status, void *userdata );
#endif
    };
} // namespace DenOfIz
