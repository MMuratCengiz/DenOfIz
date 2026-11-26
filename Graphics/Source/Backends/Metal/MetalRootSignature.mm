/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
