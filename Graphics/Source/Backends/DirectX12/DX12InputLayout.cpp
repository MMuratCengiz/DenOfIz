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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12InputLayout.h"

using namespace DenOfIz;

DX12InputLayout::DX12InputLayout( const DenOfIz_InputLayoutDesc &desc )
{
    m_strides.reserve( desc.InputGroups.NumElements );

    for ( uint32_t bindingIndex = 0; bindingIndex < desc.InputGroups.NumElements; bindingIndex++ )
    {
        const DenOfIz_InputGroupDesc &inputGroup           = desc.InputGroups.Elements[ bindingIndex ];
        D3D12_INPUT_CLASSIFICATION    inputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        uint32_t                      instanceDataStepRate = 0;

        if ( inputGroup.StepRate == DENOFIZ_STEP_RATE_PER_INSTANCE )
        {
            inputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
            instanceDataStepRate = 1;
        }

        uint32_t offset = 0;
        for ( uint32_t layoutElementIndex = 0; layoutElementIndex < inputGroup.Elements.NumElements; layoutElementIndex++ )
        {
            const DenOfIz_InputLayoutElementDesc &inputElement = inputGroup.Elements.Elements[ layoutElementIndex ];
            D3D12_INPUT_ELEMENT_DESC             &element      = m_inputElements.emplace_back( D3D12_INPUT_ELEMENT_DESC{ } );

            m_semanticStrings.emplace_back( inputElement.Semantic.Chars, inputElement.Semantic.NumChars );
            element.SemanticName         = m_semanticStrings.back( ).c_str( );
            element.SemanticIndex        = inputElement.SemanticIndex;
            element.Format               = DenOfIz_DX12EnumConverter_ConvertFormat( inputElement.Format );
            element.InputSlot            = bindingIndex;
            element.InputSlotClass       = inputSlotClass;
            element.AlignedByteOffset    = offset;
            element.InstanceDataStepRate = instanceDataStepRate;

            offset += DenOfIz_Format_NumBytes( inputElement.Format );
        }
        m_strides.push_back( inputGroup.Stride > 0 ? inputGroup.Stride : offset );
    }

    m_inputLayout                    = { };
    m_inputLayout.pInputElementDescs = m_inputElements.data( );
    m_inputLayout.NumElements        = m_inputElements.size( );
}

DX12InputLayout::~DX12InputLayout( )
{
}

uint32_t DX12InputLayout::Stride( const uint32_t bindingIndex ) const
{
    if ( bindingIndex < m_strides.size( ) )
    {
        return m_strides[ bindingIndex ];
    }
    return 0;
}

const std::vector<uint32_t> &DX12InputLayout::GetStrides( ) const
{
    return m_strides;
}

const D3D12_INPUT_LAYOUT_DESC &DX12InputLayout::GetInputLayout( ) const
{
    return m_inputLayout;
}
