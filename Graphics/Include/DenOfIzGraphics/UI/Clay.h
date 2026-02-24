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

/*
 * =============================================================================
 * CLAY UI SYSTEM - LAYOUT MANAGEMENT GUIDE
 * =============================================================================
 *
 * OVERVIEW
 * --------
 * Clay is a declarative UI layout system. Each frame you declare your UI tree
 * using OpenElement/CloseElement pairs, and Clay calculates positions and sizes.
 *
 * DPI SCALING
 * -----------
 * All sizes (padding, gaps, fixed sizes, font sizes) are specified in logical
 * points and automatically converted to physical pixels based on DPI scale.
 * This is handled transparently - you do not need to manually scale values.
 *
 * - SetDpiScale() is called automatically on creation using primary display DPI
 * - MeasureText() returns pixel dimensions matching actual rendered output
 * - GetElementBoundingBox() returns pixel coordinates for screen positioning
 * - Texture elements are NOT DPI-scaled (rendered at native resolution)
 *
 * SIZING TYPES
 * ------------
 * - FIT:     Shrink-wrap to content size, with optional min/max constraints
 * - GROW:    Expand to fill available space, sharing with other GROW siblings
 * - PERCENT: Size as percentage (0.0-1.0) of parent's available space
 * - FIXED:   Exact size in logical points (DPI-scaled automatically)
 *
 * LAYOUT FLOW
 * -----------
 * 1. Call BeginLayout()
 * 2. Declare UI tree with OpenElement()/CloseElement() and Text()
 * 3. Call EndLayout() with your command list, or EndAndRenderLayout() for automatic rendering
 *
 * ELEMENT MEASUREMENT
 * -------------------
 * - MeasureText(): Get pixel dimensions of text BEFORE layout (for planning)
 * - GetElementBoundingBox(): Get pixel bounds of element AFTER layout
 * - GetScrollContainerData(): Get scroll position and content dimensions
 *
 * TEXT CURSOR POSITIONING (for text input implementation)
 * -------------------------------------------------------
 * - GetCursorOffsetAtIndex(): Get pixel X offset for cursor at character index
 * - GetCharIndexAtOffset(): Get character index at pixel X offset (hit testing)
 * These functions enable implementing text fields without building full widgets.
 *
 * POINTER INTERACTION
 * -------------------
 * - PointerOver(): Check if pointer is over element (any frame)
 * - ElementClicked(): Check if element was clicked THIS frame (released over)
 * - ElementPressed(): Check if element is currently being pressed
 *
 * =============================================================================
 */

#pragma once

