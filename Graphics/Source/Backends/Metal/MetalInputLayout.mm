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

MetalInputLayout::MetalInputLayout( MetalContext *context, const InputLayoutDesc &desc ) : m_context( context )
{
    m_inputGroups.reserve(desc.InputGroups.NumElements);
    m_inputElements.reserve(desc.InputGroups.NumElements);
    
    for ( uint32_t i = 0; i < desc.InputGroups.NumElements; i++ )
    {
        const InputGroupDesc &inputGroup = desc.InputGroups.Elements[i];
        m_inputGroups.push_back(inputGroup);
        
        std::vector<InputLayoutElementDesc> elements;
        elements.reserve(inputGroup.Elements.NumElements);
        for ( uint32_t j = 0; j < inputGroup.Elements.NumElements; j++ )
        {
            elements.push_back(inputGroup.Elements.Elements[j]);
        }
        m_inputElements.push_back(std::move(elements));
    }
    
    m_vertexDescriptor    = [[MTLVertexDescriptor alloc] init];
    int      bindingIndex = 0;
    uint32_t location     = kIRStageInAttributeStartIndex;

    for ( uint32_t i = 0; i < m_inputGroups.size(); i++ )
    {
        const InputGroupDesc &inputGroup = m_inputGroups[i];
        auto                 *layout     = [m_vertexDescriptor.layouts objectAtIndexedSubscript:bindingIndex];
        layout.stepFunction              = inputGroup.StepRate == StepRate::PerInstance ? MTLVertexStepFunctionPerInstance : MTLVertexStepFunctionPerVertex;

        uint32_t offset = 0;
        for ( uint32_t elementIndex = 0; elementIndex < m_inputElements[i].size(); ++elementIndex )
        {
            const InputLayoutElementDesc &inputElement = m_inputElements[i][elementIndex];
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
