/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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

    // Draws text in a render pass on one B8G8R8A8_UNORM target without depth. Positions and sizes are in the units of the projection, by
    // default viewport pixels with the origin at the top left. Fonts are borrowed, not owned: a font must stay alive while it is registered,
    // until RemoveFont, its replacement by AddFont or the renderer's destruction.
    DENOFIZ_DEFINE_HANDLE( DenOfIz_TextRenderer )

    // AUTO takes the direction of the first strong character of each paragraph.
    typedef enum DenOfIz_TextDirection
    {
        DENOFIZ_TEXT_DIRECTION_AUTO,
        DENOFIZ_TEXT_DIRECTION_LEFT_TO_RIGHT,
        DENOFIZ_TEXT_DIRECTION_RIGHT_TO_LEFT
    } DenOfIz_TextDirection;

    // Font becomes font id 0, the default font when null. NumFrames is the number of frames in flight, zero is 3.
    typedef struct DenOfIz_TextRendererDesc
    {
        DenOfIz_LogicalDevice LogicalDevice;
        uint32_t              Width;
        uint32_t              Height;
        uint32_t              NumFrames;
        DenOfIz_Font          Font;
    } DenOfIz_TextRendererDesc;

    // X, Y is the top left of the text. HorizontalCenter centers each line on X, VerticalCenter the whole text on Y. Lines break only at line
    // breaks, there is no wrapping. A zero FontSize is 16, a zero LineHeight sizes each line to the tallest font drawing on it, fallbacks included.
    typedef struct DenOfIz_TextRenderDesc
    {
        float                 FontSize;
        DenOfIz_StringView    Text;
        float                 X;
        float                 Y;
        DenOfIz_Float4        Color;
        float                 LetterSpacing;
        float                 LineHeight;
        bool                  HorizontalCenter;
        bool                  VerticalCenter;
        DenOfIz_TextDirection Direction;
        uint16_t              FontId;
    } DenOfIz_TextRenderDesc;

    DZ_API DenOfIz_TextRenderer DenOfIz_TextRenderer_Create( const DenOfIz_TextRendererDesc *desc );
    DZ_API void                 DenOfIz_TextRenderer_Destroy( DenOfIz_TextRenderer textRenderer );

    DZ_API void DenOfIz_TextRenderer_SetProjectionMatrix( DenOfIz_TextRenderer textRenderer, const DenOfIz_Float4x4 *projectionMatrix );
    // Also resets the projection to an orthographic one in viewport pixels, origin at the top left.
    DZ_API void DenOfIz_TextRenderer_SetViewport( DenOfIz_TextRenderer textRenderer, const DenOfIz_Viewport *viewport );

    DZ_API uint16_t     DenOfIz_TextRenderer_AddFont( DenOfIz_TextRenderer textRenderer, DenOfIz_Font font, uint16_t fontId );
    DZ_API DenOfIz_Font DenOfIz_TextRenderer_GetFont( DenOfIz_TextRenderer textRenderer, uint16_t fontId );
    DZ_API void         DenOfIz_TextRenderer_RemoveFont( DenOfIz_TextRenderer textRenderer, uint16_t fontId );
    // Font ids tried in order, without their own fallbacks, for characters the font lacks, before the system fonts. Replaces the previous list.
    DZ_API void DenOfIz_TextRenderer_SetFontFallbacks( DenOfIz_TextRenderer textRenderer, uint16_t fontId, const DenOfIz_UInt16Array *fallbackIds );

    // Each batch uses the next of NumFrames sets of GPU buffers: the GPU must be done with the batch NumFrames batches back.
    DZ_API void DenOfIz_TextRenderer_BeginBatch( DenOfIz_TextRenderer textRenderer );
    DZ_API void DenOfIz_TextRenderer_AddText( DenOfIz_TextRenderer textRenderer, const DenOfIz_TextRenderDesc *textDesc );
    // Records the batch's draws into the render pass the caller has begun on commandList. Records nothing for an empty batch.
    DZ_API void DenOfIz_TextRenderer_EndBatch( DenOfIz_TextRenderer textRenderer, DenOfIz_CommandList commandList );

    DZ_API DenOfIz_Float2 DenOfIz_TextRenderer_MeasureText( DenOfIz_TextRenderer textRenderer, DenOfIz_StringView text, const DenOfIz_TextRenderDesc *desc );

#ifdef __cplusplus
}
#endif
