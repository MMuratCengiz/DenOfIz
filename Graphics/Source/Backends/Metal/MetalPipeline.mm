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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalEnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalPipeline.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalPipeline::MetalPipeline( MetalContext *context, const PipelineDesc &desc ) : 
    m_context( context ), 
    m_desc( desc ),
    m_computeThreadsPerThreadgroup( MTLSizeMake(32, 1, 1) ),
    m_meshThreadsPerThreadgroup( MTLSizeMake(128, 1, 1) ),
    m_objectThreadsPerThreadgroup( MTLSizeMake(0, 0, 0) ),
    m_rootSignature( static_cast<MetalRootSignature *>( desc.RootSignature ) )
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
    case BindPoint::Mesh:
        CreateMeshPipeline( );
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

    const auto &shaders = m_desc.ShaderProgram->CompiledShaders( );
    for ( uint32_t i = 0; i < shaders.NumElements; ++i )
    {
        const auto    &shader     = shaders.Elements[ i ];
        std::string    entryPoint = shader->EntryPoint.Get( );
        id<MTLLibrary> library    = LoadLibrary( shader->MSL );
        switch ( shader->Stage )
        {
        case ShaderStage::Vertex:
            vertexFunction = CreateShaderFunction( library, shader->EntryPoint.Get( ) );
            break;
        case ShaderStage::Pixel:
            fragmentFunction = CreateShaderFunction( library, shader->EntryPoint.Get( ) );
            break;
        default:
            spdlog::error( "Unsupported shader stage: {}", static_cast<int>( shader->Stage ) );
            break;
        }
    }

    if ( vertexFunction == nullptr || fragmentFunction == nullptr )
    {
        spdlog::error("Failed to create vertex or fragment function");
        return;
    }
    // Configure a pipeline descriptor that is used to create a pipeline state.
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label                        = @"Graphics Pipeline#";
    pipelineStateDescriptor.vertexFunction               = vertexFunction;
    pipelineStateDescriptor.fragmentFunction             = fragmentFunction;
    pipelineStateDescriptor.inputPrimitiveTopology       = MetalEnumConverter::ConvertTopologyClass( m_desc.Graphics.PrimitiveTopology );
    pipelineStateDescriptor.sampleCount                  = MSAASampleCountToNumSamples( m_desc.Graphics.MSAASampleCount );

    auto *mtkInputLayout = dynamic_cast<MetalInputLayout *>( m_desc.InputLayout );
    if ( mtkInputLayout != nullptr ) {
        pipelineStateDescriptor.vertexDescriptor = mtkInputLayout->GetVertexDescriptor( );
    }

    int attachmentIdx = 0;
    for ( uint32_t i = 0; i < m_desc.Graphics.RenderTargets.NumElements; ++i )
    {
        const auto                                 &attachment           = m_desc.Graphics.RenderTargets.Elements[ i ];
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
    if ( m_desc.Graphics.DepthStencilAttachmentFormat == Format::D24UnormS8Uint) {
        pipelineStateDescriptor.stencilAttachmentPixelFormat = MetalEnumConverter::ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
    }
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

    switch ( m_desc.Graphics.FillMode )
    {
    case FillMode::Solid:
        m_fillMode = MTLTriangleFillModeFill;
        break;
    case FillMode::Wireframe:
        m_fillMode = MTLTriangleFillModeLines;
        break;
    }
}

