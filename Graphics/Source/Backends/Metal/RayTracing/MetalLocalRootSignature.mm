/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalLocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalShaderLayout.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalLocalRootSignature::MetalLocalRootSignature( MetalContext *context, const DenOfIz_LocalRootSignatureDesc &desc ) : m_context( context ), m_desc( desc )
{
    if ( desc.ResourceBindings.NumElements > 0 && desc.ResourceBindings.Elements != nullptr )
    {
        m_bindingsCopy.assign( desc.ResourceBindings.Elements, desc.ResourceBindings.Elements + desc.ResourceBindings.NumElements );
        m_desc.ResourceBindings.Elements = m_bindingsCopy.data( );
    }

    for ( const DenOfIz_LocalResourceBindingDesc &binding : m_bindingsCopy )
    {
        const DenOfIz_ResourceBindingType bindingType = DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor );
        const uint64_t                    key         = MetalShaderLayout::BindingKey( bindingType, binding.Binding );
        if ( m_bindings.contains( key ) )
        {
            spdlog::error( "Duplicate local binding of type [ {} ] at register [ {} ].", static_cast<int>( bindingType ), binding.Binding );
            continue;
        }
        m_bindings[ key ] = &binding;
    }
}

const DenOfIz_LocalRootSignatureDesc &MetalLocalRootSignature::Desc( ) const
{
    return m_desc;
}

const DenOfIz_LocalResourceBindingDesc *MetalLocalRootSignature::FindBinding( const DenOfIz_ResourceBindingType type, const uint32_t binding ) const
{
    const auto it = m_bindings.find( MetalShaderLayout::BindingKey( type, binding ) );
    return it == m_bindings.end( ) ? nullptr : it->second;
}
