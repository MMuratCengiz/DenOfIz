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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUBindGroup.h"
#include <algorithm>
#include <ranges>

#include "DenOfIzGraphics/Assets/Shaders/ShaderCompiler.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUTexture.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

WebGPUBindGroup::WebGPUBindGroup( WebGPUContext *context, const DenOfIz_BindGroupDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_bindGroupLayout = dynamic_cast<WebGPUBindGroupLayout *>( DENOFIZ_FROM_HANDLE( DenOfIz::IBindGroupLayout, desc.Layout ) );
    if ( m_bindGroupLayout == nullptr )
    {
        spdlog::error( "WebGPU: Bind group layout is null" );
        return;
    }
}

WebGPUBindGroup::~WebGPUBindGroup( )
{
    if ( m_bindGroup != nullptr )
    {
        wgpuBindGroupRelease( m_bindGroup );
    }
}

IBindGroup *WebGPUBindGroup::BeginUpdate( )
{
    m_isUpdating = true;
    m_entries.clear( );
    m_bindingMap.clear( );
    m_boundBuffers.clear( );
    return this;
}

IBindGroup *WebGPUBindGroup::Cbv( const uint32_t binding, IBuffer *resource )
{
    if ( !m_isUpdating )
    {
        spdlog::error( "WebGPU: BeginUpdate must be called before binding resources" );
        return this;
    }

    const auto webgpuBuffer = dynamic_cast<WebGPUBuffer *>( resource );
    if ( webgpuBuffer == nullptr )
    {
        spdlog::error( "WebGPU: Invalid buffer resource" );
        return this;
    }

    const uint32_t           shiftedBinding = DENOFIZ_VK_SHIFT_CBV + binding;
    const WGPUBindGroupEntry entry          = CreateBindingEntry( shiftedBinding, webgpuBuffer->GetBuffer( ), 0, webgpuBuffer->GetNumBytes( ) );
    m_bindingMap[ shiftedBinding ]          = entry;
    m_boundBuffers.push_back( webgpuBuffer );

    return this;
}

IBindGroup *WebGPUBindGroup::Cbv( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    if ( !m_isUpdating )
    {
        spdlog::error( "WebGPU: BeginUpdate must be called before binding resources" );
        return this;
    }

    const auto webgpuBuffer = dynamic_cast<WebGPUBuffer *>( resource );
    if ( webgpuBuffer == nullptr )
    {
        spdlog::error( "WebGPU: Invalid buffer resource" );
        return this;
    }

    const uint32_t           shiftedBinding = DENOFIZ_VK_SHIFT_CBV + binding;
    const WGPUBindGroupEntry entry          = CreateBindingEntry( shiftedBinding, webgpuBuffer->GetBuffer( ), resourceOffset, webgpuBuffer->GetNumBytes( ) - resourceOffset );
    m_bindingMap[ shiftedBinding ]          = entry;
    m_boundBuffers.push_back( webgpuBuffer );

    return this;
}

IBindGroup *WebGPUBindGroup::Srv( const uint32_t binding, IBuffer *resource )
{
    if ( !m_isUpdating )
    {
        spdlog::error( "WebGPU: BeginUpdate must be called before binding resources" );
        return this;
    }
    const auto webgpuBuffer = dynamic_cast<WebGPUBuffer *>( resource );
    if ( webgpuBuffer == nullptr )
    {
        spdlog::error( "WebGPU: Invalid buffer resource" );
        return this;
    }

    const uint32_t           shiftedBinding = DENOFIZ_VK_SHIFT_SRV + binding;
    const WGPUBindGroupEntry entry          = CreateBindingEntry( shiftedBinding, webgpuBuffer->GetBuffer( ), 0, webgpuBuffer->GetNumBytes( ) );
    m_bindingMap[ shiftedBinding ]          = entry;
    m_boundBuffers.push_back( webgpuBuffer );
    return this;
}

IBindGroup *WebGPUBindGroup::Srv( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    if ( !m_isUpdating )
    {
        spdlog::error( "WebGPU: BeginUpdate must be called before binding resources" );
        return this;
    }

    const auto webgpuBuffer = dynamic_cast<WebGPUBuffer *>( resource );
    if ( webgpuBuffer == nullptr )
    {
        spdlog::error( "WebGPU: Invalid buffer resource" );
        return this;
    }

    const uint32_t           shiftedBinding = DENOFIZ_VK_SHIFT_SRV + binding;
    const WGPUBindGroupEntry entry          = CreateBindingEntry( shiftedBinding, webgpuBuffer->GetBuffer( ), resourceOffset, webgpuBuffer->GetNumBytes( ) - resourceOffset );
    m_bindingMap[ shiftedBinding ]          = entry;
    m_boundBuffers.push_back( webgpuBuffer );
    return this;
}

