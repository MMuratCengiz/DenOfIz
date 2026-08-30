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

#include <clay.h>

#include <chrono>
#include <memory>
#include <vector>
#include "ClayTextCache.h"
#include "DenOfIzGraphics/UI/Clay.h"
#include "DenOfIzGraphicsInternal/UI/IClayContext.h"

namespace DenOfIz
{
    struct ClayContextDesc
    {
        DenOfIz_LogicalDevice LogicalDevice                  = DENOFIZ_NULL_HANDLE;
        uint32_t              Width                          = 0;
        uint32_t              Height                         = 0;
        uint32_t              MaxNumElements                 = 8192;
        uint32_t              MaxNumTextMeasureCacheElements = 16384;
    };

    class ClayContext : public IClayContext
    {
        // Text handed to clay must stay valid for as long as clay may reference it, including
        // exit-transition replay of elements no longer declared. Retained arenas are only reused
        // after TEXT_RETENTION_SECONDS of inactivity so text pointers outlive any transition.
        struct RetainedTextArena
        {
            std::unique_ptr<char[]>               Memory;
            uint32_t                              Capacity = 0;
            uint32_t                              Used     = 0;
            std::chrono::steady_clock::time_point LastUsed;
        };
        static constexpr double TEXT_RETENTION_SECONDS = 2.0;

        std::unique_ptr<ClayTextCache> m_clayText         = nullptr;
        DenOfIz_ClayPointerState       m_pointerState     = DENOFIZ_CLAY_POINTER_STATE_RELEASED;
        DenOfIz_ClayPointerState       m_prevPointerState = DENOFIZ_CLAY_POINTER_STATE_RELEASED;
        DenOfIz_Float2                 m_pointerPosition{ 0, 0 };
        DenOfIz_Float2                 m_scrollDelta{ 0, 0 };
        Clay_Arena                     m_arena;
        Clay_Context                  *m_context;
        std::vector<uint8_t>           m_memory;
        uint16_t                       m_fontId      = 1;
        bool                           m_isDebugMode = false;
        float                          m_dpiScale    = 1.0f;

        mutable std::vector<RetainedTextArena> m_textArenas;
        mutable size_t                         m_currentTextArena  = 0;
        uint32_t                               m_textArenaCapacity = 8192;

        RetainedTextArena &AcquireTextArena( uint32_t minCapacity ) const;
        const char        *AppendRetainedText( const char *chars, uint32_t length ) const;

    public:
        explicit ClayContext( const ClayContextDesc &desc );
        ~ClayContext( ) override;

        void                    OpenElement( const DenOfIz_ClayElementDeclaration *declaration ) const override;
        void                    CloseElement( ) const override;
        void                    Text( const char *text, const DenOfIz_ClayTextDesc *desc ) const override;
        void                    Text( const DenOfIz_StringView &text, const DenOfIz_ClayTextDesc *desc ) const override;
        void                    Texture( ITexture *texture, float width, float height ) const override;
        DenOfIz_ClayDimensions  MeasureText( const char *text, uint16_t fontId, uint16_t fontSize ) const override;
        DenOfIz_ClayDimensions  MeasureText( const DenOfIz_StringView &text, uint16_t fontId, uint16_t fontSize ) const override;
        DenOfIz_ClayDimensions  MeasureTextRaw( const DenOfIz_StringView &text, uint16_t fontId, uint16_t fontSize ) const;
        uint32_t                HashString( const char *str, uint32_t index = 0, uint32_t baseId = 0 ) const override;
        uint32_t                HashString( const DenOfIz_StringView &str, uint32_t index = 0, uint32_t baseId = 0 ) const override;
        bool                    PointerOver( uint32_t id ) const override;
        DenOfIz_ClayBoundingBox GetElementBoundingBox( uint32_t id ) const override;
        DenOfIz_ClayDimensions  GetViewportSize( ) const override;
        bool                    IsDebugModeEnabled( ) const override;
        float                   GetDpiScale( ) const override;

        float PointsToPixels( float points ) const override;
        float PixelsToPoints( float pixels ) const override;

        void           BeginLayout( ) const;
        uint32_t       GetOpenElementId( ) const;
        DenOfIz_Float2 GetScrollOffset( ) const;
        void           SetViewportSize( float width, float height ) const;
        void           SetDpiScale( float dpiScale );
        void           UpdatePointerPosition( DenOfIz_Float2 position, DenOfIz_ClayPointerState state );
        void           SetPointerState( DenOfIz_Float2 position, DenOfIz_ClayPointerState state );
        void           UpdateScrollContainers( bool enableDragScrolling, DenOfIz_Float2 scrollDelta, float deltaTime );
        void           SetDebugModeEnabled( bool enabled );

