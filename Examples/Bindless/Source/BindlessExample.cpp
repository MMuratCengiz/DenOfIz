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
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Vector2d/ThorVGWrapper.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzGraphics/Utilities/InteropUtilities.h"
#include <cmath>
#include <cstring>

using namespace DenOfIz;

void BindlessExample::Init( )
{
    CreateVertexBuffer( );
    CreateTextures( );
    CreateSampler( );
    CreateConstantBuffer( );

    ShaderProgramDesc shaderProgramDesc{ };

    ShaderStageDesc &vertexShaderDesc = shaderProgramDesc.ShaderStages.EmplaceElement( );
    vertexShaderDesc.Stage            = ShaderStage::Vertex;
    vertexShaderDesc.EntryPoint       = "VSMain";
    vertexShaderDesc.Data             = VertexShader( );

    ShaderStageDesc &pixelShaderDesc = shaderProgramDesc.ShaderStages.EmplaceElement( );
    pixelShaderDesc.Stage            = ShaderStage::Pixel;
    pixelShaderDesc.EntryPoint       = "PSMain";
    pixelShaderDesc.Data             = PixelShader( );

    // Mark texture array as bindless at binding 0, space 0 with max 4 textures
    pixelShaderDesc.Bindless.MarkSrvAsBindlessArray( 0, 0, NUM_TEXTURES );

    m_program                           = std::make_unique<ShaderProgram>( shaderProgramDesc );
    const ShaderReflectDesc reflectDesc = m_program->Reflect( );
    m_inputLayout                       = std::unique_ptr<IInputLayout>( m_logicalDevice->CreateInputLayout( reflectDesc.InputLayout ) );
    m_rootSignature                     = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflectDesc.RootSignature ) );

    // Create resource bind group for bindless textures
    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_rootSignature.get( );
    bindGroupDesc.RegisterSpace = 0;
    m_bindGroup                 = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );

    m_bindGroup->BeginUpdate( );
    InteropArray<ITextureResource *> textureArray;
    for ( const auto &m_texture : m_textures )
    {
        textureArray.AddElement( m_texture.get( ) );
    }
    m_bindGroup->SrvArray( 0, textureArray );
    m_bindGroup->Sampler( 0, m_sampler.get( ) );
    m_bindGroup->EndUpdate( );

    bindGroupDesc.RegisterSpace = 1;
    m_perFrameBindGroup         = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.InputLayout   = m_inputLayout.get( );
    pipelineDesc.ShaderProgram = m_program.get( );
    pipelineDesc.RootSignature = m_rootSignature.get( );
    pipelineDesc.Graphics.RenderTargets.AddElement( { .Format = Format::B8G8R8A8Unorm } );

    m_pipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );
}

void BindlessExample::ModifyApiPreferences( APIPreference &defaultApiPreference )
{
    defaultApiPreference.Windows = APIPreferenceWindows::Vulkan;
}

void BindlessExample::Update( )
{
    m_worldData.DeltaTime = m_stepTimer.GetDeltaTime( );
    m_worldData.Camera->Update( m_worldData.DeltaTime );
    m_elapsedTime += m_worldData.DeltaTime;

    // Update texture index every second
    static float timer = 0.0f;
    timer += m_worldData.DeltaTime;
    if ( timer > 1.0f )
    {
        m_currentTextureIndex = ( m_currentTextureIndex + 1 ) % NUM_TEXTURES;
        timer                 = 0.0f;
    }

    // Update constant buffer with current texture index
    PerFrameData perFrameData;
    perFrameData.textureIndex = m_currentTextureIndex;
    perFrameData.time         = m_elapsedTime;

    void *mappedData = m_constantBuffer->MapMemory( );
    memcpy( mappedData, &perFrameData, sizeof( PerFrameData ) );
    m_constantBuffer->UnmapMemory( );

    RenderAndPresentFrame( );
}

void BindlessExample::Render( const uint32_t frameIndex, ICommandList *commandList )
{
    commandList->Begin( );
    ITextureResource *renderTarget = m_swapChain->GetRenderTarget( m_frameSync->AcquireNextImage( frameIndex ) );

    BatchTransitionDesc batchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::RenderTarget );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.AddElement( { .Resource = renderTarget } );
    commandList->BeginRendering( renderingDesc );

    const auto viewport = m_swapChain->GetViewport( );
    commandList->BindViewport( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindScissorRect( viewport.X, viewport.Y, viewport.Width, viewport.Height );
    commandList->BindPipeline( m_pipeline.get( ) );
    commandList->BindVertexBuffer( m_vertexBuffer.get( ) );
    commandList->BindResourceGroup( m_bindGroup.get( ) );

    m_perFrameBindGroup->BeginUpdate( );
    m_perFrameBindGroup->Cbv( 0, m_constantBuffer.get( ) );
    m_perFrameBindGroup->EndUpdate( );
    commandList->BindResourceGroup( m_perFrameBindGroup.get( ) );

    commandList->Draw( 3, 1, 0, 0 );

    commandList->EndRendering( );

    batchTransitionDesc = BatchTransitionDesc{ commandList };
    batchTransitionDesc.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransitionDesc );

    commandList->End( );
}

