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

#include "../../../../ImGui/ImGuiBackend.h"
#include "DenOfIzExamples/IExample.h"

namespace DenOfIz
{
    class ImGuiExample final : public IExample
    {
        ImGuiRenderer *m_imguiContext      = nullptr;
        bool           m_showDemoWindow    = true;
        bool           m_showAnotherWindow = false;
        ImVec4         m_clearColor        = ImVec4( 0.45f, 0.55f, 0.60f, 1.00f );

    public:
        ~ImGuiExample( ) override = default;
        void Init( ) override;
        void ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void HandleEvent( DenOfIz_Event &event ) override;
        void Update( ) override;
        void Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        void Quit( ) override;
    };
} // namespace DenOfIz
