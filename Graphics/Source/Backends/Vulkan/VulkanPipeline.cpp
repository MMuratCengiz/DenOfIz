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
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanInputLayout.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanPipeline.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanRootSignature.h"
#include <ranges>
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanPipeline::VulkanPipeline( VulkanContext *context, const PipelineDesc &desc ) :
    m_context( context ), m_desc( desc ), m_bindPoint( VulkanEnumConverter::ConvertPipelineBindPoint( desc.BindPoint ) )
{
    const auto rootSignature = dynamic_cast<VulkanRootSignature *>( desc.RootSignature );
    m_layout                 = rootSignature->PipelineLayout( );

    switch ( desc.BindPoint )
    {
    case BindPoint::Graphics:
        CreateGraphicsPipeline( );
        break;
    case BindPoint::Compute:
        CreateComputePipeline( );
        break;
    case BindPoint::RayTracing:
        CreateRayTracingPipeline( );
        break;
    case BindPoint::Mesh:
        CreateMeshPipeline( );
        break;
    }
}

void VulkanPipeline::CreateGraphicsPipeline( )
{
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{ };
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    std::vector<VkPipelineShaderStageCreateInfo>     pipelineStageCreateInfos     = ConfigurePipelineStages( );
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments        = { };
    VkPipelineColorBlendStateCreateInfo              colorBlending                = ConfigureColorBlend( colorBlendAttachments );
    std::vector<VkFormat>                            colorFormats                 = { };
    VkPipelineRenderingCreateInfo                    renderingCreateInfo          = ConfigureRenderingInfo( colorFormats );
    VkPipelineTessellationStateCreateInfo            tessellationStateCreateInfo  = ConfigureTessellation( );
    VkPipelineRasterizationStateCreateInfo           rasterizationStateCreateInfo = ConfigureRasterization( );
    VkPipelineViewportStateCreateInfo                viewportStateCreateInfo      = ConfigureViewport( );
    VkPipelineMultisampleStateCreateInfo             multisampleStateCreateInfo   = ConfigureMultisampling( );
    VkPipelineInputAssemblyStateCreateInfo           inputAssemblyCreateInfo      = ConfigureInputAssembly( );
    VkPipelineDepthStencilStateCreateInfo            depthStencilStateCreateInfo  = CreateDepthAttachmentImages( );
    VkPipelineVertexInputStateCreateInfo             inputStateCreateInfo         = ConfigureVertexInputState( );

    // Configure Dynamic States:
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{ };
    dynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = g_dynamicStates.size( );
    dynamicStateCreateInfo.pDynamicStates    = g_dynamicStates.data( );
    pipelineCreateInfo.pDynamicState         = &dynamicStateCreateInfo;
    // --
    // Render pass configuration, disabled for now
    pipelineCreateInfo.renderPass         = nullptr;
    pipelineCreateInfo.subpass            = 0;
    pipelineCreateInfo.basePipelineHandle = nullptr;
    pipelineCreateInfo.basePipelineIndex  = -1;
    // --
    pipelineCreateInfo.pVertexInputState   = &inputStateCreateInfo;
    pipelineCreateInfo.pTessellationState  = &tessellationStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    pipelineCreateInfo.pViewportState      = &viewportStateCreateInfo;
    pipelineCreateInfo.pDepthStencilState  = &depthStencilStateCreateInfo;
    pipelineCreateInfo.pMultisampleState   = &multisampleStateCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pColorBlendState    = &colorBlending;
    pipelineCreateInfo.stageCount          = static_cast<uint32_t>( pipelineStageCreateInfos.size( ) );
    pipelineCreateInfo.pStages             = pipelineStageCreateInfos.data( );
    pipelineCreateInfo.layout              = m_layout;

    pipelineCreateInfo.pNext = &renderingCreateInfo;
    VK_CHECK_RESULT( vkCreateGraphicsPipelines( m_context->LogicalDevice, nullptr, 1, &pipelineCreateInfo, nullptr, &m_instance ) );
}

