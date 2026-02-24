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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUPipeline.h"
#include <string>
#include <vector>
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IInputLayout.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUContext.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUEnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUInputLayout.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPURootSignature.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

#define ROOT_SIGNATURE_PTR( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IRootSignature, handle )
#define INPUT_LAYOUT_PTR( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IInputLayout, handle )

WebGPUPipeline::WebGPUPipeline( WebGPUContext *context, const DenOfIz_PipelineDesc &desc ) : m_context( context ), m_desc( desc )
{
    if ( DENOFIZ_HANDLE_IS_VALID( desc.RootSignature ) )
    {
        m_rootSignature = dynamic_cast<WebGPURootSignature *>( ROOT_SIGNATURE_PTR( desc.RootSignature ) );
        if ( m_rootSignature )
        {
            m_pipelineLayout = m_rootSignature->GetPipelineLayout( );
        }
    }

    switch ( desc.BindPoint )
    {
    case DENOFIZ_BIND_POINT_GRAPHICS:
    case DENOFIZ_BIND_POINT_MESH:
        CreateRenderPipeline( );
        break;
    case DENOFIZ_BIND_POINT_COMPUTE:
        CreateComputePipeline( );
        break;
    case DENOFIZ_BIND_POINT_RAYTRACING:
        spdlog::error( "Ray tracing pipelines not supported in WebGPU" );
        break;
    }
}

WebGPUPipeline::~WebGPUPipeline( )
{
    if ( m_renderPipeline )
    {
        wgpuRenderPipelineRelease( m_renderPipeline );
    }
    if ( m_computePipeline )
    {
        wgpuComputePipelineRelease( m_computePipeline );
    }
}

WGPURenderPipeline WebGPUPipeline::GetRenderPipeline( ) const
{
    return m_renderPipeline;
}

WGPUComputePipeline WebGPUPipeline::GetComputePipeline( ) const
{
    return m_computePipeline;
}

DenOfIz_BindPoint WebGPUPipeline::GetBindPoint( ) const
{
    return m_desc.BindPoint;
}

WebGPURootSignature *WebGPUPipeline::RootSignature( ) const
{
    return m_rootSignature;
}

