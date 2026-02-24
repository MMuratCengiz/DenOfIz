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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalBindGroupLayout.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/ContainerUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"
#include <algorithm>

using namespace DenOfIz;

namespace
{
    int GetBindingTypeSortOrder( DenOfIz_ResourceBindingType type )
    {
        switch ( type )
        {
        case DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER:
            return 0;
        case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
            return 1;
        case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
            return 2;
        case DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER:
            return 3;
        }
        return 4;
    }

    struct BindingWithType
    {
        const DenOfIz_BindingDesc    *Binding;
        DenOfIz_ResourceBindingType   Type;
    };
}

MetalBindGroupLayout::MetalBindGroupLayout( MetalContext *context, const DenOfIz_BindGroupLayoutDesc &desc ) : IBindGroupLayout( desc ), m_context( context ), m_desc( desc )
{
    uint32_t cbvSrvUavIndex = 0;
    uint32_t samplerIndex   = 0;

    for ( uint32_t i = 0; i < desc.Bindings.NumElements; ++i )
    {
        const auto &binding = desc.Bindings.Elements[ i ];
        const DenOfIz_ResourceBindingType bindingType = DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor );

        if ( binding.IsBindless )
        {
            m_hasBindless = true;
        }

        if ( desc.RegisterSpace == DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE )
        {
            m_numRootLevelBindings++;
            continue;
        }

        if ( bindingType == DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER )
        {
            m_hasSamplers = true;
        }
        else
        {
            m_hasCbvSrvUav = true;
        }
    }

    if ( m_hasCbvSrvUav )
    {
        std::vector<BindingWithType> sortedBindings;
        for ( uint32_t i = 0; i < desc.Bindings.NumElements; ++i )
        {
            const auto &binding = desc.Bindings.Elements[ i ];
            const DenOfIz_ResourceBindingType bindingType = DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor );

            if ( bindingType == DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER )
            {
                continue;
            }

            sortedBindings.push_back( { &binding, bindingType } );
        }

        std::sort( sortedBindings.begin( ), sortedBindings.end( ), []( const BindingWithType &a, const BindingWithType &b )
        {
            int orderA = GetBindingTypeSortOrder( a.Type );
            int orderB = GetBindingTypeSortOrder( b.Type );
            if ( orderA != orderB )
            {
                return orderA < orderB;
            }
            return a.Binding->Binding < b.Binding->Binding;
        } );

        for ( const auto &sortedBinding : sortedBindings )
        {
            const auto &binding     = *sortedBinding.Binding;
            const auto  bindingType = sortedBinding.Type;

            MTLRenderStages stages      = DenOfIz_MetalEnumConverter_ConvertRenderStages( binding.Stages );
            uint32_t        numElements = binding.IsBindless ? binding.ArraySize : 1;

            ContainerUtilities::EnsureSize( m_cbvSrvUavResourceIndices, binding.Binding + numElements );
            for ( uint32_t elemIdx = 0; elemIdx < numElements; ++elemIdx )
            {
                auto &bindingIndices = m_cbvSrvUavResourceIndices[ binding.Binding + elemIdx ];
                switch ( bindingType )
                {
                case DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER:
                    bindingIndices.Cbv       = cbvSrvUavIndex;
                    bindingIndices.CbvStages = stages;
                    break;
                case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
                    bindingIndices.Srv       = cbvSrvUavIndex;
                    bindingIndices.SrvStages = stages;
                    break;
                case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
                    bindingIndices.Uav       = cbvSrvUavIndex;
                    bindingIndices.UavStages = stages;
                    break;
                case DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER:
                    break;
                }
                cbvSrvUavIndex++;
            }
        }
        m_cbvSrvUavResourceCount = cbvSrvUavIndex;
    }

    if ( m_hasSamplers )
    {
        for ( uint32_t i = 0; i < desc.Bindings.NumElements; ++i )
        {
            const auto &binding = desc.Bindings.Elements[ i ];
            const DenOfIz_ResourceBindingType bindingType = DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor );

            if ( bindingType == DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER )
            {
                ContainerUtilities::EnsureSize( m_samplerResourceIndices, binding.Binding + 1 );
                ContainerUtilities::EnsureSize( m_samplerResourceStages, binding.Binding + 1 );
                m_samplerResourceIndices[ binding.Binding ] = samplerIndex;
                m_samplerResourceStages[ binding.Binding ]  = DenOfIz_MetalEnumConverter_ConvertRenderStages( binding.Stages );
                samplerIndex++;
            }
        }
    }
}

MetalBindGroupLayout::~MetalBindGroupLayout( )
{
}

const DenOfIz_BindGroupLayoutDesc &MetalBindGroupLayout::Desc( ) const
{
    return m_desc;
}

uint32_t MetalBindGroupLayout::RegisterSpace( ) const
{
    return m_desc.RegisterSpace;
}

