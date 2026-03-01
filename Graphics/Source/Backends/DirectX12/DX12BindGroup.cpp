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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12BindGroup.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12TopLevelAS.h"
#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12BindGroup::DX12BindGroup( DX12Context *context, const DenOfIz_BindGroupDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_bindGroupLayout = dynamic_cast<DX12BindGroupLayout *>( DENOFIZ_FROM_HANDLE( IBindGroupLayout, desc.Layout ) );
    DZ_NOT_NULL( m_bindGroupLayout );

    uint32_t numCbvSrvUav = m_bindGroupLayout->CbvSrvUavCount( );
    uint32_t numSamplers  = m_bindGroupLayout->SamplerCount( );

    for ( const auto &rootRange : m_bindGroupLayout->RootLevelRanges( ) )
    {
        D3D12_ROOT_PARAMETER_TYPE paramType;
        switch ( rootRange.Range.RangeType )
        {
        case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
            paramType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            break;
        case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
            paramType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            break;
        case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
            paramType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            break;
        default:
            continue;
        }
        ContainerUtilities::EnsureSize( m_rootDescriptors, rootRange.Range.BaseShaderRegister + 1 );
        m_rootDescriptors[ rootRange.Range.BaseShaderRegister ] = { 0, paramType, 0 };
    }

    if ( numCbvSrvUav > 0 )
    {
        m_cbvSrvUavHandle = m_context->ShaderVisibleCbvSrvUavDescriptorHeap->AllocateDescriptors( numCbvSrvUav );
    }
    if ( numSamplers > 0 )
    {
        m_samplerHandle = m_context->ShaderVisibleSamplerDescriptorHeap->AllocateDescriptors( numSamplers );
    }
}

DX12BindGroup::~DX12BindGroup( )
{
    if ( m_cbvSrvUavCount > 0 && m_cbvSrvUavHandle.Cpu.ptr != 0 )
    {
        m_context->ShaderVisibleCbvSrvUavDescriptorHeap->FreeDescriptors( m_cbvSrvUavHandle, m_cbvSrvUavCount );
    }
    if ( m_samplerCount > 0 && m_samplerHandle.Cpu.ptr != 0 )
    {
        m_context->ShaderVisibleSamplerDescriptorHeap->FreeDescriptors( m_samplerHandle, m_samplerCount );
    }
}

IBindGroup *DX12BindGroup::BeginUpdate( )
{
    m_cbvSrvUavCount = 0;
    m_samplerCount   = 0;
    return this;
}

IBindGroup *DX12BindGroup::Cbv( const uint32_t binding, IBuffer *resource )
{
    BindBuffer( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER ), resource );
    return this;
}

IBindGroup *DX12BindGroup::Cbv( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    auto *dx12Buffer = dynamic_cast<DX12Buffer *>( resource );
    DZ_NOT_NULL( dx12Buffer );
    if ( resourceOffset % m_context->SelectedDeviceInfo.Constants.ConstantBufferAlignment != 0 )
    {
        spdlog::error( "Constant buffer offset [ {} ] is not aligned to [{} ].", resourceOffset, m_context->SelectedDeviceInfo.Constants.ConstantBufferAlignment );
        return this;
    }
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER );

    if ( UpdateRootDescriptor( slot, dx12Buffer->Resource( )->GetGPUVirtualAddress( ) + resourceOffset ) )
    {
        return this;
    }

    const uint32_t offset = m_bindGroupLayout->GetResourceOffset( slot );
    dx12Buffer->CreateView( CpuHandleCbvSrvUav( offset ), slot.Type, resourceOffset );
    m_cbvSrvUavCount++;

    return this;
}

IBindGroup *DX12BindGroup::Srv( const uint32_t binding, IBuffer *resource )
{
    BindBuffer( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE ), resource );
    return this;
}

IBindGroup *DX12BindGroup::Srv( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    auto *dx12Buffer = dynamic_cast<DX12Buffer *>( resource );
    DZ_NOT_NULL( dx12Buffer );

    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE );

    if ( UpdateRootDescriptor( slot, dx12Buffer->Resource( )->GetGPUVirtualAddress( ) + resourceOffset ) )
    {
        return this;
    }

    const uint32_t offset = m_bindGroupLayout->GetResourceOffset( slot );
    dx12Buffer->CreateView( CpuHandleCbvSrvUav( offset ), slot.Type, resourceOffset );
    m_cbvSrvUavCount++;
    return this;
}

IBindGroup *DX12BindGroup::Srv( const uint32_t binding, ITexture *resource, const uint32_t mipLevel )
{
    BindTexture( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE ), resource, mipLevel );
    return this;
}

IBindGroup *DX12BindGroup::SrvArray( const uint32_t binding, const DenOfIz_TextureArray &resources )
{
    const DenOfIz_ResourceBindingSlot slot       = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE );
    const uint32_t                    baseOffset = m_bindGroupLayout->GetResourceOffset( slot );
    for ( uint32_t i = 0; i < resources.NumElements; ++i )
    {
        DZ_NOT_NULL( resources.Elements[ i ] );
        const uint32_t descriptorOffset = baseOffset + i;
        reinterpret_cast<DX12Texture *>( resources.Elements[ i ] )->CreateView( CpuHandleCbvSrvUav( descriptorOffset ) );
        m_cbvSrvUavCount++;
    }
    return this;
}

