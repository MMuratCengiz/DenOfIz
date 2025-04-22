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

#include <DenOfIzGraphics/Assets/Font/FontCache.h>
#include <DenOfIzGraphics/Assets/Font/FontManager.h>
#include <DenOfIzGraphics/Backends/GraphicsApi.h>
#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
#include <DenOfIzGraphics/Renderer/Sync/ResourceTracking.h>
#include <DenOfIzGraphics/Utilities/Common.h>

using namespace DirectX;

namespace DenOfIz
{
    struct TextRenderDesc
    {
        std::string Text;             // UTF-8 encoded text
        float       X;                // X position in screen space
        float       Y;                // Y position in screen space
        XMFLOAT4    Color;            // Text color
        float       Scale;            // Text scaling factor
        bool        HorizontalCenter; // Center text horizontally
        bool        VerticalCenter;   // Center text vertically

        TextRenderDesc( ) : X( 0.0f ), Y( 0.0f ), Color( 1.0f, 1.0f, 1.0f, 1.0f ), Scale( 1.0f ), HorizontalCenter( false ), VerticalCenter( false )
        {
        }
    };

    struct FontShaderUniforms
    {
        XMFLOAT4X4 Projection;
        XMFLOAT4   TextColor;
    };

    class FontRenderer
    {
        GraphicsApi     *m_graphicsApi;
        ILogicalDevice  *m_logicalDevice;
        FontManager      m_fontManager;
        ResourceTracking m_resourceTracking;

        std::unordered_map<std::string, std::shared_ptr<FontCache>> m_loadedFonts;

        std::unique_ptr<ShaderProgram>      m_fontShaderProgram;
        std::unique_ptr<IPipeline>          m_fontPipeline;
        TextureDesc                         m_fontAtlasTextureDesc{ };
        std::unique_ptr<ITextureResource>   m_fontAtlasTexture = nullptr;
        std::unique_ptr<ISampler>           m_fontSampler      = nullptr;
        BufferDesc                          m_vertexBufferDesc{ };
        std::unique_ptr<IBufferResource>    m_vertexBuffer = nullptr;
        BufferDesc                          m_indexBufferDesc{ };
        std::unique_ptr<IBufferResource>    m_indexBuffer       = nullptr;
        std::unique_ptr<IBufferResource>    m_uniformBuffer     = nullptr;
        std::unique_ptr<IRootSignature>     m_rootSignature     = nullptr;
        std::unique_ptr<IResourceBindGroup> m_resourceBindGroup = nullptr;
        std::unique_ptr<IInputLayout>       m_inputLayout       = nullptr;

        std::shared_ptr<FontCache> m_currentFont;
        XMFLOAT4X4                 m_projectionMatrix{ };
        bool                       m_atlasNeedsUpdate   = false;
        uint32_t                   m_maxVertices        = 1024;
        uint32_t                   m_maxIndices         = 1536;
        uint32_t                   m_currentVertexCount = 0;
        uint32_t                   m_currentIndexCount  = 0;
        std::vector<float>         m_vertexData;
        std::vector<uint32_t>      m_indexData;

    public:
        FontRenderer( GraphicsApi *graphicsApi, ILogicalDevice *logicalDevice );
        ~FontRenderer( );

        void                       Initialize( );
        std::shared_ptr<FontCache> LoadFont( const std::string &fontPath, uint32_t pixelSize = 24 );
        void                       SetFont( const std::string &fontPath, uint32_t pixelSize = 24 );
        void                       SetProjectionMatrix( const XMFLOAT4X4 &projectionMatrix );
        void                       BeginBatch( );
        void                       AddText( const TextRenderDesc &params );
        void                       EndBatch( ICommandList *commandList );

    private:
        void UpdateAtlasTexture( ICommandList *commandList );
        void UpdateBuffers( );
        void CalculateCenteredPosition( const std::u32string &text, TextRenderDesc &params ) const;
    };

} // namespace DenOfIz
