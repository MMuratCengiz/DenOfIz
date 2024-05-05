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

#ifdef BUILD_VK

#include <DenOfIzGraphics/Backends/Vulkan/VulkanPipeline.h>

using namespace DenOfIz;

VulkanPipeline::VulkanPipeline( VulkanContext *context, const PipelineCreateInfo &createInfo ) :
    m_Context( context ), m_CreateInfo( createInfo ), BindPoint( VulkanEnumConverter::ConvertPipelineBindPoint( createInfo.BindPoint ) )
{
    m_PipelineCreateInfo.pDepthStencilState = nullptr;

    ConfigureVertexInput();
    ConfigureViewport();
    ConfigureRasterization();
    ConfigureMultisampling();
    ConfigureColorBlend();
    ConfigureDynamicState();
    CreatePipelineLayout();
    CreateDepthAttachmentImages();
    CreateRenderPass();
    CreatePipeline();
}

void VulkanPipeline::ConfigureVertexInput()
{
    bool hasTessellationShaders = false;

    for ( const auto &[Stage, Data] : m_CreateInfo.SpvProgram.Shaders )
    {
        vk::PipelineShaderStageCreateInfo &shaderStageCreateInfo = m_PipelineStageCreateInfos.emplace_back();

        const vk::ShaderStageFlagBits stage = VulkanEnumConverter::ConvertShaderStage( Stage );
        vk::ShaderModule &shaderModule = m_ShaderModules.emplace_back( this->CreateShaderModule( Data ) );

        shaderStageCreateInfo.stage = stage;
        shaderStageCreateInfo.module = shaderModule;
        shaderStageCreateInfo.pName = "main";
        shaderStageCreateInfo.pNext = nullptr;

        hasTessellationShaders = hasTessellationShaders || stage == vk::ShaderStageFlagBits::eTessellationEvaluation;
        hasTessellationShaders = hasTessellationShaders || stage == vk::ShaderStageFlagBits::eTessellationControl;
    }

    auto &program = m_CreateInfo.SpvProgram;
    auto &vertexInputs = program.VertexInputs();

    uint32_t offsetIter = 0;
    for ( const VertexInput &vertexInput : vertexInputs )
    {
        vk::VertexInputAttributeDescription &desc = m_VertexAttributeDescriptions.emplace_back();

        if ( m_CreateInfo.InterleavedMode )
        {
            desc.binding = 0;
        }
        else
        {
            vk::VertexInputBindingDescription &bindingDesc = m_InputBindingDescriptions.emplace_back();
            bindingDesc.binding = m_InputBindingDescriptions.size() - 1;
            bindingDesc.inputRate = vk::VertexInputRate::eVertex; // TODO investigate later for instanced rendering
            bindingDesc.stride = 0;

            desc.binding = bindingDesc.binding;
        }

        desc.location = vertexInput.Location;
        desc.format = VulkanEnumConverter::ConvertImageFormat( vertexInput.Format );
        desc.offset = vertexInput.Offset;
        offsetIter += vertexInput.Size;
    }

    if ( m_CreateInfo.InterleavedMode )
    {
        vk::VertexInputBindingDescription &bindingDesc = m_InputBindingDescriptions.emplace_back();
        bindingDesc.binding = 0;
        bindingDesc.inputRate = vk::VertexInputRate::eVertex; // TODO investigate later for instanced rendering
        bindingDesc.stride = offsetIter;
    }

    m_InputStateCreateInfo.vertexBindingDescriptionCount = m_InputBindingDescriptions.size();
    m_InputStateCreateInfo.pVertexBindingDescriptions = m_InputBindingDescriptions.data();
    m_InputStateCreateInfo.vertexAttributeDescriptionCount = m_VertexAttributeDescriptions.size();
    m_InputStateCreateInfo.pVertexAttributeDescriptions = m_VertexAttributeDescriptions.data();

    m_InputAssemblyCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;
    m_InputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    m_PipelineCreateInfo.stageCount = static_cast<uint32_t>(m_PipelineStageCreateInfos.size());
    m_PipelineCreateInfo.pStages = m_PipelineStageCreateInfos.data();

    // Todo read patch control points from either pipelineRequest or from GLSLShaderSet
    m_TessellationStateCreateInfo.patchControlPoints = 3;

    if ( hasTessellationShaders )
    {
        m_InputAssemblyCreateInfo.topology = vk::PrimitiveTopology::ePatchList;
        m_PipelineCreateInfo.pTessellationState = &m_TessellationStateCreateInfo;
    }

    m_PipelineCreateInfo.pVertexInputState = &m_InputStateCreateInfo;
    m_PipelineCreateInfo.pInputAssemblyState = &m_InputAssemblyCreateInfo;
}