void VulkanPipeline::CreateComputePipeline( )
{
    VkComputePipelineCreateInfo pipelineCreateInfo{ };
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

    const std::vector<VkPipelineShaderStageCreateInfo> pipelineStageCreateInfos = ConfigurePipelineStages( );
    // Render pass configuration, disabled for now
    pipelineCreateInfo.basePipelineHandle = nullptr;
    pipelineCreateInfo.basePipelineIndex  = -1;
    // --
    pipelineCreateInfo.stage  = pipelineStageCreateInfos[ 0 ];
    pipelineCreateInfo.layout = m_layout;
    VK_CHECK_RESULT( vkCreateComputePipelines( m_context->LogicalDevice, nullptr, 1, &pipelineCreateInfo, nullptr, &m_instance ) );
}

void VulkanPipeline::CreateRayTracingPipeline( )
{
    const InteropArray<CompiledShaderStage *> &compiledShaders = m_desc.ShaderProgram->CompiledShaders( );

    m_shaderModules.reserve( compiledShaders.NumElements( ) );
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve( compiledShaders.NumElements( ) );
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
    shaderGroups.reserve( compiledShaders.NumElements( ) + m_desc.RayTracing.HitGroups.NumElements( ) );

    std::vector<VkDescriptorSetLayout> allLayouts;
    const auto                         rootSig        = dynamic_cast<VulkanRootSignature *>( m_desc.RootSignature );
    auto                               rootSigLayouts = rootSig->DescriptorSetLayouts( );
    allLayouts.insert( allLayouts.end( ), rootSigLayouts.begin( ), rootSigLayouts.end( ) );

    // Create a local root signature
    rayTracingLocalRootSignature = std::make_unique<VulkanLocalRootSignature>( m_context, LocalRootSignatureDesc{ }, false );
    auto mergeLocalRootSignature = [ & ]( ILocalRootSignature *localRootSignature )
    {
        if ( const auto *layout = dynamic_cast<VulkanLocalRootSignature *>( localRootSignature ) )
        {
            rayTracingLocalRootSignature->Merge( *layout );
        }
    };

    for ( int i = 0; i < compiledShaders.NumElements( ); ++i )
    {
        const auto                 &compiledShader = compiledShaders.GetElement( i );
        const VkShaderStageFlagBits stage          = VulkanEnumConverter::ConvertShaderStage( compiledShader->Stage );

        switch ( compiledShader->Stage )
        {
        case ShaderStage::Raygen:
        case ShaderStage::ClosestHit:
        case ShaderStage::AnyHit:
        case ShaderStage::Intersection:
        case ShaderStage::Miss:
            break;
        default:
            spdlog::error("Invalid shader stage for ray tracing pipeline");
            continue;
        }

        if ( m_desc.RayTracing.LocalRootSignatures.NumElements( ) > i )
        {
            mergeLocalRootSignature( m_desc.RayTracing.LocalRootSignatures.GetElement( i ) );
        }

        const VkShaderModule            &shaderModule = m_shaderModules.emplace_back( this->CreateShaderModule( compiledShader->SPIRV ) );
        VkPipelineShaderStageCreateInfo &shaderStage  = shaderStages.emplace_back( );
        shaderStage.sType                             = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage                             = stage;
        shaderStage.module                            = shaderModule;
        shaderStage.pName                             = compiledShader->EntryPoint.Get( );

        if ( compiledShader->Stage == ShaderStage::Raygen || compiledShader->Stage == ShaderStage::Miss )
        {
            VkRayTracingShaderGroupCreateInfoKHR &group = shaderGroups.emplace_back( );
            group.sType                                 = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            group.type                                  = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            group.generalShader                         = i;
            group.closestHitShader                      = VK_SHADER_UNUSED_KHR;
            group.anyHitShader                          = VK_SHADER_UNUSED_KHR;
            group.intersectionShader                    = VK_SHADER_UNUSED_KHR;

            m_shaderIdentifierOffsets[ compiledShader->EntryPoint.Get( ) ] = shaderGroups.size( ) - 1;
        }
    }

    for ( int i = 0; i < m_desc.RayTracing.HitGroups.NumElements( ); ++i )
    {
        const auto &hitGroup = m_desc.RayTracing.HitGroups.GetElement( i );

        VkRayTracingShaderGroupCreateInfoKHR &group = shaderGroups.emplace_back( );
        group.sType                                 = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group.type =
            hitGroup.Type == HitGroupType::Triangles ? VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR : VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;

        group.generalShader      = VK_SHADER_UNUSED_KHR;
        group.closestHitShader   = hitGroup.ClosestHitShaderIndex >= 0 ? hitGroup.ClosestHitShaderIndex : VK_SHADER_UNUSED_KHR;
        group.anyHitShader       = hitGroup.AnyHitShaderIndex >= 0 ? hitGroup.AnyHitShaderIndex : VK_SHADER_UNUSED_KHR;
        group.intersectionShader = hitGroup.IntersectionShaderIndex >= 0 ? hitGroup.IntersectionShaderIndex : VK_SHADER_UNUSED_KHR;

        m_shaderIdentifierOffsets[ hitGroup.Name.Get( ) ] = shaderGroups.size( ) - 1;

        mergeLocalRootSignature( hitGroup.LocalRootSignature );
    }

    // Add the merged local layouts
    rayTracingLocalRootSignature->Create( );
    for ( const auto &layout : rayTracingLocalRootSignature->DescriptorSetLayouts( ) )
    {
        for ( uint32_t lastLayout = allLayouts.size( ); lastLayout <= layout.Set; ++lastLayout )
        {
            allLayouts.push_back( rootSig->EmptyLayout( ) );
        }
        allLayouts[ layout.Set ] = layout.Layout;
    }
    const std::vector<VkPushConstantRange> pushConstants      = rootSig->PushConstantRanges( );
    VkPipelineLayoutCreateInfo             pipelineLayoutInfo = { };
    pipelineLayoutInfo.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount                         = allLayouts.size( );
    pipelineLayoutInfo.pSetLayouts                            = allLayouts.data( );
    pipelineLayoutInfo.pushConstantRangeCount                 = pushConstants.size( );
    pipelineLayoutInfo.pPushConstantRanges                    = pushConstants.data( );

    VK_CHECK_RESULT( vkCreatePipelineLayout( m_context->LogicalDevice, &pipelineLayoutInfo, nullptr, &m_rtLayout ) );

    VkRayTracingPipelineInterfaceCreateInfoKHR pipelineInterface{ };
    pipelineInterface.sType                          = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR;
    pipelineInterface.maxPipelineRayPayloadSize      = m_desc.ShaderProgram->Desc( ).RayTracing.MaxNumPayloadBytes;
    pipelineInterface.maxPipelineRayHitAttributeSize = m_desc.ShaderProgram->Desc( ).RayTracing.MaxNumAttributeBytes;

    VkRayTracingPipelineCreateInfoKHR pipelineInfo{ };
    pipelineInfo.sType                        = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineInfo.stageCount                   = static_cast<uint32_t>( shaderStages.size( ) );
    pipelineInfo.pStages                      = shaderStages.data( );
    pipelineInfo.groupCount                   = static_cast<uint32_t>( shaderGroups.size( ) );
    pipelineInfo.pGroups                      = shaderGroups.data( );
    pipelineInfo.maxPipelineRayRecursionDepth = m_desc.ShaderProgram->Desc( ).RayTracing.MaxRecursionDepth;
    pipelineInfo.layout                       = m_rtLayout;
    pipelineInfo.pLibraryInterface            = &pipelineInterface;

    VK_CHECK_RESULT( vkCreateRayTracingPipelinesKHR( m_context->LogicalDevice, nullptr, nullptr, 1, &pipelineInfo, nullptr, &m_instance ) );

    m_shaderIdentifiers.resize( shaderGroups.size( ) * m_context->RayTracingProperties.shaderGroupHandleSize );
    VK_CHECK_RESULT(
        vkGetRayTracingShaderGroupHandlesKHR( m_context->LogicalDevice, m_instance, 0, pipelineInfo.groupCount, m_shaderIdentifiers.size( ), m_shaderIdentifiers.data( ) ) );
}

