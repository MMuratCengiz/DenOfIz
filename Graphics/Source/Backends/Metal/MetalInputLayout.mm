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

MetalInputLayout::MetalInputLayout( MetalContext *context, InputLayoutDesc desc ) : m_context( context ), m_desc( std::move( desc ) )
{
    m_vertexDescriptor    = [[MTLVertexDescriptor alloc] init];
    int      bindingIndex = 0;
    uint32_t location     = kIRStageInAttributeStartIndex;

    for ( int i = 0; i < m_desc.InputGroups.NumElements( ); i++ )
    {
        const InputGroupDesc &inputGroup = m_desc.InputGroups.GetElement( i );
        auto                 *layout     = [m_vertexDescriptor.layouts objectAtIndexedSubscript:bindingIndex];
        layout.stepFunction              = inputGroup.StepRate == StepRate::PerInstance ? MTLVertexStepFunctionPerInstance : MTLVertexStepFunctionPerVertex;

        uint32_t offset = 0;
        for ( int elementIndex = 0; elementIndex < inputGroup.Elements.NumElements( ); ++elementIndex )
        {
            const InputLayoutElementDesc &inputElement = inputGroup.Elements.GetElement( elementIndex );
            auto *attribute       = [m_vertexDescriptor.attributes objectAtIndexedSubscript:location];
            attribute.bufferIndex = bindingIndex;
            attribute.offset      = offset;
            attribute.format      = MetalEnumConverter::ConvertFormatToVertexFormat( inputElement.Format );
            offset += FormatNumBytes( inputElement.Format );
            location++;
        }
        layout.stride = offset;
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
