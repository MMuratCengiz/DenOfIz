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

#include "ImGuiBackend.h"

#include <array>
#include <cstring>
#define SPDLOG_NO_EXCEPTIONS
#define FMT_EXCEPTIONS 0
#include <spdlog/spdlog.h>

#include "DenOfIzGraphics/Input/InputSystem.h"
#include "EmbeddedImGuiShaders.h"

using namespace DenOfIz;

namespace
{
    auto ImGuiVertexShaderSource = R"(
struct VSInput
{
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    uint   Color : COLOR0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

cbuffer ImGuiUniforms : register(b0, space1)
{
    float4x4 Projection;
    float4 ScreenSize;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Position = mul(float4(input.Position, 0.0, 1.0), Projection);
    output.TexCoord = input.TexCoord;
    output.Color = float4(
        ((input.Color >> 0) & 0xFF) / 255.0f,
        ((input.Color >> 8) & 0xFF) / 255.0f,
        ((input.Color >> 16) & 0xFF) / 255.0f,
        ((input.Color >> 24) & 0xFF) / 255.0f
    );
    return output;
})";

    auto ImGuiPixelShaderSourceArray = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

cbuffer PixelConstants : register(b0, space2)
{
    uint TextureIndex;
    uint Padding;
};

Texture2D Textures[128] : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = Textures[TextureIndex].Sample(LinearSampler, input.TexCoord);
    return texColor * input.Color;
})";

    auto ImGuiPixelShaderSourceSingle = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

