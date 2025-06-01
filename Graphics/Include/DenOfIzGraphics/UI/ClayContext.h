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

#include <DenOfIzGraphics/UI/ClayData.h>
#include <DenOfIzGraphics/UI/ClayTextCache.h>
#include <DenOfIzGraphics/UI/UIShapes.h>
#include <functional>
#include <memory>
#include <vector>

namespace DenOfIz
{
    struct ClayContextDesc
    {
        ILogicalDevice *LogicalDevice                  = nullptr;
        uint32_t        Width                          = 0;
        uint32_t        Height                         = 0;
        uint32_t        MaxNumElements                 = 8192;
        uint32_t        MaxNumTextMeasureCacheElements = 16384;
    };

    class IRenderBatch
    {
    public:
        virtual ~IRenderBatch( )                                                                                      = default;
        virtual void     AddVertices( const InteropArray<UIVertex> &vertices, const InteropArray<uint32_t> &indices ) = 0;
        virtual uint32_t GetCurrentVertexOffset( ) const                                                              = 0;
    };

    /// Not public API, intended for internal use within Clay.h and ClayRenderer
    class ClayContext
    {
        std::unique_ptr<ClayTextCache> m_clayText     = nullptr;
        ClayPointerState               m_pointerState = ClayPointerState::Released;
        Float_2                        m_pointerPosition{ 0, 0 };
        Float_2                        m_scrollDelta{ 0, 0 };
        Clay_Arena                     m_arena;
        Clay_Context                  *m_context;
        std::vector<uint8_t>           m_memory;
        uint16_t                       m_fontId      = 1;
        bool                           m_isDebugMode = false;
        float                          m_dpiScale    = 1.0f;

    public:
        explicit ClayContext( const ClayContextDesc &desc );
        ~ClayContext( );

        void           BeginLayout( ) const;
        void           SetViewportSize( float width, float height ) const;
        ClayDimensions GetViewportSize( ) const;
        void           SetDpiScale( float dpiScale );
        void           SetPointerState( Float_2 position, ClayPointerState state );
        void           UpdateScrollContainers( bool enableDragScrolling, Float_2 scrollDelta, float deltaTime );
        void           SetDebugModeEnabled( bool enabled );
        bool           IsDebugModeEnabled( ) const;

        void OpenElement( const ClayElementDeclaration &declaration ) const;
        void CloseElement( ) const;

        void  AddFont( uint16_t fontId, Font *font ) const;
        void  RemoveFont( uint16_t fontId ) const;
        Font *GetFont( uint16_t fontId ) const;
        void  Text( const InteropString &text, const ClayTextDesc &desc ) const;

        uint32_t        HashString( const InteropString &str, uint32_t index = 0, uint32_t baseId = 0 ) const;
        bool            PointerOver( uint32_t id ) const;
        ClayBoundingBox GetElementBoundingBox( uint32_t id ) const;

        ClayTextCache *GetClayText( ) const;
        ClayDimensions MeasureText( const InteropString &text, uint16_t fontId, uint16_t fontSize ) const;

        Clay_RenderCommandArray EndLayoutAndGetCommands( float deltaTime = 0.016f ) const;

        Clay_LayoutDirection           ConvertLayoutDirection( ClayLayoutDirection dir ) const;
        Clay_LayoutAlignmentX          ConvertAlignmentX( ClayAlignmentX align ) const;
        Clay_LayoutAlignmentY          ConvertAlignmentY( ClayAlignmentY align ) const;
        Clay__SizingType               ConvertSizingType( ClaySizingType type ) const;
        Clay_SizingAxis                ConvertSizingAxis( const ClaySizingAxis &axis ) const;
        Clay_Sizing                    ConvertSizing( const ClaySizing &sizing ) const;
        Clay_Padding                   ConvertPadding( const ClayPadding &padding ) const;
        Clay_ChildAlignment            ConvertChildAlignment( const ClayChildAlignment &alignment ) const;
        Clay_LayoutConfig              ConvertLayoutConfig( const ClayLayoutDesc &config ) const;
        Clay_Color                     ConvertColor( const ClayColor &color ) const;
        Clay_CornerRadius              ConvertCornerRadius( const ClayCornerRadius &radius ) const;
        Clay_BorderWidth               ConvertBorderWidth( const ClayBorderWidth &width ) const;
        Clay_BorderElementConfig       ConvertBorderConfig( const ClayBorderDesc &config ) const;
        Clay_ImageElementConfig        ConvertImageConfig( const ClayImageDesc &config ) const;
        Clay_FloatingAttachPointType   ConvertFloatingAttachPoint( ClayFloatingAttachPoint point ) const;
        Clay_FloatingAttachToElement   ConvertFloatingAttachTo( ClayFloatingAttachTo attachTo ) const;
        Clay_FloatingElementConfig     ConvertFloatingConfig( const ClayFloatingDesc &config ) const;
        Clay_ScrollElementConfig       ConvertScrollConfig( const ClayScrollDesc &config ) const;
        Clay_CustomElementConfig       ConvertCustomConfig( const ClayCustomDesc &config ) const;
        Clay_TextElementConfigWrapMode ConvertTextWrapMode( ClayTextWrapMode mode ) const;
        Clay_TextAlignment             ConvertTextAlignment( ClayTextAlignment align ) const;
        Clay_TextElementConfig         ConvertTextConfig( const ClayTextDesc &config ) const;
        ClayRenderCommandType          ConvertRenderCommandType( Clay_RenderCommandType type ) const;

    private:
        static Clay_Dimensions MeasureTextCallback( Clay_StringSlice text, Clay_TextElementConfig *config, void *userData );
    };

} // namespace DenOfIz
