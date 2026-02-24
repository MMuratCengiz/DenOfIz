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
#include "DenOfIzGraphics/Assets/Font/TextRenderer.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "DenOfIzGraphics/Utilities/FrameDebugRenderer.h"

namespace DenOfIz
{
    class TextRenderingExample final : public IExample
    {
        const std::string          m_fontAssetPath    = "Assets/Fonts/Inconsolata-Regular.dzfont";
        DenOfIz_FontLibrary        m_fontLibrary      = DENOFIZ_NULL_HANDLE;
        DenOfIz_BinaryReader       m_binaryReader     = DENOFIZ_NULL_HANDLE;
        DenOfIz_FontAssetReader    m_fontAssetReader  = DENOFIZ_NULL_HANDLE;
        DenOfIz_FontAsset          m_fontAsset        = DENOFIZ_NULL_HANDLE;
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

        struct ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc  = DenOfIz::ExampleWindowDesc( );
            windowDesc.Title = "Font Rendering Example";
            return windowDesc;
        }

    private:
        void ImportFont( ) const;
    };
} // namespace DenOfIz