Texture2D Texture : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = Texture.Sample(LinearSampler, input.TexCoord);
    return texColor * input.Color;
})";

    DenOfIz_ByteArray StringToByteArray( const char *str )
    {
        const size_t      len = strlen( str );
        DenOfIz_ByteArray result{ };
        result.Elements    = static_cast<Byte *>( malloc( len ) );
        result.NumElements = static_cast<uint32_t>( len );
        for ( size_t i = 0; i < len; i++ )
        {
            result.Elements[ i ] = static_cast<Byte>( str[ i ] );
        }
        return result;
    }

    void FreeByteArray( DenOfIz_ByteArray *arr )
    {
        if ( arr->Elements )
        {
            free( arr->Elements );
            arr->Elements    = nullptr;
            arr->NumElements = 0;
        }
    }

    void CreateShaderProgram( ImGuiBackend *backend )
    {
        if ( backend->SupportsSrvArray )
        {
            backend->ShaderProgram = EmbeddedImGuiShaders::GetImGuiArrayShaderProgram( );
        }
        else
        {
            backend->ShaderProgram = EmbeddedImGuiShaders::GetImGuiSingleShaderProgram( );
        }
    }

    void CreatePipeline( ImGuiBackend *backend )
    {
        DenOfIz_ShaderReflectDesc reflectDesc{ };
        DenOfIz_ShaderProgram_Reflect( backend->ShaderProgram, &reflectDesc );

        backend->NumBindGroupLayouts = reflectDesc.BindGroupLayouts.NumElements;
        backend->BindGroupLayouts    = static_cast<DenOfIz_BindGroupLayout *>( calloc( backend->NumBindGroupLayouts, sizeof( DenOfIz_BindGroupLayout ) ) );
        for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
        {
            DenOfIz_LogicalDevice_CreateBindGroupLayout( backend->LogicalDevice, &reflectDesc.BindGroupLayouts.Elements[ i ], &backend->BindGroupLayouts[ i ] );
        }

        DenOfIz_RootSignatureDesc rootSigDesc{ };
        rootSigDesc.BindGroupLayouts.Elements    = backend->BindGroupLayouts;
        rootSigDesc.BindGroupLayouts.NumElements = backend->NumBindGroupLayouts;
        rootSigDesc.RootConstants                = reflectDesc.RootConstants;
        DenOfIz_LogicalDevice_CreateRootSignature( backend->LogicalDevice, &rootSigDesc, &backend->RootSignature );
        DenOfIz_LogicalDevice_CreateInputLayout( backend->LogicalDevice, &reflectDesc.InputLayout, &backend->InputLayout );

        DenOfIz_PipelineDesc pipelineDesc{ };
        pipelineDesc.RootSignature = backend->RootSignature;
        pipelineDesc.InputLayout   = backend->InputLayout;
        pipelineDesc.ShaderProgram = backend->ShaderProgram;
        pipelineDesc.BindPoint     = DENOFIZ_BIND_POINT_GRAPHICS;

        pipelineDesc.Graphics.PrimitiveTopology = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
        pipelineDesc.Graphics.CullMode          = DENOFIZ_CULL_MODE_NONE;
        pipelineDesc.Graphics.FillMode          = DENOFIZ_FILL_MODE_SOLID;

        pipelineDesc.Graphics.DepthTest.Enable    = false;
        pipelineDesc.Graphics.DepthTest.CompareOp = DENOFIZ_COMPARE_OP_ALWAYS;
        pipelineDesc.Graphics.DepthTest.Write     = false;

        DenOfIz_RenderTargetDesc renderTarget{ };
        renderTarget.Format                      = backend->Desc.RenderTargetFormat;
        renderTarget.Blend.Enable                = true;
        renderTarget.Blend.SrcBlend              = DENOFIZ_BLEND_SRC_ALPHA;
        renderTarget.Blend.DstBlend              = DENOFIZ_BLEND_INV_SRC_ALPHA;
        renderTarget.Blend.BlendOp               = DENOFIZ_BLEND_OP_ADD;
        renderTarget.Blend.SrcBlendAlpha         = DENOFIZ_BLEND_ONE;
        renderTarget.Blend.DstBlendAlpha         = DENOFIZ_BLEND_INV_SRC_ALPHA;
        renderTarget.Blend.BlendOpAlpha          = DENOFIZ_BLEND_OP_ADD;
        renderTarget.Blend.RenderTargetWriteMask = 0x0F;

        pipelineDesc.Graphics.RenderTargets.Elements    = &renderTarget;
        pipelineDesc.Graphics.RenderTargets.NumElements = 1;

        DenOfIz_LogicalDevice_CreatePipeline( backend->LogicalDevice, &pipelineDesc, &backend->Pipeline );
    }

    void CreateBuffers( ImGuiBackend *backend )
    {
        DenOfIz_BufferDesc vertexBufferDesc{ };
        vertexBufferDesc.NumBytes  = backend->Desc.MaxVertices * sizeof( ImDrawVert );
        vertexBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_VERTEX_BIT;
        vertexBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
        vertexBufferDesc.DebugName = DENOFIZ_STRING( "ImGui Vertex Buffer" );
        DenOfIz_LogicalDevice_CreateBuffer( backend->LogicalDevice, &vertexBufferDesc, &backend->VertexBuffer );

        void *mappedVertexData = nullptr;
        DenOfIz_Buffer_MapMemory( backend->VertexBuffer, &mappedVertexData );
        backend->VertexBufferData = static_cast<uint8_t *>( mappedVertexData );

        DenOfIz_BufferDesc indexBufferDesc{ };
        indexBufferDesc.NumBytes  = backend->Desc.MaxIndices * sizeof( ImDrawIdx );
        indexBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_INDEX_BIT;
        indexBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
        indexBufferDesc.DebugName = DENOFIZ_STRING( "ImGui Index Buffer" );
        DenOfIz_LogicalDevice_CreateBuffer( backend->LogicalDevice, &indexBufferDesc, &backend->IndexBuffer );

        void *mappedIndexData = nullptr;
        DenOfIz_Buffer_MapMemory( backend->IndexBuffer, &mappedIndexData );
        backend->IndexBufferData = static_cast<uint8_t *>( mappedIndexData );

        backend->AlignedUniformSize = ( sizeof( ImGuiUniforms ) + 255 ) & ~255;

        DenOfIz_BufferDesc uniformBufferDesc{ };
        uniformBufferDesc.NumBytes  = backend->AlignedUniformSize * backend->Desc.NumFrames;
        uniformBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
        uniformBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
        uniformBufferDesc.DebugName = DENOFIZ_STRING( "ImGui Uniform Buffer" );
        DenOfIz_LogicalDevice_CreateBuffer( backend->LogicalDevice, &uniformBufferDesc, &backend->UniformBuffer );

        void *mappedUniformData = nullptr;
        DenOfIz_Buffer_MapMemory( backend->UniformBuffer, &mappedUniformData );
        backend->UniformBufferData = static_cast<ImGuiUniforms *>( mappedUniformData );

        backend->NumFrameData = backend->Desc.NumFrames;
        backend->FrameData    = static_cast<ImGuiFrameData *>( calloc( backend->NumFrameData, sizeof( ImGuiFrameData ) ) );

        if ( backend->SupportsSrvArray )
        {
            backend->AlignedPixelConstantsSize = ( sizeof( PixelConstants ) + 255 ) & ~255;

            DenOfIz_BufferDesc pixelConstantsBufferDesc{ };
            pixelConstantsBufferDesc.NumBytes  = backend->AlignedPixelConstantsSize * backend->Desc.MaxTextures;
            pixelConstantsBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
            pixelConstantsBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
            pixelConstantsBufferDesc.DebugName = DENOFIZ_STRING( "ImGui Pixel Constants Buffer" );
            DenOfIz_LogicalDevice_CreateBuffer( backend->LogicalDevice, &pixelConstantsBufferDesc, &backend->PixelConstantsBuffer );

            void *mappedPixelConstantsData = nullptr;
            DenOfIz_Buffer_MapMemory( backend->PixelConstantsBuffer, &mappedPixelConstantsData );
            backend->PixelConstantsData = static_cast<PixelConstants *>( mappedPixelConstantsData );

            for ( uint32_t i = 0; i < backend->Desc.MaxTextures; ++i )
            {
                const auto pixelConstantsData =
                    reinterpret_cast<PixelConstants *>( reinterpret_cast<uint8_t *>( backend->PixelConstantsData ) + i * backend->AlignedPixelConstantsSize );
                pixelConstantsData->TextureIndex = i;
            }

            for ( uint32_t i = 0; i < backend->Desc.NumFrames; ++i )
            {
                DenOfIz_BindGroupDesc constantsBindGroupDesc{ };
                constantsBindGroupDesc.Layout = backend->BindGroupLayouts[ 1 ];
                DenOfIz_LogicalDevice_CreateBindGroup( backend->LogicalDevice, &constantsBindGroupDesc, &backend->FrameData[ i ].ConstantsBindGroup );

                DenOfIz_BindGroup_BeginUpdate( backend->FrameData[ i ].ConstantsBindGroup );

                DenOfIz_BindBufferDesc bindUniformsDesc{ };
                bindUniformsDesc.Resource       = backend->UniformBuffer;
                bindUniformsDesc.ResourceOffset = i * backend->AlignedUniformSize;
                DenOfIz_BindGroup_CbvWithDesc( backend->FrameData[ i ].ConstantsBindGroup, &bindUniformsDesc );

                DenOfIz_BindGroup_EndUpdate( backend->FrameData[ i ].ConstantsBindGroup );

                DenOfIz_BindGroupDesc textureBindGroupDesc{ };
                textureBindGroupDesc.Layout = backend->BindGroupLayouts[ 0 ];
                DenOfIz_LogicalDevice_CreateBindGroup( backend->LogicalDevice, &textureBindGroupDesc, &backend->FrameData[ i ].TextureBindGroup );
            }

            backend->NumPixelConstantsBindGroups = backend->Desc.MaxTextures;
            backend->PixelConstantsBindGroups    = static_cast<DenOfIz_BindGroup *>( calloc( backend->NumPixelConstantsBindGroups, sizeof( DenOfIz_BindGroup ) ) );

            for ( uint32_t i = 0; i < backend->Desc.MaxTextures; ++i )
            {
                DenOfIz_BindGroupDesc pixelConstantsBindGroupDesc{ };
                pixelConstantsBindGroupDesc.Layout = backend->BindGroupLayouts[ 2 ];
                DenOfIz_LogicalDevice_CreateBindGroup( backend->LogicalDevice, &pixelConstantsBindGroupDesc, &backend->PixelConstantsBindGroups[ i ] );

                DenOfIz_BindGroup_BeginUpdate( backend->PixelConstantsBindGroups[ i ] );

                DenOfIz_BindBufferDesc bindPixelConstantsDesc{ };
                bindPixelConstantsDesc.Resource       = backend->PixelConstantsBuffer;
                bindPixelConstantsDesc.ResourceOffset = i * backend->AlignedPixelConstantsSize;
                DenOfIz_BindGroup_CbvWithDesc( backend->PixelConstantsBindGroups[ i ], &bindPixelConstantsDesc );

                DenOfIz_BindGroup_EndUpdate( backend->PixelConstantsBindGroups[ i ] );
            }
        }
        else
        {
            for ( uint32_t i = 0; i < backend->Desc.NumFrames; ++i )
            {
                DenOfIz_BindGroupDesc constantsBindGroupDesc{ };
                constantsBindGroupDesc.Layout = backend->BindGroupLayouts[ 1 ];
                DenOfIz_LogicalDevice_CreateBindGroup( backend->LogicalDevice, &constantsBindGroupDesc, &backend->FrameData[ i ].ConstantsBindGroup );

                DenOfIz_BindGroup_BeginUpdate( backend->FrameData[ i ].ConstantsBindGroup );

                DenOfIz_BindBufferDesc bindUniformsDesc{ };
                bindUniformsDesc.Resource       = backend->UniformBuffer;
                bindUniformsDesc.ResourceOffset = i * backend->AlignedUniformSize;
                DenOfIz_BindGroup_CbvWithDesc( backend->FrameData[ i ].ConstantsBindGroup, &bindUniformsDesc );

                DenOfIz_BindGroup_EndUpdate( backend->FrameData[ i ].ConstantsBindGroup );
            }

            backend->NumPerTextureBindGroups = backend->Desc.MaxTextures;
            backend->PerTextureBindGroups    = static_cast<DenOfIz_BindGroup *>( calloc( backend->NumPerTextureBindGroups, sizeof( DenOfIz_BindGroup ) ) );

            for ( uint32_t i = 0; i < backend->Desc.MaxTextures; ++i )
            {
                DenOfIz_BindGroupDesc perTextureBindGroupDesc{ };
                perTextureBindGroupDesc.Layout = backend->BindGroupLayouts[ 0 ];
                DenOfIz_LogicalDevice_CreateBindGroup( backend->LogicalDevice, &perTextureBindGroupDesc, &backend->PerTextureBindGroups[ i ] );
            }
        }
    }

    void CreateNullTexture( ImGuiBackend *backend )
    {
        DenOfIz_TextureDesc textureDesc{ };
        textureDesc.Width     = 1;
        textureDesc.Height    = 1;
        textureDesc.Depth     = 1;
        textureDesc.ArraySize = 1;
        textureDesc.MipLevels = 1;
        textureDesc.Format    = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Usage     = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT;
        textureDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
        textureDesc.DebugName = DENOFIZ_STRING( "ImGui Null Texture" );

        DenOfIz_LogicalDevice_CreateTexture( backend->LogicalDevice, &textureDesc, &backend->NullTexture );
        backend->Textures[ 1 ] = backend->NullTexture;
    }

    void CreateFontTexture( ImGuiBackend *backend )
    {
        ImGuiIO &io = ImGui::GetIO( );

        unsigned char *fontData;
        int            fontWidth, fontHeight;
        io.Fonts->GetTexDataAsRGBA32( &fontData, &fontWidth, &fontHeight );

        DenOfIz_TextureDesc fontTextureDesc{ };
        fontTextureDesc.Width     = static_cast<uint32_t>( fontWidth );
        fontTextureDesc.Height    = static_cast<uint32_t>( fontHeight );
        fontTextureDesc.Depth     = 1;
        fontTextureDesc.ArraySize = 1;
        fontTextureDesc.MipLevels = 1;
        fontTextureDesc.Format    = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
        fontTextureDesc.Usage     = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_COPY_DST_BIT;
        fontTextureDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
        fontTextureDesc.DebugName = DENOFIZ_STRING( "ImGui Font Texture" );
        DenOfIz_LogicalDevice_CreateTexture( backend->LogicalDevice, &fontTextureDesc, &backend->FontTexture );

        const size_t fontDataSize = fontWidth * fontHeight * 4;

        DenOfIz_BufferDesc uploadBufferDesc{ };
        uploadBufferDesc.NumBytes  = fontDataSize;
        uploadBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT;
        uploadBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
        uploadBufferDesc.DebugName = DENOFIZ_STRING( "ImGui Font Upload Buffer" );

        DenOfIz_Buffer uploadBuffer = DENOFIZ_NULL_HANDLE;
        DenOfIz_LogicalDevice_CreateBuffer( backend->LogicalDevice, &uploadBufferDesc, &uploadBuffer );

        void *uploadData = nullptr;
        DenOfIz_Buffer_MapMemory( uploadBuffer, &uploadData );
        memcpy( uploadData, fontData, fontDataSize );
        DenOfIz_Buffer_UnmapMemory( uploadBuffer );

        DenOfIz_ResourceTracking resourceTracking = DENOFIZ_NULL_HANDLE;
        DenOfIz_ResourceTracking_Create( &resourceTracking );
        DenOfIz_ResourceTracking_TrackTexture( resourceTracking, backend->FontTexture, DENOFIZ_QUEUE_TYPE_GRAPHICS );
        DenOfIz_ResourceTracking_TrackBuffer( resourceTracking, uploadBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );

        DenOfIz_CommandListArray commandLists{ };
        DenOfIz_CommandListPool_GetCommandLists( backend->CommandListPool, &commandLists );
        DenOfIz_CommandList commandList = commandLists.Elements[ 0 ];

        DenOfIz_CommandList_Begin( commandList );

        DenOfIz_CopyBufferToTextureDesc copyDesc{ };
        copyDesc.DstTexture = backend->FontTexture;
        copyDesc.SrcBuffer  = uploadBuffer;
        copyDesc.SrcOffset  = 0;
        copyDesc.DstX       = 0;
        copyDesc.DstY       = 0;
        copyDesc.DstZ       = 0;
        copyDesc.Format     = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
        copyDesc.MipLevel   = 0;
        copyDesc.ArrayLayer = 0;
        copyDesc.RowPitch   = fontWidth * 4;
        copyDesc.NumRows    = fontHeight;

        DenOfIz_CommandList_CopyBufferToTexture( commandList, &copyDesc );

        DenOfIz_ResourceTracking_TransitionTexture( resourceTracking, commandList, backend->FontTexture, DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

        DenOfIz_CommandList_End( commandList );

        DenOfIz_Fence uploadFence = DENOFIZ_NULL_HANDLE;
        DenOfIz_LogicalDevice_CreateFence( backend->LogicalDevice, &uploadFence );

        DenOfIz_ExecuteCommandListsDesc executeDesc{ };
        executeDesc.Signal                   = uploadFence;
        executeDesc.CommandLists.Elements    = &commandList;
        executeDesc.CommandLists.NumElements = 1;
        executeDesc.WaitSemaphores           = { };
        executeDesc.SignalSemaphores         = { };

        DenOfIz_CommandQueue_ExecuteCommandLists( backend->CommandQueue, &executeDesc );
        DenOfIz_Fence_Wait( uploadFence );

        DenOfIz_Fence_Destroy( uploadFence );
        DenOfIz_Buffer_Destroy( uploadBuffer );
        DenOfIz_ResourceTracking_Destroy( resourceTracking );

        backend->Textures[ 0 ] = backend->FontTexture;
        io.Fonts->SetTexID( 0 );
        backend->TexturesDirty = true;
    }

    void UpdateTextureBindings( ImGuiBackend *backend )
    {
        if ( !backend->TexturesDirty )
        {
            return;
        }

        if ( backend->SupportsSrvArray )
        {
            for ( uint32_t frameIndex = 0; frameIndex < backend->Desc.NumFrames; ++frameIndex )
            {
                DenOfIz_BindGroup_BeginUpdate( backend->FrameData[ frameIndex ].TextureBindGroup );

                DenOfIz_Texture *textureArray = static_cast<DenOfIz_Texture *>( malloc( backend->NumTextures * sizeof( DenOfIz_Texture ) ) );
                for ( uint32_t i = 0; i < backend->NumTextures; ++i )
                {
                    textureArray[ i ] = DENOFIZ_HANDLE_IS_VALID( backend->Textures[ i ] ) ? backend->Textures[ i ] : backend->NullTexture;
                }

                DenOfIz_TextureArray textureArrayDesc{ };
                textureArrayDesc.Elements    = textureArray;
                textureArrayDesc.NumElements = backend->NumTextures;

                DenOfIz_BindGroup_SrvArray( backend->FrameData[ frameIndex ].TextureBindGroup, 0, &textureArrayDesc );
                DenOfIz_BindGroup_Sampler( backend->FrameData[ frameIndex ].TextureBindGroup, 0, backend->Sampler );
                DenOfIz_BindGroup_EndUpdate( backend->FrameData[ frameIndex ].TextureBindGroup );

                free( textureArray );
            }
        }
        else
        {
            for ( uint32_t i = 0; i < backend->NumTextures; ++i )
            {
                DenOfIz_Texture texture = DENOFIZ_HANDLE_IS_VALID( backend->Textures[ i ] ) ? backend->Textures[ i ] : backend->NullTexture;

                DenOfIz_BindGroup_BeginUpdate( backend->PerTextureBindGroups[ i ] );
                DenOfIz_BindGroup_SrvTexture( backend->PerTextureBindGroups[ i ], 0, texture );
                DenOfIz_BindGroup_Sampler( backend->PerTextureBindGroups[ i ], 0, backend->Sampler );
                DenOfIz_BindGroup_EndUpdate( backend->PerTextureBindGroups[ i ] );
            }
        }

        backend->TexturesDirty = false;
    }

    void SetupRenderState( const ImGuiBackend *backend, DenOfIz_CommandList commandList, ImDrawData *drawData, uint32_t frameIndex )
    {
        DenOfIz_CommandList_BindPipeline( commandList, backend->Pipeline );
        DenOfIz_CommandList_BindVertexBuffer( commandList, backend->VertexBuffer, 0, sizeof( ImDrawVert ), 0 );
        DenOfIz_CommandList_BindIndexBuffer( commandList, backend->IndexBuffer, sizeof( ImDrawIdx ) == 2 ? DENOFIZ_INDEX_TYPE_UINT16 : DENOFIZ_INDEX_TYPE_UINT32, 0 );

        auto uniformData        = reinterpret_cast<ImGuiUniforms *>( reinterpret_cast<uint8_t *>( backend->UniformBufferData ) + frameIndex * backend->AlignedUniformSize );
        uniformData->Projection = backend->ProjectionMatrix;
        uniformData->ScreenSize = { static_cast<float>( backend->Viewport.Width ), static_cast<float>( backend->Viewport.Height ), 0.0f, 0.0f };

        DenOfIz_CommandList_BindGroup( commandList, backend->FrameData[ frameIndex ].ConstantsBindGroup );
        if ( backend->SupportsSrvArray )
        {
            DenOfIz_CommandList_BindGroup( commandList, backend->FrameData[ frameIndex ].TextureBindGroup );
        }
    }

    void RenderImDrawList( const ImGuiBackend *backend, DenOfIz_CommandList commandList, ImDrawList *cmdList, uint32_t vertexOffset, uint32_t indexOffset )
    {
        for ( int cmdIdx = 0; cmdIdx < cmdList->CmdBuffer.Size; cmdIdx++ )
        {
            const ImDrawCmd *pcmd = &cmdList->CmdBuffer[ cmdIdx ];
            if ( pcmd->UserCallback != nullptr )
            {
                if ( pcmd->UserCallback == ImDrawCallback_ResetRenderState )
                {
                    SetupRenderState( backend, commandList, ImGui::GetDrawData( ), backend->CurrentFrame );
                }
                else
                {
                    pcmd->UserCallback( cmdList, pcmd );
                }
            }
            else
            {
                const ImVec2 clipMin( pcmd->ClipRect.x, pcmd->ClipRect.y );
                const ImVec2 clipMax( pcmd->ClipRect.z, pcmd->ClipRect.w );
                if ( clipMax.x <= clipMin.x || clipMax.y <= clipMin.y )
                {
                    continue;
                }

                DenOfIz_CommandList_BindScissorRect( commandList, clipMin.x, clipMin.y, clipMax.x - clipMin.x, clipMax.y - clipMin.y );

                const auto textureIndex = static_cast<uint32_t>( pcmd->TextureId );
                if ( textureIndex < backend->NumTextures && DENOFIZ_HANDLE_IS_VALID( backend->Textures[ textureIndex ] ) )
                {
                    if ( backend->SupportsSrvArray )
                    {
                        if ( textureIndex < backend->NumPixelConstantsBindGroups )
                        {
                            DenOfIz_CommandList_BindGroup( commandList, backend->PixelConstantsBindGroups[ textureIndex ] );
                        }
                    }
                    else
                    {
                        if ( textureIndex < backend->NumPerTextureBindGroups )
                        {
                            DenOfIz_CommandList_BindGroup( commandList, backend->PerTextureBindGroups[ textureIndex ] );
                        }
                    }
                    DenOfIz_CommandList_DrawIndexed( commandList, pcmd->ElemCount, 1, pcmd->IdxOffset + indexOffset, static_cast<int32_t>( pcmd->VtxOffset + vertexOffset ), 0 );
                }
            }
        }
    }

    ImGuiKey KeyCodeToImGuiKey( DenOfIz_KeyCode keycode )
    {
        switch ( keycode )
        {
        case DENOFIZ_KEY_CODE_TAB:
            return ImGuiKey_Tab;
        case DENOFIZ_KEY_CODE_LEFT:
            return ImGuiKey_LeftArrow;
        case DENOFIZ_KEY_CODE_RIGHT:
            return ImGuiKey_RightArrow;
        case DENOFIZ_KEY_CODE_UP:
            return ImGuiKey_UpArrow;
        case DENOFIZ_KEY_CODE_DOWN:
            return ImGuiKey_DownArrow;
        case DENOFIZ_KEY_CODE_PAGEUP:
            return ImGuiKey_PageUp;
        case DENOFIZ_KEY_CODE_PAGEDOWN:
            return ImGuiKey_PageDown;
        case DENOFIZ_KEY_CODE_HOME:
            return ImGuiKey_Home;
        case DENOFIZ_KEY_CODE_END:
            return ImGuiKey_End;
        case DENOFIZ_KEY_CODE_INSERT:
            return ImGuiKey_Insert;
        case DENOFIZ_KEY_CODE_DELETE:
            return ImGuiKey_Delete;
        case DENOFIZ_KEY_CODE_BACKSPACE:
            return ImGuiKey_Backspace;
        case DENOFIZ_KEY_CODE_SPACE:
            return ImGuiKey_Space;
        case DENOFIZ_KEY_CODE_RETURN:
            return ImGuiKey_Enter;
        case DENOFIZ_KEY_CODE_ESCAPE:
            return ImGuiKey_Escape;
        case DENOFIZ_KEY_CODE_LCTRL:
            return ImGuiKey_LeftCtrl;
        case DENOFIZ_KEY_CODE_LSHIFT:
            return ImGuiKey_LeftShift;
        case DENOFIZ_KEY_CODE_LALT:
            return ImGuiKey_LeftAlt;
        case DENOFIZ_KEY_CODE_LGUI:
            return ImGuiKey_LeftSuper;
        case DENOFIZ_KEY_CODE_RCTRL:
            return ImGuiKey_RightCtrl;
        case DENOFIZ_KEY_CODE_RSHIFT:
            return ImGuiKey_RightShift;
        case DENOFIZ_KEY_CODE_RALT:
            return ImGuiKey_RightAlt;
        case DENOFIZ_KEY_CODE_RGUI:
            return ImGuiKey_RightSuper;
        case DENOFIZ_KEY_CODE_A:
            return ImGuiKey_A;
        case DENOFIZ_KEY_CODE_C:
            return ImGuiKey_C;
        case DENOFIZ_KEY_CODE_V:
            return ImGuiKey_V;
        case DENOFIZ_KEY_CODE_X:
            return ImGuiKey_X;
        case DENOFIZ_KEY_CODE_Y:
            return ImGuiKey_Y;
        case DENOFIZ_KEY_CODE_Z:
            return ImGuiKey_Z;
        default:
            return ImGuiKey_None;
        }
    }
} // namespace