        void  AddFont( uint16_t fontId, Font *font ) const;
        void  RemoveFont( uint16_t fontId ) const;
        Font *GetFont( uint16_t fontId ) const;

        ClayTextCache *GetClayText( ) const;

        DenOfIz_ClayScrollContainerData GetScrollContainerData( uint32_t id ) const;
        void                            SetScrollPosition( uint32_t id, DenOfIz_Float2 position ) const;

        bool ElementClicked( uint32_t id ) const;
        bool ElementPressed( uint32_t id ) const;

        float    GetCursorOffsetAtIndex( const DenOfIz_StringView &text, uint32_t charIndex, uint16_t fontId, uint16_t fontSize ) const;
        uint32_t GetCharIndexAtOffset( const DenOfIz_StringView &text, float pixelOffset, uint16_t fontId, uint16_t fontSize ) const;

        Clay_RenderCommandArray        EndLayoutAndGetCommands( float deltaTime = 0.016f ) const;
        Clay_LayoutDirection           ConvertLayoutDirection( DenOfIz_ClayLayoutDirection dir ) const;
        Clay_LayoutAlignmentX          ConvertAlignmentX( DenOfIz_ClayAlignmentX align ) const;
        Clay_LayoutAlignmentY          ConvertAlignmentY( DenOfIz_ClayAlignmentY align ) const;
        Clay__SizingType               ConvertSizingType( DenOfIz_ClaySizingType type ) const;
        Clay_SizingAxis                ConvertSizingAxis( const DenOfIz_ClaySizingAxis *axis, bool enableDpiScaling ) const;
        Clay_Sizing                    ConvertSizing( const DenOfIz_ClaySizing *sizing, bool enableDpiScaling ) const;
        Clay_Padding                   ConvertPadding( const DenOfIz_ClayPadding *padding, bool enableDpiScaling ) const;
        Clay_ChildAlignment            ConvertChildAlignment( const DenOfIz_ClayChildAlignment *alignment ) const;
        Clay_LayoutConfig              ConvertLayoutConfig( const DenOfIz_ClayLayoutDesc *config, bool enableDpiScaling ) const;
        Clay_Color                     ConvertColor( const DenOfIz_ClayColor *color ) const;
        Clay_CornerRadius              ConvertBorderRadius( const DenOfIz_ClayBorderRadius *radius, bool enableDpiScaling ) const;
        Clay_BorderWidth               ConvertBorderWidth( const DenOfIz_ClayBorderWidth *width, bool enableDpiScaling ) const;
        Clay_BorderElementConfig       ConvertBorderConfig( const DenOfIz_ClayBorderDesc *config, bool enableDpiScaling ) const;
        Clay_ImageElementConfig        ConvertImageConfig( const DenOfIz_ClayImageDesc *config ) const;
        Clay_AspectRatioElementConfig  ConvertAspectRatioConfig( const DenOfIz_ClayElementDeclaration *declaration ) const;
        Clay_FloatingAttachPointType   ConvertFloatingAttachPoint( DenOfIz_ClayFloatingAttachPoint point ) const;
        Clay_FloatingAttachToElement   ConvertFloatingAttachTo( DenOfIz_ClayFloatingAttachTo attachTo ) const;
        Clay_FloatingElementConfig     ConvertFloatingConfig( const DenOfIz_ClayFloatingDesc *config, bool enableDpiScaling ) const;
        Clay_ClipElementConfig         ConvertClipConfig( const DenOfIz_ClayScrollDesc *scroll, const DenOfIz_ClayClipDesc *clip, bool enableDpiScaling ) const;
        Clay_TransitionElementConfig   ConvertTransitionConfig( const DenOfIz_ClayTransitionDesc *config, bool enableDpiScaling ) const;
        Clay_CustomElementConfig       ConvertCustomConfig( const DenOfIz_ClayCustomDesc *config ) const;
        Clay_TextElementConfigWrapMode ConvertTextWrapMode( DenOfIz_ClayTextWrapMode mode ) const;
        Clay_TextAlignment             ConvertTextAlignment( DenOfIz_ClayTextAlignment align ) const;
        Clay_TextElementConfig         ConvertTextConfig( const DenOfIz_ClayTextDesc *config ) const;
        DenOfIz_ClayRenderCommandType  ConvertRenderCommandType( Clay_RenderCommandType type ) const;

    private:
        static Clay_Dimensions MeasureTextCallback( Clay_StringSlice text, Clay_TextElementConfig *config, void *userData );
    };

} // namespace DenOfIz
