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

#include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandListPool.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandQueue.h>
#include <DenOfIzGraphics/Backends/Interface/IFence.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <DenOfIzGraphics/UI/ClayContext.h>
#include <DenOfIzGraphics/UI/ClayData.h>
#include <DenOfIzGraphics/UI/ClayTextCache.h>
#include <DenOfIzGraphics/UI/FullscreenQuadPipeline.h>
#include <DenOfIzGraphics/UI/UIShapeCache.h>
#include <DenOfIzGraphics/UI/UIShapes.h>
#include <clay.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphics/Renderer/Sync/ResourceTracking.h"

#include <DirectXMath.h>

namespace DenOfIz
{

    class Widget;

    struct ClayRendererDesc
    {
        ILogicalDevice *LogicalDevice      = nullptr;
        ClayContext    *ClayContext        = nullptr;
        Format          RenderTargetFormat = Format::B8G8R8A8Unorm;
        uint32_t        NumFrames          = 3;
        uint32_t        MaxVertices        = 65536;
        uint32_t        MaxIndices         = 98304;
        uint32_t        MaxTextures        = 128;
        uint32_t        MaxNumFonts        = 16;
        float           Width              = 1024;
        float           Height             = 1024;
        uint32_t        MaxPipelineWidgets = 16;
    };

    class ClayRenderer;

    class ClayRenderBatch final : public IRenderBatch
    {
        ClayRenderer *m_renderer;

    public:
        explicit ClayRenderBatch( ClayRenderer *renderer ) : m_renderer( renderer )
        {
        }

        void     AddVertices( const InteropArray<UIVertex> &vertices, const InteropArray<uint32_t> &indices ) override;
        uint32_t GetCurrentVertexOffset( ) const override;
    };

    class ClayRenderer
    {
        struct UIUniforms
        {
            DirectX::XMFLOAT4X4 Projection;
            DirectX::XMFLOAT4   ScreenSize; // xy: screen dimensions, zw: unused
            DirectX::XMFLOAT4   FontParams; // x: atlas width, y: atlas height, z: pixel range, w: unused
        };

        ClayRendererDesc m_desc;
        ILogicalDevice  *m_logicalDevice = nullptr;
        ClayContext     *m_clayContext   = nullptr;
        ClayTextCache   *m_clayText      = nullptr;

        std::unique_ptr<ShaderProgram>  m_shaderProgram;
        std::unique_ptr<IPipeline>      m_pipeline;
        std::unique_ptr<IRootSignature> m_rootSignature;
        std::unique_ptr<IInputLayout>   m_inputLayout;

        std::unique_ptr<FullscreenQuadPipeline> m_fullscreenQuad;

        struct FrameData
        {
            std::unique_ptr<IResourceBindGroup> ConstantsBindGroup;
            std::unique_ptr<IResourceBindGroup> TextureBindGroup;
            std::unique_ptr<ITextureResource>   ColorTarget;
            std::unique_ptr<ITextureResource>   DepthBuffer;
            ICommandList                       *CommandList = nullptr;
            std::unique_ptr<IFence>             FrameFence;
        };
        std::vector<FrameData> m_frameData;

        std::unique_ptr<ICommandQueue>    m_commandQueue;
        std::unique_ptr<ICommandListPool> m_commandListPool;

        std::unique_ptr<IBufferResource> m_vertexBuffer;
        std::unique_ptr<IBufferResource> m_indexBuffer;
        uint8_t                         *m_vertexBufferData = nullptr;
        uint8_t                         *m_indexBufferData  = nullptr;

        InteropArray<UIVertex> m_batchedVertices;
        InteropArray<uint32_t> m_batchedIndices;
        float                  m_currentDepth  = 0.9f;
        static constexpr float DEPTH_INCREMENT = -0.0001f;
        struct ScissorState
        {
            bool  Enabled = false;
            float X       = 0;
            float Y       = 0;
            float Width   = 0;
            float Height  = 0;
        };
        struct DrawBatch
        {
            uint32_t     VertexOffset;
            uint32_t     IndexOffset;
            uint32_t     IndexCount;
            ScissorState Scissor;
        };
        std::vector<DrawBatch> m_drawBatches;
        uint32_t               m_totalVertexCount = 0;
        uint32_t               m_totalIndexCount  = 0;

