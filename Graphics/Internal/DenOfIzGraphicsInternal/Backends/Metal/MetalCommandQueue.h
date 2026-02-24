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
#include "MetalCommandList.h"
#include "MetalContext.h"

namespace DenOfIz
{
    class MetalCommandQueue : public ICommandQueue
    {
    private:
        MetalContext            *m_context;
        DenOfIz_CommandQueueDesc m_desc;
        id<MTLCommandQueue>      m_queue;

    public:
        MetalCommandQueue( MetalContext *context, const DenOfIz_CommandQueueDesc &desc );
        ~MetalCommandQueue( ) override;

        void WaitIdle( ) override;
        void ExecuteCommandLists( const DenOfIz_ExecuteCommandListsDesc &executeCommandListsDesc ) override;

        id<MTLCommandQueue> GetQueue( ) const
        {
            return m_queue;
        }
        DenOfIz_QueueType GetQueueType( ) const
        {
            return m_desc.QueueType;
        }
    };
} // namespace DenOfIz