ImGuiBackend *DenOfIz::ImGuiBackend_Create( const ImGuiBackendDesc &desc )
{
    ImGuiBackend *backend  = static_cast<ImGuiBackend *>( calloc( 1, sizeof( ImGuiBackend ) ) );
    backend->Desc          = desc;
    backend->LogicalDevice = desc.LogicalDevice;
    backend->Viewport      = desc.Viewport;
    backend->NumFrames     = desc.NumFrames;

    if ( !DENOFIZ_HANDLE_IS_VALID( backend->LogicalDevice ) )
    {
        spdlog::error( "ImGuiBackend: LogicalDevice cannot be null" );
        free( backend );
        return nullptr;
    }

    if ( backend->Viewport.Width == 0 || backend->Viewport.Height == 0 )
    {
        spdlog::error( "ImGuiBackend: Viewport.Width and Viewport.Height must be greater than 0" );
        free( backend );
        return nullptr;
    }

    DenOfIz_PhysicalDevice deviceInfo{ };
    DenOfIz_LogicalDevice_DeviceInfo( backend->LogicalDevice, &deviceInfo );
    backend->SupportsSrvArray = deviceInfo.Capabilities.SrvArray;

    spdlog::info( "ImGuiBackend: SrvArray support: {}", backend->SupportsSrvArray ? "yes" : "no" );

    backend->NumTextures = desc.MaxTextures;
    backend->Textures    = static_cast<DenOfIz_Texture *>( calloc( backend->NumTextures, sizeof( DenOfIz_Texture ) ) );

    DenOfIz_CommandQueueDesc commandQueueDesc{ };
    commandQueueDesc.QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    DenOfIz_LogicalDevice_CreateCommandQueue( backend->LogicalDevice, &commandQueueDesc, &backend->CommandQueue );

    DenOfIz_CommandListPoolDesc poolDesc{ };
    poolDesc.CommandQueue    = backend->CommandQueue;
    poolDesc.NumCommandLists = desc.NumFrames;
    DenOfIz_LogicalDevice_CreateCommandListPool( backend->LogicalDevice, &poolDesc, &backend->CommandListPool );

    CreateShaderProgram( backend );
    CreatePipeline( backend );
    CreateNullTexture( backend );
    CreateBuffers( backend );
    CreateFontTexture( backend );
    ImGuiBackend_SetViewport( backend, backend->Viewport );

    DenOfIz_CommandListArray commandLists{ };
    DenOfIz_CommandListPool_GetCommandLists( backend->CommandListPool, &commandLists );

    for ( uint32_t i = 0; i < desc.NumFrames && i < commandLists.NumElements; ++i )
    {
        backend->FrameData[ i ].CommandList = commandLists.Elements[ i ];
        DenOfIz_LogicalDevice_CreateFence( backend->LogicalDevice, &backend->FrameData[ i ].FrameFence );
    }

    DenOfIz_SamplerDesc samplerDesc{ };
    samplerDesc.AddressModeU = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.AddressModeV = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerDesc.AddressModeW = DENOFIZ_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    DenOfIz_LogicalDevice_CreateSampler( backend->LogicalDevice, &samplerDesc, &backend->Sampler );

    UpdateTextureBindings( backend );

    return backend;
}

