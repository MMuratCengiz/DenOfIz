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

#include <DenOfIzGraphics/Backends/DirectX12/DX12ResourceBindGroup.h>

using namespace DenOfIz;

DX12ResourceBindGroup::DX12ResourceBindGroup( DX12Context *context, const ResourceBindGroupDesc &desc ) : IResourceBindGroup( desc ), m_context( context )
{
    const auto rootSignature = dynamic_cast<DX12RootSignature *>( desc.RootSignature );
    DZ_NOT_NULL( rootSignature );
    m_dx12RootSignature = rootSignature;

    uint16_t numCbvSrvUav       = 0;
    uint16_t numSamplers        = 0;
    uint32_t rootParameterIndex = 0;

    for ( const auto &rootParameter : m_dx12RootSignature->RootParameters( ) )
    {
        switch ( rootParameter.ParameterType )
        {
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            for ( auto i = 0u; i < rootParameter.DescriptorTable.NumDescriptorRanges; i++ )
            {
                const auto &range = rootParameter.DescriptorTable.pDescriptorRanges[ i ];
                if ( range.RegisterSpace != desc.RegisterSpace )
                {
                    continue;
                }
                if ( range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER )
                {
                    numSamplers += range.NumDescriptors;
                }
                else
                {
                    numCbvSrvUav += range.NumDescriptors;
                }
            }
            break;
        case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            break;
        case D3D12_ROOT_PARAMETER_TYPE_CBV:
        case D3D12_ROOT_PARAMETER_TYPE_SRV:
        case D3D12_ROOT_PARAMETER_TYPE_UAV:
            if ( rootParameter.Descriptor.RegisterSpace == desc.RegisterSpace )
            {
                ContainerUtilities::EnsureSize( m_rootDescriptors, rootParameter.Descriptor.ShaderRegister );
                m_rootDescriptors[ rootParameter.Descriptor.ShaderRegister ] = { rootParameterIndex, rootParameter.ParameterType, 0 };
            }
            break;
        }
        ++rootParameterIndex;
    }

    if ( numCbvSrvUav > 0 )
    {
        m_cbvSrvUavHandle = m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetNextHandle( numCbvSrvUav );
    }
    if ( numSamplers > 0 )
    {
        m_samplerHandle = m_context->ShaderVisibleSamplerDescriptorHeap->GetNextHandle( numSamplers );
    }
}

void DX12ResourceBindGroup::SetRootConstants( const uint32_t binding, void *data )
{
    DZ_NOT_NULL( data );
    ContainerUtilities::EnsureSize( m_rootConstants, binding );

    DX12RootConstant &rootConstant = m_rootConstants[ binding ];
    rootConstant.Data              = data;
    rootConstant.NumBytes          = m_dx12RootSignature->RootConstants( )[ binding ].Constants.Num32BitValues * sizeof( uint32_t );
}

void DX12ResourceBindGroup::Update( const UpdateDesc &desc )
{
    m_cbvSrvUavCount = 0;
    m_samplerCount   = 0;

    IResourceBindGroup::Update( desc );
}

void DX12ResourceBindGroup::BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource )
{
    DZ_NOT_NULL( resource );
    const uint32_t offset = m_dx12RootSignature->GetResourceOffset( slot );
    reinterpret_cast<DX12TextureResource *>( resource )->CreateView( CpuHandleCbvSrvUav( offset ) );
    m_cbvSrvUavCount++;
}

void DX12ResourceBindGroup::BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource )
{
    DZ_NOT_NULL( resource );
    DZ_RETURN_IF( UpdateRootDescriptor( slot, dynamic_cast<DX12BufferResource *>( resource )->GetResource( )->GetGPUVirtualAddress( ) ) );

    const uint32_t offset = m_dx12RootSignature->GetResourceOffset( slot );
    reinterpret_cast<DX12BufferResource *>( resource )->CreateView( CpuHandleCbvSrvUav( offset ) );
    m_cbvSrvUavCount++;
}

void DX12ResourceBindGroup::BindSampler( const ResourceBindingSlot &slot, ISampler *sampler )
{
    DZ_NOT_NULL( sampler );
    const uint32_t offset = m_dx12RootSignature->GetResourceOffset( slot );
    reinterpret_cast<DX12Sampler *>( sampler )->CreateView( CpuHandleSampler( offset ) );
    m_samplerCount++;
}

bool DX12ResourceBindGroup::UpdateRootDescriptor( const ResourceBindingSlot &slot, const D3D12_GPU_VIRTUAL_ADDRESS &gpuAddress )
{
    if ( slot.RegisterSpace == DZConfiguration::Instance().RootLevelBufferRegisterSpace )
    {
        if ( slot.Binding >= m_rootDescriptors.size( ) )
        {
            LOG( ERROR ) << "Root descriptor binding [" << slot.Binding << "] is out of range.";
        }

        m_rootDescriptors[ slot.Binding ].GpuAddress = gpuAddress;
        return true;
    }

    return false;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ResourceBindGroup::CpuHandleCbvSrvUav( const uint32_t binding ) const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE( m_cbvSrvUavHandle.Cpu, binding, m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetDescriptorSize( ) );
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12ResourceBindGroup::CpuHandleSampler( const uint32_t binding ) const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE( m_samplerHandle.Cpu, binding, m_context->ShaderVisibleSamplerDescriptorHeap->GetDescriptorSize( ) );
}

DescriptorHandle DX12ResourceBindGroup::CbvSrvUavHandle( ) const
{
    return m_cbvSrvUavHandle;
}

DescriptorHandle DX12ResourceBindGroup::SamplerHandle( ) const
{
    return m_samplerHandle;
}

uint32_t DX12ResourceBindGroup::CbvSrvUavCount( ) const
{
    return m_cbvSrvUavCount;
}

uint32_t DX12ResourceBindGroup::SamplerCount( ) const
{
    return m_samplerCount;
}

DX12RootSignature *DX12ResourceBindGroup::RootSignature( ) const
{
    return m_dx12RootSignature;
}

const std::vector<DX12RootDescriptor> &DX12ResourceBindGroup::RootDescriptors( ) const
{
    return m_rootDescriptors;
}

const std::vector<DX12RootConstant> &DX12ResourceBindGroup::RootConstants( ) const
{
    return m_rootConstants;
}
