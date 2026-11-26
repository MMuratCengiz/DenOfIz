/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <deque>
#include "DenOfIzGraphicsInternal/Backends/Interface/IPipeline.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanLocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanContext.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanRootSignature.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

namespace DenOfIz
{

    class VulkanPipeline final : public IPipeline
    {
        static const std::array<VkDynamicState, 4> g_dynamicStates;

        VulkanContext       *m_context;
        VulkanRootSignature *m_rootSignature = nullptr;

        std::vector<VkShaderModule>        m_shaderModules;
        std::vector<VkDescriptorSetLayout> m_layouts;

        DenOfIz_PipelineDesc m_desc;

        std::unordered_map<std::string, VkWriteDescriptorSet> m_descriptorSets;

        VkPipeline          m_instance{ };
        VkPipelineBindPoint m_bindPoint{ };
        VkPipelineLayout    m_layout{ };
        VkPipelineLayout    m_rtLayout{ };

        std::unordered_map<std::string, uint32_t>                  m_shaderIdentifierOffsets;
        std::vector<uint8_t>                                       m_shaderIdentifiers;
        std::vector<std::pair<DenOfIz_ShaderStageFlags, uint32_t>> m_hitGroupIdentifiers;
        std::deque<std::string>                                    m_entryPointNames;

        std::unique_ptr<VulkanLocalRootSignature> rayTracingLocalRootSignature{ nullptr };

    public:
        [[nodiscard]] VkPipeline                                                        Instance( ) const;
        [[nodiscard]] VkPipelineBindPoint                                               DenOfIz_BindPoint( ) const;
        [[nodiscard]] VulkanRootSignature                                              *RootSignature( ) const;
        [[nodiscard]] void                                                             *GetShaderIdentifier( const std::string &exportName );
        [[nodiscard]] void                                                             *GetShaderIdentifier( uint32_t offset );
        [[nodiscard]] const std::vector<std::pair<DenOfIz_ShaderStageFlags, uint32_t>> &HitGroupIdentifiers( ) const;
        [[nodiscard]] VulkanPipeline( VulkanContext *context, const DenOfIz_PipelineDesc & );
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
        [[nodiscard]] VkPipelineShaderStageCreateInfo              ConfigureShaderStage( const DenOfIz_ShaderStageDesc &shaderDesc ) const;
        [[nodiscard]] VkPipelineLayoutCreateInfo                   ConfigurePipelineLayout( ) const;
        [[nodiscard]] VkPipelineColorBlendStateCreateInfo          ConfigureColorBlend( std::vector<VkPipelineColorBlendAttachmentState> &colorBlendAttachments ) const;
        [[nodiscard]] VkPipelineRasterizationStateCreateInfo       ConfigureRasterization( ) const;
        [[nodiscard]] VkPipelineViewportStateCreateInfo            ConfigureViewport( ) const;
        [[nodiscard]] VkPipelineMultisampleStateCreateInfo         ConfigureMultisampling( ) const;
        [[nodiscard]] VkPipelineDepthStencilStateCreateInfo        CreateDepthAttachmentImages( ) const;
        VkShaderModule                                             CreateShaderModule( const DenOfIz_ByteArray &blob ) const;
    };

} // namespace DenOfIz
