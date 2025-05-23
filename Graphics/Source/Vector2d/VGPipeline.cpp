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

#include <DenOfIzGraphics/Vector2d/VGPipeline.h>

#include "DenOfIzGraphics/Renderer/Graph/RenderGraphNodeDesc.h"
#include "DenOfIzGraphics/Utilities/InteropUtilities.h"

using namespace DenOfIz;

VGPipeline::VGPipeline( const VGPipelineDesc &desc )
{
    auto device = desc.LogicalDevice;
    if ( !device )
    {
        LOG( ERROR ) << "VGPipelineDesc::LogicalDevice is null";
        return;
    }

    if ( desc.SetupData )
    {
        m_alignedElementNumBytes = Utilities::Align( sizeof( Float_4x4 ), device->DeviceInfo( ).Constants.ConstantBufferAlignment );

        BufferDesc bufferDesc{ };
        bufferDesc.HeapType   = HeapType::CPU_GPU;
        bufferDesc.Usages     = ResourceUsage::VertexAndConstantBuffer;
        bufferDesc.NumBytes   = m_alignedElementNumBytes * desc.NumFrames;
        bufferDesc.Descriptor = ResourceDescriptor::Buffer;
        m_data                = std::unique_ptr<IBufferResource>( device->CreateBufferResource( bufferDesc ) );
        m_dataMappedMemory    = static_cast<Byte *>( m_data->MapMemory( ) );
    }

    InteropArray<Byte> vsData = desc.VertexShaderOverride;
    if ( vsData.NumElements( ) == 0 )
    {
        vsData = GetVertexShader( );
    }

    InteropArray<Byte> psData = desc.PixelShaderOverride;
    if ( psData.NumElements( ) == 0 )
    {
        psData = GetPixelShader( );
    }

    ShaderProgramDesc shaderProgramDesc;
    ShaderStageDesc  &vertexShaderDesc = shaderProgramDesc.ShaderStages.EmplaceElement( );
    vertexShaderDesc.Stage             = ShaderStage::Vertex;
    vertexShaderDesc.EntryPoint        = "VSMain";
    vertexShaderDesc.Data              = vsData;
    ShaderStageDesc &pixelShaderDesc   = shaderProgramDesc.ShaderStages.EmplaceElement( );
    pixelShaderDesc.Stage              = ShaderStage::Pixel;
    pixelShaderDesc.EntryPoint         = "PSMain";
    pixelShaderDesc.Data               = psData;

    m_program = std::make_unique<ShaderProgram>( shaderProgramDesc );

    ShaderReflectDesc reflectDesc = m_program->Reflect( );

    m_rootSignature = std::unique_ptr<IRootSignature>( device->CreateRootSignature( reflectDesc.RootSignature ) );
    m_inputLayout   = std::unique_ptr<IInputLayout>( device->CreateInputLayout( reflectDesc.InputLayout ) );

    m_bindGroupsPerFrame.resize( desc.NumFrames );
    for ( int frame = 0; frame < desc.NumFrames; frame++ )
    {
        auto &bindGroups = m_bindGroupsPerFrame[ frame ].BindGroups;
        for ( int i = 0; i < reflectDesc.RootSignature.ResourceBindings.NumElements( ); i++ )
        {
            auto resourceBinding = reflectDesc.RootSignature.ResourceBindings.GetElement( i );
            if ( resourceBinding.RegisterSpace >= bindGroups.size( ) )
            {
                bindGroups.resize( resourceBinding.RegisterSpace + 1 );
                ResourceBindGroupDesc resourceBindGroupDesc{ };
                resourceBindGroupDesc.RootSignature         = m_rootSignature.get( );
                resourceBindGroupDesc.RegisterSpace         = resourceBinding.RegisterSpace;
                bindGroups[ resourceBinding.RegisterSpace ] = std::unique_ptr<IResourceBindGroup>( device->CreateResourceBindGroup( resourceBindGroupDesc ) );
            }
        }
        if ( reflectDesc.RootSignature.RootConstants.NumElements( ) > 0 )
        {
            uint32_t rcSpace = DZConfiguration::Instance( ).RootConstantRegisterSpace;
            bindGroups.resize( rcSpace + 1 );
            ResourceBindGroupDesc resourceBindGroupDesc{ };
            resourceBindGroupDesc.RootSignature = m_rootSignature.get( );
            resourceBindGroupDesc.RegisterSpace = rcSpace;
            bindGroups[ rcSpace ]               = std::unique_ptr<IResourceBindGroup>( device->CreateResourceBindGroup( resourceBindGroupDesc ) );
        }
    }

    if ( desc.SetupData )
    {
        for ( int i = 0; i < desc.NumFrames; i++ )
        {
            auto &bindGroups = m_bindGroupsPerFrame[ i ].BindGroups;
            bindGroups[ 0 ]->BeginUpdate( );
            BindBufferDesc bindBufferDesc{ };
            bindBufferDesc.Binding        = 0;
            bindBufferDesc.Resource       = m_data.get( );
            bindBufferDesc.ResourceOffset = m_alignedElementNumBytes * i;
            bindGroups[ 0 ]->Cbv( bindBufferDesc );
            bindGroups[ 0 ]->EndUpdate( );
        }
    }

    PipelineDesc pipelineDesc;
    pipelineDesc.BindPoint     = BindPoint::Graphics;
    pipelineDesc.InputLayout   = m_inputLayout.get( );
    pipelineDesc.ShaderProgram = m_program.get( );
    pipelineDesc.RootSignature = m_rootSignature.get( );

    RenderTargetDesc rtDesc;
    rtDesc.Format              = Format::B8G8R8A8Unorm;
    rtDesc.Blend.Enable        = true;
    rtDesc.Blend.SrcBlend      = Blend::SrcAlpha;
    rtDesc.Blend.DstBlend      = Blend::InvSrcAlpha;
    rtDesc.Blend.DstBlendAlpha = Blend::InvSrcAlpha;

    pipelineDesc.Graphics.RenderTargets.AddElement( rtDesc );
    pipelineDesc.Graphics.PrimitiveTopology = PrimitiveTopology::Triangle;
    pipelineDesc.Graphics.CullMode          = CullMode::None;
    pipelineDesc.Graphics.FillMode          = FillMode::Solid;
    pipelineDesc.Graphics.DepthTest.Enable  = false;

    m_pipeline = std::unique_ptr<IPipeline>( device->CreatePipeline( pipelineDesc ) );
}

