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

#include <DenOfIzGraphics/Assets/Font/TextRenderer.h>
#include <DenOfIzGraphics/UI/ClayData.h>
#include <DenOfIzGraphics/UI/UIShapes.h>
#include <functional>
#include <memory>
#include <vector>

namespace DenOfIz
{
    struct DZ_API ClayContextDesc
    {
        ILogicalDevice *LogicalDevice                  = nullptr;
        uint32_t        Width                          = 0;
        uint32_t        Height                         = 0;
        uint32_t        MaxNumElements                 = 8192;
        uint32_t        MaxNumTextMeasureCacheElements = 16384;
    };

    class DZ_API IRenderBatch
    {
    public:
        virtual ~IRenderBatch( )                                                                                      = default;
        virtual void     AddVertices( const InteropArray<UIVertex> &vertices, const InteropArray<uint32_t> &indices ) = 0;
        virtual uint32_t GetCurrentVertexOffset( ) const                                                              = 0;
    };

    class DZ_API ClayContext
    {
        std::unique_ptr<TextRenderer> m_textRenderer;
        ClayPointerState              m_pointerState = ClayPointerState::Released;
        Float_2                       m_pointerPosition{ 0, 0 };
        Float_2                       m_scrollDelta{ 0, 0 };
        Clay_Arena                    m_arena;
        Clay_Context                 *m_context;
        std::vector<uint8_t>          m_memory;
        uint16_t                      m_fontId      = 1;
        bool                          m_isDebugMode = false;
        float                         m_dpiScale    = 1.0f;

    public:
        DZ_API explicit ClayContext( const ClayContextDesc &desc );
        DZ_API ~ClayContext( );

        DZ_API void           BeginLayout( );
        DZ_API void           SetViewportSize( float width, float height );
        DZ_API ClayDimensions GetViewportSize( ) const;
        DZ_API void           SetDpiScale( float dpiScale );
        DZ_API void           SetPointerState( Float_2 position, ClayPointerState state );
        DZ_API void           UpdateScrollContainers( bool enableDragScrolling, Float_2 scrollDelta, float deltaTime );
        DZ_API void           SetDebugModeEnabled( bool enabled );
        DZ_API bool           IsDebugModeEnabled( ) const;

        DZ_API void OpenElement( const ClayElementDeclaration &declaration );
        DZ_API void CloseElement( );
        DZ_API void Text( const InteropString &text, const ClayTextDesc &desc );

        DZ_API uint32_t        HashString( const InteropString &str, uint32_t index = 0, uint32_t baseId = 0 ) const;
        DZ_API bool            PointerOver( uint32_t id ) const;
        DZ_API ClayBoundingBox GetElementBoundingBox( uint32_t id ) const;

        DZ_API uint16_t       AddFont( Font *font );
        DZ_API ClayDimensions MeasureText( const InteropString &text, uint16_t fontId, uint16_t fontSize ) const;

        DZ_API void RenderTextToVertices( const InteropString &text, const ClayTextDesc &desc, const ClayBoundingBox &bounds, InteropArray<UIVertex> *outVertices,
                                          InteropArray<uint32_t> *outIndices ) const;

        DZ_API Clay_RenderCommandArray EndLayoutAndGetCommands( float deltaTime = 0.016f );

        DZ_API Clay_LayoutDirection           ConvertLayoutDirection( ClayLayoutDirection dir ) const;
        DZ_API Clay_LayoutAlignmentX          ConvertAlignmentX( ClayAlignmentX align ) const;
        DZ_API Clay_LayoutAlignmentY          ConvertAlignmentY( ClayAlignmentY align ) const;
        DZ_API Clay__SizingType               ConvertSizingType( ClaySizingType type ) const;
        DZ_API Clay_SizingAxis                ConvertSizingAxis( const ClaySizingAxis &axis ) const;
        DZ_API Clay_Sizing                    ConvertSizing( const ClaySizing &sizing ) const;
        DZ_API Clay_Padding                   ConvertPadding( const ClayPadding &padding ) const;
        DZ_API Clay_ChildAlignment            ConvertChildAlignment( const ClayChildAlignment &alignment ) const;
        DZ_API Clay_LayoutConfig              ConvertLayoutConfig( const ClayLayoutDesc &config ) const;
        DZ_API Clay_Color                     ConvertColor( const ClayColor &color ) const;
        DZ_API Clay_CornerRadius              ConvertCornerRadius( const ClayCornerRadius &radius ) const;
        DZ_API Clay_BorderWidth               ConvertBorderWidth( const ClayBorderWidth &width ) const;
        DZ_API Clay_BorderElementConfig       ConvertBorderConfig( const ClayBorderDesc &config ) const;
        DZ_API Clay_ImageElementConfig        ConvertImageConfig( const ClayImageDesc &config ) const;
        DZ_API Clay_FloatingAttachPointType   ConvertFloatingAttachPoint( ClayFloatingAttachPoint point ) const;
        DZ_API Clay_FloatingAttachToElement   ConvertFloatingAttachTo( ClayFloatingAttachTo attachTo ) const;
        DZ_API Clay_FloatingElementConfig     ConvertFloatingConfig( const ClayFloatingDesc &config ) const;
        DZ_API Clay_ScrollElementConfig       ConvertScrollConfig( const ClayScrollDesc &config ) const;
        DZ_API Clay_CustomElementConfig       ConvertCustomConfig( const ClayCustomDesc &config ) const;
        DZ_API Clay_TextElementConfigWrapMode ConvertTextWrapMode( ClayTextWrapMode mode ) const;
        DZ_API Clay_TextAlignment             ConvertTextAlignment( ClayTextAlignment align ) const;
        DZ_API Clay_TextElementConfig         ConvertTextConfig( const ClayTextDesc &config ) const;
        DZ_API ClayRenderCommandType          ConvertRenderCommandType( Clay_RenderCommandType type ) const;

    private:
        static Clay_Dimensions MeasureTextCallback( Clay_StringSlice text, Clay_TextElementConfig *config, void *userData );
    };

} // namespace DenOfIz