void WebGPUPipeline::CreateRenderPipeline( )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_desc.ShaderProgram ) )
    {
        spdlog::error( "No shader program provided for render pipeline" );
        return;
    }

    WGPURenderPipelineDescriptor pipelineDesc{ };
    pipelineDesc.layout = m_pipelineLayout;

    DenOfIz_CompiledShaderStageArray compiledShaders;
    DenOfIz_ShaderProgram_CompiledShaders( m_desc.ShaderProgram, &compiledShaders );
    DenOfIz_CompiledShaderStage *vertexShader = nullptr;
    DenOfIz_CompiledShaderStage *pixelShader  = nullptr;
    for ( size_t i = 0; i < compiledShaders.NumElements; ++i )
    {
        if ( compiledShaders.Elements[ i ]->Stage == DENOFIZ_SHADER_STAGE_VERTEX_BIT )
        {
            vertexShader = compiledShaders.Elements[ i ];
        }
        else if ( compiledShaders.Elements[ i ]->Stage == DENOFIZ_SHADER_STAGE_PIXEL_BIT )
        {
            pixelShader = compiledShaders.Elements[ i ];
        }
    }

    WGPUShaderModule vertexModule = nullptr;
    if ( vertexShader && HasCompatibleShader( vertexShader ) )
    {
        WGPUVertexState vertexState{ };
        vertexModule           = CreateModule( vertexShader );
        vertexState.module     = vertexModule;
#if DZ_WEBGPU_USE_DAWN_API
        vertexState.entryPoint = { vertexShader->EntryPoint.Chars, vertexShader->EntryPoint.NumChars };
#else
        vertexState.entryPoint = vertexShader->EntryPoint.Chars;
#endif

        if ( DENOFIZ_HANDLE_IS_VALID( m_desc.InputLayout ) )
        {
            auto *webgpuInputLayout = dynamic_cast<WebGPUInputLayout *>( INPUT_LAYOUT_PTR( m_desc.InputLayout ) );
            if ( webgpuInputLayout )
            {
                vertexState.bufferCount = webgpuInputLayout->GetNumBuffers( );
                vertexState.buffers     = webgpuInputLayout->GetVertexBufferLayouts( );
            }
        }

        pipelineDesc.vertex = vertexState;
    }
    else
    {
        spdlog::error( "No vertex shader found in shader program" );
        return;
    }

    WGPUFragmentState                 fragmentState{ };
    WGPUShaderModule                  fragmentModule = nullptr;
    std::vector<WGPUBlendState>       blendStates;
    std::vector<WGPUColorTargetState> colorTargets;

    if ( pixelShader && HasCompatibleShader( pixelShader ) )
    {
        fragmentModule           = CreateModule( pixelShader );
        fragmentState.module     = fragmentModule;
#if DZ_WEBGPU_USE_DAWN_API
        fragmentState.entryPoint = { pixelShader->EntryPoint.Chars, pixelShader->EntryPoint.NumChars };
#else
        fragmentState.entryPoint = pixelShader->EntryPoint.Chars;
#endif

        for ( uint32_t i = 0; i < m_desc.Graphics.RenderTargets.NumElements; ++i )
        {
            const auto &rtDesc = m_desc.Graphics.RenderTargets.Elements[ i ];

            WGPUColorTargetState colorTarget{ };
            colorTarget.format    = DenOfIz_WebGPUEnumConverter_ConvertFormat( rtDesc.Format );
            colorTarget.writeMask = rtDesc.Blend.RenderTargetWriteMask;

            if ( rtDesc.Blend.Enable )
            {
                WGPUBlendState blendState{ };
                blendState.color.operation = DenOfIz_WebGPUEnumConverter_ConvertBlendOp( rtDesc.Blend.BlendOp );
                blendState.color.srcFactor = DenOfIz_WebGPUEnumConverter_ConvertBlend( rtDesc.Blend.SrcBlend );
                blendState.color.dstFactor = DenOfIz_WebGPUEnumConverter_ConvertBlend( rtDesc.Blend.DstBlend );
                blendState.alpha.operation = DenOfIz_WebGPUEnumConverter_ConvertBlendOp( rtDesc.Blend.BlendOpAlpha );
                blendState.alpha.srcFactor = DenOfIz_WebGPUEnumConverter_ConvertBlend( rtDesc.Blend.SrcBlendAlpha );
                blendState.alpha.dstFactor = DenOfIz_WebGPUEnumConverter_ConvertBlend( rtDesc.Blend.DstBlendAlpha );

                blendStates.push_back( blendState );
                colorTarget.blend = &blendStates.back( );
            }

            colorTargets.push_back( colorTarget );
        }

        fragmentState.targetCount = static_cast<uint32_t>( colorTargets.size( ) );
        fragmentState.targets     = colorTargets.data( );
        pipelineDesc.fragment     = &fragmentState;
    }

    WGPUPrimitiveState primitiveState{ };
    primitiveState.topology         = DenOfIz_WebGPUEnumConverter_ConvertPrimitiveTopology( m_desc.Graphics.PrimitiveTopology );
    primitiveState.stripIndexFormat = WGPUIndexFormat_Undefined;
    primitiveState.frontFace        = m_desc.Graphics.Rasterization.FrontCounterClockwise ? WGPUFrontFace_CCW : WGPUFrontFace_CW;
    primitiveState.cullMode         = DenOfIz_WebGPUEnumConverter_ConvertCullMode( m_desc.Graphics.CullMode );
    pipelineDesc.primitive          = primitiveState;

    WGPUDepthStencilState depthStencilState{ };
    if ( m_desc.Graphics.DepthStencilAttachmentFormat != DENOFIZ_FORMAT_UNDEFINED )
    {
        depthStencilState.format            = DenOfIz_WebGPUEnumConverter_ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
#if DZ_WEBGPU_USE_DAWN_API
        depthStencilState.depthWriteEnabled = m_desc.Graphics.DepthTest.Write ? WGPUOptionalBool_True : WGPUOptionalBool_False;
#else
        depthStencilState.depthWriteEnabled = m_desc.Graphics.DepthTest.Write;
#endif
        depthStencilState.depthCompare =
            m_desc.Graphics.DepthTest.Enable ? DenOfIz_WebGPUEnumConverter_ConvertCompareOp( m_desc.Graphics.DepthTest.CompareOp ) : WGPUCompareFunction_Always;

        depthStencilState.depthBias           = m_desc.Graphics.Rasterization.DepthBias;
        depthStencilState.depthBiasSlopeScale = m_desc.Graphics.Rasterization.SlopeScaledDepthBias;
        depthStencilState.depthBiasClamp      = m_desc.Graphics.Rasterization.DepthBiasClamp;

        const DenOfIz_Format dsvFormat     = m_desc.Graphics.DepthStencilAttachmentFormat;
        bool                 enableStencil = m_desc.Graphics.StencilTest.Enable;

        if ( enableStencil && DenOfIz_Format_IsDepthOnly( dsvFormat ) )
        {
            spdlog::warn( "WebGPUPipeline: Stencil operations enabled with depth-only format ({}). "
                          "Auto-disabling stencil test to comply with WebGPU format requirements.",
                          static_cast<int>( dsvFormat ) );
            enableStencil = false;
        }

        if ( enableStencil )
        {
            depthStencilState.stencilReadMask  = m_desc.Graphics.StencilTest.ReadMask;
            depthStencilState.stencilWriteMask = m_desc.Graphics.StencilTest.WriteMask;

            depthStencilState.stencilFront.compare     = DenOfIz_WebGPUEnumConverter_ConvertCompareOp( m_desc.Graphics.StencilTest.FrontFace.CompareOp );
            depthStencilState.stencilFront.failOp      = DenOfIz_WebGPUEnumConverter_ConvertStencilOp( m_desc.Graphics.StencilTest.FrontFace.FailOp );
            depthStencilState.stencilFront.depthFailOp = DenOfIz_WebGPUEnumConverter_ConvertStencilOp( m_desc.Graphics.StencilTest.FrontFace.DepthFailOp );
            depthStencilState.stencilFront.passOp      = DenOfIz_WebGPUEnumConverter_ConvertStencilOp( m_desc.Graphics.StencilTest.FrontFace.PassOp );

            depthStencilState.stencilBack.compare     = DenOfIz_WebGPUEnumConverter_ConvertCompareOp( m_desc.Graphics.StencilTest.BackFace.CompareOp );
            depthStencilState.stencilBack.failOp      = DenOfIz_WebGPUEnumConverter_ConvertStencilOp( m_desc.Graphics.StencilTest.BackFace.FailOp );
            depthStencilState.stencilBack.depthFailOp = DenOfIz_WebGPUEnumConverter_ConvertStencilOp( m_desc.Graphics.StencilTest.BackFace.DepthFailOp );
            depthStencilState.stencilBack.passOp      = DenOfIz_WebGPUEnumConverter_ConvertStencilOp( m_desc.Graphics.StencilTest.BackFace.PassOp );
        }

        pipelineDesc.depthStencil = &depthStencilState;
    }

    WGPUMultisampleState multisampleState{ };
    multisampleState.count                  = DenOfIz_WebGPUEnumConverter_ConvertSampleCount( m_desc.Graphics.MSAASampleCount );
    multisampleState.mask                   = 0xFFFFFFFF;
    multisampleState.alphaToCoverageEnabled = m_desc.Graphics.AlphaToCoverageEnable;
    pipelineDesc.multisample                = multisampleState;

    m_renderPipeline = wgpuDeviceCreateRenderPipeline( m_context->Device, &pipelineDesc );
    if ( !m_renderPipeline )
    {
        spdlog::error( "Failed to create render pipeline" );
    }

    if ( vertexModule )
    {
        wgpuShaderModuleRelease( vertexModule );
    }
    if ( fragmentModule )
    {
        wgpuShaderModuleRelease( fragmentModule );
    }
}

