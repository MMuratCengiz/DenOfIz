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

#include <clay.h>
#include <unordered_map>
#include <vector>
#include "ClayContext.h"
#include "ClayTextCache.h"
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/UI/Clay.h"
#include "UIShapeCache.h"

#include "DenOfIzGraphics/Support/ResourceTracking.h"

namespace DenOfIz
{

    struct ClayRendererDesc
    {
        DenOfIz_LogicalDevice    LogicalDevice      = DENOFIZ_NULL_HANDLE;
        DenOfIz_ResourceTracking ResourceTracking   = DENOFIZ_NULL_HANDLE;
        ClayContext             *Context            = nullptr;
        DenOfIz_Format           RenderTargetFormat = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
        uint32_t                 NumFrames          = 3;
        uint32_t                 MaxVertices        = 65536;
        uint32_t                 MaxIndices         = 98304;
        uint32_t                 MaxTextures        = 128;
        uint32_t                 MaxNumFonts        = 16;
        float                    Width              = 1024;
        float                    Height             = 1024;
    };

    class ClayRenderer;

    class ClayRenderBatch final : public IRenderBatch
    {
        ClayRenderer *m_renderer;

    public:
        explicit ClayRenderBatch( ClayRenderer *renderer ) : m_renderer( renderer )
        {
        }

        void     AddVertices( const UIVertexArray &vertices, const DenOfIz_UInt32Array &indices ) override;
        uint32_t GetCurrentVertexOffset( ) const override;
    };

    class ClayRenderer
    {
        struct UIUniforms
        {
            DenOfIz_Float4x4 Projection;
            DenOfIz_Float4   ScreenSize;
            DenOfIz_Float4   FontParams;
        };

        ClayRendererDesc      m_desc;
        DenOfIz_LogicalDevice m_logicalDevice = DENOFIZ_NULL_HANDLE;
        ClayContext          *m_clayContext   = nullptr;
        ClayTextCache        *m_clayText      = nullptr;

        DenOfIz_ShaderProgram                m_shaderProgram = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                     m_pipeline      = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                m_rootSignature = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout                  m_inputLayout   = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> m_bindGroupLayouts;

        struct FrameData
        {
            DenOfIz_BindGroup                    ConstantsBindGroup = DENOFIZ_NULL_HANDLE;
            DenOfIz_BindGroup                    TextureBindGroup   = DENOFIZ_NULL_HANDLE;
            DenOfIz_Texture                      ColorTarget        = DENOFIZ_NULL_HANDLE;
            DenOfIz_Texture                      DepthBuffer        = DENOFIZ_NULL_HANDLE;
            DenOfIz_CommandList                  CommandList        = DENOFIZ_NULL_HANDLE;
            DenOfIz_Fence                        FrameFence         = DENOFIZ_NULL_HANDLE;
            DenOfIz_Semaphore                    FrameSemaphore     = DENOFIZ_NULL_HANDLE;
            bool                                 AreTexturesDirty   = true;
            std::vector<DenOfIz_Texture>         Textures;
            std::unordered_map<void *, uint32_t> ImageTextureIndices;
        };
        std::vector<FrameData> m_frameData;

        DenOfIz_CommandQueue    m_commandQueue    = DENOFIZ_NULL_HANDLE;
        DenOfIz_CommandListPool m_commandListPool = DENOFIZ_NULL_HANDLE;

        DenOfIz_Buffer m_vertexBuffer     = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer m_indexBuffer      = DENOFIZ_NULL_HANDLE;
        uint8_t       *m_vertexBufferData = nullptr;
        uint8_t       *m_indexBufferData  = nullptr;

        std::vector<UIVertex>  m_batchedVertices;
        std::vector<uint32_t>  m_batchedIndices;
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
            uint32_t     TextureIndex;
        };
        std::vector<DrawBatch> m_drawBatches;
        uint32_t               m_currentBatchTextureIndex = 0;
        uint32_t               m_totalVertexCount         = 0;
        uint32_t               m_totalIndexCount          = 0;

