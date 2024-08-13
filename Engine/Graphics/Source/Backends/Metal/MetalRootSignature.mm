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

#include <DenOfIzGraphics/Backends/Metal/MetalRootSignature.h>
#import <metal_irconverter/metal_irconverter.h>

using namespace DenOfIz;

MetalRootSignature::MetalRootSignature( MetalContext *context, const RootSignatureDesc &desc ) : IRootSignature( desc ), m_context( context ), m_desc( desc )
{
    for ( const auto &binding : m_desc.ResourceBindings )
    {
        ResourceBindingSlot slot = {
            .Register = binding.RegisterSpace,
            .Binding  = binding.Binding,
            .Type     = binding.BindingType,
        };

        MTLRenderStages stages = 0;
        for ( const auto &stage : binding.Stages )
        {
            if ( stage == ShaderStage::Vertex )
            {
                stages |= MTLRenderStageVertex;
            }
            if ( stage == ShaderStage::Pixel )
            {
                stages |= MTLRenderStageFragment;
            }
        }

        ContainerUtilities::SafeSet( m_bindings, slot.Key( ),
                                     MetalBindingDesc{
                                         .Name   = binding.Name,
                                         .Location = binding.LocationHint,
                                         .Stages = stages,
                                     } );
    }
}

const MetalBindingDesc &MetalRootSignature::FindBinding( const ResourceBindingSlot &slot ) const
{
    uint32_t slotIndex = slot.Key( );
    if ( slotIndex >= m_bindings.size( ) )
    {
        LOG( ERROR ) << "Resource slot not found, Register: " << slot.Register << " Binding: " << slot.Binding << " Type: " << static_cast<uint32_t>( slot.Type );
        return MetalBindingDesc( );
    }
    return m_bindings[ slotIndex ];
}

MetalRootSignature::~MetalRootSignature( )
{
}