IBindGroup *WebGPUBindGroup::Srv( const uint32_t binding, ITopLevelAS *accelerationStructure )
{
    spdlog::warn( "WebGPU: Acceleration structures are not supported" );
    return this;
}

IBindGroup *WebGPUBindGroup::Srv( const uint32_t binding, ITexture *resource, const uint32_t mipLevel )
{
    if ( !m_isUpdating )
    {
        spdlog::error( "WebGPU: BeginUpdate must be called before binding resources" );
        return this;
    }

    const auto webgpuTexture = dynamic_cast<WebGPUTexture *>( resource );
    if ( webgpuTexture == nullptr )
    {
        spdlog::error( "WebGPU: Invalid texture resource" );
        return this;
    }

    const uint32_t           shiftedBinding = DENOFIZ_VK_SHIFT_SRV + binding;
    WGPUTextureView          view           = mipLevel == DZ_ALL_MIP_LEVELS ? webgpuTexture->GetTextureView( ) : webgpuTexture->GetMipTextureView( mipLevel );
    const WGPUBindGroupEntry entry          = CreateBindingEntry( shiftedBinding, view );
    m_bindingMap[ shiftedBinding ]          = entry;
    return this;
}

IBindGroup *WebGPUBindGroup::SrvArray( const uint32_t binding, const DenOfIz_TextureArray &resources )
{
    spdlog::warn( "WebGPU: Texture arrays are not directly supported in bind groups" );
    return this;
}

IBindGroup *WebGPUBindGroup::SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITexture *resource )
{
    spdlog::warn( "WebGPU: Texture array indexing is not directly supported in bind groups" );
    return this;
}

IBindGroup *WebGPUBindGroup::Uav( const uint32_t binding, IBuffer *resource )
{
    if ( !m_isUpdating )
    {
        spdlog::error( "WebGPU: BeginUpdate must be called before binding resources" );
        return this;
    }

    const auto webgpuBuffer = dynamic_cast<WebGPUBuffer *>( resource );
    if ( webgpuBuffer == nullptr )
    {
        spdlog::error( "WebGPU: Invalid buffer resource" );
        return this;
    }

    const uint32_t           shiftedBinding = DENOFIZ_VK_SHIFT_UAV + binding;
    const WGPUBindGroupEntry entry          = CreateBindingEntry( shiftedBinding, webgpuBuffer->GetBuffer( ), 0, webgpuBuffer->GetNumBytes( ) );
    m_bindingMap[ shiftedBinding ]          = entry;
    m_boundBuffers.push_back( webgpuBuffer );
    return this;
}

IBindGroup *WebGPUBindGroup::Uav( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    if ( !m_isUpdating )
    {
        spdlog::error( "WebGPU: BeginUpdate must be called before binding resources" );
        return this;
    }

    const auto webgpuBuffer = dynamic_cast<WebGPUBuffer *>( resource );
    if ( webgpuBuffer == nullptr )
    {
        spdlog::error( "WebGPU: Invalid buffer resource" );
        return this;
    }

    const uint32_t           shiftedBinding = DENOFIZ_VK_SHIFT_UAV + binding;
    const WGPUBindGroupEntry entry          = CreateBindingEntry( shiftedBinding, webgpuBuffer->GetBuffer( ), resourceOffset, webgpuBuffer->GetNumBytes( ) - resourceOffset );
    m_bindingMap[ shiftedBinding ]          = entry;
    m_boundBuffers.push_back( webgpuBuffer );
    return this;
}

IBindGroup *WebGPUBindGroup::Uav( const uint32_t binding, ITexture *resource, const uint32_t mipLevel )
{
    if ( !m_isUpdating )
    {
        spdlog::error( "WebGPU: BeginUpdate must be called before binding resources" );
        return this;
    }

    const auto webgpuTexture = dynamic_cast<WebGPUTexture *>( resource );
    if ( webgpuTexture == nullptr )
    {
        spdlog::error( "WebGPU: Invalid texture resource" );
        return this;
    }

    const uint32_t           shiftedBinding = DENOFIZ_VK_SHIFT_UAV + binding;
    WGPUTextureView          view           = mipLevel == DZ_ALL_MIP_LEVELS ? webgpuTexture->GetTextureView( ) : webgpuTexture->GetMipTextureView( mipLevel );
    const WGPUBindGroupEntry entry          = CreateBindingEntry( shiftedBinding, view );
    m_bindingMap[ shiftedBinding ]          = entry;
    return this;
}