void DenOfIz::ImGuiBackend_Destroy( ImGuiBackend *backend )
{
    if ( !backend )
    {
        return;
    }

    if ( backend->TextInputActive )
    {
        DenOfIz_InputSystem_StopTextInput( );
    }

    if ( backend->VertexBufferData )
    {
        DenOfIz_Buffer_UnmapMemory( backend->VertexBuffer );
    }
    if ( backend->IndexBufferData )
    {
        DenOfIz_Buffer_UnmapMemory( backend->IndexBuffer );
    }
    if ( backend->UniformBufferData )
    {
        DenOfIz_Buffer_UnmapMemory( backend->UniformBuffer );
    }
    if ( backend->PixelConstantsData )
    {
        DenOfIz_Buffer_UnmapMemory( backend->PixelConstantsBuffer );
    }

    DenOfIz_Buffer_Destroy( backend->VertexBuffer );
    DenOfIz_Buffer_Destroy( backend->IndexBuffer );
    DenOfIz_Buffer_Destroy( backend->UniformBuffer );
    if ( backend->SupportsSrvArray )
    {
        DenOfIz_Buffer_Destroy( backend->PixelConstantsBuffer );
    }

    for ( uint32_t i = 0; i < backend->NumFrameData; ++i )
    {
        DenOfIz_BindGroup_Destroy( backend->FrameData[ i ].ConstantsBindGroup );
        if ( backend->SupportsSrvArray )
        {
            DenOfIz_BindGroup_Destroy( backend->FrameData[ i ].TextureBindGroup );
        }
        DenOfIz_Fence_Destroy( backend->FrameData[ i ].FrameFence );
    }
    free( backend->FrameData );

    if ( backend->SupportsSrvArray )
    {
        for ( uint32_t i = 0; i < backend->NumPixelConstantsBindGroups; ++i )
        {
            DenOfIz_BindGroup_Destroy( backend->PixelConstantsBindGroups[ i ] );
        }
        free( backend->PixelConstantsBindGroups );
    }
    else
    {
        for ( uint32_t i = 0; i < backend->NumPerTextureBindGroups; ++i )
        {
            DenOfIz_BindGroup_Destroy( backend->PerTextureBindGroups[ i ] );
        }
        free( backend->PerTextureBindGroups );
    }

    DenOfIz_TextureResource_Destroy( backend->FontTexture );
    DenOfIz_TextureResource_Destroy( backend->NullTexture );
    free( backend->Textures );

    DenOfIz_Sampler_Destroy( backend->Sampler );
    DenOfIz_Pipeline_Destroy( backend->Pipeline );
    DenOfIz_RootSignature_Destroy( backend->RootSignature );
    for ( uint32_t i = 0; i < backend->NumBindGroupLayouts; ++i )
    {
        DenOfIz_BindGroupLayout_Destroy( backend->BindGroupLayouts[ i ] );
    }
    free( backend->BindGroupLayouts );
    DenOfIz_InputLayout_Destroy( backend->InputLayout );
    DenOfIz_ShaderProgram_Destroy( backend->ShaderProgram );
    DenOfIz_CommandListPool_Destroy( backend->CommandListPool );
    DenOfIz_CommandQueue_Destroy( backend->CommandQueue );

    free( backend );
}

