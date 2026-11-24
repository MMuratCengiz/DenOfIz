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

#include <array>
#include <vector>
#include "DenOfIzGraphics/Backends/Interface/CommandList.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Support/ResourceTracking.h"
#include "DenOfIzGraphicsInternal/UI/UIGeometry.h"
#include "DenOfIzGraphicsInternal/UI/UIText.h"
#include "DenOfIzGraphicsInternal/UI/UITypes.h"

namespace DenOfIz
{
    struct UIRendererDesc
    {
        DenOfIz_LogicalDevice    LogicalDevice      = DENOFIZ_NULL_HANDLE;
        DenOfIz_ResourceTracking ResourceTracking   = DENOFIZ_NULL_HANDLE;
        DenOfIz_Format           RenderTargetFormat = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
        DenOfIz_Format           DepthFormat        = DENOFIZ_FORMAT_UNDEFINED;
        uint32_t                 NumFrames          = 3;
        UIText                  *Text               = nullptr;
    };

    class UIRenderer
    {
        struct Uniforms
        {
            DenOfIz_Float4x4 Projection;
            DenOfIz_Float4   ScreenSize;
            DenOfIz_Float4   FontParams;
        };

        struct Scissor
        {
            int32_t X      = 0;
            int32_t Y      = 0;
            int32_t Width  = 0;
            int32_t Height = 0;
        };

        struct Batch
        {
            uint32_t FirstIndex = 0;
            uint32_t IndexCount = 0;
            Scissor  Clip;
            uint32_t Slot = 0;
        };

        struct DrawState
        {
            uint32_t                                Record = 0;
            UILayerCommand                          Layer{ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
            std::array<uint32_t, UIMaxRoundedClips> Clips{ };
            uint32_t                                ClipCount = 0;
        };

        struct FrameData
        {
            DenOfIz_Buffer                                         VertexBuffer   = DENOFIZ_NULL_HANDLE;
            DenOfIz_Buffer                                         IndexBuffer    = DENOFIZ_NULL_HANDLE;
            uint8_t                                               *VertexData     = nullptr;
            uint8_t                                               *IndexData      = nullptr;
            uint32_t                                               VertexCapacity = 0;
            uint32_t                                               IndexCapacity  = 0;
            DenOfIz_Buffer                                         StateBuffer    = DENOFIZ_NULL_HANDLE;
            UIStateRecord                                         *StateData      = nullptr;
            uint32_t                                               StateCapacity  = 0;
            DenOfIz_BindGroup                                      Constants      = DENOFIZ_NULL_HANDLE;
            DenOfIz_BindGroup                                      Textures       = DENOFIZ_NULL_HANDLE;
            std::array<DenOfIz_Texture, UITextureSlots::Count>     Bound{ };
            bool                                                   BoundValid     = false;
            std::array<DenOfIz_BindGroup, UITextureSlots::Count>   SlotGroups{ };
            std::array<DenOfIz_Texture, UITextureSlots::Count>     SlotTextures{ };
            DenOfIz_Texture                                        ColorTarget    = DENOFIZ_NULL_HANDLE;
            uint32_t                                               TargetWidth    = 0;
            uint32_t                                               TargetHeight   = 0;
            DenOfIz_CommandList                                    CommandList    = DENOFIZ_NULL_HANDLE;
            DenOfIz_Fence                                          Fence          = DENOFIZ_NULL_HANDLE;
            DenOfIz_Semaphore                                      Semaphore      = DENOFIZ_NULL_HANDLE;
            bool                                                   Submitted      = false;
        };

        UIRendererDesc           m_desc;
        DenOfIz_LogicalDevice    m_device           = DENOFIZ_NULL_HANDLE;
        DenOfIz_ResourceTracking m_resourceTracking = DENOFIZ_NULL_HANDLE;
        bool                     m_ownsTracking     = false;
        bool                     m_supportsSrvArray = true;

