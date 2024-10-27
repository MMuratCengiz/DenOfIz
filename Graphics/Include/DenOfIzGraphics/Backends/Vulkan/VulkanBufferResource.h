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

#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanContext.h>

namespace DenOfIz
{

    class VulkanBufferResource final : public IBufferResource, NonCopyable
    {
        BufferDesc            m_desc{ };
        VulkanContext        *m_context = nullptr;
        VmaAllocation         m_allocation{ };
        VkBuffer              m_instance{ };
        size_t                m_offset{ };
        size_t                m_numBytes{ };
        const void           *m_data         = nullptr;
        void                 *m_mappedMemory = nullptr;
        VkDeviceAddress       m_deviceAddress{ };
        BitSet<ResourceState> m_state;

    public:
        void *MapMemory( ) override;
        void  UnmapMemory( ) override;

        explicit VulkanBufferResource( VulkanContext *context, BufferDesc desc );
        ~VulkanBufferResource( ) override;

        [[nodiscard]] BitSet<ResourceState> InitialState( ) const override;
        [[nodiscard]] size_t                NumBytes( ) const override;
        [[nodiscard]] const void           *Data( ) const override;

        [[nodiscard]] size_t                 Offset( ) const;
        [[nodiscard]] const VkBuffer        &Instance( ) const;
        [[nodiscard]] const VkDeviceAddress &DeviceAddress( ) const;

        // Interop API
        [[nodiscard]] InteropArray<Byte> GetData( ) const override;
        void                             SetData( const InteropArray<Byte> &data, bool keepMapped ) override;
    };

} // namespace DenOfIz
