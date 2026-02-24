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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalEnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalInputLayout.h"

#include <utility>

using namespace DenOfIz;

MetalInputLayout::MetalInputLayout( MetalContext *context, const DenOfIz_InputLayoutDesc &desc ) : m_context( context )
{
    m_inputGroups.reserve( desc.InputGroups.NumElements );
    m_inputElements.reserve( desc.InputGroups.NumElements );

    for ( uint32_t i = 0; i < desc.InputGroups.NumElements; i++ )
    {
        const DenOfIz_InputGroupDesc &inputGroup = desc.InputGroups.Elements[ i ];
        m_inputGroups.push_back( inputGroup );

        std::vector<DenOfIz_InputLayoutElementDesc> elements;
        elements.reserve( inputGroup.Elements.NumElements );
        for ( uint32_t j = 0; j < inputGroup.Elements.NumElements; j++ )
        {
            const DenOfIz_InputLayoutElementDesc &inputElement = inputGroup.Elements.Elements[ j ];
            DenOfIz_InputLayoutElementDesc        desc{ };
            m_semanticStrings.emplace_back( inputElement.Semantic.Chars, inputElement.Semantic.NumChars );
            desc.Semantic       = DenOfIz_StringView( m_semanticStrings.back( ).c_str( ), m_semanticStrings.back( ).length( ) );
            desc.SemanticIndex  = inputElement.SemanticIndex;
            desc.Format         = inputElement.Format;
            elements.push_back( desc );
        }
        m_inputElements.push_back( std::move( elements ) );
    }
    
    m_vertexDescriptor    = [[MTLVertexDescriptor alloc] init];
    int      bindingIndex = 0;
    uint32_t location     = kIRStageInAttributeStartIndex;

    for ( uint32_t i = 0; i < m_inputGroups.size(); i++ )
    {
        const DenOfIz_InputGroupDesc &inputGroup = m_inputGroups[i];
        auto                 *layout     = [m_vertexDescriptor.layouts objectAtIndexedSubscript:bindingIndex];
        layout.stepFunction              = inputGroup.StepRate == DENOFIZ_STEP_RATE_PER_INSTANCE ? MTLVertexStepFunctionPerInstance : MTLVertexStepFunctionPerVertex;

        uint32_t offset = 0;
        for ( uint32_t elementIndex = 0; elementIndex < m_inputElements[i].size(); ++elementIndex )
        {
            const DenOfIz_InputLayoutElementDesc &inputElement = m_inputElements[i][elementIndex];
            auto *attribute       = [m_vertexDescriptor.attributes objectAtIndexedSubscript:location];
            attribute.bufferIndex = bindingIndex;
            attribute.offset      = offset;
            attribute.format      = DenOfIz_MetalEnumConverter_ConvertFormatToVertexFormat( inputElement.Format );
            offset += DenOfIz_Format_NumBytes( inputElement.Format );
            location++;
        }
        layout.stride = inputGroup.Stride > 0 ? inputGroup.Stride : offset;
        bindingIndex++;
    }
}

MetalInputLayout::~MetalInputLayout( )
{
}

MTLVertexDescriptor *MetalInputLayout::GetVertexDescriptor( ) const
{
    return m_vertexDescriptor;
}
