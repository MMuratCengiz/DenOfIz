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

#include <unordered_map>
#include "DenOfIzGraphicsInternal/Backends/Interface/IBindGroupLayout.h"
#include "MetalContext.h"

namespace DenOfIz
{
    class MetalBindGroupLayout final : public IBindGroupLayout
    {
        MetalContext                                                *m_context;
        std::unordered_map<uint64_t, const DenOfIz_BindingDesc *>    m_bindings;

    public:
        MetalBindGroupLayout( MetalContext *context, const DenOfIz_BindGroupLayoutDesc &desc );
        ~MetalBindGroupLayout( ) override;

        [[nodiscard]] uint32_t                   RegisterSpace( ) const;
        [[nodiscard]] const DenOfIz_BindingDesc *FindBinding( DenOfIz_ResourceBindingType type, uint32_t binding ) const;
        [[nodiscard]] MTLRenderStages            ShaderStages( DenOfIz_ResourceBindingType type, uint32_t binding ) const;
    };

} // namespace DenOfIz
