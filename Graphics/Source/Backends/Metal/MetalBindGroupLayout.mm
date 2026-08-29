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
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalShaderLayout.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalBindGroupLayout::MetalBindGroupLayout( MetalContext *context, const DenOfIz_BindGroupLayoutDesc &desc ) : IBindGroupLayout( desc ), m_context( context )
{
    const DenOfIz_BindGroupLayoutDesc &storedDesc = Desc( );
    for ( uint32_t i = 0; i < storedDesc.Bindings.NumElements; ++i )
    {
        const DenOfIz_BindingDesc        &binding     = storedDesc.Bindings.Elements[ i ];
        const DenOfIz_ResourceBindingType bindingType = DenOfIz_ResourceBindingType_FromDescriptor( binding.Descriptor );
        const uint64_t                    key         = MetalShaderLayout::BindingKey( bindingType, binding.Binding );
        if ( m_bindings.contains( key ) )
        {
            spdlog::error( "Duplicate binding of type [ {} ] at register [ {} ] in space [ {} ].", static_cast<int>( bindingType ), binding.Binding, storedDesc.RegisterSpace );
            continue;
        }
        m_bindings[ key ] = &binding;
    }
}

MetalBindGroupLayout::~MetalBindGroupLayout( )
{
}

uint32_t MetalBindGroupLayout::RegisterSpace( ) const
{
    return Desc( ).RegisterSpace;
}

const DenOfIz_BindingDesc *MetalBindGroupLayout::FindBinding( const DenOfIz_ResourceBindingType type, const uint32_t binding ) const
{
    const auto it = m_bindings.find( MetalShaderLayout::BindingKey( type, binding ) );
    return it == m_bindings.end( ) ? nullptr : it->second;
}

MTLRenderStages MetalBindGroupLayout::ShaderStages( const DenOfIz_ResourceBindingType type, const uint32_t binding ) const
{
    const DenOfIz_BindingDesc *bindingDesc = FindBinding( type, binding );
    if ( bindingDesc == nullptr )
    {
        return MTLRenderStageVertex | MTLRenderStageFragment;
    }
    return DenOfIz_MetalEnumConverter_ConvertRenderStages( bindingDesc->Stages );
}
