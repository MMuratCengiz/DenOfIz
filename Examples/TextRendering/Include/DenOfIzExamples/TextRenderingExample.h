/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzExamples/IExample.h"
#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphics/Assets/Font/TextRenderer.h"
#include "DenOfIzGraphics/Utilities/FrameDebugRenderer.h"

namespace DenOfIz
{
    class TextRenderingExample final : public IExample
    {
        DenOfIz_Font               m_font             = DENOFIZ_NULL_HANDLE;
        DenOfIz_TextRenderer       m_textRenderer     = DENOFIZ_NULL_HANDLE;
        DenOfIz_FrameDebugRenderer m_debugRenderer    = DENOFIZ_NULL_HANDLE;
        float                      m_animTime         = 0.0f;
        bool                       m_debugInfoEnabled = true;

    public:
        ~TextRenderingExample( ) override = default;
        void Init( ) override;
        void ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference ) override;
        void HandleEvent( DenOfIz_Event &event ) override;
        void Update( ) override;
        void Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        void Quit( ) override;
        void OnResize( uint32_t width, uint32_t height ) override;

        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc  = DenOfIz::ExampleWindowDesc( );
            windowDesc.Title = "Font Rendering Example";
            return windowDesc;
        }
    };
} // namespace DenOfIz
