/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICUL   AR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <DenOfIzGraphics/Backends/Metal/MetalEnumConverter.h>
#include <DenOfIzGraphics/Backends/Metal/MetalPipeline.h>

using namespace DenOfIz;

MetalPipeline::MetalPipeline( MetalContext *context, const PipelineDesc &desc ) : m_context( context ), m_desc( desc )
{
    switch ( desc.BindPoint )
    {
    case BindPoint::Graphics:
        CreateGraphicsPipeline( );
        break;
    case BindPoint::Compute:
        CreateComputePipeline( );
        break;
    case BindPoint::RayTracing:
        CreateRayTracingPipeline( );
        break;
    }
}

MetalPipeline::~MetalPipeline( )
{
}

void MetalPipeline::CreateGraphicsPipeline( )
{
    id<MTLFunction> vertexFunction   = nullptr;
    id<MTLFunction> fragmentFunction = nullptr;

    for ( const auto &shader : m_desc.ShaderProgram->GetCompiledShaders( ) )
    {
        std::string entryPoint = shader->EntryPoint;
        switch ( shader->Stage )
        {
        case ShaderStage::Vertex:
            vertexFunction = CreateShaderFunction( shader->Blob, shader->EntryPoint );
            break;
        case ShaderStage::Pixel:
            fragmentFunction = CreateShaderFunction( shader->Blob, shader->EntryPoint );
            break;
        default:
            LOG( ERROR ) << "Unsupported shader stage: " << static_cast<int>( shader->Stage );
            break;
        }
    }

    if ( vertexFunction == nullptr || fragmentFunction == nullptr )
    {
        LOG( ERROR ) << "Failed to create vertex or fragment function";
        return;
    }
    // Configure a pipeline descriptor that is used to create a pipeline state.
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label                        = @"Graphics Pipeline#";
    pipelineStateDescriptor.vertexFunction               = vertexFunction;
    pipelineStateDescriptor.fragmentFunction             = fragmentFunction;
    pipelineStateDescriptor.inputPrimitiveTopology       = MetalEnumConverter::ConvertTopologyClass( m_desc.PrimitiveTopology );
    pipelineStateDescriptor.sampleCount                  = MSAASampleCountToNumSamples( m_desc.MSAASampleCount );

    auto *mtkInputLayout                     = dynamic_cast<MetalInputLayout *>( m_desc.InputLayout );
    pipelineStateDescriptor.vertexDescriptor = mtkInputLayout->GetVertexDescriptor( );

    int attachmentIdx = 0;
    for ( const auto &attachment : m_desc.Rendering.RenderTargets )
    {
        MTLRenderPipelineColorAttachmentDescriptor *metalColorAttachment = pipelineStateDescriptor.colorAttachments[ attachmentIdx ];
        metalColorAttachment.pixelFormat                                 = MetalEnumConverter::ConvertFormat( attachment.Format );

        const BlendDesc &blendDesc = attachment.Blend;

        metalColorAttachment.blendingEnabled = blendDesc.Enable;

        metalColorAttachment.sourceRGBBlendFactor      = MetalEnumConverter::ConvertBlendFactor( blendDesc.SrcBlend );
        metalColorAttachment.destinationRGBBlendFactor = MetalEnumConverter::ConvertBlendFactor( blendDesc.DstBlend );
        metalColorAttachment.rgbBlendOperation         = MetalEnumConverter::ConvertBlendOp( blendDesc.BlendOp );

        metalColorAttachment.sourceAlphaBlendFactor      = MetalEnumConverter::ConvertBlendFactor( blendDesc.SrcBlendAlpha );
        metalColorAttachment.destinationAlphaBlendFactor = MetalEnumConverter::ConvertBlendFactor( blendDesc.DstBlendAlpha );
        metalColorAttachment.alphaBlendOperation         = MetalEnumConverter::ConvertBlendOp( blendDesc.BlendOpAlpha );

        metalColorAttachment.writeMask = blendDesc.RenderTargetWriteMask;
    }

    pipelineStateDescriptor.depthAttachmentPixelFormat   = MetalEnumConverter::ConvertFormat( m_desc.Rendering.DepthStencilAttachmentFormat );
    pipelineStateDescriptor.stencilAttachmentPixelFormat = MetalEnumConverter::ConvertFormat( m_desc.Rendering.DepthStencilAttachmentFormat );
    pipelineStateDescriptor.alphaToCoverageEnabled       = m_desc.Rendering.AlphaToCoverageEnable;
    pipelineStateDescriptor.alphaToOneEnabled            = m_desc.Rendering.IndependentBlendEnable;

    NSError *error          = nullptr;
    m_graphicsPipelineState = [m_context->Device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];

    if ( !m_graphicsPipelineState )
    {
        DZ_LOG_NS_ERROR( "Failed to create pipeline state", error );
    }

    MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
    depthStencilDescriptor.depthCompareFunction       = MetalEnumConverter::ConvertCompareOp( m_desc.DepthTest.CompareOp );
    depthStencilDescriptor.depthWriteEnabled          = m_desc.DepthTest.Write;
    depthStencilDescriptor.frontFaceStencil           = [[MTLStencilDescriptor alloc] init];
    InitStencilFace( depthStencilDescriptor.frontFaceStencil, m_desc.StencilTest.FrontFace );
    depthStencilDescriptor.backFaceStencil = [[MTLStencilDescriptor alloc] init];
    InitStencilFace( depthStencilDescriptor.backFaceStencil, m_desc.StencilTest.BackFace );
    m_depthStencilState = [m_context->Device newDepthStencilStateWithDescriptor:depthStencilDescriptor];

    switch ( m_desc.CullMode )
    {
    case CullMode::None:
        m_cullMode = MTLCullModeNone;
        break;
    case CullMode::FrontFace:
        m_cullMode = MTLCullModeFront;
        break;
    case CullMode::BackFace:
        m_cullMode = MTLCullModeBack;
        break;
    }
}

