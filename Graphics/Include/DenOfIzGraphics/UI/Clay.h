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

#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphics/Input/Event.h"
#include "DenOfIzGraphics/UI/ClayData.h"

namespace DenOfIz
{
    class Widget;
    class CheckboxWidget;
    class ColorPickerWidget;
    class DropdownWidget;
    class SliderWidget;
    class TextFieldWidget;
    class ResizableContainerWidget;
    class DockableContainerWidget;
    class DockingManager;
    class IClayContext;

    using CheckboxStyle           = ClayCheckboxDesc;
    using SliderStyle             = ClaySliderDesc;
    using DropdownStyle           = ClayDropdownDesc;
    using ColorPickerStyle        = ClayColorPickerDesc;
    using TextFieldStyle          = ClayTextFieldDesc;
    using ResizableContainerStyle = ClayResizableContainerDesc;
    using DockableContainerStyle  = ClayDockableContainerDesc;

    struct DZ_API ClayDesc
    {
        ILogicalDevice *LogicalDevice                  = nullptr;
        Format          RenderTargetFormat             = Format::B8G8R8A8Unorm;
        uint32_t        NumFrames                      = 3;
        uint32_t        MaxNumQuads                    = 2048;
        uint32_t        MaxNumMaterials                = 128;
        uint32_t        MaxNumFonts                    = 16;    // Number of texture slots reserved for fonts
        uint32_t        Width                          = 0;
        uint32_t        Height                         = 0;
        uint32_t        MaxNumElements                 = 8192;
        uint32_t        MaxNumTextMeasureCacheElements = 16384; // Maybe remove
        uint32_t        MaxPipelineWidgets             = 16;    // Maximum number of widgets with their own rendering pipelines
    };

    struct DZ_API AddFontDesc
    {

        Font    *Font     = nullptr;
        uint32_t FontSize = 16;
    };

    class Clay
    {
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        DZ_API explicit Clay( const ClayDesc &desc );
        DZ_API ~Clay( );

        DZ_API void           SetViewportSize( float width, float height ) const;
        DZ_API ClayDimensions GetViewportSize( ) const;
        DZ_API void           SetDpiScale( float dpiScale ) const;
        DZ_API void           SetPointerState( Float_2 position, ClayPointerState state ) const;
        DZ_API void           UpdateScrollContainers( bool enableDragScrolling, Float_2 scrollDelta, float deltaTime ) const;
        DZ_API void           SetDebugModeEnabled( bool enabled ) const;
        DZ_API bool           IsDebugModeEnabled( ) const;

        DZ_API void BeginLayout( ) const;
        DZ_API void EndLayout( ICommandList *commandList, uint32_t frameIndex, float deltaTime = 0.016f ) const;

        DZ_API void OpenElement( const ClayElementDeclaration &declaration ) const;
        DZ_API void CloseElement( ) const;

        DZ_API void Text( const InteropString &text, const ClayTextDesc &desc ) const;

        DZ_API uint32_t HashString( const InteropString &str, uint32_t index = 0, uint32_t baseId = 0 ) const;

        DZ_API bool            PointerOver( uint32_t id ) const;
        DZ_API ClayBoundingBox GetElementBoundingBox( uint32_t id ) const;
        DZ_API void            HandleEvent( const Event &event ) const;
        DZ_API ClayDimensions  MeasureText( const InteropString &text, uint16_t fontId, uint16_t fontSize ) const;

        DZ_API void AddFont( uint16_t fontId, Font *font ) const;
        DZ_API void RemoveFont( uint16_t fontId ) const;

        DZ_API CheckboxWidget           *CreateCheckbox( uint32_t id, bool initialChecked = false, const CheckboxStyle &style = { } ) const;
        DZ_API SliderWidget             *CreateSlider( uint32_t id, float initialValue = 0.5f, const SliderStyle &style = { } ) const;
        DZ_API DropdownWidget           *CreateDropdown( uint32_t id, const StringArray &options, const DropdownStyle &style = { } ) const;
        DZ_API ColorPickerWidget        *CreateColorPicker( uint32_t id, const Float_3 &initialRgb = {}, const ColorPickerStyle &style = { } ) const;
        DZ_API TextFieldWidget          *CreateTextField( uint32_t id, const TextFieldStyle &style = { } ) const;
        DZ_API ResizableContainerWidget *CreateResizableContainer( uint32_t id ) const;
        DZ_API DockableContainerWidget  *CreateDockableContainer( uint32_t id, DockingManager *dockingManager ) const;

        DZ_API DockingManager *CreateDockingManager( ) const;

        DZ_API Widget *GetWidget( uint32_t id ) const;
        DZ_API void    RemoveWidget( uint32_t id ) const;
        DZ_API void    UpdateWidgets( float deltaTime ) const;

        DZ_API void          RegisterPipelineWidget( Widget *widget ) const;
        DZ_API IClayContext *GetContext( ) const;
    };
} // namespace DenOfIz
