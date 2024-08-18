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

VulkanRootSignature::VulkanRootSignature( VulkanContext *context, RootSignatureDesc desc ) : IRootSignature( desc ), m_desc( std::move( desc ) ), m_context( context )
{
    for ( const ResourceBindingDesc &binding : m_desc.ResourceBindings )
    {
        AddResourceBinding( binding );
    }

    for ( const StaticSamplerDesc &staticSamplerDesc : m_desc.StaticSamplers )
    {
        AddStaticSampler( staticSamplerDesc );
    }

    uint32_t maxRegisterSpace = 0;
    for ( const auto &key : m_layoutBindings | std::views::keys )
    {
        maxRegisterSpace = std::max( maxRegisterSpace, key );
    }

    for ( uint32_t i = 0; i <= maxRegisterSpace; ++i )
    {
        const auto &it = m_layoutBindings.find( i );
        if ( it == m_layoutBindings.end( ) )
        {
            m_layouts.push_back( nullptr );
            continue;
        }

        const auto &layoutBindings = it->second;

        VkDescriptorSetLayoutCreateInfo layoutInfo{ };
        layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = layoutBindings.size( );
        layoutInfo.pBindings    = layoutBindings.data( );
        //        layoutInfo.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

        VkDescriptorSetLayout layout;
        VK_CHECK_RESULT( vkCreateDescriptorSetLayout( m_context->LogicalDevice, &layoutInfo, nullptr, &layout ) );
        m_layouts.push_back( layout );
    }
}

void VulkanRootSignature::AddStaticSampler( const StaticSamplerDesc &sampler )
{
    VkDescriptorSetLayoutBinding layoutBinding = CreateDescriptorSetLayoutBinding( sampler.Binding );
    layoutBinding.pImmutableSamplers           = nullptr; // TODO

    m_bindings.push_back( layoutBinding );
}

void VulkanRootSignature::AddResourceBinding( const ResourceBindingDesc &binding )
{
    m_resourceBindingMap[ binding.Name ] = binding;

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

void VulkanRootSignature::AddRootConstant( const RootConstantResourceBinding &rootConstantBinding )
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
}
