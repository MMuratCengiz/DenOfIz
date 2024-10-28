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
#include <DenOfIzGraphics/Backends/Vulkan/VulkanPipeline.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanRootSignature.h>
#include <ranges>

using namespace DenOfIz;

VulkanPipeline::VulkanPipeline( VulkanContext *context, const PipelineDesc &desc ) :
    m_context( context ), m_desc( desc ), m_bindPoint( VulkanEnumConverter::ConvertPipelineBindPoint( desc.BindPoint ) )
{
    auto rootSignature = dynamic_cast<VulkanRootSignature *>( desc.RootSignature );
    m_layout           = rootSignature->PipelineLayout( );

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
    VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo = { };
    pipelineCreateInfo.sType                             = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    VkPipelineLayout pipelineLayout                      = m_layout;

    std::vector<VkPipelineShaderStageCreateInfo>      shaderStages;
    std::unordered_map<std::string, VkShaderModule>   visitedShaders;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

    const InteropArray<CompiledShader *> &compiledShaders = m_desc.ShaderProgram->GetCompiledShaders( );
    for ( int i = 0; i < compiledShaders.NumElements( ); ++i )
    {
        const auto                    &compiledShader = compiledShaders.GetElement( i );
        VkShaderStageFlagBits          stage;
        VkRayTracingShaderGroupTypeKHR groupType = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;

        switch ( compiledShader->Stage )
        {
        case ShaderStage::Raygen:
            stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            break;
        case ShaderStage::ClosestHit:
            stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
            break;
        case ShaderStage::AnyHit:
            stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
            break;
        case ShaderStage::Intersection:
            stage     = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
            groupType = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
            break;
        case ShaderStage::Miss:
            stage = VK_SHADER_STAGE_MISS_BIT_KHR;
            break;
        default:
            continue;
        }

        if ( visitedShaders.find( compiledShader->Path ) == visitedShaders.end( ) )
        {
            VkShaderModule shaderModule = this->CreateShaderModule( compiledShader->Blob );
            m_shaderModules.push_back( shaderModule );
            visitedShaders[ compiledShader->Path ] = shaderModule;
        }

        VkPipelineShaderStageCreateInfo &shaderStage = shaderStages.emplace_back( );
        shaderStage.sType                            = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage                            = stage;
        shaderStage.module                           = visitedShaders[ compiledShader->Path ];
        shaderStage.pName                            = compiledShader->EntryPoint.Get( );

        VkRayTracingShaderGroupCreateInfoKHR &shaderGroup = shaderGroups.emplace_back( );
        shaderGroup.sType                                 = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type                                  = groupType;
        shaderGroup.closestHitShader                      = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader                          = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader                    = VK_SHADER_UNUSED_KHR;
        shaderGroup.generalShader                         = VK_SHADER_UNUSED_KHR;

        auto shaderOffset = shaderStages.size( ) - 1;

        switch ( compiledShader->Stage )
        {
        case ShaderStage::ClosestHit:
            shaderGroup.closestHitShader = static_cast<uint32_t>( shaderOffset );
            m_hitGroupIdentifiers.push_back( std::pair( compiledShader->Stage, shaderOffset ) );
            break;
        case ShaderStage::AnyHit:
            shaderGroup.anyHitShader = static_cast<uint32_t>( shaderOffset );
            m_hitGroupIdentifiers.push_back( std::pair( compiledShader->Stage, shaderOffset ) );
            break;
        case ShaderStage::Intersection:
            shaderGroup.intersectionShader = static_cast<uint32_t>( shaderOffset );
            m_hitGroupIdentifiers.push_back( std::pair( compiledShader->Stage, shaderOffset ) );
            break;
        default:
            shaderGroup.generalShader = static_cast<uint32_t>( shaderOffset );
            break;
        }

        m_shaderIdentifierOffsets[ compiledShader->EntryPoint.Get( ) ] = shaderOffset * m_context->RayTracingProperties.shaderGroupHandleSize;
    }

    // Pipeline Interface and Configurations
    VkRayTracingPipelineInterfaceCreateInfoKHR pipelineInterface = { };
    pipelineInterface.sType                                      = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR;
    pipelineInterface.maxPipelineRayPayloadSize                  = m_desc.RayTracing.MaxNumPayloadBytes;
    pipelineInterface.maxPipelineRayHitAttributeSize             = m_desc.RayTracing.MaxNumAttributeBytes;

    // Ray tracing pipeline configuration - max recursion depth
    VkRayTracingPipelineCreateInfoKHR pipelineInfo = { };
    pipelineInfo.sType                             = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineInfo.stageCount                        = static_cast<uint32_t>( shaderStages.size( ) );
    pipelineInfo.pStages                           = shaderStages.data( );
    pipelineInfo.groupCount                        = static_cast<uint32_t>( shaderGroups.size( ) );
    pipelineInfo.pGroups                           = shaderGroups.data( );
    pipelineInfo.maxPipelineRayRecursionDepth      = m_desc.RayTracing.MaxRecursionDepth;
    pipelineInfo.layout                            = pipelineLayout;
    pipelineInfo.pLibraryInterface                 = &pipelineInterface;

    VK_CHECK_RESULT( vkCreateRayTracingPipelinesKHR( m_context->LogicalDevice, nullptr, nullptr, 1, &pipelineInfo, nullptr, &m_instance ) );

    // Get shader identifiers for the SBT
    m_shaderIdentifiers.resize( pipelineInfo.groupCount * m_context->RayTracingProperties.shaderGroupHandleSize );
    VK_CHECK_RESULT(
        vkGetRayTracingShaderGroupHandlesKHR( m_context->LogicalDevice, m_instance, 0, pipelineInfo.groupCount, m_shaderIdentifiers.size( ), m_shaderIdentifiers.data( ) ) );
}