void MetalPipeline::CreateComputePipeline( )
{
    id<MTLFunction> computeFunction = nullptr;
    ThreadGroupInfo threadGroup = { 0, 0, 0 };

    const auto &shaders = m_desc.ShaderProgram->CompiledShaders( );
    for ( uint32_t i = 0; i < shaders.NumElements; ++i )
    {
        const auto &shader  = shaders.Elements[ i ];
        auto        library = LoadLibrary( shader->MSL );
        switch ( shader->Stage )
        {
        case ShaderStage::Compute:
            computeFunction = CreateShaderFunction( library, shader->EntryPoint.Get( ) );
            threadGroup = shader->ThreadGroup;
            break;
        default:
            spdlog::error("Unsupported shader stage: {}", static_cast<int>( shader->Stage ));
            break;
        }
    }

    if ( computeFunction == nullptr )
    {
        spdlog::error("Failed to create compute function");
        return;
    }

    // Configure a pipeline descriptor that is used to create a pipeline state.
    MTLComputePipelineDescriptor *pipelineStateDescriptor = [[MTLComputePipelineDescriptor alloc] init];
    pipelineStateDescriptor.label                         = @"Compute Pipeline#";
    pipelineStateDescriptor.computeFunction               = computeFunction;
    
    if ( threadGroup.X > 0 || threadGroup.Y > 0 || threadGroup.Z > 0 )
    {
        m_computeThreadsPerThreadgroup = MTLSizeMake(
            threadGroup.X > 0 ? threadGroup.X : 1,
            threadGroup.Y > 0 ? threadGroup.Y : 1,
            threadGroup.Z > 0 ? threadGroup.Z : 1
        );
        pipelineStateDescriptor.threadGroupSizeIsMultipleOfThreadExecutionWidth = YES;
        if ( [pipelineStateDescriptor respondsToSelector:@selector(setMaxTotalThreadsPerThreadgroup:)] )
        {
            uint32_t totalThreads = threadGroup.X * threadGroup.Y * threadGroup.Z;
            if ( totalThreads > 0 )
            {
                [pipelineStateDescriptor setMaxTotalThreadsPerThreadgroup:totalThreads];
            }
        }
    }

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
    auto compiledShaders = m_desc.ShaderProgram->CompiledShaders( );

    MTLComputePipelineDescriptor    *pipelineStateDescriptor = [[MTLComputePipelineDescriptor alloc] init];
    NSMutableArray<id<MTLFunction>> *functionHandles = [NSMutableArray arrayWithCapacity:compiledShaders.NumElements + 2]; // +2 for triangle and procedural intersection shaders

    // Operations done in the visible function table need to skip the first entry, which is reserved for the null function.
    constexpr uint32_t nullFunctionOffset = 1;
    m_visibleFunctions[ "Null" ]          = 0;

    uint32_t numVisibleFunctions = nullFunctionOffset + compiledShaders.NumElements;
    for ( uint32_t i = 0; i < compiledShaders.NumElements; ++i )
    {
        const auto     &shader      = compiledShaders.Elements[ i ];
        id<MTLLibrary>  library     = LoadLibrary( shader->MSL );
        id<MTLFunction> mtlFunction = CreateShaderFunction( library, shader->EntryPoint.Get( ) );

        [functionHandles addObject:mtlFunction];
        m_visibleFunctions[ shader->EntryPoint.Get( ) ] = i + nullFunctionOffset;
    }

    for ( uint32_t i = 0; i < m_desc.RayTracing.HitGroups.NumElements; ++i )
    {
        const auto     &hitGroup       = m_desc.RayTracing.HitGroups.Elements[ i ];
        HitGroupExport &hitGroupExport = m_hitGroupExports.emplace( hitGroup.Name.Get( ), HitGroupExport( ) ).first->second;

        if ( hitGroup.ClosestHitShaderIndex != -1 )
        {
            hitGroupExport.ClosestHit = hitGroup.ClosestHitShaderIndex + nullFunctionOffset;
        }
        if ( hitGroup.AnyHitShaderIndex != -1 )
        {
            hitGroupExport.AnyHit = hitGroup.AnyHitShaderIndex + nullFunctionOffset;
        }
        if ( hitGroup.IntersectionShaderIndex != -1 )
        {
            hitGroupExport.Intersection = hitGroup.IntersectionShaderIndex + nullFunctionOffset;
        }
    }

    id<MTLLibrary>  triangleIntersectionLibrary = NewSynthesizedIntersectionLibrary( IRHitGroupTypeTriangles );
    id<MTLFunction> triangleFn                  = [triangleIntersectionLibrary newFunctionWithName:[NSString stringWithUTF8String:kIRIndirectTriangleIntersectionFunctionName]];
    [functionHandles addObject:triangleFn];

    id<MTLLibrary>  proceduralIntersectionLibrary = NewSynthesizedIntersectionLibrary( IRHitGroupTypeProceduralPrimitive );
    id<MTLFunction> proceduralFn = [proceduralIntersectionLibrary newFunctionWithName:[NSString stringWithUTF8String:kIRIndirectProceduralIntersectionFunctionName]];
    [functionHandles addObject:proceduralFn];

    MTLLinkedFunctions *linkedFunctions = [[MTLLinkedFunctions alloc] init];
    [linkedFunctions setFunctions:functionHandles];
    [pipelineStateDescriptor setLinkedFunctions:linkedFunctions];

    id<MTLLibrary>  dispatchLibrary  = NewIndirectDispatchLibrary( );
    id<MTLFunction> dispatchFunction = [dispatchLibrary newFunctionWithName:[NSString stringWithUTF8String:kIRRayDispatchIndirectionKernelName]];

    [pipelineStateDescriptor setLabel:@"RayTracing Pipeline"];
    [pipelineStateDescriptor setComputeFunction:dispatchFunction];

    NSError *error         = nil;
    m_computePipelineState = [m_context->Device newComputePipelineStateWithDescriptor:pipelineStateDescriptor options:MTLPipelineOptionNone reflection:nil error:&error];
    if ( !m_computePipelineState )
    {
        DZ_LOG_NS_ERROR( "Failed to create pipeline state", error );
    }

    MTLVisibleFunctionTableDescriptor *vftDesc = [[MTLVisibleFunctionTableDescriptor alloc] init];
    vftDesc.functionCount                      = numVisibleFunctions;
    m_visibleFunctionTable                     = [m_computePipelineState newVisibleFunctionTableWithDescriptor:vftDesc];

    for ( int i = nullFunctionOffset; i < numVisibleFunctions; ++i )
    {
        id<MTLFunction> function = linkedFunctions.functions[ i - nullFunctionOffset ];
        if ( function != nullptr )
        {
            id<MTLFunctionHandle> handle = [m_computePipelineState functionHandleWithFunction:function];
            [m_visibleFunctionTable setFunction:handle atIndex:i];
        }
    }

    MTLIntersectionFunctionTableDescriptor *iftDesc = [[MTLIntersectionFunctionTableDescriptor alloc] init];
    iftDesc.functionCount                           = 2;
    m_intersectionFunctionTable                     = [m_computePipelineState newIntersectionFunctionTableWithDescriptor:iftDesc];

    id<MTLFunctionHandle> triangleFnHandle = [m_computePipelineState functionHandleWithFunction:triangleFn];
    [m_intersectionFunctionTable setFunction:triangleFnHandle atIndex:TriangleIntersectionShader];

    id<MTLFunctionHandle> proceduralFnHandle = [m_computePipelineState functionHandleWithFunction:proceduralFn];
    [m_intersectionFunctionTable setFunction:proceduralFnHandle atIndex:ProceduralIntersectionShader];
}

