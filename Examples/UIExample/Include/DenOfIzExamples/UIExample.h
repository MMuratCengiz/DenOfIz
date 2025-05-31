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

#include <DenOfIzExamples/IExample.h>
#include <DenOfIzGraphics/Utilities/Time.h>

#include "DenOfIzGraphics/Assets/Font/FontLibrary.h"
#include "DenOfIzGraphics/UI/Clay.h"
#include "DenOfIzGraphics/UI/Widgets/CheckboxWidget.h"
#include "DenOfIzGraphics/UI/Widgets/ColorPickerWidget.h"
#include "DenOfIzGraphics/UI/Widgets/DockableContainerWidget.h"
#include "DenOfIzGraphics/UI/Widgets/DropdownWidget.h"
#include "DenOfIzGraphics/UI/Widgets/ResizableContainerWidget.h"
#include "DenOfIzGraphics/UI/Widgets/SliderWidget.h"
#include "DenOfIzGraphics/UI/Widgets/TextFieldWidget.h"

namespace DenOfIz
{
    class UIExample final : public IExample
    {
        FontLibrary           m_library{ };
        Time                  m_time;
        std::unique_ptr<Clay> m_clay;
        Float_2               m_mousePosition{ };
        bool                  m_mousePressed{ };
        bool                  m_mouseJustReleased{ };

        CheckboxWidget                 *m_darkModeCheckbox         = nullptr;
        CheckboxWidget                 *m_enableAnimationsCheckbox = nullptr;
        SliderWidget                   *m_volumeSlider             = nullptr;
        SliderWidget                   *m_brightnessSlider         = nullptr;
        DropdownWidget                 *m_themeDropdown            = nullptr;
        DropdownWidget                 *m_languageDropdown         = nullptr;
        ColorPickerWidget              *m_accentColorPicker        = nullptr;
        TextFieldWidget                *m_usernameField            = nullptr;
        TextFieldWidget                *m_passwordField            = nullptr;
        TextFieldWidget                *m_commentsField            = nullptr;
        ResizableContainerWidget       *m_resizableContainer       = nullptr;
        DockableContainerWidget        *m_dockableContainer1       = nullptr;
        DockableContainerWidget        *m_dockableContainer2       = nullptr;
        std::unique_ptr<DockingManager> m_dockingManager           = nullptr;

        uint32_t m_containerId{ };
        uint32_t m_scrollContainerId{ };

        std::string                 m_statusText = "Welcome! Explore the widgets below.";
        InteropArray<InteropString> m_themeOptions;
        InteropArray<InteropString> m_languageOptions;

    public:
        ~UIExample( ) override = default;
        void                     Init( ) override;
        void                     ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void                     HandleEvent( Event &event ) override;
        void                     Update( ) override;
        void                     Render( uint32_t frameIndex, ICommandList *commandList ) override;
        void                     Quit( ) override;
        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc   = DenOfIz::ExampleWindowDesc( );
            windowDesc.Title  = "UIExample";
            windowDesc.Width  = 1280;
            windowDesc.Height = 720;
            return windowDesc;
        }

    private:
        void CreateUI( );
        void CreateHeader( const ClayColor &textColor ) const;
        void CreateSettingsPanel( const ClayColor &cardColor, const ClayColor &textColor, const ClayColor &secondaryTextColor ) const;
        void CreateFormsPanel( const ClayColor &cardColor, const ClayColor &textColor, const ClayColor &secondaryTextColor ) const;
        void CreateFooter( const ClayColor &cardColor, const ClayColor &textColor, const ClayColor &secondaryTextColor );
        void CreateCard( const ClayColor &cardColor, const ClayColor &textColor, const char *title ) const;
        void CreateCheckboxRow( const char *label, CheckboxWidget *widget, const ClayColor &textColor ) const;
        void CreateSliderRow( const char *label, SliderWidget *widget, const ClayColor &textColor, const ClayColor &secondaryTextColor ) const;
        void CreateDropdownRow( const char *label, DropdownWidget *widget, const ClayColor &textColor ) const;
        void CreateTextFieldRow( const char *label, TextFieldWidget *widget, const ClayColor &textColor ) const;
    };
} // namespace DenOfIz