void MetalBindGroupLayout::SetCbvSrvUavTableOffset( uint32_t offset )
{
    m_cbvSrvUavTableOffset = offset;
}

void MetalBindGroupLayout::SetSamplerTableOffset( uint32_t offset )
{
    m_samplerTableOffset = offset;
}

void MetalBindGroupLayout::SetTLABIndex( uint32_t hash, uint32_t index )
{
    m_uniqueTLABIndex[ hash ] = index;
}

uint32_t MetalBindGroupLayout::CbvSrvUavTableOffset( ) const
{
    return m_cbvSrvUavTableOffset;
}

uint32_t MetalBindGroupLayout::CbvSrvUavTableSize( ) const
{
    return m_cbvSrvUavResourceCount;
}

uint32_t MetalBindGroupLayout::CbvSrvUavResourceIndex( const DenOfIz_ResourceBindingSlot &slot ) const
{
    if ( slot.Binding >= m_cbvSrvUavResourceIndices.size( ) )
    {
        spdlog::error( "Invalid binding index" );
        return 0;
    }

    auto &bindingIndices = m_cbvSrvUavResourceIndices[ slot.Binding ];
    int   resourceIndex  = -1;
    switch ( slot.Type )
    {
    case DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER:
        resourceIndex = bindingIndices.Cbv;
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
        resourceIndex = bindingIndices.Srv;
        if ( resourceIndex == -1 )
        {
            spdlog::error( "SRV binding {} in space {} NOT FOUND in shader reflection!", slot.Binding, m_desc.RegisterSpace );
        }
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
        resourceIndex = bindingIndices.Uav;
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER:
        break;
    }
    if ( resourceIndex == -1 )
    {
        spdlog::error( "Resource binding with type[ {} ],binding[ {} ],register[{} ] not found.", static_cast<int>( slot.Type ), slot.Binding, m_desc.RegisterSpace );
        return 0;
    }
    return resourceIndex;
}

MTLRenderStages MetalBindGroupLayout::CbvSrvUavResourceShaderStages( const DenOfIz_ResourceBindingSlot &slot ) const
{
    if ( slot.Binding >= m_cbvSrvUavResourceIndices.size( ) )
    {
        spdlog::error( "Invalid binding index" );
        return 0;
    }

    auto &bindingIndices = m_cbvSrvUavResourceIndices[ slot.Binding ];
    switch ( slot.Type )
    {
    case DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER:
        return bindingIndices.CbvStages;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
        return bindingIndices.SrvStages;
    case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
        return bindingIndices.UavStages;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER:
        break;
    }
    return MTLRenderStageVertex;
}

uint32_t MetalBindGroupLayout::SamplerTableOffset( ) const
{
    return m_samplerTableOffset;
}

uint32_t MetalBindGroupLayout::SamplerResourceIndex( const DenOfIz_ResourceBindingSlot &slot ) const
{
    if ( slot.Binding >= m_samplerResourceIndices.size( ) )
    {
        spdlog::error( "Invalid sampler binding index" );
        return 0;
    }
    return m_samplerResourceIndices[ slot.Binding ];
}

MTLRenderStages MetalBindGroupLayout::SamplerResourceShaderStages( const DenOfIz_ResourceBindingSlot &slot ) const
{
    if ( slot.Binding >= m_samplerResourceStages.size( ) )
    {
        spdlog::error( "Invalid sampler binding index" );
        return 0;
    }
    return m_samplerResourceStages[ slot.Binding ];
}

uint32_t MetalBindGroupLayout::TLABOffset( const DenOfIz_ResourceBindingSlot &slot ) const
{
    uint32_t hash = Utilities::HashInts( MetalBindGroupLayout::GetRootParameterType( slot.Type ), slot.RegisterSpace, slot.Binding );
    auto     it   = m_uniqueTLABIndex.find( hash );
    if ( it == m_uniqueTLABIndex.end( ) )
    {
        spdlog::error( "TLAB offset not found for binding" );
        return 0;
    }
    return it->second;
}

bool MetalBindGroupLayout::HasCbvSrvUav( ) const
{
    return m_hasCbvSrvUav;
}

bool MetalBindGroupLayout::HasSamplers( ) const
{
    return m_hasSamplers;
}

bool MetalBindGroupLayout::HasBindless( ) const
{
    return m_hasBindless;
}

uint32_t MetalBindGroupLayout::NumRootLevelBindings( ) const
{
    return m_numRootLevelBindings;
}

IRRootParameterType MetalBindGroupLayout::GetRootParameterType( DenOfIz_ResourceBindingType type )
{
    switch ( type )
    {
    case DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER:
        return IRRootParameterTypeCBV;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
        return IRRootParameterTypeSRV;
    case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
        return IRRootParameterTypeUAV;
    default:
        return IRRootParameterTypeCBV;
    }
}
