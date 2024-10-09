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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanRootSignature.h>
#include <ranges>

using namespace DenOfIz;

VulkanRootSignature::VulkanRootSignature( VulkanContext *context, RootSignatureDesc desc ) : m_desc( std::move( desc ) ), m_context( context )
{
    for ( const ResourceBindingDesc &binding : m_desc.ResourceBindings )
    {
        AddResourceBinding( binding );
    }

    for ( const StaticSamplerDesc &staticSamplerDesc : m_desc.StaticSamplers )
    {
        AddStaticSampler( staticSamplerDesc );
    }

    m_pushConstants.resize( m_desc.RootConstants.size( ) );
    for ( const RootConstantResourceBindingDesc &rootConstantBinding : m_desc.RootConstants )
    {
        AddRootConstant( rootConstantBinding );
    }

    uint32_t maxRegisterSpace = 0;
    for ( const auto &key : m_layoutBindings | std::views::keys )
    {
        maxRegisterSpace = std::max( maxRegisterSpace, key );
    }

    VkDescriptorSetLayoutCreateInfo emptyLayoutInfo = { };
    emptyLayoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    emptyLayoutInfo.bindingCount                    = 0;
    VK_CHECK_RESULT( vkCreateDescriptorSetLayout( m_context->LogicalDevice, &emptyLayoutInfo, nullptr, &m_emptyLayout ) );

    // No bindings layout
    if ( !m_layoutBindings.empty( ) )
    {
        for ( uint32_t i = 0; i <= maxRegisterSpace; ++i )
        {
            const auto &it = m_layoutBindings.find( i );
            if ( it == m_layoutBindings.end( ) )
            {
                m_layouts.push_back( m_emptyLayout );
                continue;
            }

            const auto &layoutBindings = it->second;

            VkDescriptorSetLayoutCreateInfo layoutInfo{ };
            layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = layoutBindings.size( );
            layoutInfo.pBindings    = layoutBindings.data( );

            VkDescriptorSetLayout layout;
            VK_CHECK_RESULT( vkCreateDescriptorSetLayout( m_context->LogicalDevice, &layoutInfo, nullptr, &layout ) );
            m_layouts.push_back( layout );
        }
    }
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{ };
    pipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount         = m_layouts.size( );
    pipelineLayoutCreateInfo.pSetLayouts            = m_layouts.data( );
    pipelineLayoutCreateInfo.pushConstantRangeCount = m_pushConstants.size( );
    pipelineLayoutCreateInfo.pPushConstantRanges    = m_pushConstants.data( );
    VK_CHECK_RESULT( vkCreatePipelineLayout( m_context->LogicalDevice, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout ) );
}

void VulkanRootSignature::AddStaticSampler( const StaticSamplerDesc &sampler )
{
    VkDescriptorSetLayoutBinding layoutBinding = CreateDescriptorSetLayoutBinding( sampler.Binding );
    layoutBinding.pImmutableSamplers           = nullptr; // TODO

    m_bindings.push_back( layoutBinding );
}

void VulkanRootSignature::AddResourceBinding( const ResourceBindingDesc &binding )
{
    const ResourceBindingSlot slot{
        .Binding       = binding.Binding,
        .RegisterSpace = binding.RegisterSpace,
        .Type          = binding.BindingType,
    };

    m_resourceBindingMap[ slot.Key( ) ] = binding;

    const VkDescriptorSetLayoutBinding layoutBinding = CreateDescriptorSetLayoutBinding( binding );
    m_bindings.push_back( layoutBinding );
}

