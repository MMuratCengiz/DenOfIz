/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <imgui.h>
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphics/Backends/Interface/BindGroup.h"
#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/CommandList.h"
#include "DenOfIzGraphics/Backends/Interface/CommandListPool.h"
#include "DenOfIzGraphics/Backends/Interface/CommandQueue.h"
#include "DenOfIzGraphics/Backends/Interface/Fence.h"
#include "DenOfIzGraphics/Backends/Interface/InputLayout.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/Pipeline.h"
#include "DenOfIzGraphics/Backends/Interface/RootSignature.h"
#include "DenOfIzGraphics/Backends/Interface/Texture.h"
#include "DenOfIzGraphics/Input/Event.h"
#include "DenOfIzGraphics/Support/ResourceTracking.h"

namespace DenOfIz
{
    struct ImGuiBackendDesc
    {
        DenOfIz_LogicalDevice LogicalDevice      = DENOFIZ_NULL_HANDLE;
        DenOfIz_Format        RenderTargetFormat = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
        uint32_t              NumFrames          = 3;
        uint32_t              MaxVertices        = 1310720;
        uint32_t              MaxIndices         = 1310720;
        uint32_t              MaxTextures        = 128;
        DenOfIz_Viewport      Viewport{ };
    };

    struct ImGuiUniforms
    {
        DenOfIz_Float4x4 Projection;
        DenOfIz_Float4   ScreenSize;
    };
    static_assert( sizeof( ImGuiUniforms ) == 80 && offsetof( ImGuiUniforms, ScreenSize ) == 64 );

    struct PixelConstants
    {
        uint32_t TextureIndex;
        uint32_t Padding;
    };
    static_assert( sizeof( PixelConstants ) == 8 );

    struct ImGuiFrameData
    {
        DenOfIz_BindGroup   ConstantsBindGroup = DENOFIZ_NULL_HANDLE;
        DenOfIz_BindGroup   TextureBindGroup   = DENOFIZ_NULL_HANDLE;
        DenOfIz_CommandList CommandList        = DENOFIZ_NULL_HANDLE;
        DenOfIz_Fence       FrameFence         = DENOFIZ_NULL_HANDLE;
    };

    struct ImGuiBackend
    {
        ImGuiBackendDesc      Desc;
        DenOfIz_LogicalDevice LogicalDevice = DENOFIZ_NULL_HANDLE;

        DenOfIz_ShaderProgram    ShaderProgram       = DENOFIZ_NULL_HANDLE;
        DenOfIz_Pipeline         Pipeline            = DENOFIZ_NULL_HANDLE;
        DenOfIz_RootSignature    RootSignature       = DENOFIZ_NULL_HANDLE;
        DenOfIz_InputLayout      InputLayout         = DENOFIZ_NULL_HANDLE;
        DenOfIz_BindGroupLayout *BindGroupLayouts    = nullptr;
        uint32_t                 NumBindGroupLayouts = 0;

        ImGuiFrameData *FrameData    = nullptr;
        uint32_t        NumFrameData = 0;

        DenOfIz_CommandQueue    CommandQueue    = DENOFIZ_NULL_HANDLE;
        DenOfIz_CommandListPool CommandListPool = DENOFIZ_NULL_HANDLE;

        DenOfIz_Buffer VertexBuffer     = DENOFIZ_NULL_HANDLE;
        DenOfIz_Buffer IndexBuffer      = DENOFIZ_NULL_HANDLE;
        uint8_t       *VertexBufferData = nullptr;
        uint8_t       *IndexBufferData  = nullptr;

        DenOfIz_Buffer UniformBuffer      = DENOFIZ_NULL_HANDLE;
        ImGuiUniforms *UniformBufferData  = nullptr;
        uint32_t       AlignedUniformSize = 0;

        DenOfIz_Buffer     PixelConstantsBuffer        = DENOFIZ_NULL_HANDLE;
        PixelConstants    *PixelConstantsData          = nullptr;
        uint32_t           AlignedPixelConstantsSize   = 0;
        DenOfIz_BindGroup *PixelConstantsBindGroups    = nullptr;
        uint32_t           NumPixelConstantsBindGroups = 0;

        DenOfIz_Texture *Textures      = nullptr;
        uint32_t         NumTextures   = 0;
        DenOfIz_Texture  FontTexture   = DENOFIZ_NULL_HANDLE;
        DenOfIz_Texture  NullTexture   = DENOFIZ_NULL_HANDLE;
        bool             TexturesDirty = true;

        DenOfIz_Viewport Viewport;
        DenOfIz_Float4x4 ProjectionMatrix{ };

        DenOfIz_Sampler Sampler          = DENOFIZ_NULL_HANDLE;
        uint32_t        NextFrame        = 0;
        uint32_t        CurrentFrame     = 0;
        uint32_t        NumFrames        = 0;
        bool            TextInputActive  = false;
    };

    ImGuiBackend           *ImGuiBackend_Create( const ImGuiBackendDesc &desc );
    void                    ImGuiBackend_Destroy( ImGuiBackend *backend );
    void                    ImGuiBackend_SetViewport( ImGuiBackend *backend, const DenOfIz_Viewport &viewport );
    const DenOfIz_Viewport &ImGuiBackend_GetViewport( const ImGuiBackend *backend );
    void                    ImGuiBackend_RenderDrawData( ImGuiBackend *backend, DenOfIz_CommandList commandList, ImDrawData *drawData, uint32_t frameIndex );
    void                    ImGuiBackend_RecreateFonts( ImGuiBackend *backend );
    ImTextureID             ImGuiBackend_AddTexture( ImGuiBackend *backend, DenOfIz_Texture texture );
    void                    ImGuiBackend_RemoveTexture( ImGuiBackend *backend, ImTextureID textureId );
    void                    ImGuiBackend_ProcessEvent( const ImGuiBackend *backend, const DenOfIz_Event &event );
    void                    ImGuiBackend_UpdateTextInputState( ImGuiBackend *backend );

    struct ImGuiRenderer
    {
        ImGuiBackend *Backend = nullptr;
    };

    ImGuiRenderer *ImGuiRenderer_Create( const ImGuiBackendDesc &desc );
    void           ImGuiRenderer_Destroy( ImGuiRenderer *renderer );
    void           ImGuiRenderer_ProcessEvent( const ImGuiRenderer *renderer, const DenOfIz_Event &ev );
    void           ImGuiRenderer_NewFrame( const ImGuiRenderer *renderer, uint32_t width, uint32_t height, float deltaTime );
    void           ImGuiRenderer_Render( const ImGuiRenderer *renderer, DenOfIz_Texture renderTarget, DenOfIz_CommandList commandList, uint32_t frameIndex );
    void           ImGuiRenderer_SetViewport( const ImGuiRenderer *renderer, DenOfIz_Viewport viewport );
    void           ImGuiRenderer_RecreateFonts( const ImGuiRenderer *renderer );
    ImTextureID    ImGuiRenderer_AddTexture( const ImGuiRenderer *renderer, DenOfIz_Texture texture );
    void           ImGuiRenderer_RemoveTexture( const ImGuiRenderer *renderer, ImTextureID textureId );

} // namespace DenOfIz
