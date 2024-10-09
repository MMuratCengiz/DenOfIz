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

#include <DenOfIzGraphics/Backends/DirectX12/DX12InputLayout.h>

using namespace DenOfIz;

DX12InputLayout::DX12InputLayout( const InputLayoutDesc &desc )
{
    int bindingIndex = 0;
    for ( const InputGroupDesc &inputGroup : desc.InputGroups )
    {
        D3D12_INPUT_CLASSIFICATION inputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        uint32_t                   instanceDataStepRate = 0;

        if ( inputGroup.StepRate == StepRate::PerInstance )
        {
            inputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
            instanceDataStepRate = 1;
        }

        uint32_t offset = 0;
        for ( const InputLayoutElementDesc &inputElement : inputGroup.Elements )
        {
            D3D12_INPUT_ELEMENT_DESC &element = m_inputElements.emplace_back( D3D12_INPUT_ELEMENT_DESC{ } );

            switch ( inputElement.Semantic )
            {
            case Semantic::Position:
                element.SemanticName = "POSITION";
                break;
            case Semantic::Normal:
                element.SemanticName = "NORMAL";
                break;
            case Semantic::Color:
                element.SemanticName = "COLOR";
                break;
            case Semantic::Tangent:
                element.SemanticName = "TANGENT";
                break;
            case Semantic::Binormal:
                element.SemanticName = "BINORMAL";
                break;
            case Semantic::Bitangent:
                element.SemanticName = "BITANGENT";
                break;
            case Semantic::BlendJoints:
                element.SemanticName = "BLENDJOINTS";
                break;
            case Semantic::BlendWeights:
                element.SemanticName = "BLENDWEIGHTS";
                break;
            case Semantic::TextureCoordinate:
                element.SemanticName = &"TEXCOORD"[ inputElement.SemanticIndex ];
                break;
            }

            element.SemanticIndex        = inputElement.SemanticIndex;
            element.Format               = DX12EnumConverter::ConvertFormat( inputElement.Format );
            element.InputSlot            = bindingIndex;
            element.InputSlotClass       = inputSlotClass;
            element.AlignedByteOffset    = offset;
            element.InstanceDataStepRate = instanceDataStepRate;

            offset += FormatNumBytes( inputElement.Format );
        }

        bindingIndex++;
    }

    m_inputLayout                    = { };
    m_inputLayout.pInputElementDescs = m_inputElements.data( );
    m_inputLayout.NumElements        = m_inputElements.size( );
}

DX12InputLayout::~DX12InputLayout( )
{
}

const D3D12_INPUT_LAYOUT_DESC &DX12InputLayout::GetInputLayout( ) const
{
    return m_inputLayout;
}
