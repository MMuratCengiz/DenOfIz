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

    const auto &shaders = m_desc.ShaderProgram->GetCompiledShaders( );
    for ( int i = 0; i < shaders.NumElements( ); ++i )
    {
        const auto    &shader     = shaders.GetElement( i );
        std::string    entryPoint = shader->EntryPoint.Get( );
        id<MTLLibrary> library    = LoadLibrary( shader->Blob, shader->Path );
        switch ( shader->Stage )
        {
        case ShaderStage::Vertex:
            vertexFunction = CreateShaderFunction( library, shader->EntryPoint.Get( ) );
            break;
        case ShaderStage::Pixel:
            fragmentFunction = CreateShaderFunction( library, shader->EntryPoint.Get( ) );
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
    pipelineStateDescriptor.inputPrimitiveTopology       = MetalEnumConverter::ConvertTopologyClass( m_desc.Graphics.PrimitiveTopology );
    pipelineStateDescriptor.sampleCount                  = MSAASampleCountToNumSamples( m_desc.Graphics.MSAASampleCount );

    auto *mtkInputLayout                     = dynamic_cast<MetalInputLayout *>( m_desc.InputLayout );
    pipelineStateDescriptor.vertexDescriptor = mtkInputLayout->GetVertexDescriptor( );

    int attachmentIdx = 0;
    for ( int i = 0; i < m_desc.Graphics.RenderTargets.NumElements( ); ++i )
    {
        const auto                                 &attachment           = m_desc.Graphics.RenderTargets.GetElement( i );
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

    pipelineStateDescriptor.depthAttachmentPixelFormat   = MetalEnumConverter::ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
    pipelineStateDescriptor.stencilAttachmentPixelFormat = MetalEnumConverter::ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
    pipelineStateDescriptor.alphaToCoverageEnabled       = m_desc.Graphics.AlphaToCoverageEnable;
    pipelineStateDescriptor.alphaToOneEnabled            = m_desc.Graphics.IndependentBlendEnable;

    NSError *error          = nullptr;
    m_graphicsPipelineState = [m_context->Device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];

    if ( !m_graphicsPipelineState )
    {
        DZ_LOG_NS_ERROR( "Failed to create pipeline state", error );
    }

    MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
    depthStencilDescriptor.depthCompareFunction       = MetalEnumConverter::ConvertCompareOp( m_desc.Graphics.DepthTest.CompareOp );
    depthStencilDescriptor.depthWriteEnabled          = m_desc.Graphics.DepthTest.Write;
    depthStencilDescriptor.frontFaceStencil           = [[MTLStencilDescriptor alloc] init];
    InitStencilFace( depthStencilDescriptor.frontFaceStencil, m_desc.Graphics.StencilTest.FrontFace );
    depthStencilDescriptor.backFaceStencil = [[MTLStencilDescriptor alloc] init];
    InitStencilFace( depthStencilDescriptor.backFaceStencil, m_desc.Graphics.StencilTest.BackFace );
    m_depthStencilState = [m_context->Device newDepthStencilStateWithDescriptor:depthStencilDescriptor];

    switch ( m_desc.Graphics.CullMode )
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

    const auto &shaders = m_desc.ShaderProgram->GetCompiledShaders( );
    for ( int i = 0; i < shaders.NumElements( ); ++i )
    {
        const auto &shader  = shaders.GetElement( i );
        auto        library = LoadLibrary( shader->Blob, shader->Path );
        switch ( shader->Stage )
        {
        case ShaderStage::Compute:
            computeFunction = CreateShaderFunction( library, shader->EntryPoint.Get( ) );
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
    MetalRootSignature *rootSignature   = static_cast<MetalRootSignature *>( m_desc.RootSignature );
    auto                compiledShaders = m_desc.ShaderProgram->GetCompiledShaders( );

    MTLComputePipelineDescriptor    *pipelineStateDescriptor = [[MTLComputePipelineDescriptor alloc] init];
    NSMutableArray<id<MTLFunction>> *functionHandles         = [NSMutableArray array];
    uint32_t                         numVisibleFunctions     = 1; // Null function
    m_visibleFunctions[ "Null" ]                             = ShaderFunction{ .Index = 0, .Function = nullptr };
    uint64_t shaderIndex                                     = 1; // Null function is at index 0
    for ( uint32_t i = 0; i < compiledShaders.NumElements( ); ++i )
    {
        const auto     &shader      = compiledShaders.GetElement( i );
        id<MTLLibrary>  library     = LoadLibrary( shader->Blob, shader->Path );
        id<MTLFunction> mtlFunction = CreateShaderFunction( library, shader->EntryPoint.Get( ) );

        switch ( shader->Stage )
        {
        case ShaderStage::ClosestHit:
            m_intersectionExport.ClosestHit = ShaderFunction{ .Index = shaderIndex, .Function = mtlFunction };
        case ShaderStage::Raygen:
        case ShaderStage::Miss:
            ++numVisibleFunctions;
            m_visibleFunctions[ shader->EntryPoint.Get( ) ] = ShaderFunction{ .Index = shaderIndex, .Function = mtlFunction };
            [functionHandles addObject:mtlFunction];
            break;
        case ShaderStage::AnyHit:
        case ShaderStage::Intersection:
            // TODO
            break;
        default:
            LOG( ERROR ) << "Unsupported shader stage: " << static_cast<int>( shader->Stage );
            break;
        }
        ++shaderIndex;
    }

    id<MTLLibrary>  triangleIntersectionSynthesizedLibrary = NewSynthesizedIntersectionLibrary( IRHitGroupTypeTriangles );
    ShaderFunction &triangleFnExport                       = m_intersectionExport.TriangleIntersection;
    triangleFnExport.Function = [triangleIntersectionSynthesizedLibrary newFunctionWithName:[NSString stringWithUTF8String:kIRIndirectTriangleIntersectionFunctionName]];
    triangleFnExport.Index    = 0;
    id<MTLLibrary>  proceduralIntersectionSynthesizedLibrary = NewSynthesizedIntersectionLibrary( IRHitGroupTypeProceduralPrimitive );
    ShaderFunction &proceduralFnExport                       = m_intersectionExport.ProceduralIntersection;
    proceduralFnExport.Function = [proceduralIntersectionSynthesizedLibrary newFunctionWithName:[NSString stringWithUTF8String:kIRIndirectProceduralIntersectionFunctionName]];
    proceduralFnExport.Index    = 1;
    [functionHandles addObject:triangleFnExport.Function];
    [functionHandles addObject:proceduralFnExport.Function];

    MTLLinkedFunctions *linkedFunctions = [[MTLLinkedFunctions alloc] init];
    [linkedFunctions setFunctions:functionHandles];
    [pipelineStateDescriptor setLinkedFunctions:linkedFunctions];

    id<MTLLibrary>  dispatchLibrary  = NewIndirectDispatchLibrary( );
    id<MTLFunction> dispatchFunction = [dispatchLibrary newFunctionWithName:[NSString stringWithUTF8String:kIRRayDispatchIndirectionKernelName]];

    [pipelineStateDescriptor setLabel:@"RayTracing Pipeline"];
    [pipelineStateDescriptor setComputeFunction:dispatchFunction];

    MTLNewComputePipelineStateCompletionHandler completionHandler = ^( id<MTLComputePipelineState> pipelineState, NSError *err ) {
      if ( err != nil )
      {
          DZ_LOG_NS_ERROR( "Error creating pipeline state", err );
      }
    };

    NSError *error         = nil;
    m_computePipelineState = [m_context->Device newComputePipelineStateWithDescriptor:pipelineStateDescriptor options:MTLPipelineOptionNone reflection:nil error:&error];
    if ( !m_computePipelineState )
    {
        DZ_LOG_NS_ERROR( "Failed to create pipeline state", error );
    }

    MTLVisibleFunctionTableDescriptor *vftDesc = [[MTLVisibleFunctionTableDescriptor alloc] init];
    vftDesc.functionCount                      = numVisibleFunctions;
    m_visibleFunctionTable                     = [m_computePipelineState newVisibleFunctionTableWithDescriptor:vftDesc];

    for ( auto &functions : m_visibleFunctions )
    {
        ShaderFunction &shaderFunction = functions.second;
        if ( shaderFunction.Function != nullptr )
        {
            shaderFunction.Handle = [m_computePipelineState functionHandleWithFunction:shaderFunction.Function];
            if ( shaderFunction.Index == m_intersectionExport.ClosestHit.Index )
            {
                m_intersectionExport.ClosestHit.Handle = shaderFunction.Handle;
            }
            [m_visibleFunctionTable setFunction:shaderFunction.Handle atIndex:shaderFunction.Index];
        }
    }

    MTLIntersectionFunctionTableDescriptor *iftDesc = [[MTLIntersectionFunctionTableDescriptor alloc] init];
    iftDesc.functionCount                           = 2;
    m_intersectionFunctionTable                     = [m_computePipelineState newIntersectionFunctionTableWithDescriptor:iftDesc];

    triangleFnExport.Handle = [m_computePipelineState functionHandleWithFunction:triangleFnExport.Function];
    [m_intersectionFunctionTable setFunction:triangleFnExport.Handle atIndex:triangleFnExport.Index];

    proceduralFnExport.Handle = [m_computePipelineState functionHandleWithFunction:proceduralFnExport.Function];
    [m_intersectionFunctionTable setFunction:proceduralFnExport.Handle atIndex:proceduralFnExport.Index];
}

id<MTLLibrary> MetalPipeline::LoadLibrary( IDxcBlob *&blob, const std::string &shaderPath )
{
    NSError        *error      = nullptr;
    dispatch_data_t shaderData = dispatch_data_create( blob->GetBufferPointer( ), blob->GetBufferSize( ), NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT );
    id<MTLLibrary>  library    = [m_context->Device newLibraryWithData:shaderData error:&error];
    if ( error != nil )
    {
        DZ_LOG_NS_ERROR( "Error creating library", error );
        return nil;
    }
    return library;
}

id<MTLFunction> MetalPipeline::CreateShaderFunction( id<MTLLibrary> library, const std::string &entryPoint )
{
    NSString       *entryPointName = [NSString stringWithUTF8String:entryPoint.c_str( )];
    id<MTLFunction> function       = [library newFunctionWithName:entryPointName];
    if ( function == nil )
    {
        LOG( ERROR ) << "Error creating function for entry point: " << entryPointName;
    }
    return function;
}

id<MTLLibrary> MetalPipeline::NewIndirectDispatchLibrary( )
{
    IRCompiler *pCompiler = IRCompilerCreate( );
    IRCompilerSetMinimumDeploymentTarget( pCompiler, IROperatingSystem_macOS, "15.1" );
    IRMetalLibBinary *metalLib = IRMetalLibBinaryCreate( );
    IRMetalLibSynthesizeIndirectRayDispatchFunction( pCompiler, metalLib );

    NSError       *pError = nullptr;
    id<MTLLibrary> lib    = [m_context->Device newLibraryWithData:IRMetalLibGetBytecodeData( metalLib ) error:&pError];
    assert( lib );

    IRMetalLibBinaryDestroy( metalLib );
    IRCompilerDestroy( pCompiler );

    return lib;
}

id<MTLLibrary> MetalPipeline::NewSynthesizedIntersectionLibrary( const IRHitGroupType &hitGroupType )
{
    IRCompiler *pCompiler = IRCompilerCreate( );
    IRCompilerSetMinimumDeploymentTarget( pCompiler, IROperatingSystem_macOS, "15.1" );
    IRCompilerSetHitgroupType( pCompiler, hitGroupType );

    IRMetalLibBinary *metalLib = IRMetalLibBinaryCreate( );
    IRMetalLibSynthesizeIndirectIntersectionFunction( pCompiler, metalLib );

    NSError       *pError = nullptr;
    id<MTLLibrary> lib    = [m_context->Device newLibraryWithData:IRMetalLibGetBytecodeData( metalLib ) error:&pError];
    assert( lib );

    // Clean up compiler resources
    IRMetalLibBinaryDestroy( metalLib );
    IRCompilerDestroy( pCompiler );

    return lib;
}

void MetalPipeline::InitStencilFace( MTLStencilDescriptor *stencilDesc, const StencilFace &stencilFace )
{
    stencilDesc.stencilCompareFunction    = MetalEnumConverter::ConvertCompareOp( stencilFace.CompareOp );
    stencilDesc.stencilFailureOperation   = MetalEnumConverter::ConvertStencilOp( stencilFace.FailOp );
    stencilDesc.depthFailureOperation     = MetalEnumConverter::ConvertStencilOp( stencilFace.DepthFailOp );
    stencilDesc.depthStencilPassOperation = MetalEnumConverter::ConvertStencilOp( stencilFace.PassOp );
    stencilDesc.readMask                  = m_desc.Graphics.StencilTest.ReadMask;
    stencilDesc.writeMask                 = m_desc.Graphics.StencilTest.WriteMask;
}

const MTLCullMode &MetalPipeline::CullMode( ) const
{
    return m_cullMode;
}

const id<MTLDepthStencilState> &MetalPipeline::DepthStencilState( ) const
{
    return m_depthStencilState;
}

const id<MTLRenderPipelineState> &MetalPipeline::GraphicsPipelineState( ) const
{
    return m_graphicsPipelineState;
}

const id<MTLComputePipelineState> &MetalPipeline::ComputePipelineState( ) const
{
    return m_computePipelineState;
}

[[nodiscard]] const IntersectionExport &MetalPipeline::IntersectionExport( ) const
{
    return m_intersectionExport;
}

const ShaderFunction &MetalPipeline::FindVisibleShaderFunctionByName( const std::string &name ) const
{
    auto shaderFunction = m_visibleFunctions.find( name );
    if ( shaderFunction == m_visibleFunctions.end( ) )
    {
        LOG( ERROR ) << "Shader function not found: " << name;
        return { };
    }
    return shaderFunction->second;
}

id<MTLVisibleFunctionTable> MetalPipeline::VisibleFunctionTable( ) const
{
    return m_visibleFunctionTable;
}

id<MTLIntersectionFunctionTable> MetalPipeline::IntersectionFunctionTable( ) const
{
    return m_intersectionFunctionTable;
}
