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
    auto rootSignature = reinterpret_cast<VulkanRootSignature *>( desc.RootSignature );
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
    for ( const auto &compiledShader : m_desc.ShaderProgram->GetCompiledShaders( ) )
    {
        VkPipelineShaderStageCreateInfo &shaderStageCreateInfo = pipelineStageCreateInfos.emplace_back( );
        shaderStageCreateInfo.sType                            = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        const VkShaderStageFlagBits stage        = VulkanEnumConverter::ConvertShaderStage( compiledShader->Stage );
        const VkShaderModule       &shaderModule = m_shaderModules.emplace_back( this->CreateShaderModule( compiledShader->Blob ) );

        shaderStageCreateInfo.stage  = stage;
        shaderStageCreateInfo.module = shaderModule;
        shaderStageCreateInfo.pName  = compiledShader->EntryPoint.c_str( );
        shaderStageCreateInfo.pNext  = nullptr;
    }

    return pipelineStageCreateInfos;
}

[[nodiscard]] VkPipelineRenderingCreateInfo VulkanPipeline::ConfigureRenderingInfo( std::vector<VkFormat> &colorAttachmentsStore ) const
{
    for ( auto attachment : m_desc.Rendering.RenderTargets )
    {
        colorAttachmentsStore.push_back( VulkanEnumConverter::ConvertImageFormat( attachment.Format ) );
    }

    VkPipelineRenderingCreateInfo renderingCreateInfo{ };
    renderingCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.viewMask                = m_desc.Rendering.ViewMask;
    renderingCreateInfo.colorAttachmentCount    = colorAttachmentsStore.size( );
    renderingCreateInfo.pColorAttachmentFormats = colorAttachmentsStore.data( );
    renderingCreateInfo.depthAttachmentFormat   = VulkanEnumConverter::ConvertImageFormat( m_desc.Rendering.DepthStencilAttachmentFormat );
    renderingCreateInfo.stencilAttachmentFormat = VulkanEnumConverter::ConvertImageFormat( m_desc.Rendering.DepthStencilAttachmentFormat );
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
    inputAssemblyCreateInfo.topology               = VulkanEnumConverter::ConvertPrimitiveTopology( m_desc.PrimitiveTopology );
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
    return inputAssemblyCreateInfo;
}

[[nodiscard]] VkPipelineVertexInputStateCreateInfo VulkanPipeline::ConfigureVertexInputState( ) const
{
    const auto                                *inputLayout          = reinterpret_cast<VulkanInputLayout *>( m_desc.InputLayout );
    const VkPipelineVertexInputStateCreateInfo inputStateCreateInfo = inputLayout->GetVertexInputState( );
    return inputStateCreateInfo;
}

VkPipelineMultisampleStateCreateInfo VulkanPipeline::ConfigureMultisampling( ) const
{
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{ };
    multisampleStateCreateInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;

    switch ( m_desc.MSAASampleCount )
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

    switch ( m_desc.CullMode )
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
    const uint32_t attachmentCount = m_desc.Rendering.RenderTargets.size( );
    colorBlendAttachments.resize( attachmentCount );

    for ( uint32_t i = 0; i < attachmentCount; ++i )
    {
        auto &attachment                          = m_desc.Rendering.RenderTargets[ i ];
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
    colorBlending.logicOpEnable       = m_desc.Rendering.BlendLogicOpEnable;
    colorBlending.logicOp             = VulkanEnumConverter::ConvertLogicOp( m_desc.Rendering.BlendLogicOp );
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
    depthStencilStateCreateInfo.depthTestEnable       = m_desc.DepthTest.Enable;
    depthStencilStateCreateInfo.depthWriteEnable      = m_desc.DepthTest.Write;
    depthStencilStateCreateInfo.depthCompareOp        = VulkanEnumConverter::ConvertCompareOp( m_desc.DepthTest.CompareOp );
    depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.minDepthBounds        = 0.0f;
    depthStencilStateCreateInfo.maxDepthBounds        = 1.0f;

    depthStencilStateCreateInfo.stencilTestEnable = m_desc.StencilTest.Enable;

    depthStencilStateCreateInfo.front = VkStencilOpState{ };
    depthStencilStateCreateInfo.back  = VkStencilOpState{ };

    auto initStencilState = [ =, this ]( VkStencilOpState &vkState, const StencilFace &state )
    {
        vkState.compareOp   = VulkanEnumConverter::ConvertCompareOp( state.CompareOp );
        vkState.compareMask = m_desc.StencilTest.ReadMask;
        vkState.writeMask   = m_desc.StencilTest.WriteMask;
        vkState.reference   = 0;
        vkState.failOp      = VulkanEnumConverter::ConvertStencilOp( state.FailOp );
        vkState.passOp      = VulkanEnumConverter::ConvertStencilOp( state.PassOp );
        vkState.depthFailOp = VulkanEnumConverter::ConvertStencilOp( state.DepthFailOp );
    };

    if ( m_desc.StencilTest.Enable )
    {
        initStencilState( depthStencilStateCreateInfo.front, m_desc.StencilTest.FrontFace );
        initStencilState( depthStencilStateCreateInfo.back, m_desc.StencilTest.BackFace );
    }

    return depthStencilStateCreateInfo;
}

VkShaderModule VulkanPipeline::CreateShaderModule( IDxcBlob *blob ) const
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo{ };
    shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = blob->GetBufferSize( );
    shaderModuleCreateInfo.pCode    = static_cast<const uint32_t *>( blob->GetBufferPointer( ) );

    VkShaderModule shaderModule;
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