        std::unique_ptr<IBufferResource> m_uniformBuffer;
        UIUniforms                      *m_uniformBufferData  = nullptr;
        uint32_t                         m_alignedUniformSize = 0;

        mutable UIShapeCache m_shapeCache;
        mutable uint32_t     m_currentFrame = 0;

        std::unordered_map<void *, uint32_t> m_imageTextureIndices;
        std::vector<ITextureResource *>      m_textures;
        std::unique_ptr<ITextureResource>    m_nullTexture;
        bool                                 m_texturesDirty    = true;

        float               m_viewportWidth  = 0;
        float               m_viewportHeight = 0;
        float               m_dpiScale       = 1.0f;
        float               m_deltaTime      = 0.016f;
        DirectX::XMFLOAT4X4 m_projectionMatrix;

        std::vector<ScissorState> m_scissorStack;
        std::unique_ptr<ISampler> m_linearSampler;
        ResourceTracking          m_resourceTracking;
        uint32_t                  m_currentFrameIndex = 0;

        std::unordered_map<uint32_t, Widget *> m_widgets;
        
        // Pipeline widget resources
        struct PipelineWidgetData
        {
            ICommandList                       *CommandList = nullptr;
            std::unique_ptr<ISemaphore>         Semaphore;
        };
        std::vector<PipelineWidgetData>        m_pipelineWidgetData;
        std::vector<Widget *>                  m_pipelineWidgetsToRender;
        std::unique_ptr<ICommandListPool>      m_pipelineWidgetCommandListPool;

    public:
        explicit ClayRenderer( const ClayRendererDesc &desc );
        ~ClayRenderer( );

        void Resize( float width, float height );
        void SetDpiScale( float dpiScale );
        void SetDeltaTime( float deltaTime );
        void Render( ICommandList *commandList, Clay_RenderCommandArray commands, uint32_t frameIndex );

        void           ClearCaches( );
        ClayDimensions MeasureText( const InteropString &text, const Clay_TextElementConfig &desc ) const;

        void RegisterWidget( uint32_t id, Widget *widget );
        void UnregisterWidget( uint32_t id );

        void     AddVerticesWithDepth( const InteropArray<UIVertex> &vertices, const InteropArray<uint32_t> &indices );
        uint32_t GetCurrentVertexCount( ) const;

    private:
        void CreateShaderProgram( );
        void CreatePipeline( );
        void CreateBuffers( );
        void CreateNullTexture( );
        void CreateRenderTargets( );
        void UpdateProjectionMatrix( );

        void RenderInternal( ICommandList *commandList, Clay_RenderCommandArray commands, uint32_t frameIndex );
        void ProcessRenderCommand( const Clay_RenderCommand *command, ICommandList *commandList );
        void RenderRectangle( const Clay_RenderCommand *command, ICommandList *commandList );
        void RenderBorder( const Clay_RenderCommand *command );
        void RenderText( const Clay_RenderCommand *command, ICommandList *commandList );
        void RenderSingleLineText( const Clay_RenderCommand *command, uint16_t fontId, float effectiveScale, float fontAscent );
        void RenderImage( const Clay_RenderCommand *command );
        void RenderCustom( const Clay_RenderCommand *command, ICommandList *commandList );
        void SetScissor( const Clay_RenderCommand *command );
        void ClearScissor( );
        void FlushBatchedGeometry( ICommandList *commandList );
        void FlushCurrentBatch( );
        void ExecuteDrawBatches( ICommandList *commandList ) const;

        uint32_t RegisterTexture( ITextureResource *texture );
        void     UpdateTextureBindings( uint32_t frameIndex ) const;
        void     SyncFontTexturesFromClayText( );
    };

} // namespace DenOfIz
