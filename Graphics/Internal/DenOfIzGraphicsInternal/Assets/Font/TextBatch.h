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

#include <memory>
#include "DenOfIzGraphics/Assets/Font/TextRenderer.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextLayout.h"

namespace DenOfIz
{
    struct TextBatchDesc
    {
        DenOfIz_BindGroupLayout BindGroupLayout;
        DenOfIz_LogicalDevice   LogicalDevice;
        class Font             *Font;
    };

    class TextBatch
    {
        class Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        DZ_API explicit TextBatch( const TextBatchDesc &desc );
        DZ_API ~TextBatch( );

        DZ_API void BeginBatch( ) const;
        DZ_API void AddText( const DenOfIz_AddTextDesc &desc ) const;
        DZ_API void EndBatch( DenOfIz_CommandList commandList ) const;

        DZ_API void           SetProjectionMatrix( const DenOfIz_Float4x4 &projectionMatrix ) const;
        DZ_API DenOfIz_Float2 MeasureText( DenOfIz_StringView text, const DenOfIz_AddTextDesc &desc ) const;
    };
} // namespace DenOfIz
