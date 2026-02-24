/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

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

#include <string>
#include "DenOfIzGraphicsInternal/Backends/Interface/IBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanContext.h"

namespace DenOfIz
{
    class VulkanBuffer final : public IBuffer
    {
        DenOfIz_BufferDesc m_desc{ };
        std::string        m_debugName;
        VulkanContext     *m_context = nullptr;
        VmaAllocation      m_allocation{ };
        VkBuffer           m_instance{ };
        size_t             m_offset{ };
        size_t             m_numBytes{ };
        const void        *m_data         = nullptr;
        void              *m_mappedMemory = nullptr;
        VkDeviceAddress    m_deviceAddress{ };
        uint32_t           m_state;

    public:
        void *MapMemory( ) override;
        void  UnmapMemory( ) override;

        explicit VulkanBuffer( VulkanContext *context, DenOfIz_BufferDesc desc );
        ~VulkanBuffer( ) override;

        [[nodiscard]] size_t      NumBytes( ) const override;
        [[nodiscard]] const void *Data( ) const override;

        [[nodiscard]] size_t                 Offset( ) const;
        [[nodiscard]] const VkBuffer        &Instance( ) const;
        [[nodiscard]] const VkDeviceAddress &DeviceAddress( ) const;

        // Interop API
        [[nodiscard]] DenOfIz_ByteArray GetData( ) const override;
        void                            SetData( const DenOfIz_ByteArrayView &data, bool keepMapped ) override;
        void                            WriteData( const DenOfIz_ByteArrayView &data, uint32_t bufferOffset ) override;
    };

} // namespace DenOfIz
