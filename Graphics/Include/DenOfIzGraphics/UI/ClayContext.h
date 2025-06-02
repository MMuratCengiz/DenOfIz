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
#include <DenOfIzGraphics/UI/IClayContext.h>
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

    /// Implementation of IClayContext, also provides additional internal methods
    class ClayContext : public IClayContext
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
        ~ClayContext( ) override;

        void            OpenElement( const ClayElementDeclaration &declaration ) const override;
        void            CloseElement( ) const override;
        void            Text( const InteropString &text, const ClayTextDesc &desc ) const override;
        ClayDimensions  MeasureText( const InteropString &text, uint16_t fontId, uint16_t fontSize ) const override;
        uint32_t        HashString( const InteropString &str, uint32_t index = 0, uint32_t baseId = 0 ) const override;
        bool            PointerOver( uint32_t id ) const override;
        ClayBoundingBox GetElementBoundingBox( uint32_t id ) const override;
        ClayDimensions  GetViewportSize( ) const override;
        bool            IsDebugModeEnabled( ) const override;

        void BeginLayout( ) const;
        void SetViewportSize( float width, float height ) const;
        void SetDpiScale( float dpiScale );
        void SetPointerState( Float_2 position, ClayPointerState state );
        void UpdateScrollContainers( bool enableDragScrolling, Float_2 scrollDelta, float deltaTime );
        void SetDebugModeEnabled( bool enabled );

        void  AddFont( uint16_t fontId, Font *font ) const;
        void  RemoveFont( uint16_t fontId ) const;
        Font *GetFont( uint16_t fontId ) const;

        ClayTextCache *GetClayText( ) const;

        Clay_RenderCommandArray        EndLayoutAndGetCommands( float deltaTime = 0.016f ) const;
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
