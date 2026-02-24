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

#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphics/Backends/Interface/CommandList.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_TextRenderer )

    typedef enum DenOfIz_TextDirection
    {
        DENOFIZ_TEXT_DIRECTION_LEFT_TO_RIGHT,
        DENOFIZ_TEXT_DIRECTION_RIGHT_TO_LEFT,
        DENOFIZ_TEXT_DIRECTION_AUTO
    } DenOfIz_TextDirection;

    typedef struct DenOfIz_TextRendererDesc
    {
        DenOfIz_LogicalDevice LogicalDevice;
        uint32_t              InitialAtlasWidth;
        uint32_t              InitialAtlasHeight;
        uint32_t              Width;
        uint32_t              Height;
        DenOfIz_Font          Font;
    } DenOfIz_TextRendererDesc;

    typedef struct DenOfIz_AddTextDesc
    {
        uint16_t              FontSize;
        DenOfIz_StringView    Text;
        float                 X;
        float                 Y;
        DenOfIz_Float4        Color;
        uint16_t              LetterSpacing;
        uint16_t              LineHeight;
        bool                  HorizontalCenter;
        bool                  VerticalCenter;
        DenOfIz_TextDirection Direction;
    } DenOfIz_AddTextDesc;

    typedef struct DenOfIz_TextRenderDesc
    {
        uint16_t              FontSize;
        DenOfIz_StringView    Text;
        float                 X;
        float                 Y;
        DenOfIz_Float4        Color;
        uint16_t              LetterSpacing;
        uint16_t              LineHeight;
        bool                  HorizontalCenter;
        bool                  VerticalCenter;
        DenOfIz_TextDirection Direction;
        uint16_t              FontId;
    } DenOfIz_TextRenderDesc;

    DZ_API DenOfIz_TextRenderer DenOfIz_TextRenderer_Create( const DenOfIz_TextRendererDesc *desc );
    DZ_API void                 DenOfIz_TextRenderer_Destroy( DenOfIz_TextRenderer textRenderer );

    DZ_API void DenOfIz_TextRenderer_SetProjectionMatrix( DenOfIz_TextRenderer textRenderer, const DenOfIz_Float4x4 *projectionMatrix );
    DZ_API void DenOfIz_TextRenderer_SetViewport( DenOfIz_TextRenderer textRenderer, const DenOfIz_Viewport *viewport );

    DZ_API uint16_t     DenOfIz_TextRenderer_AddFont( DenOfIz_TextRenderer textRenderer, DenOfIz_Font font, uint16_t fontId );
    DZ_API DenOfIz_Font DenOfIz_TextRenderer_GetFont( DenOfIz_TextRenderer textRenderer, uint16_t fontId );
    DZ_API void         DenOfIz_TextRenderer_RemoveFont( DenOfIz_TextRenderer textRenderer, uint16_t fontId );

    DZ_API void DenOfIz_TextRenderer_BeginBatch( DenOfIz_TextRenderer textRenderer );
    DZ_API void DenOfIz_TextRenderer_AddText( DenOfIz_TextRenderer textRenderer, const DenOfIz_TextRenderDesc *textDesc );
    DZ_API void DenOfIz_TextRenderer_EndBatch( DenOfIz_TextRenderer textRenderer, DenOfIz_CommandList commandList );

    DZ_API DenOfIz_Float2 DenOfIz_TextRenderer_MeasureText( DenOfIz_TextRenderer textRenderer, DenOfIz_StringView text, const DenOfIz_TextRenderDesc *desc );

#ifdef __cplusplus
}
#endif