void *VulkanPipeline::GetShaderIdentifier( const std::string &exportName )
{
    return m_shaderIdentifiers.data( ) + m_shaderIdentifierOffsets[ exportName ];
}

void *VulkanPipeline::GetShaderIdentifier( const uint32_t offset )
{
    return m_shaderIdentifiers.data( ) + offset;
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

    const auto &compiledShaders = m_desc.ShaderProgram->GetCompiledShaders( );
    for ( int i = 0; i < compiledShaders.NumElements( ); ++i )
    {
        const auto                      &compiledShader        = compiledShaders.GetElement( i );
        VkPipelineShaderStageCreateInfo &shaderStageCreateInfo = pipelineStageCreateInfos.emplace_back( );
        shaderStageCreateInfo.sType                            = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        const VkShaderStageFlagBits stage        = VulkanEnumConverter::ConvertShaderStage( compiledShader->Stage );
        const VkShaderModule       &shaderModule = m_shaderModules.emplace_back( this->CreateShaderModule( compiledShader->Blob ) );

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
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
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
        auto &attachment                          = m_desc.Graphics.RenderTargets.GetElement( i );
        colorBlendAttachments[ i ].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        colorBlendAttachments[ i ].blendEnable         = attachment.Blend.Enable;
        colorBlendAttachments[ i ].srcColorBlendFactor = VulkanEnumConverter::ConvertBlend( attachment.Blend.SrcBlend );
        colorBlendAttachments[ i ].dstColorBlendFactor = VulkanEnumConverter::ConvertBlend( attachment.Blend.DstBlend );
        colorBlendAttachments[ i ].srcAlphaBlendFactor = VulkanEnumConverter::ConvertBlend( attachment.Blend.SrcBlendAlpha );
        colorBlendAttachments[ i ].dstAlphaBlendFactor = VulkanEnumConverter::ConvertBlend( attachment.Blend.DstBlendAlpha );
        colorBlendAttachments[ i ].colorBlendOp        = VulkanEnumConverter::ConvertBlendOp( attachment.Blend.BlendOp );
        colorBlendAttachments[ i ].alphaBlendOp        = VulkanEnumConverter::ConvertBlendOp( attachment.Blend.BlendOpAlpha );
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

VkShaderModule VulkanPipeline::CreateShaderModule( IDxcBlob *blob ) const
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo{ };
    shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = blob->GetBufferSize( );
    shaderModuleCreateInfo.pCode    = static_cast<const uint32_t *>( blob->GetBufferPointer( ) );

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
    vkDestroyPipelineLayout( m_context->LogicalDevice, m_layout, nullptr );
}

VkPipeline VulkanPipeline::Instance( ) const
{
    return m_instance;
}
VkPipelineBindPoint VulkanPipeline::BindPoint( ) const
{
    return m_bindPoint;
}
