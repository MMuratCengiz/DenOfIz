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

#include "DenOfIzExamples/BindlessExample.h"

#include <array>
#include <cmath>
#include <cstring>
#include <string>
#include "DenOfIzGraphics/Assets/Vector2d/ThorVGWrapper.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzGraphics/Utilities/InteropUtilities.h"

using namespace DenOfIz;

void BindlessExample::Init( )
{
    CreateVertexBuffer( );
    CreateTextures( );
    CreateSampler( );
    CreateConstantBuffer( );

    std::array<DenOfIz_ShaderStageDesc, 2> shaderStages( { } );
    DenOfIz_ShaderStageDesc               &vertexShaderDesc = shaderStages[ 0 ];
    vertexShaderDesc.Stage                                  = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
    vertexShaderDesc.EntryPoint                             = DENOFIZ_STRING( "VSMain" );
    vertexShaderDesc.Data                                   = VertexShader( );

    DenOfIz_ShaderStageDesc &pixelShaderDesc = shaderStages[ 1 ];
    pixelShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
    pixelShaderDesc.EntryPoint               = DENOFIZ_STRING( "PSMain" );
    pixelShaderDesc.Data                     = PixelShader( );

    std::array<DenOfIz_BindlessSlot, 1> bindlessSlots( { } );
    bindlessSlots[ 0 ].RegisterSpace = 0;
    bindlessSlots[ 0 ].Binding       = 0;
    bindlessSlots[ 0 ].MaxArraySize  = NUM_TEXTURES;
    bindlessSlots[ 0 ].Type          = DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE;

    pixelShaderDesc.Bindless.BindlessArrays.Elements    = bindlessSlots.data( );
    pixelShaderDesc.Bindless.BindlessArrays.NumElements = bindlessSlots.size( );

    DenOfIz_ShaderProgramDesc shaderProgramDesc{ };
    shaderProgramDesc.ShaderStages.Elements    = shaderStages.data( );
    shaderProgramDesc.ShaderStages.NumElements = shaderStages.size( );
    m_program                                  = DenOfIz_ShaderProgram_Create( &shaderProgramDesc );

    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( m_program, &reflectDesc );

    m_bindGroupLayouts.resize( reflectDesc.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
    {
        DenOfIz_LogicalDevice_CreateBindGroupLayout( m_logicalDevice, &reflectDesc.BindGroupLayouts.Elements[ i ], &m_bindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc rootSigDesc{ };
    rootSigDesc.BindGroupLayouts.Elements    = m_bindGroupLayouts.data( );
    rootSigDesc.BindGroupLayouts.NumElements = m_bindGroupLayouts.size( );
    rootSigDesc.RootConstants                = reflectDesc.RootConstants;

    DenOfIz_LogicalDevice_CreateInputLayout( m_logicalDevice, &reflectDesc.InputLayout, &m_inputLayout );
    DenOfIz_LogicalDevice_CreateRootSignature( m_logicalDevice, &rootSigDesc, &m_rootSignature );

    DenOfIz_BindGroupDesc bindGroupDesc{ };
    bindGroupDesc.Layout = m_bindGroupLayouts[ 0 ];
    DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &bindGroupDesc, &m_bindGroup );

    DenOfIz_BindGroup_BeginUpdate( m_bindGroup );
    DenOfIz_TextureArray textureArray{ };
    textureArray.Elements    = m_textures;
    textureArray.NumElements = NUM_TEXTURES;
    DenOfIz_BindGroup_SrvArray( m_bindGroup, 0, &textureArray );
    DenOfIz_BindGroup_Sampler( m_bindGroup, 0, m_sampler );
    DenOfIz_BindGroup_EndUpdate( m_bindGroup );

    bindGroupDesc.Layout = m_bindGroupLayouts[ 1 ];
    DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &bindGroupDesc, &m_perFrameBindGroup );

    DenOfIz_RenderTargetDesc renderTargetDesc{ };
    renderTargetDesc.Format                      = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    renderTargetDesc.Blend.RenderTargetWriteMask = 0x0F;

    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.InputLayout                        = m_inputLayout;
    pipelineDesc.ShaderProgram                      = m_program;
    pipelineDesc.RootSignature                      = m_rootSignature;
    pipelineDesc.Graphics.PrimitiveTopology         = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTargetDesc;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;

    DenOfIz_LogicalDevice_CreatePipeline( m_logicalDevice, &pipelineDesc, &m_pipeline );
}

BindlessExample::~BindlessExample( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );

    DenOfIz_Buffer_Destroy( m_vertexBuffer );
    DenOfIz_Buffer_Destroy( m_indexBuffer );
    DenOfIz_Buffer_Destroy( m_constantBuffer );

    for ( uint32_t i = 0; i < NUM_TEXTURES; ++i )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( m_textures[ i ] ) )
        {
            DenOfIz_TextureResource_Destroy( m_textures[ i ] );
        }
    }

    DenOfIz_Sampler_Destroy( m_sampler );
    DenOfIz_Pipeline_Destroy( m_pipeline );
    DenOfIz_RootSignature_Destroy( m_rootSignature );
    for ( auto &layout : m_bindGroupLayouts )
    {
        DenOfIz_BindGroupLayout_Destroy( layout );
    }
    DenOfIz_BindGroup_Destroy( m_bindGroup );
    DenOfIz_BindGroup_Destroy( m_perFrameBindGroup );
    DenOfIz_InputLayout_Destroy( m_inputLayout );
    DenOfIz_ShaderProgram_Destroy( m_program );
}

void BindlessExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN;
}

void BindlessExample::Update( )
{
    const float deltaTime = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    m_camera->Update( deltaTime );
    m_elapsedTime += deltaTime;

    static float timer = 0.0f;
    timer += deltaTime;
    if ( timer > 1.0f )
    {
        m_currentTextureIndex = ( m_currentTextureIndex + 1 ) % NUM_TEXTURES;
        timer                 = 0.0f;
    }

    PerFrameData perFrameData;
    perFrameData.textureIndex = m_currentTextureIndex;
    perFrameData.time         = m_elapsedTime;

    void *mappedData = nullptr;
    DenOfIz_Buffer_MapMemory( m_constantBuffer, &mappedData );
    memcpy( mappedData, &perFrameData, sizeof( PerFrameData ) );
    DenOfIz_Buffer_UnmapMemory( m_constantBuffer );

    RenderAndPresentFrame( );
}

void BindlessExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    DenOfIz_CommandList_Begin( commandList );

    uint32_t imageIndex = 0;
    DenOfIz_FrameSync_AcquireNextImage( m_frameSync, &imageIndex );

    DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
    DenOfIz_SwapChain_GetRenderTarget( m_swapChain, imageIndex, &renderTarget );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource = renderTarget;

    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &attachmentDesc;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.NumLayers                 = 1;
    DenOfIz_CommandList_BeginRendering( commandList, &renderingDesc );

    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );
    DenOfIz_CommandList_BindViewport( commandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindScissorRect( commandList, viewport->X, viewport->Y, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindPipeline( commandList, m_pipeline );
    DenOfIz_CommandList_BindVertexBuffer( commandList, m_vertexBuffer, 0, 0, 0 );
    DenOfIz_CommandList_BindGroup( commandList, m_bindGroup );

    DenOfIz_BindGroup_BeginUpdate( m_perFrameBindGroup );
    DenOfIz_BindGroup_Cbv( m_perFrameBindGroup, 0, m_constantBuffer );
    DenOfIz_BindGroup_EndUpdate( m_perFrameBindGroup );
    DenOfIz_CommandList_BindGroup( commandList, m_perFrameBindGroup );

    DenOfIz_CommandList_Draw( commandList, 3, 1, 0, 0 );

    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
}

void BindlessExample::HandleEvent( DenOfIz_Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void BindlessExample::CreateVertexBuffer( )
{
    constexpr std::array vertices = { 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.5f, -0.5f, 0.0f, 1.0f, 1.0f };

    DenOfIz_BufferDesc bufferDesc{ };
    bufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_VERTEX_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    bufferDesc.NumBytes  = vertices.size( ) * sizeof( float );
    bufferDesc.DebugName = DENOFIZ_STRING( "TriangleVertexBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &bufferDesc, &m_vertexBuffer );

    DenOfIz_BatchResourceCopy     batchCopy;
    DenOfIz_BatchResourceCopyDesc batchDesc{ };
    batchDesc.Device        = m_logicalDevice;
    batchDesc.IssueBarriers = true;
    DenOfIz_BatchResourceCopy_Create( &batchDesc, &batchCopy );
    DenOfIz_BatchResourceCopy_Begin( batchCopy );

    DenOfIz_CopyToGpuBufferDesc copyDesc{ };
    copyDesc.DstBuffer        = m_vertexBuffer;
    copyDesc.Data.Elements    = reinterpret_cast<const Byte *>( vertices.data( ) );
    copyDesc.Data.NumElements = vertices.size( ) * sizeof( float );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &copyDesc );
    DenOfIz_BatchResourceCopy_Submit( batchCopy, DENOFIZ_NULL_HANDLE );
    DenOfIz_BatchResourceCopy_Destroy( batchCopy );

    DenOfIz_ResourceTracking_TrackBuffer( m_resourceTracking, m_vertexBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );
}

DenOfIz_ByteArray BindlessExample::VertexShader( )
{
    const auto shaderCode = R"(
        struct VSInput
        {
            float3 Position : POSITION;
            float2 TexCoord : TEXCOORD0;
        };

        struct PSInput
        {
            float4 Position : SV_POSITION;
            float2 TexCoord : TEXCOORD0;
        };

        PSInput VSMain(VSInput input)
        {
            PSInput output;
            output.Position = float4(input.Position, 1.0);
            output.TexCoord = input.TexCoord;
            return output;
        }
        )";
    return DenOfIz_InteropUtilities_StringToBytes( shaderCode );
}

DenOfIz_ByteArray BindlessExample::PixelShader( )
{
    const auto shaderCode = R"(
        struct PSInput
        {
            float4 Position : SV_POSITION;
            float2 TexCoord : TEXCOORD0;
        };

        cbuffer PerFrameConstants : register(b0, space1)
        {
            uint textureIndex;
            float time;
        };

        // Bindless texture array
        Texture2D<float4> g_Textures[] : register(t0, space0);
        SamplerState g_Sampler : register(s0, space0);

        float4 PSMain(PSInput input) : SV_TARGET
        {
            // Use dynamic indexing with the bindless texture array
            float4 color = g_Textures[textureIndex].Sample(g_Sampler, input.TexCoord);

            // Add a pulsing effect based on time
            float pulse = sin(time * 3.0) * 0.2 + 0.8;
            color.rgb *= pulse;

            return color;
        }
        )";
    return DenOfIz_InteropUtilities_StringToBytes( shaderCode );
}