void DenOfIz::ImGuiBackend_SetViewport( ImGuiBackend *backend, const DenOfIz_Viewport &viewport )
{
    backend->Viewport = viewport;

    constexpr float zn = 0.0f;
    constexpr float zf = 1.0f;

    backend->ProjectionMatrix._11 = 2.0f / ( viewport.Width - viewport.X );
    backend->ProjectionMatrix._22 = 2.0f / ( viewport.Y - viewport.Height );
    backend->ProjectionMatrix._33 = 1.0f / ( zf - zn );
    backend->ProjectionMatrix._41 = ( viewport.X + viewport.Width ) / ( viewport.X - viewport.Width );
    backend->ProjectionMatrix._42 = ( viewport.Height + viewport.Y ) / ( viewport.Height - viewport.Y );
    backend->ProjectionMatrix._43 = zn / ( zn - zf );
    backend->ProjectionMatrix._44 = 1.0f;
}

const DenOfIz_Viewport &DenOfIz::ImGuiBackend_GetViewport( const ImGuiBackend *backend )
{
    return backend->Viewport;
}

void DenOfIz::ImGuiBackend_RenderDrawData( ImGuiBackend *backend, DenOfIz_CommandList commandList, ImDrawData *drawData, uint32_t frameIndex )
{
    if ( !drawData || drawData->CmdListsCount == 0 )
    {
        return;
    }

    backend->CurrentFrame = backend->NextFrame;
    backend->NextFrame    = ( backend->NextFrame + 1 ) % backend->NumFrames;

    UpdateTextureBindings( backend );

    size_t totalVertexSize = 0;
    size_t totalIndexSize  = 0;
    for ( int n = 0; n < drawData->CmdListsCount; n++ )
    {
        const ImDrawList *cmdList = drawData->CmdLists[ n ];
        totalVertexSize += cmdList->VtxBuffer.Size * sizeof( ImDrawVert );
        totalIndexSize += cmdList->IdxBuffer.Size * sizeof( ImDrawIdx );
    }

    if ( totalVertexSize > backend->Desc.MaxVertices * sizeof( ImDrawVert ) || totalIndexSize > backend->Desc.MaxIndices * sizeof( ImDrawIdx ) )
    {
        spdlog::error( "ImGui draw data exceeds buffer capacity" );
        return;
    }

    uint32_t vertexOffset = 0;
    uint32_t indexOffset  = 0;

    for ( int n = 0; n < drawData->CmdListsCount; n++ )
    {
        const ImDrawList *cmdList = drawData->CmdLists[ n ];

        const size_t vertexDataSize = cmdList->VtxBuffer.Size * sizeof( ImDrawVert );
        const size_t indexDataSize  = cmdList->IdxBuffer.Size * sizeof( ImDrawIdx );

        memcpy( backend->VertexBufferData + vertexOffset * sizeof( ImDrawVert ), cmdList->VtxBuffer.Data, vertexDataSize );
        memcpy( backend->IndexBufferData + indexOffset * sizeof( ImDrawIdx ), cmdList->IdxBuffer.Data, indexDataSize );

        vertexOffset += cmdList->VtxBuffer.Size;
        indexOffset += cmdList->IdxBuffer.Size;
    }

    SetupRenderState( backend, commandList, drawData, frameIndex );

    vertexOffset = 0;
    indexOffset  = 0;
    for ( int n = 0; n < drawData->CmdListsCount; n++ )
    {
        ImDrawList *cmdList = drawData->CmdLists[ n ];
        RenderImDrawList( backend, commandList, cmdList, vertexOffset, indexOffset );
        vertexOffset += cmdList->VtxBuffer.Size;
        indexOffset += cmdList->IdxBuffer.Size;
    }
}

