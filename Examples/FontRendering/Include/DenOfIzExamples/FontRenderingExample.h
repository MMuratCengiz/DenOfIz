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
#include <DenOfIzGraphics/Assets/Font/FontRenderer.h>
#include <DenOfIzGraphics/Assets/Font/FontLibrary.h>
#include <DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryReader.h>
#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Utilities/Time.h>

namespace DenOfIz
{
    class FontRenderingExample final : public IExample
    {
        const InteropString m_fontAssetPath = "Assets/Fonts/Inconsolata-Regular.dzfont";
        Time                                m_time;
        std::unique_ptr<FontLibrary>        m_fontLibrary;
        std::unique_ptr<BinaryReader>       m_binaryReader;
        std::unique_ptr<FontAssetReader>    m_fontAssetReader;
        std::unique_ptr<FontAsset>          m_fontAsset;
        Font*                               m_font = nullptr; // Owned by FontLibrary
        std::unique_ptr<FontRenderer>       m_fontRenderer;
        XMFLOAT4X4                          m_orthoProjection{ };
        float                               m_animTime = 0.0f;

    public:
        ~FontRenderingExample( ) override = default;
        void Init( ) override;
        void ModifyApiPreferences( APIPreference &defaultApiPreference ) override;
        void HandleEvent( SDL_Event &event ) override;
        void Update( ) override;
        void Render( uint32_t frameIndex, ICommandList *commandList ) override;
        void Quit( ) override;

        struct WindowDesc WindowDesc( ) override
        {
            auto windowDesc  = DenOfIz::WindowDesc( );
            windowDesc.Title = "Font Rendering Example";
            return windowDesc;
        }
    private:
        void ImportFont( ) const;
    };
} // namespace DenOfIz
