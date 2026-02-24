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
