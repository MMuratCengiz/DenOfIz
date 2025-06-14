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

#include "DenOfIzGraphics/Backends/Interface/IFence.h"
#include "VulkanContext.h"

namespace DenOfIz
{

    class VulkanFence final : public IFence
    {
        VulkanContext *m_context;
        VkFence        m_fence{};

    public:
        explicit              VulkanFence( VulkanContext *context );
        ~                     VulkanFence( ) override;
        void                  Wait( ) override;
        void                  Reset( ) override;
        [[nodiscard]] VkFence GetFence( ) const;
    };

} // namespace DenOfIz
