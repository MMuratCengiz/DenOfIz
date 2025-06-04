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

#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphics/Assets/Font/TextLayout.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"
#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "DenOfIzGraphics/Backends/Interface/ICommandList.h"
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/IPipeline.h"
#include "DenOfIzGraphics/Renderer/Sync/ResourceTracking.h"

#include "TextBatch.h"

using namespace DirectX;

namespace DenOfIz
{
    struct DZ_API TextRendererDesc
    {
        ILogicalDevice  *LogicalDevice;
        uint32_t         InitialAtlasWidth  = 512;
        uint32_t         InitialAtlasHeight = 512;
        AntiAliasingMode AntiAliasingMode   = AntiAliasingMode::Grayscale;
        uint32_t         Width;
        uint32_t         Height;
        Font            *Font = nullptr;
    };

    struct DZ_API TextRenderDesc : AddTextDesc
    {
        uint16_t         FontId           = 0;
        AntiAliasingMode AntiAliasingMode = AntiAliasingMode::Grayscale;
    };

    class TextRenderer
    {
        TextRendererDesc m_desc;
        ILogicalDevice  *m_logicalDevice = nullptr;
        ResourceTracking m_resourceTracking;
        AntiAliasingMode m_antiAliasingMode = AntiAliasingMode::Grayscale;

        std::unique_ptr<ShaderProgram> m_fontShaderProgram;
        std::unique_ptr<IPipeline>     m_fontPipeline;

        std::unique_ptr<IRootSignature>     m_rootSignature     = nullptr;
        std::unique_ptr<IInputLayout>       m_inputLayout       = nullptr;

        Font               *m_font = nullptr;
        std::vector<Font *> m_fonts;
        XMFLOAT4X4          m_projectionMatrix{ };

        std::vector<uint32_t>                   m_validFonts;
        std::vector<std::unique_ptr<TextBatch>> m_textBatches; // index = fontId
    public:
        DZ_API explicit TextRenderer( const TextRendererDesc &desc );
        DZ_API ~TextRenderer( ) = default;

        DZ_API void SetAntiAliasingMode( AntiAliasingMode antiAliasingMode );
        DZ_API void SetProjectionMatrix( const Float_4x4 &projectionMatrix );
        DZ_API void SetViewport( const Viewport &viewport );

        // Font management
        DZ_API uint16_t AddFont( Font *font, uint16_t fontId = 0 ); // Returns assigned FontId
        DZ_API Font    *GetFont( uint16_t fontId ) const;
        DZ_API void     RemoveFont( uint16_t fontId );

        // Legacy batching API (deprecated - use RenderText instead)
        DZ_API void BeginBatch( ) const;
        DZ_API void AddText( const TextRenderDesc &params ) const;
        DZ_API void EndBatch( ICommandList *commandList ) const;

        DZ_API Float_2 MeasureText( const InteropString &text, const TextRenderDesc &desc ) const;
    };

} // namespace DenOfIz
