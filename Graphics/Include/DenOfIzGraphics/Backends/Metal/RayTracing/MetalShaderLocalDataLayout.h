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

#include <DenOfIzGraphics/Backends/Interface/RayTracing/IShaderLocalDataLayout.h>
#include <DenOfIzGraphics/Backends/Metal/MetalContext.h>

namespace DenOfIz
{
    struct MetalLocalBindingDesc
    {
        uint32_t            TLABOffset;
        size_t              NumBytes;
        ResourceBindingType Type;
    };

    class MetalShaderLocalDataLayout final : public IShaderLocalDataLayout
    {
        MetalContext             *m_context;
        ShaderLocalDataLayoutDesc m_desc;

        std::vector<MetalLocalBindingDesc> m_bindings;
        uint32_t                           m_totalInlineDataBytes = 0;
        uint32_t                           m_numSrvUavs           = 0;
        uint32_t                           m_numSamplers          = 0;

    public:
        MetalShaderLocalDataLayout( MetalContext *context, const ShaderLocalDataLayoutDesc &desc );
        uint32_t NumInlineBytes( ) const;
        uint32_t NumSrvUavs( ) const;
        uint32_t NumSamplers( ) const;

        const MetalLocalBindingDesc &GetBinding( uint32_t binding ) const;
    };
} // namespace DenOfIz