void *VulkanPipeline::GetShaderIdentifier( const std::string &exportName )
{
    const auto it = m_shaderIdentifierOffsets.find( exportName );
    if ( it == m_shaderIdentifierOffsets.end( ) )
    {
        spdlog::error("Could not find shader identifier for export {}", exportName);
        return nullptr;
    }

    const uint32_t groupIndex = it->second;
    return m_shaderIdentifiers.data( ) + groupIndex * m_context->RayTracingProperties.shaderGroupHandleSize;
}

void *VulkanPipeline::GetShaderIdentifier( const uint32_t offset )
{
    return m_shaderIdentifiers.data( ) + offset * m_context->RayTracingProperties.shaderGroupHandleSize;
}

const std::vector<std::pair<ShaderStage, uint32_t>> &VulkanPipeline::HitGroupIdentifiers( ) const
{
    return m_hitGroupIdentifiers;
}

// clang-format off
const std::array<VkDynamicState, 4> VulkanPipeline::g_dynamicStates =
{
    VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
    VK_DYNAMIC_STATE_DEPTH_BIAS,
    VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,
    VK_DYNAMIC_STATE_LINE_WIDTH
};
// clang-format on

std::vector<VkPipelineShaderStageCreateInfo> VulkanPipeline::ConfigurePipelineStages( )
{
    std::vector<VkPipelineShaderStageCreateInfo> pipelineStageCreateInfos;

    const auto &compiledShaders = m_desc.ShaderProgram->CompiledShaders( );
    for ( int i = 0; i < compiledShaders.NumElements( ); ++i )
    {
        const auto                      &compiledShader        = compiledShaders.GetElement( i );
        VkPipelineShaderStageCreateInfo &shaderStageCreateInfo = pipelineStageCreateInfos.emplace_back( );
        shaderStageCreateInfo.sType                            = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        const VkShaderStageFlagBits stage        = VulkanEnumConverter::ConvertShaderStage( compiledShader->Stage );
        const VkShaderModule       &shaderModule = m_shaderModules.emplace_back( this->CreateShaderModule( compiledShader->SPIRV ) );

        shaderStageCreateInfo.stage  = stage;
        shaderStageCreateInfo.module = shaderModule;
        shaderStageCreateInfo.pName  = compiledShader->EntryPoint.Get( );
        shaderStageCreateInfo.pNext  = nullptr;
    }

    return pipelineStageCreateInfos;
}

