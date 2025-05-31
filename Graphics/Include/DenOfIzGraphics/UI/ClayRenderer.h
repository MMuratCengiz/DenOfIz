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
#include <DenOfIzGraphics/Assets/Font/TextLayoutCache.h>
#include <DenOfIzGraphics/Backends/Common/ShaderProgram.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandListPool.h>
#include <DenOfIzGraphics/Backends/Interface/ICommandQueue.h>
#include <DenOfIzGraphics/Backends/Interface/IFence.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/IResourceBindGroup.h>
#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include <DenOfIzGraphics/Backends/Interface/ISemaphore.h>
#include <DenOfIzGraphics/UI/ClayData.h>
#include <DenOfIzGraphics/UI/FullscreenQuadPipeline.h>
#include <DenOfIzGraphics/UI/UIShapeCache.h>
#include <DenOfIzGraphics/UI/UIShapes.h>
#include <DenOfIzGraphics/UI/UITextVertexCache.h>
#include <clay.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphics/Renderer/Sync/ResourceTracking.h"

#include <DirectXMath.h>

namespace DenOfIz
{
    struct ClayResizableContainerRenderData;
    struct ClayDockableContainerRenderData;

    struct ClayRendererDesc
    {
        ILogicalDevice *LogicalDevice      = nullptr;
        Format          RenderTargetFormat = Format::B8G8R8A8Unorm;
        uint32_t        NumFrames          = 3;
        uint32_t        MaxVertices        = 65536;
        uint32_t        MaxIndices         = 98304;
        uint32_t        MaxTextures        = 128;
        float           Width              = 1024;
        float           Height             = 1024;
    };

    class ClayRenderer
    {
        struct FontData
        {
            Font                                     *FontPtr = nullptr;
            std::unique_ptr<ITextureResource>         Atlas;
            uint32_t                                  TextureIndex = 0;
            InteropArray<std::unique_ptr<TextLayout>> TextLayouts;
            uint32_t                                  CurrentLayoutIndex = 0;
        };

        struct UIUniforms
        {
            XMFLOAT4X4 Projection;
            XMFLOAT4   ScreenSize; // xy: screen dimensions, zw: unused
            XMFLOAT4   FontParams; // x: atlas width, y: atlas height, z: pixel range, w: unused
        };

        ClayRendererDesc m_desc;
        ILogicalDevice  *m_logicalDevice = nullptr;

        std::unique_ptr<ShaderProgram>  m_shaderProgram;
        std::unique_ptr<IPipeline>      m_pipeline;
        std::unique_ptr<IRootSignature> m_rootSignature;
        std::unique_ptr<IInputLayout>   m_inputLayout;

        std::unique_ptr<FullscreenQuadPipeline> m_fullscreenQuad; // Todo this needs to manage multiple frames in flight

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

        std::unordered_map<uint16_t, FontData> m_fonts;

        mutable TextLayoutCache   m_textLayoutCache;
        mutable UIShapeCache      m_shapeCache;
        mutable UITextVertexCache m_textVertexCache;
        mutable uint32_t          m_currentFrame = 0;

        std::unordered_map<void *, uint32_t> m_imageTextureIndices;
        std::vector<ITextureResource *>      m_textures;
        std::vector<bool>                    m_textureFontFlags; // true if texture is a font atlas
        std::unique_ptr<ITextureResource>    m_nullTexture;
        uint32_t                             m_nextTextureIndex = 1;
        bool                                 m_texturesDirty    = true;

        float      m_viewportWidth  = 0;
        float      m_viewportHeight = 0;
        float      m_dpiScale       = 1.0f;
        float      m_deltaTime      = 0.016f; // Default to 60 FPS
        XMFLOAT4X4 m_projectionMatrix;

        std::vector<ScissorState> m_scissorStack;
        std::unique_ptr<ISampler> m_linearSampler;
        ResourceTracking          m_resourceTracking;
        uint32_t                  m_currentFrameIndex = 0;

    public:
        explicit ClayRenderer( const ClayRendererDesc &desc );
        ~ClayRenderer( );

        void AddFont( uint16_t fontId, Font *font );
        void RemoveFont( uint16_t fontId );

        void Resize( float width, float height );
        void SetDpiScale( float dpiScale );
        void SetDeltaTime( float deltaTime );
        void Render( ICommandList *commandList, Clay_RenderCommandArray commands, uint32_t frameIndex );

        void           ClearCaches( );
        ClayDimensions MeasureText( const InteropString &text, const Clay_TextElementConfig &desc ) const;

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
        void RenderSingleLineText( const Clay_RenderCommand *command, const FontData *fontData, float effectiveScale, float fontAscent );
        void RenderImage( const Clay_RenderCommand *command );
        void RenderCustom( const Clay_RenderCommand *command, ICommandList *commandList );
        void RenderTextField( const Clay_RenderCommand *command, const ClayTextFieldRenderData *textFieldData, ICommandList *commandList );
        void RenderCheckbox( const Clay_RenderCommand *command, const ClayCheckboxRenderData *checkboxData, ICommandList *commandList );
        void RenderSlider( const Clay_RenderCommand *command, const ClaySliderRenderData *sliderData, ICommandList *commandList );
        void RenderDropdown( const Clay_RenderCommand *command, const ClayDropdownRenderData *dropdownData, ICommandList *commandList );
        void RenderColorPicker( const Clay_RenderCommand *command, const ClayColorPickerRenderData *colorPickerData, ICommandList *commandList );
        void RenderResizableContainer( const Clay_RenderCommand *command, const ClayResizableContainerRenderData *resizableData, ICommandList *commandList );
        void RenderDockableContainer( const Clay_RenderCommand *command, const ClayDockableContainerRenderData *dockableData, ICommandList *commandList );
        void SetScissor( const Clay_RenderCommand *command );
        void ClearScissor( );

        void AddVerticesWithDepth( const InteropArray<UIVertex> &vertices, const InteropArray<uint32_t> &indices );
        void FlushBatchedGeometry( ICommandList *commandList );
        void FlushCurrentBatch( );                                  // Flush current batch to buffers
        void ExecuteDrawBatches( ICommandList *commandList ) const; // Execute all batched draw calls

        uint32_t RegisterTexture( ITextureResource *texture );
        void     UpdateTextureBindings( uint32_t frameIndex ) const;

        FontData *GetFontData( uint16_t fontId );
        void      InitializeFontAtlas( FontData *fontData );

        TextLayout *GetOrCreateShapedText( const Clay_RenderCommand *command, Font *font ) const;
        TextLayout *GetOrCreateShapedTextDirect( const char *text, size_t length, uint16_t fontId, uint32_t fontSize, Font *font ) const;
        void        CleanupTextLayoutCache( ) const;
    };

} // namespace DenOfIz
