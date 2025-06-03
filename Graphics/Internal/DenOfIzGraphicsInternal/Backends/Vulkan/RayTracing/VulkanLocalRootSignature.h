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

#include "DenOfIzGraphics/Backends/Interface/RayTracing/ILocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanContext.h"

namespace DenOfIz
{
    struct VkLayoutWithSet
    {
        VkDescriptorSetLayout Layout;
        uint32_t              Set;
    };

    /// !Important, Vulkan local root signature is expected to merge into a single local root signature during ray tracing pipeline creation. To keep consistency with
    /// all DenOfIz structures, it can still initialize on constructor. However there is no real benefit of providing create = true, as the used local root signature is the one
    /// created within VulkanPipeline::CreateRayTracingPipeline( )
    class VulkanLocalRootSignature final : public ILocalRootSignature
    {
        VulkanContext         *m_context;
        LocalRootSignatureDesc m_desc;
        VkDescriptorSetLayout  m_descriptorSetLayout = nullptr;

        uint32_t m_totalInlineDataBytes = 0;

        std::vector<VkLayoutWithSet>                                            m_layouts;
        std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> m_layoutBindings;

        std::vector<uint32_t> m_inlineDataOffsets;
        std::vector<uint32_t> m_inlineDataNumBytes;

    public:
        VulkanLocalRootSignature( VulkanContext *context, const LocalRootSignatureDesc &desc, bool create = true );
        void Merge( const VulkanLocalRootSignature& other );
        void Create( );
        ~VulkanLocalRootSignature( ) override;

        [[nodiscard]] std::vector<VkLayoutWithSet> DescriptorSetLayouts( );
        [[nodiscard]] VkDescriptorSetLayout       *DescriptorSetLayout( );
        [[nodiscard]] uint32_t                     LocalDataNumBytes( ) const;
        [[nodiscard]] uint32_t                     InlineDataNumBytes( ) const;
        [[nodiscard]] uint32_t                     CbvOffset( uint32_t cbvIndex ) const;
        [[nodiscard]] uint32_t                     CbvNumBytes( uint32_t cbvIndex ) const;
    };
} // namespace DenOfIz