void WebGPUPipeline::CreateComputePipeline( )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_desc.ShaderProgram ) )
    {
        spdlog::error( "No shader program provided for compute pipeline" );
        return;
    }

    DenOfIz_CompiledShaderStageArray compiledShaders;
    DenOfIz_ShaderProgram_CompiledShaders( m_desc.ShaderProgram, &compiledShaders );
    const DenOfIz_CompiledShaderStage *computeShader = nullptr;
    for ( size_t i = 0; i < compiledShaders.NumElements; ++i )
    {
        if ( compiledShaders.Elements[ i ]->Stage == DENOFIZ_SHADER_STAGE_COMPUTE_BIT )
        {
            computeShader = compiledShaders.Elements[ i ];
            break;
        }
    }

    if ( !computeShader || !computeShader->WGSL.Elements || computeShader->WGSL.NumElements == 0 )
    {
        spdlog::error( "No compute shader found in shader program" );
        return;
    }

    const auto computeModule = CreateModule( computeShader );

    WGPUComputePipelineDescriptor pipelineDesc{ };
    pipelineDesc.layout             = m_pipelineLayout;
    pipelineDesc.compute.module     = computeModule;
#if DZ_WEBGPU_USE_DAWN_API
    pipelineDesc.compute.entryPoint = { computeShader->EntryPoint.Chars, computeShader->EntryPoint.NumChars };
