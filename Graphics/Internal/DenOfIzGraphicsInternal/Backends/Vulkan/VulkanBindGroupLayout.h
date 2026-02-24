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

#include <unordered_map>
#include "DenOfIzGraphicsInternal/Backends/Interface/IBindGroupLayout.h"
#include "VulkanContext.h"

namespace DenOfIz
{

    class VulkanBindGroupLayout final : public IBindGroupLayout
    {
    private:
        VulkanContext              *m_context;
        DenOfIz_BindGroupLayoutDesc m_desc;

        VkDescriptorSetLayout                             m_layout{ VK_NULL_HANDLE };
        std::vector<VkDescriptorSetLayoutBinding>         m_layoutBindings;
        std::vector<VkDescriptorBindingFlags>             m_bindingFlags;
        std::unordered_map<uint32_t, DenOfIz_BindingDesc> m_resourceBindingMap;

    public:
        VulkanBindGroupLayout( VulkanContext *context, const DenOfIz_BindGroupLayoutDesc &desc );
        ~VulkanBindGroupLayout( ) override;

        [[nodiscard]] const DenOfIz_BindGroupLayoutDesc &Desc( ) const;
        [[nodiscard]] uint32_t                           RegisterSpace( ) const;

        [[nodiscard]] VkDescriptorSetLayout DescriptorSetLayout( ) const;
        [[nodiscard]] DenOfIz_BindingDesc   GetVkShiftedBinding( const DenOfIz_ResourceBindingSlot &slot ) const;
        [[nodiscard]] bool                  HasBindings( ) const;

    private:
        void                                       AddResourceBinding( const DenOfIz_BindingDesc &binding );
        [[nodiscard]] VkDescriptorSetLayoutBinding CreateDescriptorSetLayoutBinding( const DenOfIz_BindingDesc &binding );
    };

} // namespace DenOfIz
