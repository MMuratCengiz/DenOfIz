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
#include "DenOfIzExamples/SimpleTriangleExample.h"
#include <array>
#include "DenOfIzGraphics/Utilities/InteropUtilities.h"

using namespace DenOfIz;

SimpleTriangleExample::~SimpleTriangleExample( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );

    DenOfIz_Buffer_Destroy( m_vertexBuffer );
    DenOfIz_Pipeline_Destroy( m_pipeline );
    DenOfIz_RootSignature_Destroy( m_rootSignature );
    for ( auto &layout : m_bindGroupLayouts )
    {
        DenOfIz_BindGroupLayout_Destroy( layout );
    }
    DenOfIz_InputLayout_Destroy( m_inputLayout );
    DenOfIz_ShaderProgram_Destroy( m_shaderProgram );
}

void SimpleTriangleExample::Init( )
{
    CreateVertexBuffer( );

    std::array<DenOfIz_ShaderStageDesc, 2> shaderStages( { } );

    DenOfIz_ShaderStageDesc &vertexShaderDesc = shaderStages[ 0 ];
    vertexShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
    vertexShaderDesc.EntryPoint               = DENOFIZ_STRING( "VSMain" );
    vertexShaderDesc.Data                     = VertexShader( );

    DenOfIz_ShaderStageDesc &pixelShaderDesc = shaderStages[ 1 ];
    pixelShaderDesc.Stage                    = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
    pixelShaderDesc.EntryPoint               = DENOFIZ_STRING( "PSMain" );
    pixelShaderDesc.Data                     = PixelShader( );

    DenOfIz_ShaderProgramDesc shaderProgramDesc{ };
    shaderProgramDesc.ShaderStages.Elements    = shaderStages.data( );
    shaderProgramDesc.ShaderStages.NumElements = shaderStages.size( );
    m_shaderProgram                            = DenOfIz_ShaderProgram_Create( &shaderProgramDesc );

    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( m_shaderProgram, &reflectDesc );

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

    DenOfIz_RenderTargetDesc renderTargetDesc{ };
    renderTargetDesc.Format                      = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    renderTargetDesc.Blend.RenderTargetWriteMask = 0x0F;

    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.BindPoint                          = DENOFIZ_BIND_POINT_GRAPHICS;
    pipelineDesc.InputLayout                        = m_inputLayout;
    pipelineDesc.ShaderProgram                      = m_shaderProgram;
    pipelineDesc.RootSignature                      = m_rootSignature;
    pipelineDesc.Graphics.PrimitiveTopology         = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    pipelineDesc.Graphics.RenderTargets.NumElements = 1;
    pipelineDesc.Graphics.RenderTargets.Elements    = &renderTargetDesc;

    DenOfIz_LogicalDevice_CreatePipeline( m_logicalDevice, &pipelineDesc, &m_pipeline );
}

void SimpleTriangleExample::ModifyApiPreferences( DenOfIz_APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN;
    defaultApiPreference.OSX     = DENOFIZ_API_PREFERENCE_OSX_WEBGPU_NATIVE;
}

void SimpleTriangleExample::Update( )
{
    const auto deltaTime = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    m_camera->Update( deltaTime );
    RenderAndPresentFrame( );
}

void SimpleTriangleExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
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
    DenOfIz_CommandList_BindVertexBuffer( commandList, m_vertexBuffer, 0, 7 * sizeof( float ), 0 );
    DenOfIz_CommandList_Draw( commandList, 3, 1, 0, 0 );

    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
}

void SimpleTriangleExample::HandleEvent( DenOfIz_Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void SimpleTriangleExample::CreateVertexBuffer( )
{
    constexpr std::array vertices = {
        // Position (XYZ)    // Color (RGBA)
        0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Top vertex (red)
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // Bottom left (green)
        0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f  // Bottom right (blue)
    };

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
    copyDesc.Data.Elements    = reinterpret_cast<const Byte *>( &vertices[ 0 ] );
    copyDesc.Data.NumElements = sizeof( vertices );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &copyDesc );
    DenOfIz_BatchResourceCopy_Submit( batchCopy, DENOFIZ_NULL_HANDLE );
    DenOfIz_BatchResourceCopy_Destroy( batchCopy );

    DenOfIz_ResourceTracking_TrackBuffer( m_resourceTracking, m_vertexBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );
}

DenOfIz_ByteArray SimpleTriangleExample::VertexShader( )
{
    const auto shaderCode = R"(
        struct VSInput
        {
            float3 Position : POSITION;
            float4 Color : COLOR;
        };

        struct PSInput
        {
            float4 Position : SV_POSITION;
            float4 Color : COLOR;
        };

        PSInput VSMain(VSInput input)
        {
            PSInput output;
            output.Position = float4(input.Position, 1.0);
            output.Color = input.Color;
            return output;
        }
        )";

    return DenOfIz_InteropUtilities_StringToBytes( shaderCode );
}

DenOfIz_ByteArray SimpleTriangleExample::PixelShader( )
{
    const auto shaderCode = R"(
        struct PSInput
            {
                float4 Position : SV_POSITION;
                float4 Color : COLOR;
            };

            float4 PSMain(PSInput input) : SV_TARGET
            {
                return input.Color;
            }
        )";

    return DenOfIz_InteropUtilities_StringToBytes( shaderCode );
}
