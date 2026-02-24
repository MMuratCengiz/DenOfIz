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
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalPipelineCache.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IInputLayout.h"
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IPipelineCache.h"

using namespace DenOfIz;

#define ROOT_SIGNATURE_PTR( handle )  DENOFIZ_FROM_HANDLE( DenOfIz::IRootSignature, handle )
#define INPUT_LAYOUT_PTR( handle )    DENOFIZ_FROM_HANDLE( DenOfIz::IInputLayout, handle )
#define PIPELINE_CACHE_PTR( handle )  DENOFIZ_FROM_HANDLE( DenOfIz::IPipelineCache, handle )

MetalPipeline::MetalPipeline( MetalContext *context, const DenOfIz_PipelineDesc &desc ) :
    m_context( context ),
    m_desc( desc ),
    m_computeThreadsPerThreadgroup( MTLSizeMake(32, 1, 1) ),
    m_meshThreadsPerThreadgroup( MTLSizeMake(128, 1, 1) ),
    m_objectThreadsPerThreadgroup( MTLSizeMake(0, 0, 0) ),
    m_rootSignature( DENOFIZ_HANDLE_IS_VALID( desc.RootSignature ) ? static_cast<MetalRootSignature *>( ROOT_SIGNATURE_PTR( desc.RootSignature ) ) : nullptr )
{
    switch ( desc.BindPoint )
    {
    case DENOFIZ_BIND_POINT_GRAPHICS:
        CreateGraphicsPipeline( );
        break;
    case DENOFIZ_BIND_POINT_COMPUTE:
        CreateComputePipeline( );
        break;
    case DENOFIZ_BIND_POINT_RAYTRACING:
        CreateRayTracingPipeline( );
        break;
    case DENOFIZ_BIND_POINT_MESH:
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

    if ( !DENOFIZ_HANDLE_IS_VALID( m_desc.ShaderProgram ) )
    {
        spdlog::error( "ShaderProgram handle is invalid" );
        return;
    }
    DenOfIz_CompiledShaderStageArray shaders;
    DenOfIz_ShaderProgram_CompiledShaders( m_desc.ShaderProgram, &shaders );
    for ( uint32_t i = 0; i < shaders.NumElements; ++i )
    {
        const auto    &shader     = shaders.Elements[ i ];
        std::string entryPoint( shader->EntryPoint.Chars, shader->EntryPoint.NumChars );

        id<MTLLibrary> library    = LoadLibrary( shader->MSL );
        if ( shader->Stage == DENOFIZ_SHADER_STAGE_VERTEX_BIT )
        {
            vertexFunction = CreateShaderFunction( library, entryPoint.c_str( ) );
        }
        else if ( shader->Stage == DENOFIZ_SHADER_STAGE_PIXEL_BIT )
        {
            fragmentFunction = CreateShaderFunction( library, entryPoint.c_str( ) );
        }
        else
        {
            spdlog::error( "Unsupported shader stage: {}", static_cast<int>( shader->Stage ) );
        }
    }

    if ( vertexFunction == nullptr )
    {
        spdlog::error("Failed to create vertex function");
        return;
    }

    bool isDepthOnlyPipeline = m_desc.Graphics.RenderTargets.NumElements == 0 &&
                               m_desc.Graphics.DepthStencilAttachmentFormat != DENOFIZ_FORMAT_UNDEFINED;

    if ( fragmentFunction == nullptr && !isDepthOnlyPipeline )
    {
        spdlog::error("Failed to create fragment function (required when render targets are specified)");
        return;
    }
    // Configure a pipeline descriptor that is used to create a pipeline state.
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label                        = @"Graphics Pipeline#";
    pipelineStateDescriptor.vertexFunction               = vertexFunction;
    pipelineStateDescriptor.fragmentFunction             = isDepthOnlyPipeline ? nil : fragmentFunction;
    pipelineStateDescriptor.inputPrimitiveTopology       = DenOfIz_MetalEnumConverter_ConvertTopologyClass( m_desc.Graphics.PrimitiveTopology );
    pipelineStateDescriptor.sampleCount                  = DenOfIz_MSAASampleCount_ToNumSamples( m_desc.Graphics.MSAASampleCount );
    if ( DENOFIZ_HANDLE_IS_VALID( m_desc.InputLayout ) )
    {
        auto *mtkInputLayout = dynamic_cast<MetalInputLayout *>( INPUT_LAYOUT_PTR( m_desc.InputLayout ) );
        if ( mtkInputLayout != nullptr )
        {
            pipelineStateDescriptor.vertexDescriptor = mtkInputLayout->GetVertexDescriptor( );
        }
    }

    int attachmentIdx = 0;
    for ( uint32_t i = 0; i < m_desc.Graphics.RenderTargets.NumElements; ++i )
    {
        const auto                                 &attachment           = m_desc.Graphics.RenderTargets.Elements[ i ];
        MTLRenderPipelineColorAttachmentDescriptor *metalColorAttachment = pipelineStateDescriptor.colorAttachments[ attachmentIdx ];
        metalColorAttachment.pixelFormat                                 = DenOfIz_MetalEnumConverter_ConvertFormat( attachment.Format );

        const DenOfIz_BlendDesc &blendDesc = attachment.Blend;

        metalColorAttachment.blendingEnabled = blendDesc.Enable;

        metalColorAttachment.sourceRGBBlendFactor      = DenOfIz_MetalEnumConverter_ConvertBlendFactor( blendDesc.SrcBlend );
        metalColorAttachment.destinationRGBBlendFactor = DenOfIz_MetalEnumConverter_ConvertBlendFactor( blendDesc.DstBlend );
        metalColorAttachment.rgbBlendOperation         = DenOfIz_MetalEnumConverter_ConvertBlendOp( blendDesc.BlendOp );

        metalColorAttachment.sourceAlphaBlendFactor      = DenOfIz_MetalEnumConverter_ConvertBlendFactor( blendDesc.SrcBlendAlpha );
        metalColorAttachment.destinationAlphaBlendFactor = DenOfIz_MetalEnumConverter_ConvertBlendFactor( blendDesc.DstBlendAlpha );
        metalColorAttachment.alphaBlendOperation         = DenOfIz_MetalEnumConverter_ConvertBlendOp( blendDesc.BlendOpAlpha );

        metalColorAttachment.writeMask = blendDesc.RenderTargetWriteMask;
    }

    if ( m_desc.Graphics.DepthStencilAttachmentFormat != DENOFIZ_FORMAT_UNDEFINED )
    {
        pipelineStateDescriptor.depthAttachmentPixelFormat = DenOfIz_MetalEnumConverter_ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
        if ( m_desc.Graphics.DepthStencilAttachmentFormat == DENOFIZ_FORMAT_D24_UNORM_S8_UINT )
        {
            pipelineStateDescriptor.stencilAttachmentPixelFormat = DenOfIz_MetalEnumConverter_ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
        }
    }
    pipelineStateDescriptor.alphaToCoverageEnabled       = m_desc.Graphics.AlphaToCoverageEnable;
    pipelineStateDescriptor.alphaToOneEnabled            = m_desc.Graphics.IndependentBlendEnable;

    NSError *error = nullptr;

    if ( DENOFIZ_HANDLE_IS_VALID( m_desc.PipelineCache ) )
    {
        auto metalCache = static_cast<MetalPipelineCache *>( PIPELINE_CACHE_PTR( m_desc.PipelineCache ) );
        if ( id<MTLBinaryArchive> archive = metalCache->GetArchive( ) )
        {
            pipelineStateDescriptor.binaryArchives = @[archive];
        }
    }

    m_graphicsPipelineState = [m_context->Device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];

    if ( !m_graphicsPipelineState )
    {
        spdlog::error( "Failed to create pipeline state" );
    }

    const DenOfIz_Format dsvFormat = m_desc.Graphics.DepthStencilAttachmentFormat;
    bool shouldInitializeStencil = m_desc.Graphics.StencilTest.Enable;

    if ( dsvFormat != DENOFIZ_FORMAT_UNDEFINED && shouldInitializeStencil )
    {
        const bool isDepthOnly = DenOfIz_Format_IsDepthOnly( dsvFormat );
        if ( isDepthOnly )
        {
            spdlog::warn( "MetalPipeline: Stencil operations enabled with depth-only format ({}). "
                          "Stencil operations may be ignored.",
                          static_cast<int>( dsvFormat ) );
        }
    }

    MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
    if ( m_desc.Graphics.DepthTest.Enable )
    {
        depthStencilDescriptor.depthCompareFunction = DenOfIz_MetalEnumConverter_ConvertCompareOp( m_desc.Graphics.DepthTest.CompareOp );
        depthStencilDescriptor.depthWriteEnabled    = m_desc.Graphics.DepthTest.Write;
    }
    else
    {
        depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
        depthStencilDescriptor.depthWriteEnabled    = NO;
    }

    if ( shouldInitializeStencil )
    {
        depthStencilDescriptor.frontFaceStencil = [[MTLStencilDescriptor alloc] init];
        InitStencilFace( depthStencilDescriptor.frontFaceStencil, m_desc.Graphics.StencilTest.FrontFace );
        depthStencilDescriptor.backFaceStencil = [[MTLStencilDescriptor alloc] init];
        InitStencilFace( depthStencilDescriptor.backFaceStencil, m_desc.Graphics.StencilTest.BackFace );
    }

    m_depthStencilState = [m_context->Device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
    m_primitiveType     = DenOfIz_MetalEnumConverter_ConvertTopologyToPrimitiveType( m_desc.Graphics.PrimitiveTopology );
    switch ( m_desc.Graphics.CullMode )
    {
    case DENOFIZ_CULL_MODE_NONE:
        m_cullMode = MTLCullModeNone;
        break;
    case DENOFIZ_CULL_MODE_FRONT_FACE:
        m_cullMode = MTLCullModeFront;
        break;
    case DENOFIZ_CULL_MODE_BACK_FACE:
        m_cullMode = MTLCullModeBack;
        break;
    }

    switch ( m_desc.Graphics.FillMode )
    {
    case DENOFIZ_FILL_MODE_SOLID:
        m_fillMode = MTLTriangleFillModeFill;
        break;
    case DENOFIZ_FILL_MODE_WIREFRAME:
        m_fillMode = MTLTriangleFillModeLines;
        break;
    }

    m_frontFaceWinding   = m_desc.Graphics.Rasterization.FrontCounterClockwise ? MTLWindingCounterClockwise : MTLWindingClockwise;
    m_depthBias          = static_cast<float>( m_desc.Graphics.Rasterization.DepthBias );
    m_depthBiasSlopeScale = m_desc.Graphics.Rasterization.SlopeScaledDepthBias;
    m_depthBiasClamp     = m_desc.Graphics.Rasterization.DepthBiasClamp;
}

void MetalPipeline::CreateComputePipeline( )
{
    id<MTLFunction> computeFunction = nullptr;
    DenOfIz_ThreadGroupDesc threadGroup = { 0, 0, 0 };

    if ( !DENOFIZ_HANDLE_IS_VALID( m_desc.ShaderProgram ) )
    {
        spdlog::error( "ShaderProgram handle is invalid" );
        return;
    }
    DenOfIz_CompiledShaderStageArray shaders;
    DenOfIz_ShaderProgram_CompiledShaders( m_desc.ShaderProgram, &shaders );
    for ( uint32_t i = 0; i < shaders.NumElements; ++i )
    {
        const auto &shader  = shaders.Elements[ i ];
        auto        library = LoadLibrary( shader->MSL );
        if ( shader->Stage == DENOFIZ_SHADER_STAGE_COMPUTE_BIT )
        {
            const std::string entryPoint( shader->EntryPoint.Chars, shader->EntryPoint.NumChars );
            computeFunction = CreateShaderFunction( library, entryPoint.c_str( ) );
            threadGroup = shader->ThreadGroup;
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
          spdlog::error( "Error creating pipeline state" );
      }
    };

    NSError          *error   = nil;
    MTLPipelineOption options = MTLPipelineOptionArgumentInfo;

    if ( DENOFIZ_HANDLE_IS_VALID( m_desc.PipelineCache ) )
    {
        auto metalCache = static_cast<MetalPipelineCache *>( PIPELINE_CACHE_PTR( m_desc.PipelineCache ) );
        if ( id<MTLBinaryArchive> archive = metalCache->GetArchive( ) )
        {
            pipelineStateDescriptor.binaryArchives = @[archive];
        }
    }

    m_computePipelineState    = [m_context->Device newComputePipelineStateWithDescriptor:pipelineStateDescriptor options:MTLPipelineOptionNone reflection:nil error:&error];

    if ( !m_computePipelineState )
    {
        spdlog::error( "Failed to create pipeline state" );
    }
}

void MetalPipeline::CreateRayTracingPipeline( )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_desc.ShaderProgram ) )
    {
        spdlog::error( "ShaderProgram handle is invalid" );
        return;
    }
    DenOfIz_CompiledShaderStageArray compiledShaders;
    DenOfIz_ShaderProgram_CompiledShaders( m_desc.ShaderProgram, &compiledShaders );
    DenOfIz_ShaderProgramDesc shaderProgramDesc;
    DenOfIz_ShaderProgram_Desc( m_desc.ShaderProgram, &shaderProgramDesc );

    MTLComputePipelineDescriptor    *pipelineStateDescriptor = [[MTLComputePipelineDescriptor alloc] init];
    NSMutableArray<id<MTLFunction>> *functionHandles = [NSMutableArray arrayWithCapacity:compiledShaders.NumElements + 2]; // +2 for triangle and procedural intersection shaders

    // Operations done in the visible function table need to skip the first entry, which is reserved for the null function.
    constexpr uint32_t nullFunctionOffset = 1;
    m_visibleFunctions[ "Null" ]          = 0;

    uint32_t numVisibleFunctions = nullFunctionOffset + compiledShaders.NumElements;
    for ( uint32_t i = 0; i < compiledShaders.NumElements; ++i )
    {
        const auto     &shader      = compiledShaders.Elements[ i ];
        std::string entryPoint( shader->EntryPoint.Chars, shader->EntryPoint.NumChars );
        id<MTLLibrary>  library     = LoadLibrary( shader->MSL );
        id<MTLFunction> mtlFunction = CreateShaderFunction( library, entryPoint.c_str( ) );

        [functionHandles addObject:mtlFunction];
        m_visibleFunctions[ entryPoint.c_str( ) ] = i + nullFunctionOffset;
    }

    for ( uint32_t i = 0; i < m_desc.RayTracing.HitGroups.NumElements; ++i )
    {
        const auto     &hitGroup       = m_desc.RayTracing.HitGroups.Elements[ i ];
        std::string    hitGroupNameStr( hitGroup.Name.Chars, hitGroup.Name.NumChars );
        HitGroupExport &hitGroupExport = m_hitGroupExports.emplace( hitGroupNameStr, HitGroupExport( ) ).first->second;

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
    [pipelineStateDescriptor setMaxCallStackDepth:shaderProgramDesc.RayTracing.MaxRecursionDepth + 1];

    if ( DENOFIZ_HANDLE_IS_VALID( m_desc.PipelineCache ) )
    {
        auto metalCache = static_cast<MetalPipelineCache *>( PIPELINE_CACHE_PTR( m_desc.PipelineCache ) );
        if ( id<MTLBinaryArchive> archive = metalCache->GetArchive( ) )
        {
            pipelineStateDescriptor.binaryArchives = @[archive];
        }
    }

    NSError *error         = nil;
    m_computePipelineState = [m_context->Device newComputePipelineStateWithDescriptor:pipelineStateDescriptor options:MTLPipelineOptionNone reflection:nil error:&error];
    if ( !m_computePipelineState )
    {
        spdlog::error( "Failed to create pipeline state" );
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
    [m_intersectionFunctionTable setFunction:triangleFnHandle atIndex:DENOFIZ_METAL_TRIANGLE_INTERSECTION_SHADER];

    id<MTLFunctionHandle> proceduralFnHandle = [m_computePipelineState functionHandleWithFunction:proceduralFn];
    [m_intersectionFunctionTable setFunction:proceduralFnHandle atIndex:DENOFIZ_METAL_PROCEDURAL_INTERSECTION_SHADER];
}

id<MTLLibrary> MetalPipeline::LoadLibrary( DenOfIz_ByteArray &blob )
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

void MetalPipeline::InitStencilFace( MTLStencilDescriptor *stencilDesc, const DenOfIz_StencilFace &stencilFace )
{
    stencilDesc.stencilCompareFunction    = DenOfIz_MetalEnumConverter_ConvertCompareOp( stencilFace.CompareOp );
    stencilDesc.stencilFailureOperation   = DenOfIz_MetalEnumConverter_ConvertStencilOp( stencilFace.FailOp );
    stencilDesc.depthFailureOperation     = DenOfIz_MetalEnumConverter_ConvertStencilOp( stencilFace.DepthFailOp );
    stencilDesc.depthStencilPassOperation = DenOfIz_MetalEnumConverter_ConvertStencilOp( stencilFace.PassOp );
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

const MTLWinding &MetalPipeline::FrontFaceWinding( ) const
{
    return m_frontFaceWinding;
}

float MetalPipeline::DepthBias( ) const
{
    return m_depthBias;
}

float MetalPipeline::DepthBiasSlopeScale( ) const
{
    return m_depthBiasSlopeScale;
}

float MetalPipeline::DepthBiasClamp( ) const
{
    return m_depthBiasClamp;
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

const DenOfIz_BindPoint &MetalPipeline::DenOfIz_BindPoint( ) const
{
    return m_desc.BindPoint;
}

const MTLPrimitiveType &MetalPipeline::PrimitiveType( ) const
{
    return m_primitiveType;
}

const DenOfIz_PipelineDesc &MetalPipeline::Desc( ) const
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

    DenOfIz_ThreadGroupDesc objectThreadGroup = { 0, 0, 0 }; // Task shader thread group
    DenOfIz_ThreadGroupDesc meshThreadGroup = { 0, 0, 0 };   // Mesh shader thread group

    if ( !DENOFIZ_HANDLE_IS_VALID( m_desc.ShaderProgram ) )
    {
        spdlog::error( "ShaderProgram handle is invalid" );
        return;
    }
    DenOfIz_CompiledShaderStageArray shaders;
    DenOfIz_ShaderProgram_CompiledShaders( m_desc.ShaderProgram, &shaders );
    for ( uint32_t i = 0; i < shaders.NumElements; ++i )
    {
        const auto &shader = shaders.Elements[ i ];
        id<MTLLibrary> library = LoadLibrary( shader->MSL );

        std::string entryPoint( shader->EntryPoint.Chars, shader->EntryPoint.NumChars );

        if ( shader->Stage & DENOFIZ_SHADER_STAGE_TASK_BIT )
        {
            objectFunction              = CreateShaderFunction( library, entryPoint.c_str( ) );
            objectThreadGroup           = shader->ThreadGroup;
            m_usedMeshShaderStages.Task = true;
        }
        else if ( shader->Stage & DENOFIZ_SHADER_STAGE_MESH_BIT )
        {
            meshFunction                = CreateShaderFunction( library, entryPoint.c_str( ) );
            meshThreadGroup             = shader->ThreadGroup;
            m_usedMeshShaderStages.Mesh = true;
        }
        else if ( shader->Stage & DENOFIZ_SHADER_STAGE_PIXEL_BIT )
        {
            fragmentFunction             = CreateShaderFunction( library, entryPoint.c_str( ) );
            m_usedMeshShaderStages.Pixel = true;
        }
        else
        {
            spdlog::error("Unsupported shader stage for mesh pipeline: {}", static_cast<int>( shader->Stage ));
        }
    }

    if ( meshFunction == nullptr )
    {
        spdlog::error("Failed to create mesh function");
        return;
    }

    bool isDepthOnlyPipeline = m_desc.Graphics.RenderTargets.NumElements == 0 &&
                               m_desc.Graphics.DepthStencilAttachmentFormat != DENOFIZ_FORMAT_UNDEFINED;

    MTLMeshRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLMeshRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"Mesh Pipeline";
    pipelineStateDescriptor.objectFunction = objectFunction;
    pipelineStateDescriptor.meshFunction = meshFunction;
    pipelineStateDescriptor.fragmentFunction = isDepthOnlyPipeline ? nil : fragmentFunction;
    pipelineStateDescriptor.rasterSampleCount = DenOfIz_MSAASampleCount_ToNumSamples( m_desc.Graphics.MSAASampleCount );

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
        metalColorAttachment.pixelFormat = DenOfIz_MetalEnumConverter_ConvertFormat( attachment.Format );

        const DenOfIz_BlendDesc &blendDesc = attachment.Blend;

        metalColorAttachment.blendingEnabled = blendDesc.Enable;

        metalColorAttachment.sourceRGBBlendFactor      = DenOfIz_MetalEnumConverter_ConvertBlendFactor( blendDesc.SrcBlend );
        metalColorAttachment.destinationRGBBlendFactor = DenOfIz_MetalEnumConverter_ConvertBlendFactor( blendDesc.DstBlend );
        metalColorAttachment.rgbBlendOperation         = DenOfIz_MetalEnumConverter_ConvertBlendOp( blendDesc.BlendOp );

        metalColorAttachment.sourceAlphaBlendFactor      = DenOfIz_MetalEnumConverter_ConvertBlendFactor( blendDesc.SrcBlendAlpha );
        metalColorAttachment.destinationAlphaBlendFactor = DenOfIz_MetalEnumConverter_ConvertBlendFactor( blendDesc.DstBlendAlpha );
        metalColorAttachment.alphaBlendOperation         = DenOfIz_MetalEnumConverter_ConvertBlendOp( blendDesc.BlendOpAlpha );

        metalColorAttachment.writeMask = blendDesc.RenderTargetWriteMask;

        attachmentIdx++;
    }

    pipelineStateDescriptor.depthAttachmentPixelFormat = DenOfIz_MetalEnumConverter_ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
    if ( m_desc.Graphics.DepthStencilAttachmentFormat == DENOFIZ_FORMAT_D24_UNORM_S8_UINT) {
        pipelineStateDescriptor.stencilAttachmentPixelFormat = DenOfIz_MetalEnumConverter_ConvertFormat( m_desc.Graphics.DepthStencilAttachmentFormat );
    }
    pipelineStateDescriptor.alphaToCoverageEnabled = m_desc.Graphics.AlphaToCoverageEnable;
    NSError *error = nullptr;
    m_graphicsPipelineState = [m_context->Device newRenderPipelineStateWithMeshDescriptor:pipelineStateDescriptor
                                                                                  options:MTLPipelineOptionNone
                                                                               reflection:nil
                                                                                    error:&error];


    const DenOfIz_Format dsvFormat = m_desc.Graphics.DepthStencilAttachmentFormat;
    bool shouldInitializeStencil = m_desc.Graphics.StencilTest.Enable;

    if ( dsvFormat != DENOFIZ_FORMAT_UNDEFINED && shouldInitializeStencil )
    {
        const bool isDepthOnly = DenOfIz_Format_IsDepthOnly( dsvFormat );

        if ( isDepthOnly )
        {
            spdlog::warn( "MetalPipeline: Stencil operations enabled with depth-only format ({}) in mesh pipeline. "
                          "Stencil operations may be ignored.",
                          static_cast<int>( dsvFormat ) );
        }
    }

    MTLDepthStencilDescriptor *depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
    if ( m_desc.Graphics.DepthTest.Enable )
    {
        depthStencilDescriptor.depthCompareFunction = DenOfIz_MetalEnumConverter_ConvertCompareOp( m_desc.Graphics.DepthTest.CompareOp );
        depthStencilDescriptor.depthWriteEnabled    = m_desc.Graphics.DepthTest.Write;
    }
    else
    {
        depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
        depthStencilDescriptor.depthWriteEnabled    = NO;
    }

    if ( shouldInitializeStencil )
    {
        depthStencilDescriptor.frontFaceStencil = [[MTLStencilDescriptor alloc] init];
        InitStencilFace( depthStencilDescriptor.frontFaceStencil, m_desc.Graphics.StencilTest.FrontFace );
        depthStencilDescriptor.backFaceStencil = [[MTLStencilDescriptor alloc] init];
        InitStencilFace( depthStencilDescriptor.backFaceStencil, m_desc.Graphics.StencilTest.BackFace );
    }
    m_depthStencilState = [m_context->Device newDepthStencilStateWithDescriptor:depthStencilDescriptor];

    switch ( m_desc.Graphics.CullMode )
    {
    case DENOFIZ_CULL_MODE_NONE:
        m_cullMode = MTLCullModeNone;
        break;
    case DENOFIZ_CULL_MODE_FRONT_FACE:
        m_cullMode = MTLCullModeFront;
        break;
    case DENOFIZ_CULL_MODE_BACK_FACE:
        m_cullMode = MTLCullModeBack;
        break;
    }

    switch ( m_desc.Graphics.FillMode )
    {
    case DENOFIZ_FILL_MODE_SOLID:
        m_fillMode = MTLTriangleFillModeFill;
        break;
    case DENOFIZ_FILL_MODE_WIREFRAME:
        m_fillMode = MTLTriangleFillModeLines;
        break;
    }

    m_frontFaceWinding   = m_desc.Graphics.Rasterization.FrontCounterClockwise ? MTLWindingCounterClockwise : MTLWindingClockwise;
    m_depthBias          = static_cast<float>( m_desc.Graphics.Rasterization.DepthBias );
    m_depthBiasSlopeScale = m_desc.Graphics.Rasterization.SlopeScaledDepthBias;
    m_depthBiasClamp     = m_desc.Graphics.Rasterization.DepthBiasClamp;
}

id<MTLIntersectionFunctionTable> MetalPipeline::IntersectionFunctionTable( ) const
{
    return m_intersectionFunctionTable;
}
