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

#include "ICommandList.h"
#include "ISemaphore.h"

namespace DenOfIz
{
    struct DZ_API CommandQueueFlags
    {
        bool RequirePresentationSupport = false;
    };

    enum class QueuePriority
    {
        Low,
        Normal,
        High,
    };

    struct DZ_API CommandQueueDesc
    {
        QueueType         QueueType;
        QueuePriority     Priority = QueuePriority::Normal;
        CommandQueueFlags Flags;
    };

    struct DZ_API ExecuteCommandListsDesc
    {
        IFence           *Signal;
        ICommandListArray CommandLists;
        ISemaphoreArray   WaitSemaphores;
        ISemaphoreArray   SignalSemaphores;
    };

    class DZ_API ICommandQueue
    {
    public:
        virtual ~ICommandQueue( )                                                                  = default;
        virtual void WaitIdle( )                                                                   = 0;
        virtual void ExecuteCommandLists( const ExecuteCommandListsDesc &executeCommandListsDesc ) = 0;
    };
} // namespace DenOfIz
