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

#include <DenOfIzGraphics/Backends/Interface/ICommandQueue.h>
#include "VulkanCommandList.h"
#include "VulkanContext.h"

namespace DenOfIz
{
    class DZ_API VulkanCommandQueue final : public ICommandQueue
    {
        VulkanContext   *m_context;
        CommandQueueDesc m_desc;
        VkQueue          m_queue;
        uint32_t         m_queueFamilyIndex;
        uint32_t         m_queueIndex;

    public:
        VulkanCommandQueue( VulkanContext *context, const CommandQueueDesc &desc );
        ~VulkanCommandQueue( ) override;

        void          WaitIdle( ) override;
        void          ExecuteCommandLists( const ExecuteCommandListsDesc &executeCommandListsDesc ) override;

        [[nodiscard]] uint32_t  GetQueueFamilyIndex( ) const;
        [[nodiscard]] VkQueue   GetQueue( ) const;
        [[nodiscard]] QueueType GetQueueType( ) const;

    private:
        void FindQueueFamilyIndex( VkQueueFlags requiredFlags );
    };
} // namespace DenOfIz
