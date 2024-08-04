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

#include "../Interface/ISemaphore.h"
#include "VulkanContext.h"

namespace DenOfIz
{

    class VulkanSemaphore final : public ISemaphore
    {
        VulkanContext *m_context;
        VkSemaphore    m_semaphore{ };
        uint64_t       m_value = 0;

    public:
        explicit VulkanSemaphore( VulkanContext *context );
        ~VulkanSemaphore( ) override;
        void Wait( ) override;
        void Notify( ) override;

        [[nodiscard]] VkSemaphore GetSemaphore( ) const
        {
            return m_semaphore;
        }
    };

} // namespace DenOfIz