        DenOfIz_ShaderProgram                m_shaderProgram    = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature                m_rootSignature    = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout                  m_inputLayout      = DENOFIZ_NULL_HANDLE;
        std::vector<DenOfIz_BindGroupLayout> m_bindGroupLayouts;
        DenOfIz_Pipeline                     m_pipeline         = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline                     m_texturePipeline  = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer                       m_uniformBuffer    = DENOFIZ_NULL_HANDLE;
        uint8_t                             *m_uniformData      = nullptr;
        uint32_t                             m_alignedUniforms  = 0;
        DenOfIz_Sampler                      m_sampler          = DENOFIZ_NULL_HANDLE;
        DenOfIz_Texture                      m_nullTexture      = DENOFIZ_NULL_HANDLE;
        DenOfIz_CommandQueue                 m_queue            = DENOFIZ_NULL_HANDLE;
        DenOfIz_CommandListPool              m_commandListPool  = DENOFIZ_NULL_HANDLE;
        std::vector<FrameData>               m_frames;

        FrameData                                         *m_target      = nullptr;
        uint32_t                                           m_targetIndex = 0;
        uint32_t                                           m_vertexCount = 0;
        uint32_t                                           m_indexCount  = 0;
        std::vector<Batch>                                 m_batches;
        std::vector<Scissor>                               m_clipStack;
        std::vector<DrawState>                             m_stateStack;
        uint32_t                                           m_recordCount = 0;
        uint32_t                                           m_stateBits   = 0;
        std::array<DenOfIz_Texture, UITextureSlots::Count> m_slots{ };
        uint32_t                                           m_nextImageSlot = UITextureSlots::FirstImage;
        bool                                               m_slotsExhaustedReported = false;
        UIShapeCache                                       m_shapes;
        uint64_t                                           m_frameCounter = 0;
        float                                              m_pixelsPerPoint = 1.0f;
        DenOfIz_UIRect                                     m_clipPoints{ };

    public:
        explicit UIRenderer( const UIRendererDesc &desc );
        ~UIRenderer( );

        void Render( const std::vector<UICommand> &commands, uint32_t frameIndex, DenOfIz_CommandList commandList, float pixelWidth, float pixelHeight, float pixelsPerPoint );
        void RenderToTexture( const std::vector<UICommand> &commands, uint32_t frameIndex, uint32_t pixelWidth, uint32_t pixelHeight, float pixelsPerPoint,
                              const DenOfIz_SemaphoreArray *waitSemaphores, DenOfIz_UIRenderResult *outResult );

    private:
        DenOfIz_Pipeline CreatePipeline( DenOfIz_Format depthFormat ) const;
        void             CreateFrameResources( );
        void             EnsureGeometryCapacity( FrameData &frame, uint32_t vertexCount, uint32_t indexCount, uint32_t keepVertices, uint32_t keepIndices ) const;
        void             EnsureStateCapacity( uint32_t frameIndex, uint32_t recordCount, uint32_t keepRecords );
        void             BindFrameConstants( uint32_t frameIndex ) const;
        uint32_t         AllocateRecord( );
        void             PushState( const DrawState &state );
        UIVertex        *AllocateVertices( uint32_t count );
        uint32_t        *AllocateIndices( uint32_t count );
        void             EnsureColorTarget( FrameData &frame, uint32_t width, uint32_t height ) const;

        void     Build( const std::vector<UICommand> &commands, uint32_t frameIndex, float pixelWidth, float pixelHeight, float pixelsPerPoint );
        void     Upload( uint32_t frameIndex, float pixelWidth, float pixelHeight );
        void     Record( uint32_t frameIndex, DenOfIz_CommandList commandList, DenOfIz_Pipeline pipeline, float pixelWidth, float pixelHeight );
        uint32_t SlotOf( DenOfIz_Texture texture );
        void     SetScissor( const Scissor &scissor );
        void     StartBatch( uint32_t slot );
        void     EmitQuad( float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, const DenOfIz_UIColor &color, uint32_t slot );
        void     EmitShape( const UIShape &shape, const DenOfIz_UIRect &rect, const DenOfIz_UIColor &color, uint32_t slot, const DenOfIz_Float4 *uv );
        void     EmitText( const UICommand &command );
        DenOfIz_Float4 ShadeColor( const DenOfIz_UIColor &color ) const;
    };
} // namespace DenOfIz