void VulkanPipeline::CreateMeshPipeline( )
{
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{ };
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    std::vector<VkPipelineShaderStageCreateInfo> pipelineStageCreateInfos = ConfigureMeshPipelineStages( );

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{ };
    VkPipelineColorBlendStateCreateInfo              colorBlending = ConfigureColorBlend( colorBlendAttachments );

    std::vector<VkFormat>         colorFormats{ };
    VkPipelineRenderingCreateInfo renderingCreateInfo = ConfigureRenderingInfo( colorFormats );

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = ConfigureRasterization( );
    VkPipelineViewportStateCreateInfo      viewportStateCreateInfo      = ConfigureViewport( );
    VkPipelineMultisampleStateCreateInfo   multisampleStateCreateInfo   = ConfigureMultisampling( );
    VkPipelineDepthStencilStateCreateInfo  depthStencilStateCreateInfo  = CreateDepthAttachmentImages( );

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{ };
    dynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>( g_dynamicStates.size( ) );
    dynamicStateCreateInfo.pDynamicStates    = g_dynamicStates.data( );
    pipelineCreateInfo.pDynamicState         = &dynamicStateCreateInfo;

    pipelineCreateInfo.pVertexInputState   = nullptr; // MUST be null for mesh pipelines
    pipelineCreateInfo.pInputAssemblyState = nullptr; // MUST be null for mesh pipelines
    pipelineCreateInfo.pTessellationState  = nullptr; // MUST be null for mesh pipelines

    pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    pipelineCreateInfo.pViewportState      = &viewportStateCreateInfo;
    pipelineCreateInfo.pDepthStencilState  = &depthStencilStateCreateInfo;
    pipelineCreateInfo.pMultisampleState   = &multisampleStateCreateInfo;
    pipelineCreateInfo.pColorBlendState    = &colorBlending;

    pipelineCreateInfo.stageCount = static_cast<uint32_t>( pipelineStageCreateInfos.size( ) );
    pipelineCreateInfo.pStages    = pipelineStageCreateInfos.data( );
    pipelineCreateInfo.layout     = m_layout;

    pipelineCreateInfo.renderPass         = VK_NULL_HANDLE;
    pipelineCreateInfo.subpass            = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex  = -1;

    pipelineCreateInfo.pNext = &renderingCreateInfo;

    VK_CHECK_RESULT( vkCreateGraphicsPipelines( m_context->LogicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_instance ) );
}

