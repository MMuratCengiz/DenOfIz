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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

#define BIND_GROUP_LAYOUT_IMPL( handle ) static_cast<VulkanBindGroupLayout *>( DENOFIZ_FROM_HANDLE( IBindGroupLayout, handle ) )

VulkanRootSignature::VulkanRootSignature( VulkanContext *context, DenOfIz_RootSignatureDesc desc ) : m_desc( std::move( desc ) ), m_context( context )
{
    uint32_t maxRegisterSpace = 0;
    for ( uint32_t i = 0; i < m_desc.BindGroupLayouts.NumElements; ++i )
    {
        VulkanBindGroupLayout *layout = BIND_GROUP_LAYOUT_IMPL( m_desc.BindGroupLayouts.Elements[ i ] );
        maxRegisterSpace              = std::max( maxRegisterSpace, layout->RegisterSpace( ) );
    }

    m_bindGroupLayouts.resize( maxRegisterSpace + 1, nullptr );
    for ( uint32_t i = 0; i < m_desc.BindGroupLayouts.NumElements; ++i )
    {
        VulkanBindGroupLayout *layout                  = BIND_GROUP_LAYOUT_IMPL( m_desc.BindGroupLayouts.Elements[ i ] );
        m_bindGroupLayouts[ layout->RegisterSpace( ) ] = layout;
    }

    m_pushConstants.resize( m_desc.RootConstants.NumElements );
    for ( uint32_t i = 0; i < m_desc.RootConstants.NumElements; ++i )
    {
        const DenOfIz_RootConstantBindingDesc &rootConstantBinding = m_desc.RootConstants.Elements[ i ];
        AddRootConstant( rootConstantBinding );
    }

    VkDescriptorSetLayoutCreateInfo emptyLayoutInfo = { };
    emptyLayoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    emptyLayoutInfo.bindingCount                    = 0;
    VK_CHECK_RESULT( vkCreateDescriptorSetLayout( m_context->LogicalDevice, &emptyLayoutInfo, nullptr, &m_emptyLayout ) );

    for ( uint32_t i = 0; i <= maxRegisterSpace; ++i )
    {
        VulkanBindGroupLayout *layout = m_bindGroupLayouts[ i ];
        if ( layout == nullptr || !layout->HasBindings( ) )
        {
            m_layouts.push_back( m_emptyLayout );
        }
        else
        {
            m_layouts.push_back( layout->DescriptorSetLayout( ) );
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

void VulkanRootSignature::AddRootConstant( const DenOfIz_RootConstantBindingDesc &rootConstantBinding )
{
    if ( rootConstantBinding.Binding >= m_pushConstants.size( ) )
    {
        spdlog::critical( "Root constant binding out of range: {}", rootConstantBinding.Binding );
    }

    uint32_t offset = 0;
    for ( uint32_t i = 0; i < m_desc.RootConstants.NumElements; ++i )
    {
        const DenOfIz_RootConstantBindingDesc &binding = m_desc.RootConstants.Elements[ i ];
        if ( binding.Binding < rootConstantBinding.Binding )
        {
            offset += binding.NumBytes;
        }
    }

    VkPushConstantRange &pushConstantRange = m_pushConstants[ rootConstantBinding.Binding ];
    pushConstantRange.offset               = offset;
    pushConstantRange.size                 = rootConstantBinding.NumBytes;
    pushConstantRange.stageFlags           = DenOfIz_VulkanEnumConverter_ConvertShaderStage( rootConstantBinding.Stages );
}

VulkanRootSignature::~VulkanRootSignature( )
{
    if ( m_pipelineLayout != VK_NULL_HANDLE )
    {
        vkDestroyPipelineLayout( m_context->LogicalDevice, m_pipelineLayout, nullptr );
    }

    if ( m_emptyLayout != VK_NULL_HANDLE )
    {
        vkDestroyDescriptorSetLayout( m_context->LogicalDevice, m_emptyLayout, nullptr );
    }
}

uint32_t VulkanRootSignature::NumRootConstants( ) const
{
    return m_pushConstants.size( );
}

std::vector<VkPushConstantRange> VulkanRootSignature::PushConstantRanges( ) const
{
    return m_pushConstants;
}

VkPushConstantRange VulkanRootSignature::PushConstantRange( const uint32_t binding ) const
{
    if ( binding >= m_pushConstants.size( ) )
    {
        spdlog::error( "Root constant not found for binding {}", binding );
    }
    return m_pushConstants[ binding ];
}

std::vector<VkDescriptorSetLayout> VulkanRootSignature::DescriptorSetLayouts( ) const
{
    return m_layouts;
}

const VkDescriptorSetLayout &VulkanRootSignature::DescriptorSetLayout( const uint32_t registerSpace ) const
{
    if ( registerSpace >= m_layouts.size( ) )
    {
        spdlog::error( "Descriptor set not found for register space {}", registerSpace );
    }
    return m_layouts[ registerSpace ];
}

VkPipelineLayout VulkanRootSignature::PipelineLayout( ) const
{
    return m_pipelineLayout;
}

VkDescriptorSetLayout VulkanRootSignature::EmptyLayout( ) const
{
    return m_emptyLayout;
}

const std::vector<VulkanBindGroupLayout *> &VulkanRootSignature::BindGroupLayouts( ) const
{
    return m_bindGroupLayouts;
}

VulkanBindGroupLayout *VulkanRootSignature::GetBindGroupLayout( uint32_t registerSpace ) const
{
    if ( registerSpace >= m_bindGroupLayouts.size( ) )
    {
        spdlog::error( "Register space {} does not exist.", registerSpace );
        return nullptr;
    }
    return m_bindGroupLayouts[ registerSpace ];
}
