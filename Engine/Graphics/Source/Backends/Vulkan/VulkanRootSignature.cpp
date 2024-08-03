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

using namespace DenOfIz;

VulkanRootSignature::VulkanRootSignature( VulkanContext *context, RootSignatureDesc desc ) : m_context( context ), m_desc( std::move( desc ) )
{
    for ( const ResourceBindingDesc &binding : m_desc.ResourceBindings )
    {
        AddResourceBinding( binding );
    }

    for ( const StaticSamplerDesc &staticSamplerDesc : m_desc.StaticSamplers )
    {
        AddStaticSampler( staticSamplerDesc );
    }

    int registerSpace = 0;
    for ( int i = 0; i < m_layoutBindings.size( ); )
    {
        if ( m_layoutBindings.find( i ) == m_layoutBindings.end( ) )
        {
            registerSpace++;
            continue;
        }

        const auto &layoutBindings = m_layoutBindings[ i++ ];

        VkDescriptorSetLayoutCreateInfo layoutInfo{ };
        layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = layoutBindings.size( );
        layoutInfo.pBindings    = layoutBindings.data( );
        layoutInfo.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

        VkDescriptorSetLayout layout;
        VK_CHECK_RESULT( vkCreateDescriptorSetLayout( m_context->LogicalDevice, &layoutInfo, nullptr, &layout ) );
        m_layouts.push_back( layout );
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{ };
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = m_bindings.size( );
    layoutInfo.pBindings    = m_bindings.data( );
    layoutInfo.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

    VK_CHECK_RESULT( vkCreateDescriptorSetLayout( m_context->LogicalDevice, &layoutInfo, nullptr, &m_layouts[ 0 ] ) );

    std::vector<VkDescriptorPoolSize> poolSizes;
    for ( const auto &bindingDesc : m_resourceBindingMap )
    {
        const auto          &binding = bindingDesc.second;
        VkDescriptorPoolSize poolSize{ };
        poolSize.type            = VulkanEnumConverter::ConvertResourceDescriptorToDescriptorType( binding.Descriptor );
        poolSize.descriptorCount = binding.ArraySize;
        poolSizes.push_back( poolSize );
    }

    VkDescriptorPoolCreateInfo poolInfo{ };
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size( );
    poolInfo.pPoolSizes    = poolSizes.data( );
    poolInfo.maxSets       = m_layouts.size( );

    VK_CHECK_RESULT( vkCreateDescriptorPool( m_context->LogicalDevice, &poolInfo, nullptr, &m_descriptorPool ) );

    std::vector<VkDescriptorSetLayout> layouts( m_layouts.size( ) );
    for ( const auto &layout : m_layouts )
    {
        layouts.push_back( layout );
    }

    VkDescriptorSetAllocateInfo setAllocateInfo{ };
    setAllocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocateInfo.descriptorPool     = m_descriptorPool;
    setAllocateInfo.descriptorSetCount = m_layouts.size( );
    setAllocateInfo.pSetLayouts        = m_layouts.data( );

    m_descriptorSets.resize( m_layouts.size( ) );
    VK_CHECK_RESULT( vkAllocateDescriptorSets( m_context->LogicalDevice, &setAllocateInfo, m_descriptorSets.data( ) ) );
}

void VulkanRootSignature::AddStaticSampler( const StaticSamplerDesc &sampler )
{
    VkDescriptorSetLayoutBinding layoutBinding = CreateDescriptorSetLayoutBinding( sampler.Binding );
    layoutBinding.pImmutableSamplers           = nullptr; // TODO

    m_bindings.push_back( layoutBinding );
}

void VulkanRootSignature::AddResourceBindingInternal( const ResourceBindingDesc &binding )
{
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
    m_resourceBindingMap[ binding.Name ].Binding = layoutBinding.binding;
    return layoutBinding;
}

void VulkanRootSignature::AddRootConstantInternal( const RootConstantResourceBinding &rootConstantBinding )
{
    m_rootConstantMap[ rootConstantBinding.Name ] = rootConstantBinding;

    VkPushConstantRange pushConstantRange{ };

    pushConstantRange.offset = rootConstantBinding.Binding;
    pushConstantRange.size   = rootConstantBinding.Size;

    for ( auto stage : rootConstantBinding.Stages )
    {
        pushConstantRange.stageFlags |= VulkanEnumConverter::ConvertShaderStage( stage );
    }

    m_pushConstants.push_back( pushConstantRange );
}

VulkanRootSignature::~VulkanRootSignature( )
{
    for ( const auto layout : m_layouts )
    {
        vkDestroyDescriptorSetLayout( m_context->LogicalDevice, layout, nullptr );
    }
    vkDestroyDescriptorPool( m_context->LogicalDevice, m_descriptorPool, nullptr );
}