std::vector<VkPipelineShaderStageCreateInfo> VulkanPipeline::ConfigureMeshPipelineStages( )
{
    std::vector<VkPipelineShaderStageCreateInfo> pipelineStageCreateInfos;

    const auto &compiledShaders = m_desc.ShaderProgram->CompiledShaders( );
    for ( int i = 0; i < compiledShaders.NumElements( ); ++i )
    {
        const auto &compiledShader = compiledShaders.GetElement( i );

        // Only include task, mesh, and pixel/fragment shaders for mesh pipeline
        if ( compiledShader->Stage != ShaderStage::Task && compiledShader->Stage != ShaderStage::Mesh && compiledShader->Stage != ShaderStage::Pixel )
        {
            spdlog::warn("Skipping non-mesh shader stage in mesh pipeline: {}", static_cast<int>( compiledShader->Stage ));
            continue;
        }

        VkPipelineShaderStageCreateInfo &shaderStageCreateInfo = pipelineStageCreateInfos.emplace_back( );
        shaderStageCreateInfo.sType                            = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        const VkShaderStageFlagBits stage        = VulkanEnumConverter::ConvertShaderStage( compiledShader->Stage );
        const VkShaderModule       &shaderModule = m_shaderModules.emplace_back( this->CreateShaderModule( compiledShader->SPIRV ) );

        shaderStageCreateInfo.stage  = stage;
        shaderStageCreateInfo.module = shaderModule;
        shaderStageCreateInfo.pName  = compiledShader->EntryPoint.Get( );
        shaderStageCreateInfo.pNext  = nullptr;
    }

    return pipelineStageCreateInfos;
}

[[nodiscard]] VkPipelineRenderingCreateInfo VulkanPipeline::ConfigureRenderingInfo( std::vector<VkFormat> &colorAttachmentsStore ) const
{
    for ( int i = 0; i < m_desc.Graphics.RenderTargets.NumElements( ); ++i )
    {
        colorAttachmentsStore.push_back( VulkanEnumConverter::ConvertImageFormat( m_desc.Graphics.RenderTargets.GetElement( i ).Format ) );
    }

    VkPipelineRenderingCreateInfo renderingCreateInfo{ };
    renderingCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.viewMask                = m_desc.Graphics.ViewMask;
    renderingCreateInfo.colorAttachmentCount    = colorAttachmentsStore.size( );
    renderingCreateInfo.pColorAttachmentFormats = colorAttachmentsStore.data( );
    renderingCreateInfo.depthAttachmentFormat   = VulkanEnumConverter::ConvertImageFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
    renderingCreateInfo.stencilAttachmentFormat = VulkanEnumConverter::ConvertImageFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
    return renderingCreateInfo;
}

[[nodiscard]] VkPipelineTessellationStateCreateInfo VulkanPipeline::ConfigureTessellation( ) const
{
    VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo{ };
    tessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    // Todo read this value from somewhere
    tessellationStateCreateInfo.patchControlPoints = 3;
    return tessellationStateCreateInfo;
}

[[nodiscard]] VkPipelineInputAssemblyStateCreateInfo VulkanPipeline::ConfigureInputAssembly( ) const
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{ };
    inputAssemblyCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology               = VulkanEnumConverter::ConvertPrimitiveTopology( m_desc.Graphics.PrimitiveTopology );
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
    return inputAssemblyCreateInfo;
}

[[nodiscard]] VkPipelineVertexInputStateCreateInfo VulkanPipeline::ConfigureVertexInputState( ) const
{
    const auto                                *inputLayout          = dynamic_cast<VulkanInputLayout *>( m_desc.InputLayout );
    if ( inputLayout == nullptr )
    {
        return VkPipelineVertexInputStateCreateInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    }
    const VkPipelineVertexInputStateCreateInfo inputStateCreateInfo = inputLayout->GetVertexInputState( );
    return inputStateCreateInfo;
}

