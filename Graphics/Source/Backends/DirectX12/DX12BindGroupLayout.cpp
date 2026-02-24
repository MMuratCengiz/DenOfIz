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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12BindGroupLayout.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12EnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/CommonDataUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12BindGroupLayout::DX12BindGroupLayout( DX12Context *context, const DenOfIz_BindGroupLayoutDesc &desc ) : IBindGroupLayout( desc ), m_context( context ), m_desc( desc )
{
    for ( uint32_t i = 0; i < desc.Bindings.NumElements; ++i )
    {
        AddResourceBinding( desc.Bindings.Elements[ i ] );
    }
}

DX12BindGroupLayout::~DX12BindGroupLayout( )
{
}

void DX12BindGroupLayout::AddResourceBinding( const DenOfIz_BindingDesc &binding )
{
    const DenOfIz_ResourceBindingSlot slot{
        .Type          = DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor ),
        .Binding       = binding.Binding,
        .RegisterSpace = m_desc.RegisterSpace,
    };

    CD3DX12_DESCRIPTOR_RANGE descriptorRange = { };
    descriptorRange.Init( DenOfIz_DX12EnumConverter_ConvertResourceDescriptorToDescriptorRangeType( binding.Descriptor ), binding.ArraySize, binding.Binding,
                          m_desc.RegisterSpace );

    if ( binding.Descriptor & DENOFIZ_RESOURCE_DESCRIPTOR_SAMPLER_BIT )
    {
        if ( binding.IsBindless )
        {
            spdlog::warn( "Bindless samplers not yet implemented in DX12" );
            return;
        }
        m_samplerRanges.push_back( descriptorRange );
        uint32_t slotKey               = DenOfIz_ResourceBindingSlot_Key( &slot );
        m_resourceOffsetMap[ slotKey ] = m_samplerCount++;

        D3D12_SHADER_VISIBILITY usedStage = DenOfIz_DX12EnumConverter_ConvertShaderStageToShaderVisibility( binding.Stages );
        m_samplerVisibilities.insert( usedStage );
        m_usedStages |= binding.Stages;
    }
    else if ( !binding.IsBindless && m_desc.RegisterSpace == DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE &&
              ( binding.Descriptor & ( DENOFIZ_RESOURCE_DESCRIPTOR_UNIFORM_BUFFER_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_BUFFER_BIT | DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT ) ) )
    {
        DX12RootLevelDescriptorRange &rootLevelRange = m_rootLevelRanges.emplace_back( );
        rootLevelRange.Range                         = descriptorRange;
        rootLevelRange.Visibility                    = DenOfIz_DX12EnumConverter_ConvertShaderStageToShaderVisibility( binding.Stages );
        m_usedStages |= binding.Stages;
        uint32_t slotKey               = DenOfIz_ResourceBindingSlot_Key( &slot );
        m_resourceOffsetMap[ slotKey ] = m_rootLevelBufferCount++;
    }
    else
    {
        m_cbvSrvUavRanges.push_back( descriptorRange );
        uint32_t slotKey               = DenOfIz_ResourceBindingSlot_Key( &slot );
        m_resourceOffsetMap[ slotKey ] = m_cbvSrvUavCount;
        m_cbvSrvUavCount += binding.IsBindless ? binding.ArraySize : 1;

        D3D12_SHADER_VISIBILITY usedStage = DenOfIz_DX12EnumConverter_ConvertShaderStageToShaderVisibility( binding.Stages );
        m_cbvSrvUavVisibilities.insert( usedStage );
        m_usedStages |= binding.Stages;
    }
}

uint32_t DX12BindGroupLayout::GetResourceOffset( const DenOfIz_ResourceBindingSlot &slot ) const
{
    uint32_t slotKey = DenOfIz_ResourceBindingSlot_Key( &slot );
    return ContainerUtilities::SafeGetMapValue( m_resourceOffsetMap, slotKey, "Binding slot does not exist in bind group layout: " + ResourceBindingSlotToString( slot ) );
}

const DenOfIz_BindGroupLayoutDesc &DX12BindGroupLayout::Desc( ) const
{
    return m_desc;
}

uint32_t DX12BindGroupLayout::RegisterSpace( ) const
{
    return m_desc.RegisterSpace;
}

uint32_t DX12BindGroupLayout::CbvSrvUavCount( ) const
{
    return m_cbvSrvUavCount;
}

uint32_t DX12BindGroupLayout::SamplerCount( ) const
{
    return m_samplerCount;
}

uint32_t DX12BindGroupLayout::RootLevelBufferCount( ) const
{
    return m_rootLevelBufferCount;
}

uint32_t DX12BindGroupLayout::UsedStages( ) const
{
    return m_usedStages;
}

const std::vector<DX12RootLevelDescriptorRange> &DX12BindGroupLayout::RootLevelRanges( ) const
{
    return m_rootLevelRanges;
}

const std::vector<CD3DX12_DESCRIPTOR_RANGE> &DX12BindGroupLayout::CbvSrvUavRanges( ) const
{
    return m_cbvSrvUavRanges;
}

const std::vector<CD3DX12_DESCRIPTOR_RANGE> &DX12BindGroupLayout::SamplerRanges( ) const
{
    return m_samplerRanges;
}

D3D12_SHADER_VISIBILITY DX12BindGroupLayout::CbvSrvUavVisibility( ) const
{
    if ( m_cbvSrvUavVisibilities.size( ) == 1 )
    {
        return *m_cbvSrvUavVisibilities.begin( );
    }
    return D3D12_SHADER_VISIBILITY_ALL;
}

D3D12_SHADER_VISIBILITY DX12BindGroupLayout::SamplerVisibility( ) const
{
    if ( m_samplerVisibilities.size( ) == 1 )
    {
        return *m_samplerVisibilities.begin( );
    }
    return D3D12_SHADER_VISIBILITY_ALL;
}
