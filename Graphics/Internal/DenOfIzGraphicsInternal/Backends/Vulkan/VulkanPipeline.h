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

#include "DenOfIzGraphics/Backends/Interface/IPipeline.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanContext.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanLocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

namespace DenOfIz
{

    class VulkanPipeline final : public IPipeline
    {
        static const std::array<VkDynamicState, 4> g_dynamicStates;

        VulkanContext *m_context;

        std::vector<VkShaderModule>        m_shaderModules;
        std::vector<VkDescriptorSetLayout> m_layouts;

        PipelineDesc m_desc;

        std::unordered_map<std::string, VkWriteDescriptorSet> m_descriptorSets;

        VkPipeline          m_instance{ };
        VkPipelineBindPoint m_bindPoint{ };
        VkPipelineLayout    m_layout{ };
        VkPipelineLayout    m_rtLayout{ };

        std::unordered_map<std::string, uint32_t>     m_shaderIdentifierOffsets;
        std::vector<uint8_t>                          m_shaderIdentifiers;
        std::vector<std::pair<ShaderStage, uint32_t>> m_hitGroupIdentifiers;

        std::unique_ptr<VulkanLocalRootSignature> rayTracingLocalRootSignature{ nullptr };

    public:
        [[nodiscard]] VkPipeline                                           Instance( ) const;
        [[nodiscard]] VkPipelineBindPoint                                  BindPoint( ) const;
        [[nodiscard]] void                                                *GetShaderIdentifier( const std::string &exportName );
        [[nodiscard]] void                                                *GetShaderIdentifier( uint32_t offset );
        [[nodiscard]] const std::vector<std::pair<ShaderStage, uint32_t>> &HitGroupIdentifiers( ) const;
        [[nodiscard]] VulkanPipeline( VulkanContext *context, const PipelineDesc & );
        ~VulkanPipeline( ) override;

    private:
        void                                                       CreateGraphicsPipeline( );
        void                                                       CreateComputePipeline( );
        void                                                       CreateRayTracingPipeline( );
        void                                                       CreateMeshPipeline( );
        [[nodiscard]] std::vector<VkPipelineShaderStageCreateInfo> ConfigurePipelineStages( );
        [[nodiscard]] std::vector<VkPipelineShaderStageCreateInfo> ConfigureMeshPipelineStages( );
        [[nodiscard]] VkPipelineRenderingCreateInfo                ConfigureRenderingInfo( std::vector<VkFormat> &colorAttachmentsStore ) const;
        [[nodiscard]] VkPipelineTessellationStateCreateInfo        ConfigureTessellation( ) const;
        [[nodiscard]] VkPipelineInputAssemblyStateCreateInfo       ConfigureInputAssembly( ) const;
        [[nodiscard]] VkPipelineVertexInputStateCreateInfo         ConfigureVertexInputState( ) const;
        [[nodiscard]] VkPipelineShaderStageCreateInfo              ConfigureShaderStage( const ShaderStageDesc &shaderDesc ) const;
        [[nodiscard]] VkPipelineLayoutCreateInfo                   ConfigurePipelineLayout( ) const;
        [[nodiscard]] VkPipelineColorBlendStateCreateInfo          ConfigureColorBlend( std::vector<VkPipelineColorBlendAttachmentState> &colorBlendAttachments ) const;
        [[nodiscard]] VkPipelineRasterizationStateCreateInfo       ConfigureRasterization( ) const;
        [[nodiscard]] VkPipelineViewportStateCreateInfo            ConfigureViewport( ) const;
        [[nodiscard]] VkPipelineMultisampleStateCreateInfo         ConfigureMultisampling( ) const;
        [[nodiscard]] VkPipelineDepthStencilStateCreateInfo        CreateDepthAttachmentImages( ) const;
        VkShaderModule                                             CreateShaderModule( const ByteArray& blob ) const;
    };

} // namespace DenOfIz