VkDescriptorSetLayoutBinding VulkanRootSignature::CreateDescriptorSetLayoutBinding( const ResourceBindingDesc &binding )
{
    VkDescriptorSetLayoutBinding layoutBinding{ };
    // HLSL buffer views in vulkan each have a specific offset
    uint32_t bindingTypeOffset = 0;
    switch ( ResourceDescriptorBindingType( binding.Descriptor ) )
    {
    case DescriptorBufferBindingType::ConstantBuffer:
        bindingTypeOffset = ShaderCompiler::VkShiftCbv;
        break;
    case DescriptorBufferBindingType::ShaderResource:
        bindingTypeOffset = ShaderCompiler::VkShiftSrv;
        break;
    case DescriptorBufferBindingType::UnorderedAccess:
        bindingTypeOffset = ShaderCompiler::VkShiftUav;
        break;
    case DescriptorBufferBindingType::Sampler:
        bindingTypeOffset = ShaderCompiler::VkShiftSampler;
        break;
    }

    layoutBinding.binding         = bindingTypeOffset + binding.Binding;
    layoutBinding.descriptorType  = VulkanEnumConverter::ConvertResourceDescriptorToDescriptorType( binding.Descriptor );
    layoutBinding.descriptorCount = binding.ArraySize;
    layoutBinding.stageFlags      = 0;
    for ( auto stage : binding.Stages )
    {
        layoutBinding.stageFlags |= VulkanEnumConverter::ConvertShaderStage( stage );
    }
    m_layoutBindings[ binding.RegisterSpace ].push_back( layoutBinding );
    // Update binding to include the offset
    const ResourceBindingSlot slot{
        .Binding       = binding.Binding,
        .RegisterSpace = binding.RegisterSpace,
        .Type          = binding.BindingType,
    };
    m_resourceBindingMap[ slot.Key( ) ].Binding = layoutBinding.binding;
    return layoutBinding;
}

void VulkanRootSignature::AddRootConstant( const RootConstantResourceBindingDesc &rootConstantBinding )
{
    if ( rootConstantBinding.Binding >= m_pushConstants.size( ) )
    {
        LOG( FATAL ) << "Root constant binding out of range: " << rootConstantBinding.Binding;
    }

    uint32_t offset = 0;
    for ( const RootConstantResourceBindingDesc &binding : m_desc.RootConstants )
    {
        if ( binding.Binding < rootConstantBinding.Binding )
        {
            offset += binding.NumBytes;
        }
    }

    VkPushConstantRange &pushConstantRange = m_pushConstants[ rootConstantBinding.Binding ];
    pushConstantRange.offset               = offset;
    pushConstantRange.size                 = rootConstantBinding.NumBytes;
    for ( auto stage : rootConstantBinding.Stages )
    {
        pushConstantRange.stageFlags |= VulkanEnumConverter::ConvertShaderStage( stage );
    }
}

VulkanRootSignature::~VulkanRootSignature( )
{
    for ( const auto layout : m_layouts )
    {
        if ( layout != m_emptyLayout )
        {
            vkDestroyDescriptorSetLayout( m_context->LogicalDevice, layout, nullptr );
        }
    }

    vkDestroyDescriptorSetLayout( m_context->LogicalDevice, m_emptyLayout, nullptr );
}

ResourceBindingDesc VulkanRootSignature::GetVkShiftedBinding( const ResourceBindingSlot &slot ) const
{
    return ContainerUtilities::SafeGetMapValue( m_resourceBindingMap, slot.Key( ), "Binding slot does not exist in root signature: " + slot.ToString( ) );
}

uint32_t VulkanRootSignature::NumRootConstants( ) const
{
    return m_pushConstants.size( );
}

VkPushConstantRange VulkanRootSignature::PushConstantRange( const uint32_t binding ) const
{
    if ( binding >= m_pushConstants.size( ) )
    {
        LOG( ERROR ) << "Root constant not found for binding " << binding;
    }
    return m_pushConstants[ binding ];
}

const VkDescriptorSetLayout &VulkanRootSignature::DescriptorSetLayout( const uint32_t registerSpace ) const
{
    if ( registerSpace >= m_layouts.size( ) )
    {
        LOG( ERROR ) << "Descriptor set not found for register space " << registerSpace;
    }

    return m_layouts[ registerSpace ];
}

VkPipelineLayout VulkanRootSignature::PipelineLayout( ) const
{
    return m_pipelineLayout;
}