VkPipelineMultisampleStateCreateInfo VulkanPipeline::ConfigureMultisampling( ) const
{
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{ };
    multisampleStateCreateInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;

    switch ( m_desc.Graphics.MSAASampleCount )
    {
    case MSAASampleCount::_0:
        multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    case MSAASampleCount::_1:
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        break;
    case MSAASampleCount::_2:
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
        break;
    case MSAASampleCount::_4:
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
        break;
    case MSAASampleCount::_8:
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;
        break;
    case MSAASampleCount::_16:
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT;
        break;
    case MSAASampleCount::_32:
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_32_BIT;
        break;
    case MSAASampleCount::_64:
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_64_BIT;
        break;
    }

    multisampleStateCreateInfo.minSampleShading      = 1.0f;
    multisampleStateCreateInfo.pSampleMask           = nullptr;
    multisampleStateCreateInfo.alphaToCoverageEnable = m_desc.Graphics.AlphaToCoverageEnable ? VK_TRUE : VK_FALSE;
    multisampleStateCreateInfo.alphaToOneEnable      = VK_FALSE;
    multisampleStateCreateInfo.sampleShadingEnable   = VK_TRUE;
    multisampleStateCreateInfo.minSampleShading      = .2f;
    return multisampleStateCreateInfo;
}

VkPipelineViewportStateCreateInfo VulkanPipeline::ConfigureViewport( ) const
{
    // Todo test, these are dynamic states
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{ };
    viewportStateCreateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 0;
    viewportStateCreateInfo.pViewports    = nullptr;
    viewportStateCreateInfo.scissorCount  = 0;
    viewportStateCreateInfo.pScissors     = nullptr;
    return viewportStateCreateInfo;
}

VkPipelineRasterizationStateCreateInfo VulkanPipeline::ConfigureRasterization( ) const
{
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{ };
    rasterizationStateCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.depthClampEnable        = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.lineWidth               = 1.0f;

    switch ( m_desc.Graphics.CullMode )
    {
    case CullMode::BackFace:
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        break;
    case CullMode::FrontFace:
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
        break;
    case CullMode::None:
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
        break;
    }
    switch ( m_desc.Graphics.FillMode )
    {
    case FillMode::Solid:
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        break;
    case FillMode::Wireframe:
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_LINE;
        break;
    }

    rasterizationStateCreateInfo.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizationStateCreateInfo.depthBiasEnable         = false; // Todo, test if works with dynamic state
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizationStateCreateInfo.depthBiasClamp          = 0.0f;
    rasterizationStateCreateInfo.depthBiasSlopeFactor    = 0.0f;
    return rasterizationStateCreateInfo;
}

VkPipelineColorBlendStateCreateInfo VulkanPipeline::ConfigureColorBlend( std::vector<VkPipelineColorBlendAttachmentState> &colorBlendAttachments ) const
{
    const uint32_t attachmentCount = m_desc.Graphics.RenderTargets.NumElements( );
    colorBlendAttachments.resize( attachmentCount );

    for ( uint32_t i = 0; i < attachmentCount; ++i )
    {
        auto &attachment                               = m_desc.Graphics.RenderTargets.GetElement( i );
        colorBlendAttachments[ i ].blendEnable         = attachment.Blend.Enable;
        colorBlendAttachments[ i ].srcColorBlendFactor = VulkanEnumConverter::ConvertBlend( attachment.Blend.SrcBlend );
        colorBlendAttachments[ i ].dstColorBlendFactor = VulkanEnumConverter::ConvertBlend( attachment.Blend.DstBlend );
        colorBlendAttachments[ i ].srcAlphaBlendFactor = VulkanEnumConverter::ConvertBlend( attachment.Blend.SrcBlendAlpha );
        colorBlendAttachments[ i ].dstAlphaBlendFactor = VulkanEnumConverter::ConvertBlend( attachment.Blend.DstBlendAlpha );
        colorBlendAttachments[ i ].colorBlendOp        = VulkanEnumConverter::ConvertBlendOp( attachment.Blend.BlendOp );
        colorBlendAttachments[ i ].alphaBlendOp        = VulkanEnumConverter::ConvertBlendOp( attachment.Blend.BlendOpAlpha );

        colorBlendAttachments[ i ].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        if ( attachment.Blend.RenderTargetWriteMask != 0x0F )
        {
            colorBlendAttachments[ i ].colorWriteMask = 0;
            if ( attachment.Blend.RenderTargetWriteMask & 0x01 )
            {
                colorBlendAttachments[ i ].colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
            }
            if ( attachment.Blend.RenderTargetWriteMask & 0x02 )
            {
                colorBlendAttachments[ i ].colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
            }
            if ( attachment.Blend.RenderTargetWriteMask & 0x04 )
            {
                colorBlendAttachments[ i ].colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
            }
            if ( attachment.Blend.RenderTargetWriteMask & 0x08 )
            {
                colorBlendAttachments[ i ].colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
            }
        }
    }

    // This overwrites the above
    VkPipelineColorBlendStateCreateInfo colorBlending{ };
    colorBlending.sType               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable       = m_desc.Graphics.BlendLogicOpEnable;
    colorBlending.logicOp             = VulkanEnumConverter::ConvertLogicOp( m_desc.Graphics.BlendLogicOp );
    colorBlending.attachmentCount     = attachmentCount;
    colorBlending.pAttachments        = colorBlendAttachments.data( );
    colorBlending.blendConstants[ 0 ] = 0.0f;
    colorBlending.blendConstants[ 1 ] = 0.0f;
    colorBlending.blendConstants[ 2 ] = 0.0f;
    colorBlending.blendConstants[ 3 ] = 0.0f;
    return colorBlending;
}

