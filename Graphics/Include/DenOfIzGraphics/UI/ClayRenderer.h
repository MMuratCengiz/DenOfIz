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
#include <memory>
#include <unordered_map>
#include "ClayInternal.h"

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
        uint32_t        Width  = 1024;
        uint32_t        Height = 1024;
    };

    class ClayRenderer
    {
    public:
        DZ_API explicit ClayRenderer( const ClayRendererDesc &desc );
        DZ_API ~ClayRenderer( );

        DZ_API void SetViewportSize( float width, float height );
        DZ_API void Render( ICommandList *commandList, const InteropArray<ClayRenderCommand> &commands, uint32_t frameIndex );
        DZ_API void ClearCaches( );
        DZ_API void InvalidateLayout( );

        DZ_API ClayDimensions MeasureText( const InteropString &text, const ClayTextDesc &desc ) const;

    private:
        ClayRendererDesc              m_desc;
        std::unique_ptr<QuadRenderer> m_quadRenderer;
        std::unique_ptr<ThorVGCanvas> m_vectorCanvas;

        struct ShapeCache
        {
            ITextureResource *Texture = nullptr;
        };
        std::unordered_map<uint64_t, ShapeCache> m_shapeCache;

        uint32_t m_nextMaterialId = 0;
        uint32_t m_nextQuadId     = 0;

        // Track if we need to clear caches
        bool m_needsClear = true;

        // Frame tracking for reusing quads
        uint32_t m_currentFrameQuadIndex     = 0;
        uint32_t m_currentFrameMaterialIndex = 0;

        // Track viewport size to detect changes
        float m_viewportWidth  = 0;
        float m_viewportHeight = 0;

        void RenderRectangle( ICommandList *commandList, const ClayBoundingBox &bounds, const ClayRectangleRenderData &data, uint32_t frameIndex );
        void RenderRoundedRectangle( ICommandList *commandList, const ClayBoundingBox &bounds, const ClayRectangleRenderData &data, uint32_t frameIndex );
        void RenderBorder( ICommandList *commandList, const ClayBoundingBox &bounds, const ClayBorderRenderData &data, uint32_t frameIndex );
        void RenderText( ICommandList *commandList, const ClayBoundingBox &bounds, const ClayTextRenderData &data, uint32_t frameIndex ) const;
        void RenderImage( ICommandList *commandList, const ClayBoundingBox &bounds, const ClayImageRenderData &data, uint32_t frameIndex );

        ITextureResource *GetOrCreateRoundedRectTexture( const ClayBoundingBox &bounds, const ClayRectangleRenderData &data );
        uint64_t          GetShapeHash( const ClayBoundingBox &bounds, const ClayRectangleRenderData &data ) const;
        void              CreateVectorShape( const ClayBoundingBox &bounds, const ClayRectangleRenderData &data, ThorVGCanvas &canvas ) const;

        uint32_t GetOrCreateMaterial( const Float_4 &color, ITextureResource *texture = nullptr );
        uint32_t GetOrCreateQuad( const ClayBoundingBox &bounds, uint32_t materialId );
    };

} // namespace DenOfIz