void MetalPipeline::CreateComputePipeline( )
{
    id<MTLFunction> computeFunction = nullptr;

    for ( const auto &shader : m_desc.ShaderProgram->GetCompiledShaders( ) )
    {
        switch ( shader->Stage )
        {
        case ShaderStage::Compute:
            computeFunction = CreateShaderFunction( shader->Blob, shader->EntryPoint );
            break;
        default:
            LOG( ERROR ) << "Unsupported shader stage: " << static_cast<int>( shader->Stage );
            break;
        }
    }

    if ( computeFunction == nullptr )
    {
        LOG( ERROR ) << "Failed to create compute function";
        return;
    }

    // Configure a pipeline descriptor that is used to create a pipeline state.
    MTLComputePipelineDescriptor *pipelineStateDescriptor = [[MTLComputePipelineDescriptor alloc] init];
    pipelineStateDescriptor.label                         = @"Compute Pipeline#";
    pipelineStateDescriptor.computeFunction               = computeFunction;

    MTLNewComputePipelineStateCompletionHandler completionHandler = ^( id<MTLComputePipelineState> pipelineState, NSError *err ) {
      if ( err != nil )
      {
          DZ_LOG_NS_ERROR( "Error creating pipeline state", err );
      }
    };

    NSError          *error   = nil;
    MTLPipelineOption options = MTLPipelineOptionArgumentInfo;
    m_computePipelineState    = [m_context->Device newComputePipelineStateWithDescriptor:pipelineStateDescriptor options:MTLPipelineOptionNone reflection:nil error:&error];

    if ( !m_computePipelineState )
    {
        DZ_LOG_NS_ERROR( "Failed to create pipeline state", error );
    }
}

void MetalPipeline::CreateRayTracingPipeline( )
{
    // TODO
}

id<MTLFunction> MetalPipeline::CreateShaderFunction( IDxcBlob *&blob, const std::string &entryPoint )
{
    NSError        *error      = nullptr;
    dispatch_data_t shaderData = dispatch_data_create( blob->GetBufferPointer( ), blob->GetBufferSize( ), NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT );
    id<MTLLibrary>  library    = [m_context->Device newLibraryWithData:shaderData error:&error];
    if ( error != nil )
    {
        DZ_LOG_NS_ERROR( "Error creating library", error );
        return nil;
    }

    // Create a function from the library
    NSString       *entryPointName = [NSString stringWithUTF8String:entryPoint.c_str( )];
    id<MTLFunction> function       = [library newFunctionWithName:entryPointName];
    if ( function == nil )
    {
        LOG( ERROR ) << "Error creating function for entry point: " << entryPointName;
    }
    return function;
}

void MetalPipeline::InitStencilFace( MTLStencilDescriptor *stencilDesc, const StencilFace &stencilFace )
{
    stencilDesc.stencilCompareFunction    = MetalEnumConverter::ConvertCompareOp( stencilFace.CompareOp );
    stencilDesc.stencilFailureOperation   = MetalEnumConverter::ConvertStencilOp( stencilFace.FailOp );
    stencilDesc.depthFailureOperation     = MetalEnumConverter::ConvertStencilOp( stencilFace.DepthFailOp );
    stencilDesc.depthStencilPassOperation = MetalEnumConverter::ConvertStencilOp( stencilFace.PassOp );
    stencilDesc.readMask                  = m_desc.StencilTest.ReadMask;
    stencilDesc.writeMask                 = m_desc.StencilTest.WriteMask;
}
