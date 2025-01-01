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

#include <DenOfIzGraphics/Backends/Vulkan/RayTracing/VulkanLocalRootSignature.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h>
#include <DenOfIzGraphics/Utilities/ContainerUtilities.h>
#include <DenOfIzGraphics/Utilities/Utilities.h>

using namespace DenOfIz;

VulkanLocalRootSignature::VulkanLocalRootSignature( VulkanContext *context, const LocalRootSignatureDesc &desc ) : m_context( context ), m_desc( desc )
{
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

    VkPhysicalDeviceProperties2 properties2{ };
    properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    properties2.pNext = nullptr;
    vkGetPhysicalDeviceProperties2( context->PhysicalDevice, &properties2 );

    uint32_t minAlignment  = properties2.properties.limits.minStorageBufferOffsetAlignment;
    uint32_t currentOffset = 0;
    for ( uint32_t i = 0; i < desc.ResourceBindings.NumElements( ); ++i )
    {
        const auto &binding = desc.ResourceBindings.GetElement( i );

        uint32_t bindingTypeOffset = 0;
        switch ( binding.BindingType )
        {
        case ResourceBindingType::ConstantBuffer:
            {
                bindingTypeOffset          = ShaderCompiler::VkShiftCbv;
                const uint32_t alignedSize = Utilities::Align( binding.Reflection.NumBytes, minAlignment );

                m_inlineDataOffsets.resize( std::max<size_t>( m_inlineDataOffsets.size( ), binding.Binding + 1 ) );
                m_inlineDataNumBytes.resize( m_inlineDataOffsets.size( ) );

                m_inlineDataOffsets[ binding.Binding ]  = currentOffset;
                m_inlineDataNumBytes[ binding.Binding ] = binding.Reflection.NumBytes;

                currentOffset += alignedSize;
                m_totalInlineDataBytes += alignedSize;
            }
            break;
        case ResourceBindingType::ShaderResource:
            bindingTypeOffset = ShaderCompiler::VkShiftSrv;
            break;
        case ResourceBindingType::UnorderedAccess:
            bindingTypeOffset = ShaderCompiler::VkShiftUav;
            break;
        case ResourceBindingType::Sampler:
            bindingTypeOffset = ShaderCompiler::VkShiftSampler;
            break;
        }

        VkDescriptorSetLayoutBinding layoutBinding{ };
        layoutBinding.binding         = bindingTypeOffset + binding.Binding;
        layoutBinding.descriptorCount = 1;
        layoutBinding.descriptorType  = VulkanEnumConverter::ConvertResourceDescriptorToDescriptorType( binding.Descriptor );
        layoutBinding.stageFlags      = 0;

        for ( uint32_t j = 0; j < binding.Stages.NumElements( ); ++j )
        {
            layoutBinding.stageFlags |= VulkanEnumConverter::ConvertShaderStage( binding.Stages.GetElement( j ) );
        }

        // Store binding by register space
        m_layoutBindings[ binding.RegisterSpace ].push_back( layoutBinding );
    }

    for ( auto &[ space, bindings ] : m_layoutBindings )
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo{ };
        layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = bindings.size( );
        layoutInfo.pBindings    = bindings.data( );

        VkDescriptorSetLayout layout;
        VK_CHECK_RESULT( vkCreateDescriptorSetLayout( context->LogicalDevice, &layoutInfo, nullptr, &layout ) );

        m_layouts.push_back( { layout, space } );
    }
}

VulkanLocalRootSignature::~VulkanLocalRootSignature( )
{
    for ( auto layout : m_layouts )
    {
        vkDestroyDescriptorSetLayout( m_context->LogicalDevice, layout.Layout, nullptr );
    }
}

[[nodiscard]] std::vector<VkLayoutWithSet> VulkanLocalRootSignature::DescriptorSetLayouts( )
{
    return m_layouts;
}

VkDescriptorSetLayout *VulkanLocalRootSignature::DescriptorSetLayout( )
{
    if ( m_descriptorSetLayout == VK_NULL_HANDLE )
    {
        return nullptr;
    }
    return &m_descriptorSetLayout;
}

uint32_t VulkanLocalRootSignature::LocalDataNumBytes( ) const
{
    return m_totalInlineDataBytes + ( m_descriptorSetLayout != VK_NULL_HANDLE ? sizeof( VkDescriptorSet ) : 0 );
}

uint32_t VulkanLocalRootSignature::InlineDataNumBytes( ) const
{
    return m_totalInlineDataBytes;
}

uint32_t VulkanLocalRootSignature::CbvOffset( uint32_t cbvIndex ) const
{
    if ( cbvIndex >= m_inlineDataOffsets.size( ) )
    {
        LOG( ERROR ) << "Invalid binding index for inline data(" << cbvIndex << ")";
        return 0;
    }
    return m_inlineDataOffsets[ cbvIndex ];
}

uint32_t VulkanLocalRootSignature::CbvNumBytes( uint32_t cbvIndex ) const
{
    if ( cbvIndex >= m_inlineDataNumBytes.size( ) )
    {
        LOG( ERROR ) << "Invalid binding index for inline data size(" << cbvIndex << ")";
        return 0;
    }
    return m_inlineDataNumBytes[ cbvIndex ];
}