void DenOfIz::ImGuiBackend_RecreateFonts( ImGuiBackend *backend )
{
    CreateFontTexture( backend );
}

ImTextureID DenOfIz::ImGuiBackend_AddTexture( ImGuiBackend *backend, DenOfIz_Texture texture )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
    {
        return 0;
    }

    for ( uint32_t i = 2; i < backend->NumTextures; ++i )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( backend->Textures[ i ] ) )
        {
            backend->Textures[ i ] = texture;
            backend->TexturesDirty = true;
            return static_cast<ImTextureID>( i );
        }
    }

    spdlog::error( "ImGui texture capacity exceeded" );
    return 0;
}

void DenOfIz::ImGuiBackend_RemoveTexture( ImGuiBackend *backend, ImTextureID textureId )
{
    const auto index = static_cast<uint32_t>( textureId );
    if ( index < backend->NumTextures && index >= 2 )
    {
        backend->Textures[ index ] = DENOFIZ_NULL_HANDLE;
        backend->TexturesDirty     = true;
    }
}

void DenOfIz::ImGuiBackend_ProcessEvent( const ImGuiBackend *backend, const DenOfIz_Event &event )
{
    ImGuiIO &io = ImGui::GetIO( );

    switch ( event.Type )
    {
    case DENOFIZ_EVENT_TYPE_MOUSE_MOTION:
        {
            io.AddMousePosEvent( event.MouseMotion.X, event.MouseMotion.Y );
            break;
        }
    case DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_DOWN:
    case DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_UP:
        {
            int mouseButton = 0;
            switch ( event.MouseButton.Button )
            {
            case DENOFIZ_MOUSE_BUTTON_LEFT:
                mouseButton = 0;
                break;
            case DENOFIZ_MOUSE_BUTTON_RIGHT:
                mouseButton = 1;
                break;
            case DENOFIZ_MOUSE_BUTTON_MIDDLE:
                mouseButton = 2;
                break;
            default:
                break;
            }
            io.AddMouseButtonEvent( mouseButton, event.Type == DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_DOWN );
            break;
        }
    case DENOFIZ_EVENT_TYPE_MOUSE_WHEEL:
        {
            io.AddMouseWheelEvent( event.MouseWheel.X, event.MouseWheel.Y );
            break;
        }
    case DENOFIZ_EVENT_TYPE_KEY_DOWN:
    case DENOFIZ_EVENT_TYPE_KEY_UP:
        {
            const ImGuiKey key = KeyCodeToImGuiKey( event.Key.KeyCode );
            io.AddKeyEvent( key, event.Type == DENOFIZ_EVENT_TYPE_KEY_DOWN );
            break;
        }
    case DENOFIZ_EVENT_TYPE_TEXT_INPUT:
        {
            io.AddInputCharactersUTF8( event.Text.Text.Chars );
            break;
        }
    default:
        break;
    }
}