        DenOfIz_Buffer m_uniformBuffer      = DENOFIZ_NULL_HANDLE;
        UIUniforms    *m_uniformBufferData  = nullptr;
        uint32_t       m_alignedUniformSize = 0;

        mutable UIShapeCache m_shapeCache;
        mutable uint32_t     m_currentFrame = 0;

        DenOfIz_Texture m_nullTexture   = DENOFIZ_NULL_HANDLE;
        bool            m_texturesDirty = true;

        float            m_viewportWidth  = 0;
        float            m_viewportHeight = 0;
        float            m_dpiScale       = 1.0f;
        float            m_deltaTime      = 0.016f;
        DenOfIz_Float4x4 m_projectionMatrix;

        std::vector<ScissorState> m_scissorStack;
        DenOfIz_Sampler           m_linearSampler        = DENOFIZ_NULL_HANDLE;
        DenOfIz_ResourceTracking  m_resourceTracking     = DENOFIZ_NULL_HANDLE;
        bool                      m_ownsResourceTracking = false;
        uint32_t                  m_currentFrameIndex    = 0;

        bool                           m_supportsSrvArray = true;
        std::vector<DenOfIz_BindGroup> m_perTextureBindGroups;

    public:
        explicit ClayRenderer( const ClayRendererDesc &desc );
        ~ClayRenderer( );

        void Resize( float width, float height );
        void SetDeltaTime( float deltaTime );
        void Render( Clay_RenderCommandArray commands, uint32_t frameIndex, DenOfIz_Texture *outTexture, DenOfIz_Semaphore *outSemaphore );
        void RenderToCommandList( Clay_RenderCommandArray commands, uint32_t frameIndex, DenOfIz_CommandList commandList );

        void                   ClearCaches( );
        DenOfIz_ClayDimensions MeasureText( const DenOfIz_StringView &text, const Clay_TextElementConfig &desc ) const;

        void     AddVerticesWithDepth( const UIVertexArray &vertices, const DenOfIz_UInt32Array &indices );
        uint32_t GetCurrentVertexCount( ) const;

    private:
        void CreateShaderProgram( );
        void CreatePipeline( );
        void CreateBuffers( );
        void CreateNullTexture( );
        void CreateRenderTargets( );
        void UpdateProjectionMatrix( );

        void RenderInternal( Clay_RenderCommandArray commands, uint32_t frameIndex, DenOfIz_Texture *outTexture, DenOfIz_Semaphore *outSemaphore );
        void ProcessRenderCommand( const Clay_RenderCommand *command, DenOfIz_CommandList commandList, uint32_t frameIndex );
        void RenderRectangle( const Clay_RenderCommand *command, DenOfIz_CommandList commandList );
        void RenderBorder( const Clay_RenderCommand *command );
        void RenderText( const Clay_RenderCommand *command, DenOfIz_CommandList commandList );
        void RenderSingleLineText( const Clay_RenderCommand *command, uint16_t fontId, float effectiveScale, float fontAscent );
        void RenderImage( const Clay_RenderCommand *command, uint32_t frameIndex );
        void RenderCustom( const Clay_RenderCommand *command, uint32_t frameIndex );
        void SetScissor( const Clay_RenderCommand *command );
        void ClearScissor( );
        void FlushBatchedGeometry( DenOfIz_CommandList commandList );
        void FlushCurrentBatch( );
        void ExecuteDrawBatches( DenOfIz_CommandList commandList ) const;

        uint32_t RegisterTexture( DenOfIz_Texture texture, uint32_t frameIndex );
        void     UpdateTextureBindings( uint32_t frameIndex ) const;
        void     SyncFontTexturesFromClayText( );
        void     AddVerticesWithDepthVec( std::vector<UIVertex> &vertices, std::vector<uint32_t> &indices );
    };

} // namespace DenOfIz
