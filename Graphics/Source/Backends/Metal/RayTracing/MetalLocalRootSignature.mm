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
