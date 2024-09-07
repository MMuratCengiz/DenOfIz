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

#include <DenOfIzGraphics/Backends/Metal/MetalEnumConverter.h>
#include <DenOfIzGraphics/Backends/Metal/MetalRootSignature.h>

using namespace DenOfIz;

MetalRootSignature::MetalRootSignature( MetalContext *context, const RootSignatureDesc &desc ) : IRootSignature( desc ), m_context( context ), m_desc( desc )
{
    std::vector<uint32_t> registerSpaceSize;

    int numTables = 0;
    for ( const auto &binding : m_desc.ResourceBindings )
    {
        ResourceBindingSlot slot = {
            .Binding  = binding.Binding,
            .Register = binding.RegisterSpace,
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

        uint32_t previousStageOffset = 0;

        ContainerUtilities::EnsureSize( m_argumentDescriptors, binding.RegisterSpace );
        if ( m_argumentDescriptors[ binding.RegisterSpace ] == nullptr )
        {
            m_argumentDescriptors[ binding.RegisterSpace ] = [[NSMutableArray alloc] init];
        }

        if ( binding.Reflection.DescriptorOffset >= numTables )
        {
            numTables = binding.Reflection.DescriptorOffset + 1;
        }

        m_metalBindings[ slot.Key( ) ] = { .Parent = binding, .Stages = stages };
    }

    // Todo 10 is arbitrary, find a good number
    m_topLevelArgumentBuffer = std::make_unique<MetalArgumentBuffer>(m_context, numTables, 10);
}

const MetalBindingDesc &MetalRootSignature::FindMetalBinding( const ResourceBindingSlot &slot ) const
{
    auto it = m_metalBindings.find( slot.Key( ) );
    if ( it == m_metalBindings.end( ) )
    {
        LOG( ERROR ) << "Unable to find slot with type[" << static_cast<int>( slot.Type ) << "],binding[" << slot.Binding << "],register[" << slot.Register << "].";
    }
    return it->second;
}

MetalRootSignature::~MetalRootSignature( )
{
}