void BindlessExample::CreateTextures( )
{
    for ( int i = 0; i < NUM_TEXTURES; ++i )
    {
        constexpr int    width  = 256;
        constexpr int    height = 256;
        ThorVGCanvasDesc canvasDesc{ };
        canvasDesc.Width  = width;
        canvasDesc.Height = height;
        ThorVGCanvas canvas( canvasDesc );

        ThorVGShape shape;

        switch ( i )
        {
        case 0:
            {
                for ( int y = 0; y < 3; ++y )
                {
                    for ( int x = 0; x < 3; ++x )
                    {
                        ThorVGShape circle;
                        circle.AppendCircle( x * 85.0f + 42.5f, y * 85.0f + 42.5f, 30.0f, 30.0f );
                        circle.Fill( 255 - x * 80, y * 80, 128 + x * 40, 255 );
                        canvas.Push( &circle );
                    }
                }
            }
            break;

        case 1:
            {
                shape.AppendRect( 0, 0, width, height );

                ThorVGLinearGradient gradient;
                gradient.Linear( 0, 0, width, height );

                std::vector<ThorVGColorStop> colorStops( 4 );
                colorStops[ 0 ] = { 0.0f, 255, 0, 128, 255 };
                colorStops[ 1 ] = { 0.33f, 255, 255, 0, 255 };
                colorStops[ 2 ] = { 0.66f, 0, 255, 255, 255 };
                colorStops[ 3 ] = { 1.0f, 128, 0, 255, 255 };
                gradient.ColorStops( { colorStops.data( ), static_cast<uint32_t>( colorStops.size( ) ) } );

                shape.Fill( &gradient );
                canvas.Push( &shape );
            }
            break;

        case 2:
            {
                constexpr float cx     = width / 2.0f;
                constexpr float cy     = height / 2.0f;
                constexpr float radius = 100.0f;
                constexpr float inner  = radius * 0.4f;

                shape.MoveTo( cx, cy - radius );
                for ( int j = 0; j < 10; ++j )
                {
                    const float angle = ( j * 36.0f - 90.0f ) * 3.14159f / 180.0f;
                    const float r     = j % 2 == 0 ? radius : inner;
                    const float x     = cx + r * cos( angle );
                    const float y     = cy + r * sin( angle );
                    shape.LineTo( x, y );
                }
                shape.Close( );
                shape.Fill( 255, 215, 0, 255 );
                shape.Stroke( 255, 140, 0, 255 );
                shape.Stroke( 3.0f );
                canvas.Push( &shape );
            }
            break;

        case 3:
            {
                constexpr int cellSize = 32;
                for ( int y = 0; y < height / cellSize; ++y )
                {
                    for ( int x = 0; x < width / cellSize; ++x )
                    {
                        ThorVGShape rect;
                        rect.AppendRect( x * cellSize, y * cellSize, cellSize, cellSize );
                        if ( ( x + y ) % 2 == 0 )
                        {
                            rect.Fill( 64, 64, 64, 255 );
                        }
                        else
                        {
                            rect.Fill( 192, 192, 192, 255 );
                        }
                        canvas.Push( &rect );
                    }
                }
            }
            break;
        default:
            break;
        }

        canvas.Draw( );
        canvas.Sync( );

        DenOfIz_TextureDesc textureDesc{ };
        textureDesc.Width           = width;
        textureDesc.Height          = height;
        textureDesc.Depth           = 1;
        textureDesc.ArraySize       = 1;
        textureDesc.MipLevels       = 1;
        textureDesc.Format          = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Usage           = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_COPY_DST_BIT;
        const std::string debugName = "BindlessTexture_" + std::to_string( i );
        textureDesc.DebugName       = DenOfIz_StringView( debugName.c_str( ), static_cast<uint32_t>( debugName.size( ) ) );

        DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &textureDesc, &m_textures[ i ] );

        DenOfIz_BatchResourceCopy     batchCopy;
        DenOfIz_BatchResourceCopyDesc batchDesc{ };
        batchDesc.Device        = m_logicalDevice;
        batchDesc.IssueBarriers = true;
        DenOfIz_BatchResourceCopy_Create( &batchDesc, &batchCopy );
        DenOfIz_BatchResourceCopy_Begin( batchCopy );

        auto                          dataAsBytes = canvas.GetDataAsBytes( );
        DenOfIz_CopyDataToTextureDesc copyDesc{ };
        copyDesc.Data.Elements    = dataAsBytes.Elements;
        copyDesc.Data.NumElements = dataAsBytes.NumElements;
        copyDesc.DstTexture       = m_textures[ i ];
        copyDesc.MipLevel         = 0;
        DenOfIz_BatchResourceCopy_CopyDataToTexture( batchCopy, &copyDesc );

        DenOfIz_BatchResourceCopy_Submit( batchCopy, DENOFIZ_NULL_HANDLE );
        DenOfIz_BatchResourceCopy_Destroy( batchCopy );

        DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, m_textures[ i ], DENOFIZ_QUEUE_TYPE_GRAPHICS );
    }
}