id<MTLLibrary> MetalPipeline::LoadLibrary( ByteArray &blob )
{
    NSError        *error      = nullptr;
    dispatch_data_t shaderData = dispatch_data_create( blob.Elements, blob.NumElements, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT );
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
    if (function == nil)
    {
        NSString *description = entryPointName;
        std::string entryPointStr = [description UTF8String];
        spdlog::error("Error creating function for entry point: {}", entryPointStr);
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

MetalRootSignature *MetalPipeline::RootSignature( ) const
{
    return m_rootSignature;
}

const MTLCullMode &MetalPipeline::CullMode( ) const
{
    return m_cullMode;
}

const MTLTriangleFillMode &MetalPipeline::FillMode( ) const
{
    return m_fillMode;
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

const BindPoint &MetalPipeline::BindPoint( ) const
{
    return m_desc.BindPoint;
}

const PipelineDesc &MetalPipeline::Desc( ) const
{
    return m_desc;
}

const MTLSize &MetalPipeline::ComputeThreadsPerThreadgroup( ) const
{
    return m_computeThreadsPerThreadgroup;
}

const MTLSize &MetalPipeline::MeshThreadsPerThreadgroup( ) const
{
    return m_meshThreadsPerThreadgroup;
}

const MTLSize &MetalPipeline::ObjectThreadsPerThreadgroup( ) const
{
    return m_objectThreadsPerThreadgroup;
}

const MeshShaderUsedStages &MetalPipeline::MeshShaderUsedStages( )
{
    return m_usedMeshShaderStages;
}

const uint64_t &MetalPipeline::FindVisibleShaderIndexByName( const std::string &name ) const
{
    auto shaderFunction = m_visibleFunctions.find( name );
    if ( shaderFunction == m_visibleFunctions.end( ) )
    {
        spdlog::error("Shader function not found: {}", name);
        return { };
    }
    return shaderFunction->second;
}

const HitGroupExport &MetalPipeline::FindHitGroupExport( const std::string &name ) const
{
    return m_hitGroupExports.at( name );
}

id<MTLVisibleFunctionTable> MetalPipeline::VisibleFunctionTable( ) const
{
    return m_visibleFunctionTable;
}

void MetalPipeline::CreateMeshPipeline( )
{
    id<MTLFunction> objectFunction = nullptr; // Task (Amplification) shader
    id<MTLFunction> meshFunction = nullptr;   // Mesh shader
    id<MTLFunction> fragmentFunction = nullptr;
    m_usedMeshShaderStages = {};
    
    ThreadGroupInfo objectThreadGroup = { 0, 0, 0 }; // Task shader thread group
    ThreadGroupInfo meshThreadGroup = { 0, 0, 0 };   // Mesh shader thread group
    
    const auto &shaders = m_desc.ShaderProgram->CompiledShaders( );
    for ( uint32_t i = 0; i < shaders.NumElements; ++i )
    {
        const auto &shader = shaders.Elements[ i ];
        id<MTLLibrary> library = LoadLibrary( shader->MSL );
        
        switch ( shader->Stage )
        {
            case ShaderStage::Task:
                objectFunction              = CreateShaderFunction( library, shader->EntryPoint.Get( ) );
                objectThreadGroup           = shader->ThreadGroup;
                m_usedMeshShaderStages.Task = true;
                break;
            case ShaderStage::Mesh:
                meshFunction                = CreateShaderFunction( library, shader->EntryPoint.Get( ) );
                meshThreadGroup             = shader->ThreadGroup;
                m_usedMeshShaderStages.Mesh = true;
                break;
            case ShaderStage::Pixel:
                fragmentFunction             = CreateShaderFunction( library, shader->EntryPoint.Get( ) );
                m_usedMeshShaderStages.Pixel = true;
                break;
            default:
                spdlog::error("Unsupported shader stage for mesh pipeline: {}", static_cast<int>( shader->Stage ));
                break;
        }
    }
    
    if ( meshFunction == nullptr )
    {
        spdlog::error("Failed to create mesh function");
        return;
    }

    MTLMeshRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLMeshRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"Mesh Pipeline";
    pipelineStateDescriptor.objectFunction = objectFunction;
    pipelineStateDescriptor.meshFunction = meshFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.rasterSampleCount = MSAASampleCountToNumSamples( m_desc.Graphics.MSAASampleCount );

    // Compute thread group count from reflection
    if (objectFunction && (objectThreadGroup.X > 0 || objectThreadGroup.Y > 0 || objectThreadGroup.Z > 0))
    {
        m_objectThreadsPerThreadgroup = MTLSizeMake(
            objectThreadGroup.X > 0 ? objectThreadGroup.X : 1,
            objectThreadGroup.Y > 0 ? objectThreadGroup.Y : 1,
            objectThreadGroup.Z > 0 ? objectThreadGroup.Z : 1
        );
        if ([pipelineStateDescriptor respondsToSelector:@selector(setMaxTotalThreadsPerObjectThreadgroup:)])
        {
            uint32_t totalThreads = objectThreadGroup.X * objectThreadGroup.Y * objectThreadGroup.Z;
            if (totalThreads > 0)
            {
                [pipelineStateDescriptor setMaxTotalThreadsPerObjectThreadgroup:totalThreads];
            }
        }
        pipelineStateDescriptor.objectThreadgroupSizeIsMultipleOfThreadExecutionWidth = YES;
    }
    
    if ( meshFunction && ( meshThreadGroup.X > 0 || meshThreadGroup.Y > 0 || meshThreadGroup.Z > 0 ) )
    {
        m_meshThreadsPerThreadgroup = MTLSizeMake(
            meshThreadGroup.X > 0 ? meshThreadGroup.X : 1,
            meshThreadGroup.Y > 0 ? meshThreadGroup.Y : 1,
            meshThreadGroup.Z > 0 ? meshThreadGroup.Z : 1
        );
        if ( [pipelineStateDescriptor respondsToSelector:@selector(setMaxTotalThreadsPerMeshThreadgroup:)] )
        {
            uint32_t totalThreads = meshThreadGroup.X * meshThreadGroup.Y * meshThreadGroup.Z;
            if ( totalThreads > 0 )
            {
                [pipelineStateDescriptor setMaxTotalThreadsPerMeshThreadgroup:totalThreads];
            }
        }
        pipelineStateDescriptor.meshThreadgroupSizeIsMultipleOfThreadExecutionWidth = YES;
    }

    int attachmentIdx = 0;
    for ( uint32_t i = 0; i < m_desc.Graphics.RenderTargets.NumElements; ++i )
    {
        const auto &attachment = m_desc.Graphics.RenderTargets.Elements[ i ];
        MTLRenderPipelineColorAttachmentDescriptor *metalColorAttachment = pipelineStateDescriptor.colorAttachments[ attachmentIdx ];
        metalColorAttachment.pixelFormat = MetalEnumConverter::ConvertFormat( attachment.Format );
        
        const BlendDesc &blendDesc = attachment.Blend;
        
        metalColorAttachment.blendingEnabled = blendDesc.Enable;
        
        metalColorAttachment.sourceRGBBlendFactor = MetalEnumConverter::ConvertBlendFactor( blendDesc.SrcBlend );
        metalColorAttachment.destinationRGBBlendFactor = MetalEnumConverter::ConvertBlendFactor( blendDesc.DstBlend );
        metalColorAttachment.rgbBlendOperation = MetalEnumConverter::ConvertBlendOp( blendDesc.BlendOp );
        
        metalColorAttachment.sourceAlphaBlendFactor = MetalEnumConverter::ConvertBlendFactor( blendDesc.SrcBlendAlpha );
        metalColorAttachment.destinationAlphaBlendFactor = MetalEnumConverter::ConvertBlendFactor( blendDesc.DstBlendAlpha );
        metalColorAttachment.alphaBlendOperation = MetalEnumConverter::ConvertBlendOp( blendDesc.BlendOpAlpha );
        
        metalColorAttachment.writeMask = blendDesc.RenderTargetWriteMask;
        
        attachmentIdx++;
    }
    
    pipelineStateDescriptor.depthAttachmentPixelFormat = MetalEnumConverter::ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
    if ( m_desc.Graphics.DepthStencilAttachmentFormat == Format::D24UnormS8Uint) {
        pipelineStateDescriptor.stencilAttachmentPixelFormat = MetalEnumConverter::ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
    }
    pipelineStateDescriptor.alphaToCoverageEnabled = m_desc.Graphics.AlphaToCoverageEnable;
    NSError *error = nullptr;
    m_graphicsPipelineState = [m_context->Device newRenderPipelineStateWithMeshDescriptor:pipelineStateDescriptor 
                                                                                 options:MTLPipelineOptionNone 
                                                                              reflection:nil 
                                                                                   error:&error];
    

    MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
    depthStencilDescriptor.depthCompareFunction = MetalEnumConverter::ConvertCompareOp( m_desc.Graphics.DepthTest.CompareOp );
    depthStencilDescriptor.depthWriteEnabled = m_desc.Graphics.DepthTest.Write;
    depthStencilDescriptor.frontFaceStencil = [[MTLStencilDescriptor alloc] init];
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
    
    switch ( m_desc.Graphics.FillMode )
    {
        case FillMode::Solid:
            m_fillMode = MTLTriangleFillModeFill;
            break;
        case FillMode::Wireframe:
            m_fillMode = MTLTriangleFillModeLines;
            break;
    }
}

id<MTLIntersectionFunctionTable> MetalPipeline::IntersectionFunctionTable( ) const
{
    return m_intersectionFunctionTable;
}