IBindGroup *DX12BindGroup::SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITexture *resource )
{
    const DenOfIz_ResourceBindingSlot slot             = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE );
    const uint32_t                    baseOffset       = m_bindGroupLayout->GetResourceOffset( slot );
    const uint32_t                    descriptorOffset = baseOffset + arrayIndex;

    DZ_NOT_NULL( resource );
    reinterpret_cast<DX12Texture *>( resource )->CreateView( CpuHandleCbvSrvUav( descriptorOffset ) );

    return this;
}

IBindGroup *DX12BindGroup::Srv( const uint32_t binding, ITopLevelAS *accelerationStructure )
{
    return Srv( binding, dynamic_cast<DX12TopLevelAS *>( accelerationStructure )->Buffer( ) );
}

IBindGroup *DX12BindGroup::Uav( const uint32_t binding, IBuffer *resource )
{
    BindBuffer( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS ), resource );
    return this;
}

IBindGroup *DX12BindGroup::Uav( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    auto *dx12Buffer = dynamic_cast<DX12Buffer *>( resource );
    DZ_NOT_NULL( dx12Buffer );

    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS );
    if ( UpdateRootDescriptor( slot, dx12Buffer->Resource( )->GetGPUVirtualAddress( ) + resourceOffset ) )
    {
        return this;
    }

    const uint32_t offset = m_bindGroupLayout->GetResourceOffset( slot );
    dx12Buffer->CreateView( CpuHandleCbvSrvUav( offset ), slot.Type, resourceOffset );
    m_cbvSrvUavCount++;
    return this;
}

IBindGroup *DX12BindGroup::Uav( const uint32_t binding, ITexture *resource, const uint32_t mipLevel )
{
    BindTexture( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS ), resource, mipLevel );
    return this;
}

IBindGroup *DX12BindGroup::Sampler( const uint32_t binding, ISampler *sampler )
{
    BindSampler( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER ), sampler );
    return this;
}

void DX12BindGroup::EndUpdate( )
{
}

void DX12BindGroup::BindTexture( const DenOfIz_ResourceBindingSlot &slot, ITexture *resource, const uint32_t mipLevel )
{
    DZ_NOT_NULL( resource );
    const uint32_t offset = m_bindGroupLayout->GetResourceOffset( slot );
    reinterpret_cast<DX12Texture *>( resource )->CreateView( CpuHandleCbvSrvUav( offset ), mipLevel );
    m_cbvSrvUavCount++;
}

void DX12BindGroup::BindBuffer( const DenOfIz_ResourceBindingSlot &slot, IBuffer *resource )
{
    DZ_NOT_NULL( resource );
    DZ_RETURN_IF( UpdateRootDescriptor( slot, dynamic_cast<DX12Buffer *>( resource )->Resource( )->GetGPUVirtualAddress( ) ) );

    const uint32_t offset = m_bindGroupLayout->GetResourceOffset( slot );
    reinterpret_cast<DX12Buffer *>( resource )->CreateView( CpuHandleCbvSrvUav( offset ), slot.Type );
    m_cbvSrvUavCount++;
}

void DX12BindGroup::BindSampler( const DenOfIz_ResourceBindingSlot &slot, ISampler *sampler )
{
    DZ_NOT_NULL( sampler );
    const uint32_t offset = m_bindGroupLayout->GetResourceOffset( slot );
    reinterpret_cast<DX12Sampler *>( sampler )->CreateView( CpuHandleSampler( offset ) );
    m_samplerCount++;
}

bool DX12BindGroup::UpdateRootDescriptor( const DenOfIz_ResourceBindingSlot &slot, const D3D12_GPU_VIRTUAL_ADDRESS &gpuAddress )
{
    if ( slot.RegisterSpace == DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE )
    {
        if ( slot.Binding >= m_rootDescriptors.size( ) )
        {
            spdlog::error( "Root descriptor binding [ {} ] is out of range.", slot.Binding );
        }

        m_rootDescriptors[ slot.Binding ].GpuAddress = gpuAddress;
        return true;
    }

    return false;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12BindGroup::CpuHandleCbvSrvUav( const uint32_t binding ) const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE( m_cbvSrvUavHandle.Cpu, binding, m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetDescriptorSize( ) );
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12BindGroup::CpuHandleSampler( const uint32_t binding ) const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE( m_samplerHandle.Cpu, binding, m_context->ShaderVisibleSamplerDescriptorHeap->GetDescriptorSize( ) );
}

DescriptorHandle DX12BindGroup::CbvSrvUavHandle( ) const
{
    return m_cbvSrvUavHandle;
}

DescriptorHandle DX12BindGroup::SamplerHandle( ) const
{
    return m_samplerHandle;
}

uint32_t DX12BindGroup::CbvSrvUavCount( ) const
{
    return m_cbvSrvUavCount;
}

uint32_t DX12BindGroup::SamplerCount( ) const
{
    return m_samplerCount;
}

DX12BindGroupLayout *DX12BindGroup::BindGroupLayout( ) const
{
    return m_bindGroupLayout;
}

const std::vector<DX12RootDescriptor> &DX12BindGroup::RootDescriptors( ) const
{
    return m_rootDescriptors;
}

uint32_t DX12BindGroup::RegisterSpace( ) const
{
    return m_bindGroupLayout->RegisterSpace( );
}

DenOfIz_ResourceBindingSlot DX12BindGroup::GetSlot( uint32_t binding, const DenOfIz_ResourceBindingType &type ) const
{
    return DenOfIz_ResourceBindingSlot{ type, binding, m_bindGroupLayout->RegisterSpace( ) };
}