void VulkanPipeline::ConfigureMultisampling()
{
    m_MultisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;

    switch ( m_CreateInfo.MSAASampleCount )
    {
    case MSAASampleCount::_0:
        m_MultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    case MSAASampleCount::_1:
        m_MultisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
        break;
    case MSAASampleCount::_2:
        m_MultisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e2;
        break;
    case MSAASampleCount::_4:
        m_MultisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e4;
        break;
    case MSAASampleCount::_8:
        m_MultisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e8;
        break;
    case MSAASampleCount::_16:
        m_MultisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e16;
        break;
    case MSAASampleCount::_32:
        m_MultisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e32;
        break;
    case MSAASampleCount::_64:
        m_MultisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e64;
        break;
    }

    m_MultisampleStateCreateInfo.minSampleShading = 1.0f;
    m_MultisampleStateCreateInfo.pSampleMask = nullptr;
    m_MultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
    m_MultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
    m_MultisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;
    m_MultisampleStateCreateInfo.minSampleShading = .2f;

    m_PipelineCreateInfo.pMultisampleState = &m_MultisampleStateCreateInfo;
}

void VulkanPipeline::ConfigureViewport()
{
    // Todo test, these are dynamic states
    m_ViewportStateCreateInfo.viewportCount = 0;
    m_ViewportStateCreateInfo.pViewports = nullptr;
    m_ViewportStateCreateInfo.scissorCount = 0;
    m_ViewportStateCreateInfo.pScissors = nullptr;
    m_PipelineCreateInfo.pViewportState = &m_ViewportStateCreateInfo;
}

void VulkanPipeline::ConfigureRasterization()
{
    m_RasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    m_RasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    m_RasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;
    m_RasterizationStateCreateInfo.lineWidth = 1.0f;

    switch ( m_CreateInfo.CullMode )
    {
    case CullMode::FrontAndBackFace:
        m_RasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eFrontAndBack;
        break;
    case CullMode::BackFace:
        m_RasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
        break;
    case CullMode::FrontFace:
        m_RasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eFront;
        break;
    case CullMode::None:
        m_RasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eNone;
        break;
    }

    m_RasterizationStateCreateInfo.frontFace = vk::FrontFace::eCounterClockwise;
    m_RasterizationStateCreateInfo.depthBiasEnable = false; // Todo, test if works with dynamic state
    m_RasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
    m_RasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    m_RasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

    m_PipelineCreateInfo.pRasterizationState = &m_RasterizationStateCreateInfo;
}

