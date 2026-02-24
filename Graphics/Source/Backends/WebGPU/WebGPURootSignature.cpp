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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPURootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

#define BIND_GROUP_LAYOUT_IMPL( handle ) static_cast<WebGPUBindGroupLayout *>( DENOFIZ_FROM_HANDLE( IBindGroupLayout, handle ) )

WebGPURootSignature::WebGPURootSignature( WebGPUContext *context, const DenOfIz_RootSignatureDesc &desc ) : m_context( context ), m_desc( desc )
{
    uint32_t maxRegisterSpace = 0;
    for ( uint32_t i = 0; i < desc.BindGroupLayouts.NumElements; ++i )
    {
        WebGPUBindGroupLayout *layout = BIND_GROUP_LAYOUT_IMPL( desc.BindGroupLayouts.Elements[ i ] );
        maxRegisterSpace              = std::max( maxRegisterSpace, layout->RegisterSpace( ) );
    }

    m_bindGroupLayouts.resize( maxRegisterSpace + 1, nullptr );
    m_wgpuBindGroupLayouts.resize( maxRegisterSpace + 1, nullptr );

    for ( uint32_t i = 0; i < desc.BindGroupLayouts.NumElements; ++i )
    {
        WebGPUBindGroupLayout *layout                  = BIND_GROUP_LAYOUT_IMPL( desc.BindGroupLayouts.Elements[ i ] );
        m_bindGroupLayouts[ layout->RegisterSpace( ) ] = layout;

        WGPUBindGroupLayout wgpuLayout = layout->GetBindGroupLayout( );
        if ( wgpuLayout != nullptr )
        {
            m_wgpuBindGroupLayouts[ layout->RegisterSpace( ) ] = wgpuLayout;
        }
    }

    for ( uint32_t i = 0; i < m_desc.RootConstants.NumElements; ++i )
    {
        const DenOfIz_RootConstantBindingDesc &rootConstant = m_desc.RootConstants.Elements[ i ];
        WebGPURootConstantInfo                 info;
        info.Binding  = rootConstant.Binding;
        info.NumBytes = rootConstant.NumBytes;
        m_rootConstants.push_back( info );
    }

    bool hasBindless = false;
    for ( uint32_t i = 0; i < desc.BindGroupLayouts.NumElements; ++i )
    {
        WebGPUBindGroupLayout *layout = BIND_GROUP_LAYOUT_IMPL( desc.BindGroupLayouts.Elements[ i ] );
        if ( layout->HasBindless( ) )
        {
            hasBindless = true;
            break;
        }
    }
    if ( hasBindless )
    {
        spdlog::warn( "WebGPU: Bindless resources are not supported" );
    }

    WGPUBindGroupLayoutDescriptor emptyLayoutDesc{ };
    emptyLayoutDesc.label      = DZ_WEBGPU_NULL_STRING;
    emptyLayoutDesc.entryCount = 0;
    emptyLayoutDesc.entries    = nullptr;
    m_emptyLayout              = wgpuDeviceCreateBindGroupLayout( m_context->Device, &emptyLayoutDesc );

    WGPUBindGroupDescriptor emptyBindGroupDesc{ };
    emptyBindGroupDesc.label      = DZ_WEBGPU_NULL_STRING;
    emptyBindGroupDesc.layout     = m_emptyLayout;
    emptyBindGroupDesc.entryCount = 0;
    emptyBindGroupDesc.entries    = nullptr;
    m_emptyBindGroup              = wgpuDeviceCreateBindGroup( m_context->Device, &emptyBindGroupDesc );

    CreatePipelineLayout( );
}

WebGPURootSignature::~WebGPURootSignature( )
{
    if ( m_pipelineLayout != nullptr )
    {
        wgpuPipelineLayoutRelease( m_pipelineLayout );
    }
    if ( m_emptyBindGroup != nullptr )
    {
        wgpuBindGroupRelease( m_emptyBindGroup );
    }
    if ( m_emptyLayout != nullptr )
    {
        wgpuBindGroupLayoutRelease( m_emptyLayout );
    }
}

void WebGPURootSignature::CreatePipelineLayout( )
{
    std::vector<WGPUBindGroupLayout> layouts;
    for ( auto layout : m_wgpuBindGroupLayouts )
    {
        if ( layout != nullptr )
        {
            layouts.push_back( layout );
        }
        else
        {
            layouts.push_back( m_emptyLayout );
        }
    }

    WGPUPipelineLayoutDescriptor layoutDesc{ };
    layoutDesc.label                = DZ_WEBGPU_NULL_STRING;
    layoutDesc.bindGroupLayoutCount = static_cast<uint32_t>( layouts.size( ) );
    layoutDesc.bindGroupLayouts     = layouts.data( );

    m_pipelineLayout = wgpuDeviceCreatePipelineLayout( m_context->Device, &layoutDesc );

    if ( m_pipelineLayout == nullptr )
    {
        spdlog::error( "WebGPU: Failed to create pipeline layout" );
    }
}

WGPUPipelineLayout WebGPURootSignature::GetPipelineLayout( ) const
{
    return m_pipelineLayout;
}

const std::vector<WGPUBindGroupLayout> &WebGPURootSignature::GetWGPUBindGroupLayouts( ) const
{
    return m_wgpuBindGroupLayouts;
}

const std::vector<WebGPURootConstantInfo> &WebGPURootSignature::RootConstants( ) const
{
    return m_rootConstants;
}

uint32_t WebGPURootSignature::NumRootConstants( ) const
{
    return static_cast<uint32_t>( m_rootConstants.size( ) );
}

const std::vector<WebGPUBindGroupLayout *> &WebGPURootSignature::BindGroupLayouts( ) const
{
    return m_bindGroupLayouts;
}

WebGPUBindGroupLayout *WebGPURootSignature::GetBindGroupLayout( uint32_t registerSpace ) const
{
    if ( registerSpace >= m_bindGroupLayouts.size( ) )
    {
        spdlog::error( "Register space {} does not exist.", registerSpace );
        return nullptr;
    }
    return m_bindGroupLayouts[ registerSpace ];
}

WGPUBindGroup WebGPURootSignature::GetEmptyBindGroup( ) const
{
    return m_emptyBindGroup;
}

uint32_t WebGPURootSignature::GetNumBindGroupSlots( ) const
{
    return static_cast<uint32_t>( m_wgpuBindGroupLayouts.size( ) );
}
