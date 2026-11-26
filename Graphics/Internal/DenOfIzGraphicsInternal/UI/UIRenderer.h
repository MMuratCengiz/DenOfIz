/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <array>
#include <vector>
#include "DenOfIzGraphics/Backends/Interface/CommandList.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Support/ResourceTracking.h"
#include "DenOfIzGraphicsInternal/Assets/Font/GlyphBuffers.h"
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
        };
        static_assert( sizeof( Uniforms ) == 80 && offsetof( Uniforms, ScreenSize ) == 64 );

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
        };

        struct DrawState
        {
            uint32_t                                Record = 0;
            UILayerCommand                          Layer{ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
            std::array<uint32_t, UIMaxRoundedClips> Clips{ };
            uint32_t                                ClipCount = 0;
        };

        enum Stream : uint32_t
        {
            Vertices,
            Quads,
            Indices,
            States,
            NumStreams
        };

        struct FrameStream
        {
            DenOfIz_Buffer Buffer   = DENOFIZ_NULL_HANDLE;
            uint8_t       *Data     = nullptr;
            uint32_t       Capacity = 0;
        };

        struct FrameData
        {
            std::array<FrameStream, NumStreams>                    Streams{ };
            DenOfIz_BindGroup                                      Constants      = DENOFIZ_NULL_HANDLE;
            bool                                                   ConstantsStale  = true;
            uint32_t                                               GlyphGeneration = 0;
            DenOfIz_BindGroup                                      Textures       = DENOFIZ_NULL_HANDLE;
            std::array<DenOfIz_Texture, UITextureSlots::Count>     Bound{ };
            bool                                                   BoundValid     = false;
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
        GlyphBuffers                         m_glyphBuffers;

        FrameData                                         *m_target      = nullptr;
        uint32_t                                           m_vertexCount = 0;
        uint32_t                                           m_quadCount   = 0;
        uint32_t                                           m_indexCount  = 0;
        std::vector<Batch>                                 m_batches;
        std::vector<Scissor>                               m_clipStack;
        std::vector<DrawState>                             m_stateStack;
        uint32_t                                           m_recordCount = 0;
        uint32_t                                           m_stateBits   = 0;
        std::array<DenOfIz_Texture, UITextureSlots::Count> m_slots{ };
        uint32_t                                           m_nextImageSlot = UITextureSlots::FirstImage;
        bool                                               m_slotsExhaustedReported = false;
        bool                                               m_geometryExhaustedReported = false;
        UIShapeCache                                       m_shapes;
        uint64_t                                           m_frameCounter = 0;
        float                                              m_pixelsPerPoint = 1.0f;
        DenOfIz_UIRect                                     m_clipPoints{ };
        DenOfIz_Texture                                    m_captureTarget   = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer                                     m_captureReadback = DENOFIZ_NULL_HANDLE;
        uint32_t                                           m_captureWidth    = 0;
        uint32_t                                           m_captureHeight   = 0;

    public:
        explicit UIRenderer( const UIRendererDesc &desc );
        ~UIRenderer( );

        void Render( const std::vector<UICommand> &commands, uint32_t frameIndex, DenOfIz_CommandList commandList, float pixelWidth, float pixelHeight, float pixelsPerPoint );
        void RenderToTexture( const std::vector<UICommand> &commands, uint32_t frameIndex, uint32_t pixelWidth, uint32_t pixelHeight, float pixelsPerPoint,
                              const DenOfIz_SemaphoreArray *waitSemaphores, DenOfIz_UIRenderResult *outResult );
        bool Capture( const std::vector<UICommand> &commands, uint32_t frameIndex, uint32_t pixelWidth, uint32_t pixelHeight, float pixelsPerPoint,
                      const DenOfIz_SemaphoreArray *waitSemaphores, DenOfIz_ByteArray *outPixels );

    private:
        DenOfIz_Pipeline CreatePipeline( DenOfIz_Format depthFormat ) const;
        void             CreateFrameResources( );
        void             EnsureCaptureTarget( uint32_t width, uint32_t height );
        void             DestroyCaptureTarget( );
        void             Reserve( FrameData &frame, Stream stream, uint32_t count, uint32_t keep ) const;
        template <typename T>
        T               *StreamData( Stream stream, uint32_t first ) const;
        void             BindFrameConstants( uint32_t frameIndex );
        uint32_t         AllocateRecord( );
        void             PushState( const DrawState &state );
        UIVertex        *AllocateVertices( uint32_t count );
        UIQuad          *AllocateQuad( );
        uint32_t        *AllocateIndices( uint32_t count );
        void             EnsureColorTarget( FrameData &frame, uint32_t width, uint32_t height ) const;

        void     Build( const std::vector<UICommand> &commands, uint32_t frameIndex, float pixelWidth, float pixelHeight, float pixelsPerPoint );
        void     Upload( uint32_t frameIndex, float pixelWidth, float pixelHeight );
        void     Record( uint32_t frameIndex, DenOfIz_CommandList commandList, DenOfIz_Pipeline pipeline, float pixelWidth, float pixelHeight );
        uint32_t SlotOf( DenOfIz_Texture texture );
        void     SetScissor( const Scissor &scissor );
        void     EmitQuad( float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, const DenOfIz_UIColor &color, uint32_t slot );
        void     EmitShape( const UIShape &shape, const DenOfIz_UIRect &rect, const DenOfIz_UIColor &color, uint32_t slot, const DenOfIz_Float4 *uv );
        void     EmitText( const UICommand &command );
        DenOfIz_Float4 ShadeColor( const DenOfIz_UIColor &color ) const;
    };
} // namespace DenOfIz
