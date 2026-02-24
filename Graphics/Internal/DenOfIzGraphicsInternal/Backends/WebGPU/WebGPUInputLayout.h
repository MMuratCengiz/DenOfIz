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
#include <vector>
#include <webgpu/webgpu.h>

namespace DenOfIz
{
    class WebGPUInputLayout final : public IInputLayout
    {
        std::vector<WGPUVertexBufferLayout>           m_vertexBufferLayouts{ };
        std::vector<std::vector<WGPUVertexAttribute>> m_vertexAttributes{ };

    public:
        explicit WebGPUInputLayout( const DenOfIz_InputLayoutDesc &desc );
        ~WebGPUInputLayout( ) override;

        [[nodiscard]] const WGPUVertexBufferLayout *GetVertexBufferLayouts( ) const;
        [[nodiscard]] uint32_t                      GetNumBuffers( ) const;
    };

} // namespace DenOfIz
