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

#include <memory>
#include <vector>
#include "DenOfIzGraphics/Backends/Interface/ICommandList.h"
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/ITextureResource.h"
#include "DenOfIzGraphics/Input/Event.h"
#include "DenOfIzGraphics/UI/ClayData.h"
#include "DenOfIzGraphics/UI/IClayContext.h"

namespace DenOfIz
{
    struct WidgetExecutePipelineDesc
    {
        ICommandList   *CommandList;
        uint32_t        FrameIndex;
        Viewport        ScissorRect;
        ClayBoundingBox BoundingBox;
    };

    class Widget
    {
    protected:
        uint32_t      m_id;
        IClayContext *m_clayContext;
        bool          m_isHovered = false;
        bool          m_isFocused = false;

        static constexpr uint32_t   m_numFrames = 3; // todo size needs to be input
        std::vector<std::unique_ptr<ITextureResource>> m_renderTargets;

        bool     m_hasPipeline  = false;
        uint32_t m_textureIndex = 0;

    public:
        DZ_API Widget( IClayContext *clayContext, uint32_t id );
        DZ_API virtual ~Widget( );

        DZ_API virtual void Update( float deltaTime )                                              = 0;
        DZ_API virtual void CreateLayoutElement( )                                                 = 0;
        DZ_API virtual void Render( const ClayBoundingBox &boundingBox, IRenderBatch *renderBatch ) = 0;
        DZ_API virtual void HandleEvent( const Event &event )                                      = 0;

        DZ_API virtual bool HasPipeline( ) const;
        DZ_API virtual void InitializeRenderResources( ILogicalDevice *device, uint32_t width, uint32_t height );
        DZ_API virtual void ResizeRenderResources( uint32_t width, uint32_t height );
        DZ_API virtual void ExecuteCustomPipeline( const WidgetExecutePipelineDesc &context );

        DZ_API ITextureResource *GetRenderTarget( uint32_t frameIndex ) const;
        DZ_API void              SetTextureIndex( uint32_t index );
        DZ_API uint32_t          GetTextureIndex( ) const;

        DZ_API uint32_t GetId( ) const;
        DZ_API bool     IsHovered( ) const;
        DZ_API bool     IsFocused( ) const;

        DZ_API void UpdateHoverState( bool hovered );
        DZ_API void UpdateHoverState( );

        DZ_API ClayBoundingBox GetBoundingBox( ) const;

    protected:
        DZ_API void AddRectangle( IRenderBatch *renderBatch, const ClayBoundingBox &bounds, const ClayColor &color,
                                  const ClayCornerRadius &cornerRadius = ClayCornerRadius( ) ) const;
        DZ_API void AddBorder( IRenderBatch *renderBatch, const ClayBoundingBox &bounds, const ClayColor &color, const ClayBorderWidth &width,
                               const ClayCornerRadius &cornerRadius = ClayCornerRadius( ) ) const;
    };

    struct WidgetRenderData
    {
        ClayCustomWidgetType Type;
        Widget              *WidgetPtr;
    };

} // namespace DenOfIz
