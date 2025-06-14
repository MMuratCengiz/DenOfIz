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
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Utilities/InteropUtilities.h"

using namespace DenOfIz;

void SimpleTriangleExample::Init( )
{
    CreateVertexBuffer( );

    std::array<ShaderStageDesc, 2> shaderStages( { } );

    ShaderStageDesc &vertexShaderDesc = shaderStages[ 0 ];
    vertexShaderDesc.Stage            = ShaderStage::Vertex;
    vertexShaderDesc.EntryPoint       = "VSMain";
    vertexShaderDesc.Data             = VertexShader( );

    ShaderStageDesc &pixelShaderDesc = shaderStages[ 1 ];
    pixelShaderDesc.Stage            = ShaderStage::Pixel;
    pixelShaderDesc.EntryPoint       = "PSMain";
    pixelShaderDesc.Data             = PixelShader( );

    ShaderProgramDesc shaderProgramDesc{ };
    shaderProgramDesc.ShaderStages.Elements    = shaderStages.data( );
    shaderProgramDesc.ShaderStages.NumElements = shaderStages.size( );
    m_program                                  = std::make_unique<ShaderProgram>( shaderProgramDesc );
    std::free( vertexShaderDesc.Data.Elements );
    std::free( pixelShaderDesc.Data.Elements );

    const ShaderReflectDesc reflectDesc = m_program->Reflect( );
    m_inputLayout                       = std::unique_ptr<IInputLayout>( m_logicalDevice->CreateInputLayout( reflectDesc.InputLayout ) );
    m_rootSignature                     = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflectDesc.RootSignature ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.InputLayout   = m_inputLayout.get( );
    pipelineDesc.ShaderProgram = m_program.get( );
    pipelineDesc.RootSignature = m_rootSignature.get( );
    pipelineDesc.Graphics.RenderTargets.AddElement( { .Format = Format::B8G8R8A8Unorm } );

    m_pipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );
}

void SimpleTriangleExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    // defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void SimpleTriangleExample::Update( )
{
    m_worldData.DeltaTime = m_stepTimer.GetDeltaTime( );
    m_worldData.Camera->Update( m_worldData.DeltaTime );
    RenderAndPresentFrame( );
}

void SimpleTriangleExample::Render( const uint32_t frameIndex, ICommandList *commandList )
{
    commandList->Begin( );
    ITextureResource *renderTarget = m_swapChain->GetRenderTarget( m_frameSync->AcquireNextImage( frameIndex ) );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::RenderTarget );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource = renderTarget;

    RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &attachmentDesc;
    renderingDesc.RTAttachments.NumElements = 1;
    commandList->BeginRendering( renderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindPipeline( m_pipeline.get( ) );
    commandList->BindVertexBuffer( m_vertexBuffer.get( ) );
    commandList->Draw( 3, 1, 0, 0 );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void SimpleTriangleExample::HandleEvent( Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void SimpleTriangleExample::Quit( )
{
    m_frameSync->WaitIdle( );
    IExample::Quit( );
}

void SimpleTriangleExample::CreateVertexBuffer( )
{
    constexpr std::array vertices = {
        // Position (XYZ)    // Color (RGBA)
        0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Top vertex (red)
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // Bottom left (green)
        0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f  // Bottom right (blue)
    };

    BufferDesc bufferDesc{ };
    bufferDesc.Descriptor = ResourceDescriptor::VertexBuffer;
    bufferDesc.NumBytes   = vertices.size( ) * sizeof( float );
    bufferDesc.DebugName  = "TriangleVertexBuffer";

    m_vertexBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( bufferDesc ) );

    BatchResourceCopy batchCopy( m_logicalDevice );
    batchCopy.Begin( );

    CopyToGpuBufferDesc copyDesc{ };
    copyDesc.DstBuffer        = m_vertexBuffer.get( );
    copyDesc.Data.Elements    = reinterpret_cast<const Byte *>( &vertices[ 0 ] );
    copyDesc.Data.NumElements = sizeof( vertices );
    batchCopy.CopyToGPUBuffer( copyDesc );
    batchCopy.Submit( );

    m_resourceTracking.TrackBuffer( m_vertexBuffer.get( ), ResourceUsage::VertexAndConstantBuffer );
}

ByteArray SimpleTriangleExample::VertexShader( )
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

    return InteropUtilities::StringToBytes( shaderCode );
}

ByteArray SimpleTriangleExample::PixelShader( )
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

    return InteropUtilities::StringToBytes( shaderCode );
}
