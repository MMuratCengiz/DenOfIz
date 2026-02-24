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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUBindGroupLayout.h"
#include "DenOfIzGraphics/Assets/Shaders/ShaderCompiler.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

WebGPUBindGroupLayout::WebGPUBindGroupLayout( WebGPUContext *context, const DenOfIz_BindGroupLayoutDesc &desc ) : IBindGroupLayout( desc ), m_context( context ), m_desc( desc )
{
    for ( uint32_t i = 0; i < desc.Bindings.NumElements; ++i )
    {
        AddResourceBinding( desc.Bindings.Elements[ i ] );
    }

    if ( m_hasBindless )
    {
        spdlog::warn( "WebGPU: Bindless resources are not supported" );
    }

    if ( !m_entries.empty( ) )
    {
        WGPUBindGroupLayoutDescriptor layoutDesc{ };
        layoutDesc.label      = DZ_WEBGPU_NULL_STRING;
        layoutDesc.entryCount = static_cast<uint32_t>( m_entries.size( ) );
        layoutDesc.entries    = m_entries.data( );

        m_layout = wgpuDeviceCreateBindGroupLayout( m_context->Device, &layoutDesc );

        if ( m_layout == nullptr )
        {
            spdlog::error( "WebGPU: Failed to create bind group layout for register space {}", m_desc.RegisterSpace );
        }
    }
}

WebGPUBindGroupLayout::~WebGPUBindGroupLayout( )
{
    if ( m_layout != nullptr )
    {
        wgpuBindGroupLayoutRelease( m_layout );
    }
}

void WebGPUBindGroupLayout::AddResourceBinding( const DenOfIz_BindingDesc &binding )
{
    if ( binding.IsBindless )
    {
        m_hasBindless = true;
    }

    const DenOfIz_ResourceBindingType bindingType = DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor );
    WGPUBindGroupLayoutEntry          entry{ };
    uint32_t                          bindingOffset = 0;

    switch ( bindingType )
    {
    case DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER:
        bindingOffset = DENOFIZ_VK_SHIFT_CBV;
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
        bindingOffset = DENOFIZ_VK_SHIFT_SRV;
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
        bindingOffset = DENOFIZ_VK_SHIFT_UAV;
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER:
        bindingOffset = DENOFIZ_VK_SHIFT_SAMPLER;
        break;
    }

    entry.binding    = bindingOffset + binding.Binding;
    entry.visibility = DenOfIz_WebGPUEnumConverter_ConvertShaderStage( binding.Stages );

    switch ( bindingType )
    {
    case DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER:
        entry.buffer.type             = WGPUBufferBindingType_Uniform;
        entry.buffer.hasDynamicOffset = false;
        entry.buffer.minBindingSize   = 0;
        break;

    case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
        if ( binding.Descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_BUFFER_BIT )
        {
            entry.buffer.type             = WGPUBufferBindingType_ReadOnlyStorage;
            entry.buffer.hasDynamicOffset = false;
            entry.buffer.minBindingSize   = 0;
        }
        else if ( binding.Descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT )
        {
            entry.texture.sampleType    = WGPUTextureSampleType_Float;
            entry.texture.viewDimension = WGPUTextureViewDimension_2D;
            entry.texture.multisampled  = false;
        }
        else if ( binding.Descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_ACCELERATION_STRUCTURE_BIT )
        {
            spdlog::warn( "WebGPU: Acceleration structures are not supported" );
            return;
        }
        break;

    case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
        if ( binding.Descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT )
        {
            entry.buffer.type             = WGPUBufferBindingType_Storage;
            entry.buffer.hasDynamicOffset = false;
            entry.buffer.minBindingSize   = 0;
        }
        else if ( binding.Descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT )
        {
            entry.storageTexture.access        = WGPUStorageTextureAccess_WriteOnly;
            entry.storageTexture.format        = WGPUTextureFormat_RGBA8Unorm;
            entry.storageTexture.viewDimension = WGPUTextureViewDimension_2D;
        }
        break;

    case DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER:
        entry.sampler.type = WGPUSamplerBindingType_Filtering;
        break;
    }

    m_entries.push_back( entry );
}

const DenOfIz_BindGroupLayoutDesc &WebGPUBindGroupLayout::Desc( ) const
{
    return m_desc;
}

uint32_t WebGPUBindGroupLayout::RegisterSpace( ) const
{
    return m_desc.RegisterSpace;
}

WGPUBindGroupLayout WebGPUBindGroupLayout::GetBindGroupLayout( ) const
{
    return m_layout;
}

bool WebGPUBindGroupLayout::HasBindless( ) const
{
    return m_hasBindless;
}