void DenOfIz::ImGuiBackend_UpdateTextInputState( ImGuiBackend *backend )
{
    const ImGuiIO &io            = ImGui::GetIO( );
    const bool     wantTextInput = io.WantTextInput;
    if ( wantTextInput && !backend->TextInputActive )
    {
        DenOfIz_InputSystem_StartTextInput( );
        backend->TextInputActive = true;
    }
    else if ( !wantTextInput && backend->TextInputActive )
    {
        DenOfIz_InputSystem_StopTextInput( );
        backend->TextInputActive = false;
    }
}

ImGuiRenderer *DenOfIz::ImGuiRenderer_Create( const ImGuiBackendDesc &desc )
{
    IMGUI_CHECKVERSION( );
    ImGui::CreateContext( );
    ImGuiIO &io = ImGui::GetIO( );
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark( );

    io.BackendRendererName = "imgui_impl_denofiz";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    ImGuiRenderer *renderer = static_cast<ImGuiRenderer *>( calloc( 1, sizeof( ImGuiRenderer ) ) );
    renderer->Backend       = ImGuiBackend_Create( desc );
    return renderer;
}

void DenOfIz::ImGuiRenderer_Destroy( ImGuiRenderer *renderer )
{
    if ( !renderer )
    {
        return;
    }

    ImGuiBackend_Destroy( renderer->Backend );
    ImGui::DestroyContext( );
    free( renderer );
}

