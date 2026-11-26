/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/IInputLayout.h>
#include <deque>
#include "DX12Context.h"
#include "DX12EnumConverter.h"

namespace DenOfIz
{

    class DX12InputLayout final : public IInputLayout
    {
        std::vector<uint32_t>                 m_strides;
        std::deque<std::string>               m_semanticStrings;
        std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElements;
        D3D12_INPUT_LAYOUT_DESC               m_inputLayout;

    public:
        explicit DX12InputLayout( const DenOfIz_InputLayoutDesc &desc );
        const D3D12_INPUT_LAYOUT_DESC &GetInputLayout( ) const;
        ~DX12InputLayout( ) override;
        [[nodiscard]] uint32_t                     Stride( uint32_t bindingIndex = 0 ) const;
        [[nodiscard]] const std::vector<uint32_t> &GetStrides( ) const;
    };

} // namespace DenOfIz