void VGPipeline::UpdateProjection( const uint32_t &frameIndex, const Float_4x4 &projection ) const
{
    std::memcpy( m_dataMappedMemory + m_alignedElementNumBytes * frameIndex, &projection, sizeof( Float_4x4 ) );
}

IResourceBindGroup *VGPipeline::GetBindGroup( const uint32_t &frameIndex, const uint32_t &registerSpace ) const
{
    return m_bindGroupsPerFrame[ frameIndex ].BindGroups[ registerSpace ].get( );
}

IPipeline *VGPipeline::GetPipeline( ) const
{
    return m_pipeline.get( );
}

IInputLayout *VGPipeline::GetInputLayout( ) const
{
    return m_inputLayout.get( );
}

InteropArray<Byte> VGPipeline::GetVertexShader( )
{
    const auto shaderCode = R"(
    cbuffer MatrixBuffer : register(b0)
    {
        float4x4 Projection;
    };

    struct VSInput
    {
        float2 Position : POSITION;
        float4 Color : COLOR;
        float2 TexCoord : TEXCOORD0;
        float4 GradientData : TEXCOORD1;
    };

    struct PSInput
    {
        float4 Position : SV_POSITION;
        float4 Color : COLOR;
        float2 TexCoord : TEXCOORD0;
        float4 GradientData : TEXCOORD1;
    };

    PSInput VSMain(VSInput input)
    {
        PSInput output;
        output.Position = mul(float4(input.Position, 0.0, 1.0), Projection);
        output.Color = input.Color;
        output.TexCoord = input.TexCoord;
        output.GradientData = input.GradientData;
        return output;
    }
    )";

    const InteropString str( shaderCode );
    return InteropUtilities::StringToBytes( str );
}

InteropArray<Byte> VGPipeline::GetPixelShader( )
{
    const auto shaderCode = R"(
    struct PSInput
    {
        float4 Position : SV_POSITION;
        float4 Color : COLOR;
        float2 TexCoord : TEXCOORD0;
        float4 GradientData : TEXCOORD1;
    };

    float4 PSMain(PSInput input) : SV_TARGET
    {
        float4 color = input.Color;
        float edgeDistance = input.GradientData.z;
        if (edgeDistance > 0.0f)
        {
            float alpha = 1.0f - saturate(edgeDistance);
            color.a *= alpha;
        }
        
        return color;
    }
    )";

    const InteropString str( shaderCode );
    return InteropUtilities::StringToBytes( str );
}