void DenOfIz::ImGuiRenderer_ProcessEvent( const ImGuiRenderer *renderer, const DenOfIz_Event &ev )
{
    ImGuiBackend_ProcessEvent( renderer->Backend, ev );
}

void DenOfIz::ImGuiRenderer_NewFrame( const ImGuiRenderer *renderer, uint32_t width, uint32_t height, float deltaTime )
{
    ImGuiIO &io    = ImGui::GetIO( );
    io.DisplaySize = ImVec2( static_cast<float>( width ), static_cast<float>( height ) );
    io.DeltaTime   = deltaTime;
    ImGui::NewFrame( );
    ImGuiBackend_UpdateTextInputState( renderer->Backend );
}

void DenOfIz::ImGuiRenderer_Render( const ImGuiRenderer *renderer, DenOfIz_Texture renderTarget, DenOfIz_CommandList commandList, uint32_t frameIndex )
{
    DenOfIz_RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource = renderTarget;

    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &attachmentDesc;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.NumLayers                 = 1;

    DenOfIz_CommandList_BeginRendering( commandList, &renderingDesc );

    const DenOfIz_Viewport &viewport = ImGuiBackend_GetViewport( renderer->Backend );
    DenOfIz_CommandList_BindViewport( commandList, viewport.X, viewport.Y, viewport.Width, viewport.Height );
    DenOfIz_CommandList_BindScissorRect( commandList, viewport.X, viewport.Y, viewport.Width, viewport.Height );

    ImGuiBackend_RenderDrawData( renderer->Backend, commandList, ImGui::GetDrawData( ), frameIndex );

    DenOfIz_CommandList_EndRendering( commandList );
}

void DenOfIz::ImGuiRenderer_SetViewport( const ImGuiRenderer *renderer, DenOfIz_Viewport viewport )
{
    ImGuiBackend_SetViewport( renderer->Backend, viewport );
}

void DenOfIz::ImGuiRenderer_RecreateFonts( const ImGuiRenderer *renderer )
{
    ImGuiBackend_RecreateFonts( renderer->Backend );
}

ImTextureID DenOfIz::ImGuiRenderer_AddTexture( const ImGuiRenderer *renderer, DenOfIz_Texture texture )
{
    return ImGuiBackend_AddTexture( renderer->Backend, texture );
}

void DenOfIz::ImGuiRenderer_RemoveTexture( const ImGuiRenderer *renderer, ImTextureID textureId )
{
    ImGuiBackend_RemoveTexture( renderer->Backend, textureId );
}
