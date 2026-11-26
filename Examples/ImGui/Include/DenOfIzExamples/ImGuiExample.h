/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
