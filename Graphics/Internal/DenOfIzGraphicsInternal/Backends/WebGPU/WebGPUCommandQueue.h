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

#include <DenOfIzGraphicsInternal/Backends/Interface/ICommandQueue.h>
#include <webgpu/webgpu.h>
#include "WebGPUContext.h"
#include "WebGPUFence.h"

namespace DenOfIz
{
    class WebGPUCommandQueue final : public ICommandQueue
    {
        WebGPUContext               *m_context;
        DenOfIz_CommandQueueDesc     m_desc;
        std::unique_ptr<WebGPUFence> m_waitIdleFence;

    public:
        WebGPUCommandQueue( WebGPUContext *context, const DenOfIz_CommandQueueDesc &desc );
        ~WebGPUCommandQueue( ) override;

        void WaitIdle( ) override;
        void ExecuteCommandLists( const DenOfIz_ExecuteCommandListsDesc &executeCommandListsDesc ) override;

        [[nodiscard]] WGPUQueue         GetNativeQueue( ) const;
        [[nodiscard]] DenOfIz_QueueType GetQueueType( ) const;
    };

} // namespace DenOfIz