#else
    pipelineDesc.compute.entryPoint = computeShader->EntryPoint.Chars;
#endif

    m_computePipeline = wgpuDeviceCreateComputePipeline( m_context->Device, &pipelineDesc );
    if ( !m_computePipeline )
    {
        spdlog::error( "Failed to create compute pipeline" );
    }

    if ( computeModule )
    {
        wgpuShaderModuleRelease( computeModule );
    }
}

bool WebGPUPipeline::HasCompatibleShader( const DenOfIz_CompiledShaderStage *stage )
{
    return stage->WGSL.Elements && stage->WGSL.NumElements > 0 || stage->SPIRV.Elements && stage->SPIRV.NumElements > 0;
}

WGPUShaderModule WebGPUPipeline::CreateModule( const DenOfIz_CompiledShaderStage *stage )
{
    WGPUShaderModuleDescriptor shaderDesc{ };
    shaderDesc.label = DZ_WEBGPU_NULL_STRING;

    if ( stage->WGSL.Elements && stage->WGSL.NumElements > 0 )
    {
#if DZ_WEBGPU_USE_DAWN_API
        WGPUShaderSourceWGSL wgslDesc{ };
        wgslDesc.chain.sType   = WGPUSType_ShaderSourceWGSL;
        wgslDesc.chain.next    = nullptr;
        wgslDesc.code.length   = stage->WGSL.NumElements;
        wgslDesc.code.data     = reinterpret_cast<const char *>( stage->WGSL.Elements );
        shaderDesc.nextInChain = &wgslDesc.chain;
        return wgpuDeviceCreateShaderModule( m_context->Device, &shaderDesc );
#else
        std::string wgslCode( reinterpret_cast<const char *>( stage->WGSL.Elements ), stage->WGSL.NumElements );
        WGPUShaderModuleWGSLDescriptor wgslDesc{ };
        wgslDesc.chain.sType   = WGPUSType_ShaderModuleWGSLDescriptor;
        wgslDesc.chain.next    = nullptr;
        wgslDesc.code          = wgslCode.c_str( );
        shaderDesc.nextInChain = &wgslDesc.chain;
        return wgpuDeviceCreateShaderModule( m_context->Device, &shaderDesc );
#endif
    }
    else if ( stage->SPIRV.Elements && stage->SPIRV.NumElements > 0 )
    {
#if DZ_WEBGPU_USE_DAWN_API
        WGPUShaderSourceSPIRV spirvDesc{ };
        spirvDesc.chain.sType  = WGPUSType_ShaderSourceSPIRV;
        spirvDesc.chain.next   = nullptr;
        spirvDesc.codeSize     = stage->SPIRV.NumElements / sizeof( uint32_t );
        spirvDesc.code         = reinterpret_cast<uint32_t *>( stage->SPIRV.Elements );
        shaderDesc.nextInChain = &spirvDesc.chain;
#else
        WGPUShaderModuleSPIRVDescriptor spirvDesc{ };
        spirvDesc.chain.sType  = WGPUSType_ShaderModuleSPIRVDescriptor;
        spirvDesc.chain.next   = nullptr;
        spirvDesc.codeSize     = stage->SPIRV.NumElements / sizeof( uint32_t );
        spirvDesc.code         = reinterpret_cast<uint32_t *>( stage->SPIRV.Elements );
        shaderDesc.nextInChain = &spirvDesc.chain;
#endif
        return wgpuDeviceCreateShaderModule( m_context->Device, &shaderDesc );
    }
    return nullptr;
}