void BindlessExample::HandleEvent( Event &event )
{
    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void BindlessExample::Quit( )
{
    m_frameSync->WaitIdle( );
    IExample::Quit( );
}

void BindlessExample::CreateVertexBuffer( )
{
    constexpr std::array vertices = {
        // Position (XYZ)    // UV (XY)
        0.0f,  0.5f,  0.0f, 0.5f, 0.0f, // Top vertex
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom left
        0.5f,  -0.5f, 0.0f, 1.0f, 1.0f  // Bottom right
    };

    BufferDesc bufferDesc{ };
    bufferDesc.Descriptor.Set( ResourceDescriptor::VertexBuffer );
    bufferDesc.NumBytes  = vertices.size( ) * sizeof( float );
    bufferDesc.DebugName = "TriangleVertexBuffer";

    m_vertexBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( bufferDesc ) );

    BatchResourceCopy batchCopy( m_logicalDevice );
    batchCopy.Begin( );

    CopyToGpuBufferDesc copyDesc{ };
    copyDesc.DstBuffer = m_vertexBuffer.get( );
    copyDesc.Data.MemCpy( vertices.data( ), vertices.size( ) * sizeof( float ) );
    batchCopy.CopyToGPUBuffer( copyDesc );
    batchCopy.Submit( );

    m_resourceTracking.TrackBuffer( m_vertexBuffer.get( ), ResourceUsage::VertexAndConstantBuffer );
}

InteropArray<Byte> BindlessExample::VertexShader( )
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

    const InteropString str( shaderCode );
    return InteropUtilities::StringToBytes( str );
}

InteropArray<Byte> BindlessExample::PixelShader( )
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

    const InteropString str( shaderCode );
    return InteropUtilities::StringToBytes( str );
}

void BindlessExample::CreateTextures( )
{
    // Create 4 different textures
    for ( int i = 0; i < NUM_TEXTURES; ++i )
    {
        constexpr int    width  = 256;
        constexpr int    height = 256;
        ThorVGCanvasDesc canvasDesc{ };
        canvasDesc.Width  = width;
        canvasDesc.Height = height;
        ThorVGCanvas canvas( canvasDesc );

        // Create different patterns for each texture
        ThorVGShape shape;

        switch ( i )
        {
        case 0: // Circles pattern
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

        case 1: // Gradient rectangle
            {
                shape.AppendRect( 0, 0, width, height );

                ThorVGLinearGradient gradient;
                gradient.Linear( 0, 0, width, height );

                InteropArray<ThorVGColorStop> colorStops;
                colorStops.Resize( 4 );
                colorStops.SetElement( 0, { 0.0f, 255, 0, 128, 255 } );
                colorStops.SetElement( 1, { 0.33f, 255, 255, 0, 255 } );
                colorStops.SetElement( 2, { 0.66f, 0, 255, 255, 255 } );
                colorStops.SetElement( 3, { 1.0f, 128, 0, 255, 255 } );
                gradient.ColorStops( colorStops );

                shape.Fill( &gradient );
                canvas.Push( &shape );
            }
            break;

        case 2: // Star pattern
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

        case 3: // Checkerboard pattern
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

        TextureDesc textureDesc{ };
        textureDesc.Width      = width;
        textureDesc.Height     = height;
        textureDesc.Format     = Format::R8G8B8A8Unorm;
        textureDesc.Descriptor = ResourceDescriptor::Texture;
        textureDesc.Usages     = ResourceUsage::ShaderResource;
        textureDesc.DebugName  = InteropString( "BindlessTexture_" ).Append( std::to_string( i ).c_str( ) );

        m_textures[ i ] = std::unique_ptr<ITextureResource>( m_logicalDevice->CreateTextureResource( textureDesc ) );

        BatchResourceCopy batchCopy( m_logicalDevice );
        batchCopy.Begin( );

        CopyDataToTextureDesc copyDesc{ };
        copyDesc.Data       = canvas.GetDataAsBytes( );
        copyDesc.DstTexture = m_textures[ i ].get( );
        copyDesc.MipLevel   = 0;
        batchCopy.CopyDataToTexture( copyDesc );

        batchCopy.Submit( );

        m_resourceTracking.TrackTexture( m_textures[ i ].get( ), ResourceUsage::ShaderResource );
    }
}

void BindlessExample::CreateSampler( )
{
    SamplerDesc samplerDesc{ };
    samplerDesc.MinFilter     = Filter::Linear;
    samplerDesc.MagFilter     = Filter::Linear;
    samplerDesc.MipmapMode    = MipmapMode::Linear;
    samplerDesc.AddressModeU  = SamplerAddressMode::Repeat;
    samplerDesc.AddressModeV  = SamplerAddressMode::Repeat;
    samplerDesc.AddressModeW  = SamplerAddressMode::Repeat;
    samplerDesc.MaxAnisotropy = 1.0f;
    samplerDesc.MinLod        = 0.0f;
    samplerDesc.MaxLod        = 1.0f;

    m_sampler = std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( samplerDesc ) );
}

void BindlessExample::CreateConstantBuffer( )
{
    BufferDesc bufferDesc{ };
    bufferDesc.Descriptor.Set( ResourceDescriptor::UniformBuffer );
    bufferDesc.NumBytes  = sizeof( PerFrameData );
    bufferDesc.DebugName = "PerFrameConstantBuffer";
    bufferDesc.HeapType  = HeapType::CPU_GPU;

    m_constantBuffer = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( bufferDesc ) );

    PerFrameData initialData;
    initialData.textureIndex = 0;
    initialData.time         = 0.0f;

    // Map and write initial data
    void *mappedData = m_constantBuffer->MapMemory( );
    memcpy( mappedData, &initialData, sizeof( PerFrameData ) );
    m_constantBuffer->UnmapMemory( );

    m_resourceTracking.TrackBuffer( m_constantBuffer.get( ), ResourceUsage::VertexAndConstantBuffer );
}
