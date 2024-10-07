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
#include <algorithm>

using namespace DenOfIz;

MetalRootSignature::MetalRootSignature( MetalContext *context, const RootSignatureDesc &desc ) : IRootSignature( desc ), m_context( context ), m_desc( desc )
{
    std::vector<uint32_t> registerSpaceSize;

    int numTables = 0;
    for ( const auto &binding : m_desc.ResourceBindings )
    {
        ResourceBindingSlot slot = {
            .Binding       = binding.Binding,
            .RegisterSpace = binding.RegisterSpace,
            .Type          = binding.BindingType,
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
        if ( binding.Reflection.TLABOffset >= m_numTLABAddresses )
        {
            m_numTLABAddresses = binding.Reflection.TLABOffset + 1;
        }

        m_metalBindings[ slot.Key( ) ] = { .Parent = binding, .Stages = stages };
    }

    m_rootConstants.resize( m_desc.RootConstants.size( ) );
    for ( int i = 0; i < m_desc.RootConstants.size( ); i++ )
    {
        const auto &trueIndex = m_desc.RootConstants[ i ].Binding;
        if ( trueIndex >= m_desc.RootConstants.size( ) )
        {
            LOG( FATAL ) << "Root constant binding index is out of range. Make sure all bindings are provided in ascending order.";
        }
        const auto &rootConstant     = m_desc.RootConstants[ trueIndex ];
        m_rootConstants[ trueIndex ] = { .Offset = m_numRootConstantBytes, .NumBytes = rootConstant.NumBytes };
        m_numRootConstantBytes += rootConstant.NumBytes;
    }
}

const MetalBindingDesc &MetalRootSignature::FindMetalBinding( const ResourceBindingSlot &slot ) const
{
    auto it = m_metalBindings.find( slot.Key( ) );
    if ( it == m_metalBindings.end( ) )
    {
        LOG( ERROR ) << "Unable to find slot with type[" << static_cast<int>( slot.Type ) << "],binding[" << slot.Binding << "],register[" << slot.RegisterSpace << "].";
    }
    return it->second;
}

const uint32_t MetalRootSignature::NumTLABAddresses( ) const
{
    return m_numTLABAddresses;
}

[[nodiscard]] const uint32_t &MetalRootSignature::NumRootConstantBytes( ) const
{
    return m_numRootConstantBytes;
}

const std::vector<MetalRootConstant> &MetalRootSignature::RootConstants( ) const
{
    return m_rootConstants;
}

MetalRootSignature::~MetalRootSignature( )
{
}
