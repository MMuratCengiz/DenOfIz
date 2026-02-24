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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUInputLayout.h"
#include <DenOfIzGraphics/Backends/Interface/CommonData.h>
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

WebGPUInputLayout::WebGPUInputLayout( const DenOfIz_InputLayoutDesc &desc )
{
    m_vertexBufferLayouts.reserve( desc.InputGroups.NumElements );
    m_vertexAttributes.reserve( desc.InputGroups.NumElements );

    for ( uint32_t bindingIndex = 0; bindingIndex < desc.InputGroups.NumElements; bindingIndex++ )
    {
        const DenOfIz_InputGroupDesc &inputGroup = desc.InputGroups.Elements[ bindingIndex ];

        m_vertexAttributes.emplace_back( );
        std::vector<WGPUVertexAttribute> &attributes = m_vertexAttributes.back( );
        attributes.reserve( inputGroup.Elements.NumElements );

        uint64_t offset = 0;
        for ( uint32_t elementIndex = 0; elementIndex < inputGroup.Elements.NumElements; elementIndex++ )
        {
            const DenOfIz_InputLayoutElementDesc &inputElement = inputGroup.Elements.Elements[ elementIndex ];

            WGPUVertexAttribute &attribute = attributes.emplace_back( );
            attribute.format               = DenOfIz_WebGPUEnumConverter_ConvertVertexFormat( inputElement.Format );
            attribute.offset               = offset;
            attribute.shaderLocation       = elementIndex;

            offset += DenOfIz_Format_NumBytes( inputElement.Format );
        }

        WGPUVertexBufferLayout &bufferLayout = m_vertexBufferLayouts.emplace_back( );
        bufferLayout.arrayStride             = inputGroup.Stride > 0 ? inputGroup.Stride : offset;
        bufferLayout.stepMode                = DenOfIz_WebGPUEnumConverter_ConvertStepRate( inputGroup.StepRate );
        bufferLayout.attributeCount          = static_cast<uint32_t>( attributes.size( ) );
        bufferLayout.attributes              = attributes.data( );
    }
}

WebGPUInputLayout::~WebGPUInputLayout( ) = default;

const WGPUVertexBufferLayout *WebGPUInputLayout::GetVertexBufferLayouts( ) const
{
    return m_vertexBufferLayouts.data( );
}

uint32_t WebGPUInputLayout::GetNumBuffers( ) const
{
    return static_cast<uint32_t>( m_vertexBufferLayouts.size( ) );
}
