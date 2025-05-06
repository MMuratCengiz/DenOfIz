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

using namespace DirectX;

namespace DenOfIz
{
    struct DZ_API TextRendererDesc
    {
        GraphicsApi    *GraphicsApi;
        ILogicalDevice *LogicalDevice;
        uint32_t        InitialAtlasWidth  = 512;
        uint32_t        InitialAtlasHeight = 512;
    };

    struct DZ_API TextRenderDesc
    {
        InteropString    Text;
        float            X;
        float            Y;
        Float_4          Color;
        float            Scale;
        bool             HorizontalCenter;
        bool             VerticalCenter;
        TextDirection    Direction;
        AntiAliasingMode AntiAliasingMode = AntiAliasingMode::Grayscale;

        TextRenderDesc( ) :
            X( 0.0f ), Y( 0.0f ), Color( 1.0f, 1.0f, 1.0f, 1.0f ), Scale( 1.0f ), HorizontalCenter( false ), VerticalCenter( false ), Direction( TextDirection::Auto )
        {
        }
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
        GraphicsApi     *m_graphicsApi   = nullptr;
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
        explicit TextRenderer( const TextRendererDesc &desc );
        ~TextRenderer( );

        void Initialize( );
        void SetFont( Font *font );
        void SetAntiAliasingMode( AntiAliasingMode antiAliasingMode );
        void SetProjectionMatrix( const Float_4x4 &projectionMatrix );
        void BeginBatch( );
        void AddText( const TextRenderDesc &params );
        void EndBatch( ICommandList *commandList );

    private:
        void UpdateAtlasTexture( ICommandList *commandList );
        void UpdateBuffers( );
    };

} // namespace DenOfIz