IBindGroup *WebGPUBindGroup::Sampler( const uint32_t binding, ISampler *sampler )
{
    if ( !m_isUpdating )
    {
        spdlog::error( "WebGPU: BeginUpdate must be called before binding resources" );
        return this;
    }

    const auto webgpuSampler = dynamic_cast<WebGPUSampler *>( sampler );
    if ( webgpuSampler == nullptr )
    {
        spdlog::error( "WebGPU: Invalid sampler" );
        return this;
    }

    const uint32_t           shiftedBinding = DENOFIZ_VK_SHIFT_SAMPLER + binding;
    const WGPUBindGroupEntry entry          = CreateBindingEntry( shiftedBinding, webgpuSampler->GetSampler( ) );
    m_bindingMap[ shiftedBinding ]          = entry;
    return this;
}

void WebGPUBindGroup::EndUpdate( )
{
    if ( !m_isUpdating )
    {
        spdlog::error( "WebGPU: EndUpdate called without BeginUpdate" );
        return;
    }

    m_isUpdating = false;

    m_entries.clear( );
    for ( const auto &entry : m_bindingMap | std::views::values )
    {
        m_entries.push_back( entry );
    }

    CreateBindGroup( );
}

WGPUBindGroup WebGPUBindGroup::GetBindGroup( ) const
{
    return m_bindGroup;
}

uint32_t WebGPUBindGroup::GetRegisterSpace( ) const
{
    return m_bindGroupLayout->RegisterSpace( );
}

void WebGPUBindGroup::CreateBindGroup( )
{
    if ( m_bindGroup != nullptr )
    {
        wgpuBindGroupRelease( m_bindGroup );
        m_bindGroup = nullptr;
    }

    WGPUBindGroupLayout layout = m_bindGroupLayout->GetBindGroupLayout( );
    if ( layout == nullptr )
    {
        spdlog::error( "WebGPU: Bind group layout is null for register space {}", m_bindGroupLayout->RegisterSpace( ) );
        return;
    }

    WGPUBindGroupDescriptor bindGroupDesc{ };
    bindGroupDesc.label      = DZ_WEBGPU_NULL_STRING;
    bindGroupDesc.layout     = layout;
    bindGroupDesc.entryCount = static_cast<uint32_t>( m_entries.size( ) );
    bindGroupDesc.entries    = m_entries.data( );

    m_bindGroup = wgpuDeviceCreateBindGroup( m_context->Device, &bindGroupDesc );

    if ( m_bindGroup == nullptr )
    {
        spdlog::error( "WebGPU: Failed to create bind group" );
    }
}

WGPUBindGroupEntry WebGPUBindGroup::CreateBindingEntry( const uint32_t binding, const WGPUBuffer &buffer, const uint64_t offset, const uint64_t size ) const
{
    WGPUBindGroupEntry entry{ };
    entry.binding     = binding;
    entry.buffer      = buffer;
    entry.offset      = offset;
    entry.size        = size;
    entry.sampler     = nullptr;
    entry.textureView = nullptr;
    return entry;
}
WGPUBindGroupEntry WebGPUBindGroup::CreateBindingEntry( const uint32_t binding, const WGPUTextureView &textureView ) const

{
    WGPUBindGroupEntry entry{ };
    entry.binding     = binding;
    entry.buffer      = nullptr;
    entry.offset      = 0;
    entry.size        = 0;
    entry.sampler     = nullptr;
    entry.textureView = textureView;
    return entry;
}

WGPUBindGroupEntry WebGPUBindGroup::CreateBindingEntry( const uint32_t binding, const WGPUSampler &sampler ) const
{
    WGPUBindGroupEntry entry{ };
    entry.binding     = binding;
    entry.buffer      = nullptr;
    entry.offset      = 0;
    entry.size        = 0;
    entry.sampler     = sampler;
    entry.textureView = nullptr;
    return entry;
}

void WebGPUBindGroup::SyncBuffers( ) const
{
    for ( auto *buffer : m_boundBuffers )
    {
        if ( buffer != nullptr )
        {
            buffer->SyncShadowBuffer( );
        }
    }
}