void VulkanPipeline::ConfigureColorBlend()
{
    const uint32_t attachmentCount = m_CreateInfo.BlendModes.size();
    m_ColorBlendAttachments.resize( attachmentCount );

    for ( int i = 0; i < attachmentCount; ++i )
    {
        m_ColorBlendAttachments[ i ].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        m_ColorBlendAttachments[ i ].blendEnable = false;

        if ( m_CreateInfo.BlendModes[ i ] == BlendMode::AlphaBlend )
        {
            m_ColorBlendAttachments[ i ].blendEnable = true;
            m_ColorBlendAttachments[ i ].srcColorBlendFactor = vk::BlendFactor::eOne;
            m_ColorBlendAttachments[ i ].dstColorBlendFactor = vk::BlendFactor::eOne;

            m_ColorBlendAttachments[ i ].srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
            m_ColorBlendAttachments[ i ].dstAlphaBlendFactor = vk::BlendFactor::eDstAlpha;

            m_ColorBlendAttachments[ i ].colorBlendOp = vk::BlendOp::eAdd;
            m_ColorBlendAttachments[ i ].alphaBlendOp = vk::BlendOp::eAdd;
        }
    }

    // This overwrites the above
    m_ColorBlending.logicOpEnable = false;
    m_ColorBlending.logicOp = vk::LogicOp::eCopy;
    m_ColorBlending.attachmentCount = attachmentCount;
    m_ColorBlending.pAttachments = m_ColorBlendAttachments.data();
    m_ColorBlending.blendConstants[ 0 ] = 0.0f;
    m_ColorBlending.blendConstants[ 1 ] = 0.0f;
    m_ColorBlending.blendConstants[ 2 ] = 0.0f;
    m_ColorBlending.blendConstants[ 3 ] = 0.0f;
    m_PipelineCreateInfo.pColorBlendState = &m_ColorBlending;
}

void VulkanPipeline::ConfigureDynamicState()
{
    m_DynamicStateCreateInfo.dynamicStateCount = m_DynamicStates.size();
    m_DynamicStateCreateInfo.pDynamicStates = m_DynamicStates.data();
    m_PipelineCreateInfo.pDynamicState = &m_DynamicStateCreateInfo;
}

void VulkanPipeline::CreatePipelineLayout()
{
    // Layout binding per set!
    std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>> bindings;

    for ( const ShaderUniformInput &input : m_CreateInfo.SpvProgram.UniformInputs() )
    {
        vk::DescriptorSetLayoutBinding &binding = bindings[ input.BoundDescriptorSet ].emplace_back();

        binding.binding = input.Binding;

        switch ( input.Type )
        {
        case UniformType::Struct:
            binding.descriptorType = vk::DescriptorType::eUniformBuffer;
            break;
        case UniformType::Sampler:
            binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
            break;
        }
        binding.descriptorCount = input.ArraySize;
        binding.stageFlags = VulkanEnumConverter::ConvertShaderStage( input.Stage );

        vk::WriteDescriptorSet &writeDescriptorSet = m_DescriptorSets[ input.Name ];
        writeDescriptorSet.descriptorType = binding.descriptorType;
        writeDescriptorSet.dstBinding = input.Binding;
    }

    for ( auto binding : bindings | std::views::values )
    {
        vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.bindingCount = binding.size();
        layoutCreateInfo.pBindings = binding.data();
        layoutCreateInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR;

        m_Layouts.emplace_back( m_Context->LogicalDevice.createDescriptorSetLayout( layoutCreateInfo ) );
    }

    m_PipelineLayoutCreateInfo.setLayoutCount = m_Layouts.size();
    m_PipelineLayoutCreateInfo.pSetLayouts = m_Layouts.data();

    for ( const PushConstant &pushConstant : m_CreateInfo.SpvProgram.PushConstants() )
    {
        vk::PushConstantRange &pushConstantRange = m_PushConstants.emplace_back();
        pushConstantRange.stageFlags = VulkanEnumConverter::ConvertShaderStage( pushConstant.Stage );
        pushConstantRange.offset = pushConstant.Offset;
        pushConstantRange.size = pushConstant.Size;
    }

    m_PipelineLayoutCreateInfo.pushConstantRangeCount = m_PushConstants.size();
    m_PipelineLayoutCreateInfo.pPushConstantRanges = m_PushConstants.data();

    Layout = m_Context->LogicalDevice.createPipelineLayout( m_PipelineLayoutCreateInfo );
    m_PipelineCreateInfo.layout = Layout;
}

void VulkanPipeline::CreateRenderPass()
{
    m_PipelineCreateInfo.renderPass = nullptr;
    m_PipelineCreateInfo.subpass = 0;
    m_PipelineCreateInfo.basePipelineHandle = nullptr;
    m_PipelineCreateInfo.basePipelineIndex = -1;
}