void BindlessExample::CreateSampler( )
{
    DenOfIz_SamplerDesc samplerDesc{ };
    samplerDesc.MinFilter     = DENOFIZ_FILTER_LINEAR;
    samplerDesc.MagFilter     = DENOFIZ_FILTER_LINEAR;
    samplerDesc.MipmapMode    = DENOFIZ_MIPMAP_MODE_LINEAR;
    samplerDesc.AddressModeU  = DENOFIZ_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerDesc.AddressModeV  = DENOFIZ_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerDesc.AddressModeW  = DENOFIZ_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerDesc.MaxAnisotropy = 1.0f;
    samplerDesc.MinLod        = 0.0f;
    samplerDesc.MaxLod        = 1.0f;

    DenOfIz_LogicalDevice_CreateSampler( m_logicalDevice, &samplerDesc, &m_sampler );
}

void BindlessExample::CreateConstantBuffer( )
{
    DenOfIz_BufferDesc bufferDesc{ };
    bufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    bufferDesc.NumBytes  = sizeof( PerFrameData );
    bufferDesc.DebugName = DENOFIZ_STRING( "PerFrameConstantBuffer" );
    bufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &bufferDesc, &m_constantBuffer );

    PerFrameData initialData;
    initialData.textureIndex = 0;
    initialData.time         = 0.0f;

    void *mappedData = nullptr;
    DenOfIz_Buffer_MapMemory( m_constantBuffer, &mappedData );
    memcpy( mappedData, &initialData, sizeof( PerFrameData ) );
    DenOfIz_Buffer_UnmapMemory( m_constantBuffer );

    DenOfIz_ResourceTracking_TrackBuffer( m_resourceTracking, m_constantBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );
}
