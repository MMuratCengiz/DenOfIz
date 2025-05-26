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

namespace DenOfIz
{
    class UIExample final : public IExample
    {
        FontLibrary           m_library{ };
        Time                  m_time;
        std::unique_ptr<Clay> m_clay;
        Float_2               m_mousePosition{ };
        bool                  m_mousePressed{ };
        uint32_t              m_buttonId{ };
        uint32_t              m_textId{ };
        uint32_t              m_containerId{ };

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
        void CreateUI( ) const;
    };
} // namespace DenOfIz
