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
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

#define BIND_GROUP_LAYOUT_IMPL( handle ) static_cast<MetalBindGroupLayout *>( DENOFIZ_FROM_HANDLE( IBindGroupLayout, handle ) )

MetalRootSignature::MetalRootSignature( MetalContext *context, const DenOfIz_RootSignatureDesc &desc ) : m_context( context ), m_desc( desc )
{
    uint32_t maxRegisterSpace = 0;
    for ( uint32_t i = 0; i < desc.BindGroupLayouts.NumElements; ++i )
    {
        MetalBindGroupLayout *layout = BIND_GROUP_LAYOUT_IMPL( desc.BindGroupLayouts.Elements[ i ] );
        maxRegisterSpace             = std::max( maxRegisterSpace, layout->RegisterSpace( ) );
    }

    m_bindGroupLayouts.resize( desc.BindGroupLayouts.NumElements == 0 ? 0 : maxRegisterSpace + 1, nullptr );
    for ( uint32_t i = 0; i < desc.BindGroupLayouts.NumElements; ++i )
    {
        MetalBindGroupLayout *layout = BIND_GROUP_LAYOUT_IMPL( desc.BindGroupLayouts.Elements[ i ] );
        if ( m_bindGroupLayouts[ layout->RegisterSpace( ) ] != nullptr )
        {
            spdlog::error( "Multiple bind group layouts provided for register space {}.", layout->RegisterSpace( ) );
        }
        m_bindGroupLayouts[ layout->RegisterSpace( ) ] = layout;
    }
}

const std::vector<MetalBindGroupLayout *> &MetalRootSignature::BindGroupLayouts( ) const
{
    return m_bindGroupLayouts;
}

MetalBindGroupLayout *MetalRootSignature::GetBindGroupLayout( const uint32_t registerSpace ) const
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