VkPipelineDepthStencilStateCreateInfo VulkanPipeline::CreateDepthAttachmentImages( ) const
{
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{ };
    depthStencilStateCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateCreateInfo.depthTestEnable       = m_desc.Graphics.DepthTest.Enable;
    depthStencilStateCreateInfo.depthWriteEnable      = m_desc.Graphics.DepthTest.Write;
    depthStencilStateCreateInfo.depthCompareOp        = VulkanEnumConverter::ConvertCompareOp( m_desc.Graphics.DepthTest.CompareOp );
    depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.minDepthBounds        = 0.0f;
    depthStencilStateCreateInfo.maxDepthBounds        = 1.0f;

    depthStencilStateCreateInfo.stencilTestEnable = m_desc.Graphics.StencilTest.Enable;

    depthStencilStateCreateInfo.front = VkStencilOpState{ };
    depthStencilStateCreateInfo.back  = VkStencilOpState{ };

    auto initStencilState = [ =, this ]( VkStencilOpState &vkState, const StencilFace &state )
    {
        vkState.compareOp   = VulkanEnumConverter::ConvertCompareOp( state.CompareOp );
        vkState.compareMask = m_desc.Graphics.StencilTest.ReadMask;
        vkState.writeMask   = m_desc.Graphics.StencilTest.WriteMask;
        vkState.reference   = 0;
        vkState.failOp      = VulkanEnumConverter::ConvertStencilOp( state.FailOp );
        vkState.passOp      = VulkanEnumConverter::ConvertStencilOp( state.PassOp );
        vkState.depthFailOp = VulkanEnumConverter::ConvertStencilOp( state.DepthFailOp );
    };

    if ( m_desc.Graphics.StencilTest.Enable )
    {
        initStencilState( depthStencilStateCreateInfo.front, m_desc.Graphics.StencilTest.FrontFace );
        initStencilState( depthStencilStateCreateInfo.back, m_desc.Graphics.StencilTest.BackFace );
    }

    return depthStencilStateCreateInfo;
}

VkShaderModule VulkanPipeline::CreateShaderModule( const InteropArray<Byte>& blob ) const
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo{ };
    shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = blob.NumElements( );
    shaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t *>( blob.Data( ) );

    VkShaderModule shaderModule{ };
    VK_CHECK_RESULT( vkCreateShaderModule( m_context->LogicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule ) );
    return shaderModule;
}

VulkanPipeline::~VulkanPipeline( )
{
    for ( const auto &module : m_shaderModules )
    {
        vkDestroyShaderModule( m_context->LogicalDevice, module, nullptr );
    }

    for ( const auto &layout : m_layouts )
    {
        vkDestroyDescriptorSetLayout( m_context->LogicalDevice, layout, nullptr );
    }

    vkDestroyPipeline( m_context->LogicalDevice, m_instance, nullptr );
    if ( m_rtLayout )
    {
        vkDestroyPipelineLayout( m_context->LogicalDevice, m_rtLayout, nullptr );
    }
    if ( m_layout )
    {
        vkDestroyPipelineLayout( m_context->LogicalDevice, m_layout, nullptr );
    }
}

VkPipeline VulkanPipeline::Instance( ) const
{
    return m_instance;
}
VkPipelineBindPoint VulkanPipeline::BindPoint( ) const
{
    return m_bindPoint;
}
