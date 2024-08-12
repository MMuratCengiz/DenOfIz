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
    NSError *error = nullptr;

    id<MTLFunction> vertexFunction   = nullptr;
    id<MTLFunction> fragmentFunction = nullptr;

    for ( const auto &shader : m_desc.ShaderProgram->GetCompiledShaders( ) )
    {
        const char *rawSource = reinterpret_cast<const char *>( shader->Blob->GetBufferPointer( ) );
        // TODO validate:
        std::string mslSource = std::string( rawSource, shader->Blob->GetBufferSize( ) );

        std::string entryPoint;

        switch ( shader->Stage )
        {
        case ShaderStage::Vertex:
            entryPoint     = shader->EntryPoint;
            vertexFunction = CreateShaderFunction( mslSource, entryPoint, &error );
            break;
        case ShaderStage::Pixel:
            entryPoint       = shader->EntryPoint;
            fragmentFunction = CreateShaderFunction( mslSource, entryPoint, &error );
            break;
        default:
            LOG( ERROR ) << "Unsupported shader stage: " << static_cast<int>( shader->Stage );
            break;
        }

        if ( error != nullptr )
        {
            LOG( ERROR ) << "Error creating shader function: " << [error localizedDescription];
            return;
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
    for ( const auto &attachment : m_desc.Rendering.ColorAttachmentFormats )
    {
        pipelineStateDescriptor.colorAttachments[ attachmentIdx ].pixelFormat = MetalEnumConverter::ConvertFormat( attachment );
    }
    pipelineStateDescriptor.depthAttachmentPixelFormat   = MetalEnumConverter::ConvertFormat( m_desc.Rendering.DepthAttachmentFormat );
    pipelineStateDescriptor.stencilAttachmentPixelFormat = MetalEnumConverter::ConvertFormat( m_desc.Rendering.StencilAttachmentFormat );

    m_graphicsPipelineState = [m_context->Device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];

    if ( !m_graphicsPipelineState )
    {
        LOG( ERROR ) << "Failed to create pipeline state" << error;
    }
}

void MetalPipeline::CreateComputePipeline( )
{
    NSError        *error           = nullptr;
    id<MTLFunction> computeFunction = nullptr;

    for ( const auto &shader : m_desc.ShaderProgram->GetCompiledShaders( ) )
    {
        const char *rawSource = reinterpret_cast<const char *>( shader->Blob->GetBufferPointer( ) );
        // TODO validate:
        std::string mslSource = std::string( rawSource, shader->Blob->GetBufferSize( ) );
        std::string entryPoint;
        switch ( shader->Stage )
        {
        case ShaderStage::Compute:
            entryPoint      = shader->EntryPoint;
            computeFunction = CreateShaderFunction( mslSource, entryPoint, &error );
            break;
        default:
            LOG( ERROR ) << "Unsupported shader stage: " << static_cast<int>( shader->Stage );
            break;
        }

        if ( error != nullptr )
        {
            LOG( ERROR ) << "Error creating shader function: " << [error localizedDescription];
            return;
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
          LOG( ERROR ) << "Error creating pipeline state: " << [err localizedDescription];
      }
    };

    MTLPipelineOption options = MTLPipelineOptionArgumentInfo;
    m_computePipelineState    = [m_context->Device newComputePipelineStateWithDescriptor:pipelineStateDescriptor options:MTLPipelineOptionNone reflection:nil error:&error];

    if ( !m_computePipelineState )
    {
        LOG( ERROR ) << "Failed to create pipeline state: " << error;
    }
}

void MetalPipeline::CreateRayTracingPipeline( )
{
    // TODO
}

id<MTLFunction> MetalPipeline::CreateShaderFunction( const std::string &shaderSource, const std::string &entryPoint, NSError **error )
{
    NSString      *mslSource = [NSString stringWithUTF8String:shaderSource.c_str( )];
    id<MTLLibrary> library   = [m_context->Device newLibraryWithSource:mslSource options:nil error:error];
    if ( *error != nil )
    {
        LOG( ERROR ) << "Error creating library: " << [*error localizedDescription];
        return nil;
    }

    // Create a function from the library
    NSString       *entryPointName = [NSString stringWithUTF8String:entryPoint.c_str( )];
    id<MTLFunction> function       = [library newFunctionWithName:entryPointName];
    if ( function == nil )
    {
        LOG( ERROR ) << "Error creating function for entry point: " << entryPointName;
    }
    [library release];
    return function;
}
