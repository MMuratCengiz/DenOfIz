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

#include <vector>
#include "DenOfIzGraphicsInternal/Backends/Interface/IBindGroupLayout.h"
#include "WebGPUContext.h"

namespace DenOfIz
{

    class WebGPUBindGroupLayout final : public IBindGroupLayout
    {
    private:
        WebGPUContext              *m_context;
        DenOfIz_BindGroupLayoutDesc m_desc;

        WGPUBindGroupLayout                   m_layout = nullptr;
        std::vector<WGPUBindGroupLayoutEntry> m_entries;
        bool                                  m_hasBindless = false;

    public:
        WebGPUBindGroupLayout( WebGPUContext *context, const DenOfIz_BindGroupLayoutDesc &desc );
        ~WebGPUBindGroupLayout( ) override;

        [[nodiscard]] const DenOfIz_BindGroupLayoutDesc &Desc( ) const;
        [[nodiscard]] uint32_t                           RegisterSpace( ) const;

        [[nodiscard]] WGPUBindGroupLayout GetBindGroupLayout( ) const;
        [[nodiscard]] bool                HasBindless( ) const;

    private:
        void AddResourceBinding( const DenOfIz_BindingDesc &binding );
    };

} // namespace DenOfIz
