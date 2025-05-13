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

#include <DenOfIzGraphics/Assets/Font/Font.h>
#include <DenOfIzGraphics/Assets/Font/TextLayout.h>
#include <DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h>
#include <DenOfIzGraphics/Backends/GraphicsApi.h>
#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
#include <DenOfIzGraphics/Renderer/Sync/ResourceTracking.h>

#include <DirectXMath.h>
using namespace DirectX;

namespace DenOfIz
{
    struct DZ_API TextRendererDesc
    {
        ILogicalDevice *LogicalDevice;
        uint32_t        InitialAtlasWidth  = 512;
        uint32_t        InitialAtlasHeight = 512;
    };

    struct DZ_API TextRenderDesc
    {
        InteropString    Text;
        float            X                = 0.0f;
        float            Y                = 0.0f;
        Float_4          Color            = Float_4{ 1.0f, 1.0f, 1.0f, 1.0f };
        float            Scale            = 1.0f;
        bool             HorizontalCenter = false;
        bool             VerticalCenter   = false;
        TextDirection    Direction        = TextDirection::Auto;
        AntiAliasingMode AntiAliasingMode = AntiAliasingMode::Grayscale;
    };

    class TextRenderer
    {
        struct FontShaderUniforms
        {
            XMFLOAT4X4 Projection;
            XMFLOAT4   TextColor;
            XMFLOAT4   TextureSizeParams; // xy: texture dimensions, z: pixel range, w: unused
        };

        TextRendererDesc m_desc;
        ILogicalDevice  *m_logicalDevice = nullptr;
        ResourceTracking m_resourceTracking;
        AntiAliasingMode m_antiAliasingMode = AntiAliasingMode::Grayscale;

        std::unordered_map<std::string, Font *> m_loadedFonts;

        std::unique_ptr<ShaderProgram>      m_fontShaderProgram;
        std::unique_ptr<IPipeline>          m_fontPipeline;
        TextureDesc                         m_fontAtlasTextureDesc{ };
        std::unique_ptr<ITextureResource>   m_fontAtlasTexture       = nullptr;
        std::unique_ptr<IBufferResource>    m_fontAtlasStagingBuffer = nullptr;
        std::unique_ptr<ISampler>           m_fontSampler            = nullptr;
        BufferDesc                          m_vertexBufferDesc{ };
        std::unique_ptr<IBufferResource>    m_vertexBuffer = nullptr;
        BufferDesc                          m_indexBufferDesc{ };
        std::unique_ptr<IBufferResource>    m_indexBuffer       = nullptr;
        std::unique_ptr<IBufferResource>    m_uniformBuffer     = nullptr;
        std::unique_ptr<IRootSignature>     m_rootSignature     = nullptr;
        std::unique_ptr<IResourceBindGroup> m_resourceBindGroup = nullptr;
        std::unique_ptr<IInputLayout>       m_inputLayout       = nullptr;

        Font                                    *m_currentFont = nullptr;
        std::vector<std::unique_ptr<TextLayout>> m_textLayouts;
        uint32_t                                 m_currentTextLayoutIndex = 0;
        XMFLOAT4X4                               m_projectionMatrix{ };
        bool                                     m_atlasNeedsUpdate   = false;
        uint32_t                                 m_maxVertices        = 4096;
        uint32_t                                 m_maxIndices         = 4096;
        uint32_t                                 m_currentVertexCount = 0;
        uint32_t                                 m_currentIndexCount  = 0;
        InteropArray<GlyphVertex>                m_glyphVertices;
        InteropArray<uint32_t>                   m_indexData;

    public:
        DZ_API explicit TextRenderer( const TextRendererDesc &desc );
        DZ_API ~TextRenderer( );

        DZ_API void Initialize( );
        DZ_API void SetFont( Font *font );
        DZ_API void SetAntiAliasingMode( AntiAliasingMode antiAliasingMode );
        DZ_API void SetProjectionMatrix( const Float_4x4 &projectionMatrix );
        DZ_API void BeginBatch( );
        DZ_API void AddText( const TextRenderDesc &params );
        DZ_API void EndBatch( ICommandList *commandList );

    private:
        void UpdateAtlasTexture( ICommandList *commandList );
        void UpdateBuffers( );
    };

} // namespace DenOfIz
