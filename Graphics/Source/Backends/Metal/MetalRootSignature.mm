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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalRootSignature.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

#define BIND_GROUP_LAYOUT_IMPL( handle ) static_cast<MetalBindGroupLayout *>( DENOFIZ_FROM_HANDLE( IBindGroupLayout, handle ) )

MetalRootSignature::MetalRootSignature( MetalContext *context, const DenOfIz_RootSignatureDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_rootConstants.resize( m_desc.RootConstants.NumElements );
    for ( uint32_t i = 0; i < m_desc.RootConstants.NumElements; i++ )
    {
        const auto &trueIndex = m_desc.RootConstants.Elements[ i ].Binding;
        if ( trueIndex >= m_desc.RootConstants.NumElements )
        {
            spdlog::critical( "Root constant binding index is out of range. Make sure all bindings are provided in ascending order." );
        }
        const auto &rootConstant     = m_desc.RootConstants.Elements[ trueIndex ];
        m_rootConstants[ trueIndex ] = { .Offset = m_numRootConstantBytes, .NumBytes = rootConstant.NumBytes };
        m_numRootConstantBytes += rootConstant.NumBytes;
    }

    uint32_t maxRegisterSpace = 0;
    for ( uint32_t i = 0; i < desc.BindGroupLayouts.NumElements; ++i )
    {
        MetalBindGroupLayout *layout = BIND_GROUP_LAYOUT_IMPL( desc.BindGroupLayouts.Elements[ i ] );
        maxRegisterSpace             = std::max( maxRegisterSpace, layout->RegisterSpace( ) );
    }

    m_bindGroupLayouts.resize( maxRegisterSpace + 1, nullptr );
    for ( uint32_t i = 0; i < desc.BindGroupLayouts.NumElements; ++i )
    {
        MetalBindGroupLayout *layout                    = BIND_GROUP_LAYOUT_IMPL( desc.BindGroupLayouts.Elements[ i ] );
        m_bindGroupLayouts[ layout->RegisterSpace( ) ] = layout;
    }

    int currentTLABOffset = m_numRootConstantBytes / sizeof( uint64_t );

    bool hasBindlessResources = false;
    for ( uint32_t i = 0; i < desc.BindGroupLayouts.NumElements; ++i )
    {
        MetalBindGroupLayout *layout = BIND_GROUP_LAYOUT_IMPL( desc.BindGroupLayouts.Elements[ i ] );
        if ( layout->HasBindless( ) )
        {
            hasBindlessResources = true;
            break;
        }
    }

    for ( uint32_t space = 0; space <= maxRegisterSpace; ++space )
    {
        MetalBindGroupLayout *layout = m_bindGroupLayouts[ space ];
        if ( layout == nullptr )
        {
            continue;
        }

        for ( uint32_t j = 0; j < layout->Desc( ).Bindings.NumElements; ++j )
        {
            const auto                       &binding     = layout->Desc( ).Bindings.Elements[ j ];
            const DenOfIz_ResourceBindingType bindingType = DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor );

            if ( layout->RegisterSpace( ) == DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE )
            {
                uint32_t hash = Utilities::HashInts( MetalBindGroupLayout::GetRootParameterType( bindingType ), layout->RegisterSpace( ), binding.Binding );
                layout->SetTLABIndex( hash, currentTLABOffset++ );
                m_numTLABAddresses++;
            }
        }

        if ( layout->HasCbvSrvUav( ) )
        {
            if ( layout->HasBindless( ) && space == 0 )
            {
                layout->SetCbvSrvUavTableOffset( 0 );
            }
            else if ( hasBindlessResources && space == 1 )
            {
                layout->SetCbvSrvUavTableOffset( 2 );
            }
            else
            {
                layout->SetCbvSrvUavTableOffset( currentTLABOffset++ );
            }
            m_numTLABAddresses++;
        }

        if ( layout->HasSamplers( ) )
        {
            if ( hasBindlessResources && space == 0 )
            {
                layout->SetSamplerTableOffset( 1 );
            }
            else
            {
                layout->SetSamplerTableOffset( currentTLABOffset++ );
            }
            m_numTLABAddresses++;
        }
    }
}

uint32_t MetalRootSignature::NumTLABAddresses( ) const
{
    return m_numTLABAddresses;
}

const uint32_t &MetalRootSignature::NumRootConstantBytes( ) const
{
    return m_numRootConstantBytes;
}

const std::vector<MetalRootConstant> &MetalRootSignature::RootConstants( ) const
{
    return m_rootConstants;
}

const std::vector<MetalBindGroupLayout *> &MetalRootSignature::BindGroupLayouts( ) const
{
    return m_bindGroupLayouts;
}

MetalBindGroupLayout *MetalRootSignature::GetBindGroupLayout( uint32_t registerSpace ) const
{
    if ( registerSpace >= m_bindGroupLayouts.size( ) )
    {
        spdlog::error( "Register space {} does not exist.", registerSpace );
        return nullptr;
    }
    return m_bindGroupLayouts[ registerSpace ];
}

MetalRootSignature::~MetalRootSignature( )
{
}
