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
#include "DenOfIzGraphics/UI/Clay.h"
#include "Spinning3DCubeWidget.h"

namespace DenOfIz
{
    class UIExample final : public IExample
    {
        DenOfIz_Clay      m_clay        = DENOFIZ_NULL_HANDLE;
        DenOfIz_Semaphore m_uiSemaphore = DENOFIZ_NULL_HANDLE;
        DenOfIz_Float2    m_mousePosition{ };

        std::unique_ptr<Spinning3DCubeWidget> m_spinningCubeWidget;

        uint32_t m_containerId{ };
        uint32_t m_showCubeButtonId{ };
        uint32_t m_hideCubeButtonId{ };
        uint32_t m_textFieldId{ };

        bool m_showCubeWindow = true;

        char     m_textBuffer[ 256 ] = "Click to edit text";
        uint32_t m_cursorIndex       = 0;
        bool     m_textFieldFocused  = false;
        float    m_cursorBlinkTimer  = 0.0f;

        DenOfIz_ClayColor m_bgColor{ };
        DenOfIz_ClayColor m_cardColor{ };
        DenOfIz_ClayColor m_textColor{ };
        DenOfIz_ClayColor m_buttonColor{ };
        DenOfIz_ClayColor m_buttonHoverColor{ };
        DenOfIz_ClayColor m_cursorColor{ };

    public:
        ~UIExample( ) override = default;
        void Init( ) override;
        void ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void HandleEvent( DenOfIz_Event &event ) override;
        void Update( ) override;
        void Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        void Quit( ) override;

    private:
        void CreateUI( uint32_t frameIndex );
        void CreateButton( const char *text, uint32_t buttonId );
        void CreateTextField( );
    };
} // namespace DenOfIz
