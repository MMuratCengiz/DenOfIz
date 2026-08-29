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
#include <vector>
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
    bindlessSlots[ 0 ].Descriptor    = DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT;

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

namespace
{
    struct Rgba
    {
        uint8_t R, G, B, A;
    };

    auto PatternPixel( const int pattern, const int x, const int y, const int width, const int height ) -> Rgba
    {
        switch ( pattern )
        {
        case 0:
            {
                const int   cellX = x / 85;
                const int   cellY = y / 85;
                const float dx    = static_cast<float>( x ) - ( cellX * 85.0f + 42.5f );
                const float dy    = static_cast<float>( y ) - ( cellY * 85.0f + 42.5f );
                if ( dx * dx + dy * dy <= 30.0f * 30.0f )
                {
                    return { static_cast<uint8_t>( 255 - cellX * 80 ), static_cast<uint8_t>( cellY * 80 ), static_cast<uint8_t>( 128 + cellX * 40 ), 255 };
                }
                return { 255, 255, 255, 255 };
            }
        case 1:
            {
                constexpr Rgba stops[ 4 ] = { { 255, 0, 128, 255 }, { 255, 255, 0, 255 }, { 0, 255, 255, 255 }, { 128, 0, 255, 255 } };
                const float    t          = ( static_cast<float>( x ) / width + static_cast<float>( y ) / height ) * 0.5f;
                const float    scaled     = std::min( t * 3.0f, 2.999f );
                const int      index      = static_cast<int>( scaled );
                const float    frac       = scaled - static_cast<float>( index );
                const Rgba    &a          = stops[ index ];
                const Rgba    &b          = stops[ index + 1 ];
                return { static_cast<uint8_t>( a.R + ( b.R - a.R ) * frac ), static_cast<uint8_t>( a.G + ( b.G - a.G ) * frac ), static_cast<uint8_t>( a.B + ( b.B - a.B ) * frac ),
                         255 };
            }
        case 2:
            {
                constexpr float cx     = 128.0f;
                constexpr float cy     = 128.0f;
                constexpr float radius = 100.0f;
                constexpr float inner  = radius * 0.4f;
                const float     dx     = static_cast<float>( x ) - cx;
                const float     dy     = static_cast<float>( y ) - cy;
                const float     dist   = std::sqrt( dx * dx + dy * dy );
                // Five pointed star: the boundary radius oscillates between the outer and inner radius over each 36 degree sector.
                float angle = std::atan2( dy, dx ) + 3.14159f / 2.0f;
                angle       = std::fmod( angle + 2.0f * 3.14159f, 2.0f * 3.14159f );
                const float sector    = std::fmod( angle, 2.0f * 3.14159f / 5.0f ) / ( 2.0f * 3.14159f / 5.0f );
                const float w         = std::abs( sector - 0.5f ) * 2.0f;
                const float threshold = inner + ( radius - inner ) * w;
                if ( dist <= threshold )
                {
                    return dist >= threshold - 3.0f ? Rgba{ 255, 140, 0, 255 } : Rgba{ 255, 215, 0, 255 };
                }
                return { 255, 255, 255, 255 };
            }
        default:
            {
                constexpr int cellSize = 32;
                return ( x / cellSize + y / cellSize ) % 2 == 0 ? Rgba{ 64, 64, 64, 255 } : Rgba{ 192, 192, 192, 255 };
            }
        }
    }
} // namespace

void BindlessExample::CreateTextures( )
{
    for ( int i = 0; i < NUM_TEXTURES; ++i )
    {
        constexpr int     width  = 256;
        constexpr int     height = 256;
        std::vector<Rgba> pixels( width * height );
        for ( int y = 0; y < height; ++y )
        {
            for ( int x = 0; x < width; ++x )
            {
                pixels[ y * width + x ] = PatternPixel( i, x, y, width, height );
            }
        }

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

        DenOfIz_CopyDataToTextureDesc copyDesc{ };
        copyDesc.Data.Elements    = reinterpret_cast<const Byte *>( pixels.data( ) );
        copyDesc.Data.NumElements = pixels.size( ) * sizeof( Rgba );
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
