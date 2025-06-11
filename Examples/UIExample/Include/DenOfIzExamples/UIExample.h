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

#include "DenOfIzExamples/IExample.h"
#include "DenOfIzGraphics/Assets/Font/FontLibrary.h"
#include "DenOfIzGraphics/UI/Clay.h"
#include "DenOfIzGraphics/UI/Widgets/CheckboxWidget.h"
#include "DenOfIzGraphics/UI/Widgets/DockableContainerWidget.h"
#include "DenOfIzGraphics/UI/Widgets/DropdownWidget.h"
#include "DenOfIzGraphics/UI/Widgets/SliderWidget.h"
#include "DenOfIzGraphics/UI/Widgets/TextFieldWidget.h"
#include "Spinning3DCubeWidget.h"

namespace DenOfIz
{
    class UIExample final : public IExample
    {
        FontLibrary           m_library{ };
        std::unique_ptr<Clay> m_clay;
        Float_2               m_mousePosition{ };
        bool                  m_mousePressed{ };
        bool                  m_mouseJustReleased{ };

        CheckboxWidget  *m_darkModeCheckbox   = nullptr;
        SliderWidget    *m_cubeRotationSlider = nullptr;
        DropdownWidget  *m_dpiScaleDropdown   = nullptr;
        TextFieldWidget *m_multilineTextField = nullptr;

        DockableContainerWidget              *m_cubeContainer  = nullptr;
        DockableContainerWidget              *m_textContainer  = nullptr;
        std::unique_ptr<DockingManager>       m_dockingManager = nullptr;
        std::unique_ptr<Spinning3DCubeWidget> m_spinningCubeWidget;

        uint32_t                m_containerId{ };
        std::vector<StringView> m_dpiScaleOptions;

        bool      m_darkMode  = false;
        ClayColor m_bgColor   = { };
        ClayColor m_cardColor = { };
        ClayColor m_textColor = { };

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
            windowDesc.Width  = 1600;
            windowDesc.Height = 900;
            return windowDesc;
        }

    private:
        void CreateUI( );
        void CreateHeader( const ClayColor &textColor ) const;
        void CreateMainContent( const ClayColor &cardColor, const ClayColor &textColor ) const;
        void CreateCard( const ClayColor &cardColor, const ClayColor &textColor, const char *title ) const;
        void CreateCheckboxRow( const char *label, CheckboxWidget *widget, const ClayColor &textColor ) const;
        void CreateDropdownRow( const char *label, DropdownWidget *widget, const ClayColor &textColor ) const;
        void CreateButton( const char *text, const ClayColor &bgColor, const ClayColor &textColor, uint32_t buttonId ) const;
    };
} // namespace DenOfIz
