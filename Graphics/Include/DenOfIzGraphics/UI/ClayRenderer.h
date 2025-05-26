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

#include <DenOfIzGraphics/Assets/Font/TextRenderer.h>
#include <DenOfIzGraphics/Assets/Vector2d/QuadRenderer.h>
#include <DenOfIzGraphics/Assets/Vector2d/ThorVGWrapper.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <clay.h>
#include <memory>
#include <unordered_map>
#include "ClayData.h"

namespace DenOfIz
{
    struct ClayRendererDesc
    {
        ILogicalDevice *LogicalDevice      = nullptr;
        TextRenderer   *TextRenderer       = nullptr;
        Format          RenderTargetFormat = Format::B8G8R8A8Unorm;
        uint32_t        NumFrames          = 3;
        uint32_t        MaxNumQuads        = 2048;
        uint32_t        MaxNumMaterials    = 128;
        uint32_t        Width              = 1024;
        uint32_t        Height             = 1024;
    };

    /// This class is intentionally not public API, users should not need to know about the internals of the renderer
    class ClayRenderer
    {
        ClayRendererDesc              m_desc;
        std::unique_ptr<QuadRenderer> m_quadRenderer;
        std::unique_ptr<ThorVGCanvas> m_vectorCanvas;

        struct ShapeCache
        {
            ITextureResource *Texture = nullptr;
        };
        std::unordered_map<uint64_t, ShapeCache> m_shapeCache;

        struct MaterialKey
        {
            Clay_Color        Color;
            ITextureResource *Texture;

            bool operator==( const MaterialKey &other ) const
            {
                return Color.r == other.Color.r && Color.g == other.Color.g && Color.b == other.Color.b && Color.a == other.Color.a && Texture == other.Texture;
            }
        };

        struct MaterialKeyHash
        {
            std::size_t operator( )( const MaterialKey &key ) const noexcept
            {
                const std::size_t h1 = std::hash<float>{ }( key.Color.r );
                const std::size_t h2 = std::hash<float>{ }( key.Color.g );
                const std::size_t h3 = std::hash<float>{ }( key.Color.b );
                const std::size_t h4 = std::hash<float>{ }( key.Color.a );
                const std::size_t h5 = std::hash<void *>{ }( key.Texture );
                return h1 ^ h2 << 1 ^ h3 << 2 ^ h4 << 3 ^ h5 << 4;
            }
        };

        std::unordered_map<MaterialKey, uint32_t, MaterialKeyHash> m_materialCache;

        uint32_t m_nextMaterialId = 0;
        uint32_t m_nextQuadId     = 0;
        bool     m_needsClear     = true;

        uint32_t m_currentFrameQuadIndex = 0;
        uint32_t m_currentFrameIndex     = 0;

        float m_viewportWidth  = 0;
        float m_viewportHeight = 0;
        float m_dpiScale       = 1.0f;

    public:
        explicit ClayRenderer( const ClayRendererDesc &desc );
        ~ClayRenderer( );
        void           Resize( float width, float height );
        void           SetDpiScale( float dpiScale );
        void           Render( ICommandList *commandList, const Clay_RenderCommandArray &commands, uint32_t frameIndex );
        void           ClearCaches( );
        void           InvalidateLayout( );
        ClayDimensions MeasureText( const InteropString &text, const Clay_TextElementConfig &desc ) const;

    private:
        void RenderRectangle( const Clay_RenderCommand *command, uint32_t frameIndex );
        void RenderRoundedRectangle( const Clay_RenderCommand *command, uint32_t frameIndex );
        void RenderBorder( const Clay_RenderCommand *command, uint32_t frameIndex );
        void RenderText( const Clay_RenderCommand *command, uint32_t frameIndex, ICommandList *commandList ) const;
        void RenderImage( const Clay_RenderCommand *command, uint32_t frameIndex );

        ITextureResource *GetOrCreateRoundedRectTexture( const Clay_BoundingBox &bounds, const Clay_RectangleRenderData &data );
        uint64_t          GetShapeHash( const Clay_BoundingBox &bounds, const Clay_RectangleRenderData &data ) const;
        void              CreateVectorShape( const Clay_BoundingBox &bounds, const Clay_RectangleRenderData &data, ThorVGCanvas &canvas ) const;

        uint32_t GetOrCreateMaterial( const Clay_Color &color, ITextureResource *texture = nullptr );
        uint32_t GetOrCreateQuad( const Clay_BoundingBox &bounds, uint32_t materialId );
    };

} // namespace DenOfIz