void VulkanPipeline::CreateDepthAttachmentImages()
{
    m_DepthStencilStateCreateInfo.depthTestEnable = m_CreateInfo.EnableDepthTest;
    m_DepthStencilStateCreateInfo.depthWriteEnable = m_CreateInfo.EnableDepthTest;
    m_DepthStencilStateCreateInfo.depthCompareOp = VulkanEnumConverter::ConvertCompareOp( m_CreateInfo.DepthCompareOp );
    m_DepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    m_DepthStencilStateCreateInfo.minDepthBounds = 0.0f;
    m_DepthStencilStateCreateInfo.maxDepthBounds = 1.0f;

    bool enableStencilTest = m_CreateInfo.StencilTestStateFront.enabled || m_CreateInfo.StencilTestStateBack.enabled;

    m_DepthStencilStateCreateInfo.stencilTestEnable = enableStencilTest;

    m_DepthStencilStateCreateInfo.front = vk::StencilOpState{};
    m_DepthStencilStateCreateInfo.back = vk::StencilOpState{};

    auto initStencilState = [=]( vk::StencilOpState &vkState, const StencilTestState &state )
    {
        ReturnIf( !state.enabled );
        vkState.compareOp = VulkanEnumConverter::ConvertCompareOp( state.compareOp );
        vkState.compareMask = state.compareMask;
        vkState.writeMask = state.writeMask;
        vkState.reference = state.ref;
        vkState.failOp = VulkanEnumConverter::ConvertStencilOp( state.failOp );
        vkState.passOp = VulkanEnumConverter::ConvertStencilOp( state.passOp );
        vkState.depthFailOp = VulkanEnumConverter::ConvertStencilOp( state.depthFailOp );
    };

    initStencilState( m_DepthStencilStateCreateInfo.front, m_CreateInfo.StencilTestStateFront );
    initStencilState( m_DepthStencilStateCreateInfo.back, m_CreateInfo.StencilTestStateFront );

    m_PipelineCreateInfo.pDepthStencilState = &m_DepthStencilStateCreateInfo;
}

vk::ShaderModule VulkanPipeline::CreateShaderModule( const std::vector<uint32_t> &data ) const
{
    vk::ShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.codeSize = data.size() * sizeof( uint32_t );
    shaderModuleCreateInfo.pCode = data.data();

    return m_Context->LogicalDevice.createShaderModule( shaderModuleCreateInfo );
}

void VulkanPipeline::CreatePipeline()
{
    m_ColorFormats.resize( m_CreateInfo.Rendering.ColorAttachmentFormats.size() );
    for ( auto attachment : m_CreateInfo.Rendering.ColorAttachmentFormats )
    {
        m_ColorFormats.push_back( VulkanEnumConverter::ConvertImageFormat( attachment ) );
    }

    m_RenderingCreateInfo = vk::PipelineRenderingCreateInfo( m_CreateInfo.Rendering.ViewMask, m_ColorFormats,
                                                           VulkanEnumConverter::ConvertImageFormat( m_CreateInfo.Rendering.DepthAttachmentFormat ),
                                                           VulkanEnumConverter::ConvertImageFormat( m_CreateInfo.Rendering.StencilAttachmentFormat ) );

    m_PipelineCreateInfo.pNext = &m_RenderingCreateInfo;
    Instance = m_Context->LogicalDevice.createGraphicsPipeline( nullptr, m_PipelineCreateInfo ).value;
}

vk::WriteDescriptorSet VulkanPipeline::GetWriteDescriptorSet( const std::string &name )
{
    if ( !m_DescriptorSets.contains( name ) )
    {
        LOG( Verbosity::Critical, "VulkanPipeline", "Invalid descriptor set, about to crash!" );
    }

    return m_DescriptorSets[ name ];
}

VulkanPipeline::~VulkanPipeline()
{
    for ( const auto &module : m_ShaderModules )
    {
        m_Context->LogicalDevice.destroyShaderModule( module );
    }

    for ( const auto &layout : m_Layouts )
    {
        m_Context->LogicalDevice.destroyDescriptorSetLayout( layout );
    }

    m_Context->LogicalDevice.destroyPipeline( Instance );
    m_Context->LogicalDevice.destroyPipelineLayout( Layout );
}

#endif
