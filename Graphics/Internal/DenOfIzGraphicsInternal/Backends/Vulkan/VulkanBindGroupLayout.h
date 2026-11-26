/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
        // Descriptor sets can be bound where the pipeline layout expects `other` when it is the same layout or one defined identically.
        [[nodiscard]] bool IsCompatibleWith( const VulkanBindGroupLayout *other ) const;

    private:
        void                                       AddResourceBinding( const DenOfIz_BindingDesc &binding );
        [[nodiscard]] VkDescriptorSetLayoutBinding CreateDescriptorSetLayoutBinding( const DenOfIz_BindingDesc &binding );
    };

} // namespace DenOfIz