#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Input/Event.h"
#include "DenOfIzGraphics/Support/ResourceTracking.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_Clay )
    DENOFIZ_DEFINE_HANDLE( DenOfIz_Font )

    typedef enum DenOfIz_ClayPointerState
    {
        DENOFIZ_CLAY_POINTER_STATE_PRESSED,
        DENOFIZ_CLAY_POINTER_STATE_RELEASED
    } DenOfIz_ClayPointerState;

    typedef enum DenOfIz_ClayLayoutDirection
    {
        DENOFIZ_CLAY_LAYOUT_DIRECTION_LEFT_TO_RIGHT,
        DENOFIZ_CLAY_LAYOUT_DIRECTION_TOP_TO_BOTTOM
    } DenOfIz_ClayLayoutDirection;

    typedef enum DenOfIz_ClayAlignmentX
    {
        DENOFIZ_CLAY_ALIGNMENT_X_LEFT,
        DENOFIZ_CLAY_ALIGNMENT_X_RIGHT,
        DENOFIZ_CLAY_ALIGNMENT_X_CENTER
    } DenOfIz_ClayAlignmentX;

    typedef enum DenOfIz_ClayAlignmentY
    {
        DENOFIZ_CLAY_ALIGNMENT_Y_TOP,
        DENOFIZ_CLAY_ALIGNMENT_Y_BOTTOM,
        DENOFIZ_CLAY_ALIGNMENT_Y_CENTER
    } DenOfIz_ClayAlignmentY;

    typedef enum DenOfIz_ClaySizingType
    {
        DENOFIZ_CLAY_SIZING_TYPE_FIT,
        DENOFIZ_CLAY_SIZING_TYPE_GROW,
        DENOFIZ_CLAY_SIZING_TYPE_PERCENT,
        DENOFIZ_CLAY_SIZING_TYPE_FIXED
    } DenOfIz_ClaySizingType;

    typedef enum DenOfIz_ClayTextWrapMode
    {
        DENOFIZ_CLAY_TEXT_WRAP_MODE_WORDS,
        DENOFIZ_CLAY_TEXT_WRAP_MODE_NEWLINES,
        DENOFIZ_CLAY_TEXT_WRAP_MODE_NONE
    } DenOfIz_ClayTextWrapMode;

    typedef enum DenOfIz_ClayTextAlignment
    {
        DENOFIZ_CLAY_TEXT_ALIGNMENT_LEFT,
        DENOFIZ_CLAY_TEXT_ALIGNMENT_CENTER,
        DENOFIZ_CLAY_TEXT_ALIGNMENT_RIGHT
    } DenOfIz_ClayTextAlignment;

    typedef enum DenOfIz_ClayFloatingAttachPoint
    {
        DENOFIZ_CLAY_FLOATING_ATTACH_POINT_LEFT_TOP,
        DENOFIZ_CLAY_FLOATING_ATTACH_POINT_LEFT_CENTER,
        DENOFIZ_CLAY_FLOATING_ATTACH_POINT_LEFT_BOTTOM,
        DENOFIZ_CLAY_FLOATING_ATTACH_POINT_CENTER_TOP,
        DENOFIZ_CLAY_FLOATING_ATTACH_POINT_CENTER_CENTER,
        DENOFIZ_CLAY_FLOATING_ATTACH_POINT_CENTER_BOTTOM,
        DENOFIZ_CLAY_FLOATING_ATTACH_POINT_RIGHT_TOP,
        DENOFIZ_CLAY_FLOATING_ATTACH_POINT_RIGHT_CENTER,
        DENOFIZ_CLAY_FLOATING_ATTACH_POINT_RIGHT_BOTTOM
    } DenOfIz_ClayFloatingAttachPoint;

    typedef enum DenOfIz_ClayFloatingAttachTo
    {
        DENOFIZ_CLAY_FLOATING_ATTACH_TO_NONE,
        DENOFIZ_CLAY_FLOATING_ATTACH_TO_PARENT,
        DENOFIZ_CLAY_FLOATING_ATTACH_TO_ELEMENT_WITH_ID,
        DENOFIZ_CLAY_FLOATING_ATTACH_TO_ROOT
    } DenOfIz_ClayFloatingAttachTo;

    typedef enum DenOfIz_ClayRenderCommandType
    {
        DENOFIZ_CLAY_RENDER_COMMAND_TYPE_NONE,
        DENOFIZ_CLAY_RENDER_COMMAND_TYPE_RECTANGLE,
        DENOFIZ_CLAY_RENDER_COMMAND_TYPE_BORDER,
        DENOFIZ_CLAY_RENDER_COMMAND_TYPE_TEXT,
        DENOFIZ_CLAY_RENDER_COMMAND_TYPE_IMAGE,
        DENOFIZ_CLAY_RENDER_COMMAND_TYPE_SCISSOR_START,
        DENOFIZ_CLAY_RENDER_COMMAND_TYPE_SCISSOR_END,
        DENOFIZ_CLAY_RENDER_COMMAND_TYPE_CUSTOM
    } DenOfIz_ClayRenderCommandType;

    typedef struct DenOfIz_ClayDimensions
    {
        float Width;
        float Height;
    } DenOfIz_ClayDimensions;

    typedef struct DenOfIz_ClayColor
    {
        uint32_t R;
        uint32_t G;
        uint32_t B;
        uint32_t A;
    } DenOfIz_ClayColor;

    typedef struct DenOfIz_ClayBoundingBox
    {
        float X;
        float Y;
        float Width;
        float Height;
    } DenOfIz_ClayBoundingBox;

    typedef struct DenOfIz_ClayBorderRadius
    {
        float TopLeft;
        float TopRight;
        float BottomLeft;
        float BottomRight;
    } DenOfIz_ClayBorderRadius;

    typedef DenOfIz_ClayBorderRadius DenOfIz_ClayCornerRadius;

    typedef struct DenOfIz_ClayBorderWidth
    {
        float Left;
        float Right;
        float Top;
        float Bottom;
        float BetweenChildren;
    } DenOfIz_ClayBorderWidth;

    typedef struct DenOfIz_ClayPadding
    {
        float Left;
        float Right;
        float Top;
        float Bottom;
    } DenOfIz_ClayPadding;

    typedef struct DenOfIz_ClaySizingAxis_MinMaxFloats
    {
        float Min;
        float Max;
    } DenOfIz_ClaySizingAxis_MinMaxFloats;

    typedef struct DenOfIz_ClaySizingAxis_Union
    {
        DenOfIz_ClaySizingAxis_MinMaxFloats MinMaxFloats;
        float                               Percent;
    } DenOfIz_ClaySizingAxis_Union;

    typedef struct DenOfIz_ClaySizingAxis
    {
        DenOfIz_ClaySizingType       Type;
        DenOfIz_ClaySizingAxis_Union Size;
    } DenOfIz_ClaySizingAxis;

    typedef struct DenOfIz_ClaySizing
    {
        DenOfIz_ClaySizingAxis Width;
        DenOfIz_ClaySizingAxis Height;
    } DenOfIz_ClaySizing;

    typedef struct DenOfIz_ClayChildAlignment
    {
        DenOfIz_ClayAlignmentX X;
        DenOfIz_ClayAlignmentY Y;
    } DenOfIz_ClayChildAlignment;

    typedef struct DenOfIz_ClayLayoutDesc
    {
        DenOfIz_ClaySizing          Sizing;
        DenOfIz_ClayPadding         Padding;
        uint16_t                    ChildGap;
        DenOfIz_ClayChildAlignment  ChildAlignment;
        DenOfIz_ClayLayoutDirection LayoutDirection;
    } DenOfIz_ClayLayoutDesc;

    typedef struct DenOfIz_ClayTextDesc
    {
        DenOfIz_ClayColor         TextColor;
        uint16_t                  FontId;
        uint16_t                  FontSize;
        uint16_t                  LetterSpacing;
        uint16_t                  LineHeight;
        DenOfIz_ClayTextWrapMode  WrapMode;
        DenOfIz_ClayTextAlignment TextAlignment;
        bool                      HashStringContents;
    } DenOfIz_ClayTextDesc;

    typedef struct DenOfIz_ClayImageDesc
    {
        Byte                  *ImageData;
        DenOfIz_ClayDimensions SourceDimensions;
    } DenOfIz_ClayImageDesc;

    typedef struct DenOfIz_ClayFloatingDesc
    {
        DenOfIz_Float2                  Offset;
        DenOfIz_ClayDimensions          Expand;
        float                           ZIndex;
        uint32_t                        ParentId;
        DenOfIz_ClayFloatingAttachPoint ElementAttachPoint;
        DenOfIz_ClayFloatingAttachPoint ParentAttachPoint;
        DenOfIz_ClayFloatingAttachTo    AttachTo;
    } DenOfIz_ClayFloatingDesc;

    typedef struct DenOfIz_ClayBorderDesc
    {
        DenOfIz_ClayColor       Color;
        DenOfIz_ClayBorderWidth Width;
    } DenOfIz_ClayBorderDesc;

    typedef struct DenOfIz_ClayScrollDesc
    {
        bool Horizontal;
        bool Vertical;
    } DenOfIz_ClayScrollDesc;

    typedef struct DenOfIz_ClayCustomDesc
    {
        void *CustomData;
    } DenOfIz_ClayCustomDesc;

    typedef struct DenOfIz_ClayElementDeclaration
    {
        uint32_t                 Id;
        DenOfIz_ClayLayoutDesc   Layout;
        DenOfIz_ClayColor        BackgroundColor;
        DenOfIz_ClayBorderRadius BorderRadius;
        DenOfIz_ClayImageDesc    Image;
        DenOfIz_ClayFloatingDesc Floating;
        DenOfIz_ClayCustomDesc   Custom;
        DenOfIz_ClayScrollDesc   Scroll;
        DenOfIz_ClayBorderDesc   Border;
        void                    *UserData;
    } DenOfIz_ClayElementDeclaration;

    typedef struct DenOfIz_ClayDesc
    {
        DenOfIz_LogicalDevice    LogicalDevice;
        DenOfIz_ResourceTracking ResourceTracking;
        DenOfIz_Format           RenderTargetFormat;
        uint32_t                 NumFrames;
        uint32_t                 MaxNumQuads;
        uint32_t                 MaxNumMaterials;
        uint32_t                 MaxNumFonts;
        uint32_t                 Width;
        uint32_t                 Height;
        uint32_t                 MaxNumElements;
        uint32_t                 MaxNumTextMeasureCacheElements;
    } DenOfIz_ClayDesc;

    typedef struct DenOfIz_ClayRenderResult
    {
        DenOfIz_Texture   Texture;
        DenOfIz_Semaphore Semaphore;
    } DenOfIz_ClayRenderResult;

    DZ_API DenOfIz_ClaySizingAxis DenOfIz_ClaySizingAxis_Fit( float min, float max );
    DZ_API DenOfIz_ClaySizingAxis DenOfIz_ClaySizingAxis_Grow( float min, float max );
    DZ_API DenOfIz_ClaySizingAxis DenOfIz_ClaySizingAxis_Fixed( float size );
    DZ_API DenOfIz_ClaySizingAxis DenOfIz_ClaySizingAxis_Percent( float percent );

    DZ_API DenOfIz_ClayColor        DenOfIz_ClayColor_Create( uint32_t r, uint32_t g, uint32_t b, uint32_t a );
    DZ_API DenOfIz_Float4           DenOfIz_ClayColor_ToFloat4( const DenOfIz_ClayColor *color );
    DZ_API DenOfIz_ClayBorderRadius DenOfIz_ClayBorderRadius_Create( float topLeft, float topRight, float bottomLeft, float bottomRight );
    DZ_API DenOfIz_ClayBorderRadius DenOfIz_ClayBorderRadius_CreateUniform( float radius );
    DZ_API DenOfIz_ClayBorderWidth  DenOfIz_ClayBorderWidth_Create( float left, float right, float top, float bottom, float betweenChildren );
    DZ_API DenOfIz_ClayBorderWidth  DenOfIz_ClayBorderWidth_CreateUniform( float width );
    DZ_API DenOfIz_ClayPadding      DenOfIz_ClayPadding_Create( float left, float right, float top, float bottom );
    DZ_API DenOfIz_ClayPadding      DenOfIz_ClayPadding_CreateUniform( float padding );

    DZ_API DenOfIz_ClayElementDeclaration DenOfIz_ClayElementDeclaration_Default( );
    DZ_API DenOfIz_ClayTextDesc           DenOfIz_ClayTextDesc_Default( );

    DZ_API DenOfIz_Clay DenOfIz_Clay_Create( const DenOfIz_ClayDesc *desc );
    DZ_API void         DenOfIz_Clay_Destroy( DenOfIz_Clay clay );

    DZ_API void DenOfIz_Clay_SetViewportSize( DenOfIz_Clay clay, float width, float height );
    DZ_API void DenOfIz_Clay_GetViewportSize( DenOfIz_Clay clay, DenOfIz_ClayDimensions *outSize );
    DZ_API void DenOfIz_Clay_SetDpiScale( DenOfIz_Clay clay, float dpiScale );
    DZ_API void DenOfIz_Clay_SetPointerState( DenOfIz_Clay clay, DenOfIz_Float2 position, DenOfIz_ClayPointerState state );
    DZ_API void DenOfIz_Clay_UpdateScrollContainers( DenOfIz_Clay clay, bool enableDragScrolling, DenOfIz_Float2 scrollDelta, float deltaTime );
    DZ_API void DenOfIz_Clay_SetDebugModeEnabled( DenOfIz_Clay clay, bool enabled );
    DZ_API bool DenOfIz_Clay_IsDebugModeEnabled( DenOfIz_Clay clay );

    DZ_API void DenOfIz_Clay_BeginLayout( DenOfIz_Clay clay );
    DZ_API void DenOfIz_Clay_EndLayout( DenOfIz_Clay clay, uint32_t frameIndex, float deltaTime, DenOfIz_CommandList commandList );
    DZ_API void DenOfIz_Clay_EndAndRenderLayout( DenOfIz_Clay clay, uint32_t frameIndex, float deltaTime, DenOfIz_ClayRenderResult *outResult );

    DZ_API void DenOfIz_Clay_HandleEvent( DenOfIz_Clay clay, const DenOfIz_Event *ev );

    DZ_API void DenOfIz_Clay_OpenElement( DenOfIz_Clay clay, const DenOfIz_ClayElementDeclaration *declaration );
    DZ_API void DenOfIz_Clay_CloseElement( DenOfIz_Clay clay );

    DZ_API void DenOfIz_Clay_Text( DenOfIz_Clay clay, DenOfIz_StringView text, const DenOfIz_ClayTextDesc *desc );
    DZ_API void DenOfIz_Clay_Texture( DenOfIz_Clay clay, DenOfIz_Texture texture, float width, float height );

    DZ_API uint32_t DenOfIz_Clay_HashString( DenOfIz_Clay clay, DenOfIz_StringView str, uint32_t index, uint32_t baseId );
    DZ_API bool     DenOfIz_Clay_PointerOver( DenOfIz_Clay clay, uint32_t id );
    DZ_API void     DenOfIz_Clay_GetElementBoundingBox( DenOfIz_Clay clay, uint32_t id, DenOfIz_ClayBoundingBox *outBox );
    DZ_API void     DenOfIz_Clay_MeasureText( DenOfIz_Clay clay, DenOfIz_StringView text, uint16_t fontId, uint16_t fontSize, DenOfIz_ClayDimensions *outDimensions );

    DZ_API void DenOfIz_Clay_AddFont( DenOfIz_Clay clay, uint16_t fontId, DenOfIz_Font font );
    DZ_API void DenOfIz_Clay_RemoveFont( DenOfIz_Clay clay, uint16_t fontId );

    typedef struct DenOfIz_ClayScrollContainerData
    {
        DenOfIz_Float2         ScrollPosition;
        DenOfIz_ClayDimensions ContainerSize;
        DenOfIz_ClayDimensions ContentSize;
        bool                   Found;
    } DenOfIz_ClayScrollContainerData;

    DZ_API void DenOfIz_Clay_GetScrollContainerData( DenOfIz_Clay clay, uint32_t id, DenOfIz_ClayScrollContainerData *outData );
    DZ_API void DenOfIz_Clay_SetScrollPosition( DenOfIz_Clay clay, uint32_t id, DenOfIz_Float2 position );

    DZ_API bool DenOfIz_Clay_ElementClicked( DenOfIz_Clay clay, uint32_t id );
    DZ_API bool DenOfIz_Clay_ElementPressed( DenOfIz_Clay clay, uint32_t id );

    DZ_API float    DenOfIz_Clay_GetCursorOffsetAtIndex( DenOfIz_Clay clay, DenOfIz_StringView text, uint32_t charIndex, uint16_t fontId, uint16_t fontSize );
    DZ_API uint32_t DenOfIz_Clay_GetCharIndexAtOffset( DenOfIz_Clay clay, DenOfIz_StringView text, float pixelOffset, uint16_t fontId, uint16_t fontSize );

    DZ_API float DenOfIz_Clay_GetDpiScale( DenOfIz_Clay clay );
    DZ_API float DenOfIz_Clay_PointsToPixels( DenOfIz_Clay clay, float points );
    DZ_API float DenOfIz_Clay_PixelsToPoints( DenOfIz_Clay clay, float pixels );

#ifdef __cplusplus
}
#endif
