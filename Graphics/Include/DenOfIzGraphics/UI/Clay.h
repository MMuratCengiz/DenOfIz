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

#include <DenOfIzGraphics/Assets/Font/TextRenderer.h>
#include <functional>
#include <memory>
#include "ClayRenderer.h"
#include "DenOfIzGraphics/Input/Event.h"
#include "DenOfIzGraphics/Utilities/Time.h"

namespace DenOfIz
{
    struct DZ_API ClayDesc
    {
        ILogicalDevice *LogicalDevice                  = nullptr;
        Format          RenderTargetFormat             = Format::B8G8R8A8Unorm;
        uint32_t        NumFrames                      = 3;
        uint32_t        MaxNumQuads                    = 2048;
        uint32_t        MaxNumMaterials                = 128;
        uint32_t        Width                          = 0;
        uint32_t        Height                         = 0;
        uint32_t        MaxNumElements                 = 8192;
        uint32_t        MaxNumTextMeasureCacheElements = 16384; // Maybe remove
    };

    struct DZ_API AddFontDesc
    {

        Font    *Font     = nullptr;
        uint32_t FontSize = 16;
    };

    class Clay
    {
        Time                          m_time;
        std::unique_ptr<TextRenderer> m_textRenderer;
        std::unique_ptr<ClayRenderer> m_renderer;
        ClayPointerState              m_pointerState = ClayPointerState::Released;
        Float_2                       m_pointerPosition{ 0, 0 };
        Float_2                       m_scrollDelta{ 0, 0 };
        Clay_Arena                    m_arena;
        Clay_Context                 *m_context;
        std::vector<uint8_t>          m_memory;
        uint16_t                      m_fontId      = 1;
        bool                          m_isDebugMode = false;

    public:
        DZ_API explicit Clay( const ClayDesc &desc );
        DZ_API ~Clay( );

        DZ_API uint16_t AddFont( Font *font ) const;
        DZ_API void     SetViewportSize( float width, float height ) const;
        DZ_API void     SetDpiScale( float dpiScale ) const;
        DZ_API void     SetPointerState( Float_2 position, ClayPointerState state ) const;
        DZ_API void     UpdateScrollContainers( bool enableDragScrolling, Float_2 scrollDelta, float deltaTime );
        DZ_API void     SetDebugModeEnabled( bool enabled );
        DZ_API bool     IsDebugModeEnabled( ) const;

        DZ_API void BeginLayout( );
        DZ_API void EndLayout( ICommandList *commandList, uint32_t frameIndex ) const;

        DZ_API void OpenElement( const ClayElementDeclaration &declaration ) const;
        DZ_API void CloseElement( ) const;

        DZ_API void Text( const InteropString &text, const ClayTextDesc &desc ) const;

        DZ_API uint32_t HashString( const InteropString &str, uint32_t index = 0, uint32_t baseId = 0 ) const;

        DZ_API bool            PointerOver( uint32_t id ) const;
        DZ_API ClayBoundingBox GetElementBoundingBox( uint32_t id ) const;
        DZ_API void            HandleEvent( const Event &event );

    private:
        static Clay_Dimensions MeasureTextCallback( Clay_StringSlice text, Clay_TextElementConfig *config, void *userData );

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
        Clay_FloatingElementConfig     ConvertFloatingConfig( const ClayFloatingDesc &config ) const;
        Clay_ScrollElementConfig       ConvertScrollConfig( const ClayScrollDesc &config ) const;
        Clay_CustomElementConfig       ConvertCustomConfig( const ClayCustomDesc &config ) const;
        Clay_TextElementConfigWrapMode ConvertTextWrapMode( ClayTextWrapMode mode ) const;
        Clay_TextAlignment             ConvertTextAlignment( ClayTextAlignment align ) const;
        Clay_TextElementConfig         ConvertTextConfig( const ClayTextDesc &config ) const;
        const Clay_TextElementConfig  *GetOrCreateCachedTextConfig( const ClayTextDesc &desc );
        ClayRenderCommandType          ConvertRenderCommandType( Clay_RenderCommandType type ) const;
    };
} // namespace DenOfIz
