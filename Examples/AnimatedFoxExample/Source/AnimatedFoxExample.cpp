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

#include "DenOfIzExamples/AnimatedFoxExample.h"

#include <array>
#include <cstring>
#include <string>
#include <unordered_map>
#include "DenOfIzExamples/FileIO.h"
#include "DenOfIzExamples/InteropMathConverter.h"

using namespace DirectX;
using namespace DenOfIz;

void AnimatedFoxExample::Init( )
{
    LoadFoxAssets( );
    SetupAnimation( );
    CreateBuffers( );
    CreateShaders( );

    m_camera->SetPosition( XMVECTOR{ 0.0f, 50.0f, -200.0f, 1.0f } );
    m_camera->SetFront( XMVECTOR{ 0.0f, 0.0f, 1.0f, 0.0f } );

    DenOfIz_TextureDesc depthTextureDesc{ };
    depthTextureDesc.Format                = DENOFIZ_FORMAT_D32_FLOAT;
    depthTextureDesc.Width                 = m_windowDesc.Width;
    depthTextureDesc.Height                = m_windowDesc.Height;
    depthTextureDesc.Depth                 = 1;
    depthTextureDesc.ArraySize             = 1;
    depthTextureDesc.MipLevels             = 1;
    depthTextureDesc.Usage                 = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
    depthTextureDesc.ClearDepthStencilHint = { 1.0f, 0.0f };

    for ( int i = 0; i < 3; ++i )
    {
        const std::string debugName  = "DepthTexture" + std::to_string( i );
        depthTextureDesc.DebugName   = DENOFIZ_STRING( debugName.c_str( ) );
        DenOfIz_Texture depthTexture = DENOFIZ_NULL_HANDLE;
        DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &depthTextureDesc, &depthTexture );
        m_depthTextures.push_back( depthTexture );
        DenOfIz_ResourceTracking_TrackTexture( m_resourceTracking, depthTexture, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    }
}

AnimatedFoxExample::~AnimatedFoxExample( )
{
    if ( DENOFIZ_HANDLE_IS_VALID( m_ozzAnimation ) )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( m_walkContext ) )
        {
            DenOfIz_OzzAnimation_DestroyContext( m_ozzAnimation, m_walkContext );
        }
        if ( DENOFIZ_HANDLE_IS_VALID( m_runContext ) )
        {
            DenOfIz_OzzAnimation_DestroyContext( m_ozzAnimation, m_runContext );
        }
        DenOfIz_OzzAnimation_Destroy( m_ozzAnimation );
    }

    if ( m_modelTransforms.Elements )
    {
        free( m_modelTransforms.Elements );
    }

    m_gltfLoader.reset( );

    for ( auto &depthTexture : m_depthTextures )
    {
        DenOfIz_TextureResource_Destroy( depthTexture );
    }

    DenOfIz_Buffer_Destroy( m_vertexBuffer );
    DenOfIz_Buffer_Destroy( m_indexBuffer );

    DenOfIz_Buffer_UnmapMemory( m_boneTransformsBuffer );
    DenOfIz_Buffer_Destroy( m_boneTransformsBuffer );

    DenOfIz_Buffer_UnmapMemory( m_perFrameBuffer );
    DenOfIz_Buffer_Destroy( m_perFrameBuffer );

    DenOfIz_Buffer_UnmapMemory( m_materialBuffer );
    DenOfIz_Buffer_Destroy( m_materialBuffer );
    DenOfIz_TextureResource_Destroy( m_texture );
    DenOfIz_Sampler_Destroy( m_defaultSampler );
    DenOfIz_Pipeline_Destroy( m_skinnedMeshPipeline );
    DenOfIz_RootSignature_Destroy( m_skinnedMeshRootSignature );
    for ( auto &layout : m_bindGroupLayouts )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( layout ) )
        {
            DenOfIz_BindGroupLayout_Destroy( layout );
        }
    }
    DenOfIz_BindGroup_Destroy( m_bindGroup );
    DenOfIz_InputLayout_Destroy( m_inputLayout );
    DenOfIz_ShaderProgram_Destroy( m_shaderProgram );
}

void AnimatedFoxExample::ModifyApiPreferences( DenOfIz_APIPreference &apiPreference )
{
    apiPreference.Windows = DENOFIZ_API_PREFERENCE_WINDOWS_VULKAN;
}

void AnimatedFoxExample::LoadFoxAssets( )
{
    DenOfIz_StringView gltfPath         = DENOFIZ_STRING( "Assets/Models/Fox.gltf" );
    DenOfIz_StringView skeletonPath     = DENOFIZ_STRING( "Assets/Models/Fox_Exported_skeleton.ozzskel" );
    DenOfIz_StringView walkAnimPath     = DENOFIZ_STRING( "Assets/Models/Fox_Exported_Survey.ozzanim" );
    DenOfIz_StringView runAnimPath      = DENOFIZ_STRING( "Assets/Models/Fox_Exported_Run.ozzanim" );
    DenOfIz_StringView exportedGltfPath = DENOFIZ_STRING( "Assets/Models/Fox_Exported_exported.glb" );

    bool needsExport = !FileIO::FileExists( skeletonPath ) || !FileIO::FileExists( walkAnimPath ) || !FileIO::FileExists( runAnimPath );

    if ( needsExport )
    {
        spdlog::warn( "One or more ozz assets are missing. Exporting using GltfExporter..." );
        if ( !ExportFoxModel( gltfPath ) )
        {
            spdlog::critical( "Failed to export fox model!" );
            return;
        }
        spdlog::info( "Successfully exported fox model with ozz animations." );
    }

    LoadMeshFromGltf( "Assets/Models/Fox_Exported.glb" );
}

bool AnimatedFoxExample::ExportFoxModel( const DenOfIz_StringView &gltfPath )
{
    if ( !FileIO::FileExists( gltfPath ) )
    {
        std::string pathStr( gltfPath.Chars, gltfPath.NumChars );
        spdlog::critical( "Source GLTF file not found at: {}", pathStr );
        return false;
    }

    DenOfIz_GltfExporter gltfExporter = DenOfIz_GltfExporter_Create( );
    if ( !DenOfIz_GltfExporter_ValidateFile( gltfExporter, gltfPath ) )
    {
        spdlog::error( "GltfExporter cannot process the file" );
        DenOfIz_GltfExporter_Destroy( gltfExporter );
        return false;
    }

    DenOfIz_GltfExportDesc exportDesc   = { };
    exportDesc.SourceFilePath           = gltfPath;
    exportDesc.TargetDirectory          = DENOFIZ_STRING( "Assets/Models/" );
    exportDesc.AssetNamePrefix          = DENOFIZ_STRING( "Fox_Exported" );
    exportDesc.OutputFormat             = DENOFIZ_GLTF_EXPORT_FORMAT_GLB;
    exportDesc.EmbedTextures            = true;
    exportDesc.OverwriteExisting        = true;
    exportDesc.OptimizeMeshes           = true;
    exportDesc.ScaleFactor              = 0.1f;
    exportDesc.JoinIdenticalVertices    = true;
    exportDesc.PreTransformVertices     = false;
    exportDesc.LimitBoneWeights         = true;
    exportDesc.MaxBoneWeightsPerVertex  = 4;
    exportDesc.RemoveRedundantMaterials = true;
    exportDesc.MergeMeshes              = false;
    exportDesc.OptimizeGraph            = false;
    exportDesc.GenerateNormals          = true;
    exportDesc.SmoothNormals            = true;
    exportDesc.SmoothNormalsAngle       = 80.0f;
    exportDesc.TriangulateMeshes        = true;
    exportDesc.PreservePivots           = true;
    exportDesc.DropNormals              = false;
    exportDesc.CalculateTangentSpace    = true;
    exportDesc.FixInfacingNormals       = false;

    DenOfIz_GltfExportResult gltfResult = DenOfIz_GltfExporter_Export( gltfExporter, &exportDesc );
    DenOfIz_GltfExporter_Destroy( gltfExporter );

    if ( gltfResult.ResultCode != DENOFIZ_GLTF_EXPORT_SUCCESS )
    {
        std::string errorMessage( gltfResult.ErrorMessage.Chars, gltfResult.ErrorMessage.NumChars );
        spdlog::error( "GLTF export failed: {}", errorMessage );
        DenOfIz_GltfExportResult_Destroy( &gltfResult );
        return false;
    }

    spdlog::info( "Exported GLTF: {}", gltfResult.GltfFilePath.Chars ? gltfResult.GltfFilePath.Chars : "N/A" );

    DenOfIz_OzzExporter ozzExporter = DenOfIz_OzzExporter_Create( );

    DenOfIz_OzzExportDesc ozzExportDesc{ };
    ozzExportDesc.GltfSourcePath    = gltfResult.GltfFilePath;
    ozzExportDesc.OutputDirectory   = DENOFIZ_STRING( "Assets/Models/" );
    ozzExportDesc.AssetNamePrefix   = DENOFIZ_STRING( "Fox_Exported" );
    ozzExportDesc.ExportSkeleton    = true;
    ozzExportDesc.ExportAnimations  = true;
    ozzExportDesc.OverwriteExisting = true;

    DenOfIz_OzzExportResult ozzResult = DenOfIz_OzzExporter_Export( ozzExporter, &ozzExportDesc );
    DenOfIz_OzzExporter_Destroy( ozzExporter );

    DenOfIz_GltfExportResult_Destroy( &gltfResult );

    if ( ozzResult.ResultCode != DENOFIZ_OZZ_EXPORT_SUCCESS )
    {
        std::string errorMessage( ozzResult.ErrorMessage.Chars, ozzResult.ErrorMessage.NumChars );
        spdlog::error( "Ozz export failed: {}", errorMessage );
        DenOfIz_OzzExportResult_Destroy( &ozzResult );
        return false;
    }

    spdlog::info( "Exported skeleton: {}", ozzResult.SkeletonFilePath.Chars ? ozzResult.SkeletonFilePath.Chars : "N/A" );
    for ( uint32_t i = 0; i < ozzResult.AnimationFilePaths.NumElements; ++i )
    {
        const auto &animPath = ozzResult.AnimationFilePaths.Elements[ i ];
        spdlog::info( "Exported animation: {}", animPath.Chars ? animPath.Chars : "N/A" );
    }

    DenOfIz_OzzExportResult_Destroy( &ozzResult );
    return true;
}

void AnimatedFoxExample::LoadMeshFromGltf( const char *gltfPath )
{
    m_gltfLoader = std::make_unique<GltfLoader>( );

    GltfLoadResult result = m_gltfLoader->Load( gltfPath, true );

    if ( !result.Success )
    {
        spdlog::error( "Failed to load GLTF file: {}", result.ErrorMessage );
        return;
    }

    if ( result.Meshes.empty( ) )
    {
        spdlog::error( "No meshes found in GLTF file" );
        return;
    }

    for ( const auto &mesh : result.Meshes )
    {
        size_t baseVertex = m_vertices.size( );
        m_vertices.resize( baseVertex + mesh.Vertices.size( ) );

        for ( size_t v = 0; v < mesh.Vertices.size( ); ++v )
        {
            const GltfVertex &gv = mesh.Vertices[ v ];
            SkinnedVertex    &sv = m_vertices[ baseVertex + v ];

            sv.Position     = gv.Position;
            sv.Normal       = gv.Normal;
            sv.TexCoord     = gv.TexCoord;
            sv.Tangent      = gv.Tangent;
            sv.BlendIndices = gv.BlendIndices;
            sv.BoneWeights  = gv.BoneWeights;
        }

        size_t baseIndex = m_indices.size( );
        m_indices.resize( baseIndex + mesh.Indices.size( ) );

        for ( size_t i = 0; i < mesh.Indices.size( ); ++i )
        {
            m_indices[ baseIndex + i ] = mesh.Indices[ i ] + static_cast<uint32_t>( baseVertex );
        }
    }

    if ( !result.Skins.empty( ) )
    {
        const GltfSkin &skin  = result.Skins[ 0 ];
        m_inverseBindMatrices = skin.InverseBindMatrices;
    }

    spdlog::info( "Loaded mesh with {} vertices and {} indices", m_vertices.size( ), m_indices.size( ) );
}

void AnimatedFoxExample::SetupAnimation( )
{
    DenOfIz_StringView skeletonPath = DENOFIZ_STRING( "Assets/Models/Fox_Exported_skeleton.ozzskel" );
    m_ozzAnimation                  = DenOfIz_OzzAnimation_Create( skeletonPath );

    if ( !DenOfIz_OzzAnimation_IsValid( m_ozzAnimation ) )
    {
        spdlog::error( "Failed to load skeleton" );
        return;
    }

    m_walkContext   = DenOfIz_OzzAnimation_NewContext( m_ozzAnimation );
    m_runContext    = DenOfIz_OzzAnimation_NewContext( m_ozzAnimation );
    m_activeContext = m_walkContext;

    DenOfIz_StringView walkAnimPath = DENOFIZ_STRING( "Assets/Models/Fox_Exported_Survey.ozzanim" );
    DenOfIz_StringView runAnimPath  = DENOFIZ_STRING( "Assets/Models/Fox_Exported_Run.ozzanim" );

    if ( !DenOfIz_OzzAnimation_LoadAnimation( m_ozzAnimation, walkAnimPath, m_walkContext ) )
    {
        spdlog::error( "Failed to load walk animation" );
    }

    if ( !DenOfIz_OzzAnimation_LoadAnimation( m_ozzAnimation, runAnimPath, m_runContext ) )
    {
        spdlog::error( "Failed to load run animation" );
    }

    int numJoints = DenOfIz_OzzAnimation_GetNumJoints( m_ozzAnimation );

    m_modelTransforms.NumElements = numJoints;
    m_modelTransforms.Elements    = static_cast<DenOfIz_Float4x4 *>( malloc( m_modelTransforms.NumElements * sizeof( DenOfIz_Float4x4 ) ) );

    spdlog::info( "Animation setup complete: {} joints", numJoints );
}

void AnimatedFoxExample::CreateBuffers( )
{
    DenOfIz_BufferDesc vbDesc{ };
    vbDesc.Usage     = DENOFIZ_BUFFER_USAGE_VERTEX_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    vbDesc.NumBytes  = m_vertices.size( ) * sizeof( SkinnedVertex );
    vbDesc.DebugName = DENOFIZ_STRING( "FoxMesh_VertexBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &vbDesc, &m_vertexBuffer );

    DenOfIz_BufferDesc ibDesc{ };
    ibDesc.Usage     = DENOFIZ_BUFFER_USAGE_INDEX_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    ibDesc.NumBytes  = m_indices.size( ) * sizeof( uint32_t );
    ibDesc.DebugName = DENOFIZ_STRING( "FoxMesh_IndexBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &ibDesc, &m_indexBuffer );

    DenOfIz_BufferDesc boneBufferDesc{ };
    boneBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    boneBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    boneBufferDesc.NumBytes  = sizeof( SkinnedModelConstantBuffer );
    boneBufferDesc.DebugName = DENOFIZ_STRING( "FoxMesh_BoneTransformsBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &boneBufferDesc, &m_boneTransformsBuffer );

    void *mappedBoneData = nullptr;
    DenOfIz_Buffer_MapMemory( m_boneTransformsBuffer, &mappedBoneData );
    m_boneTransformsData = static_cast<SkinnedModelConstantBuffer *>( mappedBoneData );

    DenOfIz_BufferDesc perFrameBufferDesc{ };
    perFrameBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    perFrameBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    perFrameBufferDesc.NumBytes  = sizeof( PerFrameConstantBuffer );
    perFrameBufferDesc.DebugName = DENOFIZ_STRING( "FoxMesh_PerFrameBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &perFrameBufferDesc, &m_perFrameBuffer );

    void *mappedPerFrameData = nullptr;
    DenOfIz_Buffer_MapMemory( m_perFrameBuffer, &mappedPerFrameData );
    m_perFrameData                 = static_cast<PerFrameConstantBuffer *>( mappedPerFrameData );
    m_perFrameData->ViewProjection = XMMatrixIdentity( );

    DenOfIz_BatchResourceCopy     batchCopy;
    DenOfIz_BatchResourceCopyDesc batchDesc{ };
    batchDesc.Device = m_logicalDevice;
    DenOfIz_BatchResourceCopy_Create( &batchDesc, &batchCopy );
    DenOfIz_BatchResourceCopy_Begin( batchCopy );

    DenOfIz_CopyToGpuBufferDesc vertexCopyDesc;
    vertexCopyDesc.DstBuffer        = m_vertexBuffer;
    vertexCopyDesc.DstBufferOffset  = 0;
    vertexCopyDesc.Data.Elements    = reinterpret_cast<const Byte *>( m_vertices.data( ) );
    vertexCopyDesc.Data.NumElements = m_vertices.size( ) * sizeof( SkinnedVertex );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &vertexCopyDesc );

    DenOfIz_CopyToGpuBufferDesc indexCopyDesc;
    indexCopyDesc.DstBuffer        = m_indexBuffer;
    indexCopyDesc.DstBufferOffset  = 0;
    indexCopyDesc.Data.Elements    = reinterpret_cast<Byte *>( m_indices.data( ) );
    indexCopyDesc.Data.NumElements = m_indices.size( ) * sizeof( uint32_t );
    DenOfIz_BatchResourceCopy_CopyToGPUBuffer( batchCopy, &indexCopyDesc );

    DenOfIz_BatchResourceCopy_Submit( batchCopy, DENOFIZ_NULL_HANDLE );
    DenOfIz_BatchResourceCopy_Destroy( batchCopy );
}

void AnimatedFoxExample::CreateShaders( )
{
    std::array<DenOfIz_ShaderStageDesc, 2> shaderStages( { } );

    DenOfIz_ShaderStageDesc &vsDesc = shaderStages[ 0 ];
    vsDesc.Stage                    = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
    vsDesc.Path                     = DENOFIZ_STRING( "Assets/Shaders/SkinnedMesh.vs.hlsl" );
    vsDesc.EntryPoint               = DENOFIZ_STRING( "main" );

    DenOfIz_ShaderStageDesc &psDesc = shaderStages[ 1 ];
    psDesc.Stage                    = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
    psDesc.Path                     = DENOFIZ_STRING( "Assets/Shaders/SkinnedMesh.ps.hlsl" );
    psDesc.EntryPoint               = DENOFIZ_STRING( "main" );

    DenOfIz_ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements    = shaderStages.data( );
    programDesc.ShaderStages.NumElements = shaderStages.size( );
    m_shaderProgram                      = DenOfIz_ShaderProgram_Create( &programDesc );

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
    DenOfIz_LogicalDevice_CreateRootSignature( m_logicalDevice, &rootSigDesc, &m_skinnedMeshRootSignature );
    DenOfIz_LogicalDevice_CreateInputLayout( m_logicalDevice, &reflectDesc.InputLayout, &m_inputLayout );

    DenOfIz_RenderTargetDesc renderTargetDesc{ };
    renderTargetDesc.Format                      = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    renderTargetDesc.Blend.RenderTargetWriteMask = 0x0F;

    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.InputLayout                           = m_inputLayout;
    pipelineDesc.RootSignature                         = m_skinnedMeshRootSignature;
    pipelineDesc.ShaderProgram                         = m_shaderProgram;
    pipelineDesc.Graphics.PrimitiveTopology            = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    pipelineDesc.Graphics.CullMode                     = DENOFIZ_CULL_MODE_BACK_FACE;
    pipelineDesc.Graphics.RenderTargets.Elements       = &renderTargetDesc;
    pipelineDesc.Graphics.RenderTargets.NumElements    = 1;
    pipelineDesc.Graphics.DepthTest.Enable             = true;
    pipelineDesc.Graphics.DepthTest.Write              = true;
    pipelineDesc.Graphics.DepthTest.CompareOp          = DENOFIZ_COMPARE_OP_LESS_OR_EQUAL;
    pipelineDesc.Graphics.DepthStencilAttachmentFormat = DENOFIZ_FORMAT_D32_FLOAT;

    DenOfIz_LogicalDevice_CreatePipeline( m_logicalDevice, &pipelineDesc, &m_skinnedMeshPipeline );

    DenOfIz_BindGroupDesc bindGroupDesc{ };
    bindGroupDesc.Layout = m_bindGroupLayouts[ 0 ];
    DenOfIz_LogicalDevice_CreateBindGroup( m_logicalDevice, &bindGroupDesc, &m_bindGroup );

    DenOfIz_BindGroup_BeginUpdate( m_bindGroup );
    DenOfIz_BindGroup_Cbv( m_bindGroup, 0, m_boneTransformsBuffer );
    DenOfIz_BindGroup_Cbv( m_bindGroup, 1, m_perFrameBuffer );

    DenOfIz_BufferDesc materialBufferDesc{ };
    materialBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    materialBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    materialBufferDesc.NumBytes  = sizeof( MaterialConstantBuffer );
    materialBufferDesc.DebugName = DENOFIZ_STRING( "FoxMesh_MaterialBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( m_logicalDevice, &materialBufferDesc, &m_materialBuffer );

    void *mappedMaterialData = nullptr;
    DenOfIz_Buffer_MapMemory( m_materialBuffer, &mappedMaterialData );
    m_materialData = static_cast<MaterialConstantBuffer *>( mappedMaterialData );

    m_materialData->DiffuseColor      = XMVECTOR{ 0.8f, 0.8f, 0.8f, 1.0f };
    m_materialData->AmbientColor      = XMVECTOR{ 0.2f, 0.2f, 0.2f, 1.0f };
    m_materialData->SpecularPower     = 32.0f;
    m_materialData->SpecularIntensity = 0.5f;

    DenOfIz_BindGroup_Cbv( m_bindGroup, 2, m_materialBuffer );

    CreateTexture( );
    CreateDefaultSampler( );

    DenOfIz_BindGroup_SrvTexture( m_bindGroup, 0, m_texture );
    DenOfIz_BindGroup_Sampler( m_bindGroup, 0, m_defaultSampler );

    DenOfIz_BindGroup_EndUpdate( m_bindGroup );
}

void AnimatedFoxExample::Update( )
{
    const auto deltaTime = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    m_camera->Update( deltaTime );

    if ( m_perFrameData != nullptr )
    {
        m_perFrameData->ViewProjection = m_camera->ViewProjectionMatrix( );
        m_perFrameData->CameraPosition = m_camera->Position( );
        float elapsedSeconds           = static_cast<float>( DenOfIz_StepTimer_GetElapsedSeconds( m_stepTimer ) );
        m_perFrameData->Time           = XMVectorSet( elapsedSeconds, deltaTime, 0.0f, 0.0f );
    }

    if ( m_animPlaying && DENOFIZ_HANDLE_IS_VALID( m_activeContext ) )
    {
        float duration = DenOfIz_OzzAnimation_GetAnimationDuration( m_activeContext );
        m_animationRatio += ( deltaTime * m_animSpeed ) / duration;
        while ( m_animationRatio > 1.0f )
        {
            m_animationRatio -= 1.0f;
        }

        DenOfIz_SamplingJobDesc samplingDesc{ };
        samplingDesc.Context       = m_activeContext;
        samplingDesc.Ratio         = m_animationRatio;
        samplingDesc.OutTransforms = m_modelTransforms;
        if ( DenOfIz_OzzAnimation_RunSamplingJob( m_ozzAnimation, &samplingDesc ) )
        {
            UpdateBoneTransforms( );
        }
    }

    RenderAndPresentFrame( );
}

void AnimatedFoxExample::UpdateBoneTransforms( )
{
    const size_t numBones = m_modelTransforms.NumElements;
    const size_t maxBones = std::min( numBones, static_cast<size_t>( 128 ) );

    for ( size_t i = 0; i < maxBones; ++i )
    {
        const DenOfIz_Float4x4 &modelMatrix = m_modelTransforms.Elements[ i ];

        auto           xmModelMatrix = InteropMathConverter::Float_4x4ToXMFLOAT4X4( modelMatrix );
        const XMMATRIX modelMat      = XMLoadFloat4x4( &xmModelMatrix );

        XMMATRIX finalMat;
        if ( i < m_inverseBindMatrices.size( ) )
        {
            auto           xmInvBindMat = InteropMathConverter::Float_4x4ToXMFLOAT4X4( m_inverseBindMatrices[ i ] );
            const XMMATRIX invBindMat   = XMLoadFloat4x4( &xmInvBindMat );
            finalMat                    = XMMatrixMultiply( invBindMat, modelMat );
        }
        else
        {
            finalMat = modelMat;
        }

        XMFLOAT4X4 f4x4FinalMat{ };
        XMStoreFloat4x4( &f4x4FinalMat, finalMat );
        m_boneTransformsData->BoneTransforms[ i ] = InteropMathConverter::Float_4x4FromXMFLOAT4X4( f4x4FinalMat );
    }
}

void AnimatedFoxExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    DenOfIz_CommandList_Begin( commandList );

    uint32_t imageIndex = 0;
    DenOfIz_FrameSync_AcquireNextImage( m_frameSync, &imageIndex );

    DenOfIz_Texture renderTarget = DENOFIZ_NULL_HANDLE;
    DenOfIz_SwapChain_GetRenderTarget( m_swapChain, imageIndex, &renderTarget );
    DenOfIz_Texture depthTarget = m_depthTextures[ frameIndex ];

    DenOfIz_TransitionTextureDesc transitions[ 2 ] = { };
    transitions[ 0 ].Texture                       = renderTarget;
    transitions[ 0 ].NewUsage                      = DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT;
    transitions[ 0 ].QueueType                     = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    transitions[ 1 ].Texture                       = depthTarget;
    transitions[ 1 ].NewUsage                      = DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT;
    transitions[ 1 ].QueueType                     = DENOFIZ_QUEUE_TYPE_GRAPHICS;

    DenOfIz_BatchTransitionDesc batchTransitionDesc{ };
    batchTransitionDesc.Textures.Elements    = transitions;
    batchTransitionDesc.Textures.NumElements = 2;
    DenOfIz_ResourceTracking_BatchTransition( m_resourceTracking, commandList, &batchTransitionDesc );

    const DenOfIz_Viewport *viewport = nullptr;
    DenOfIz_SwapChain_GetViewport( m_swapChain, &viewport );

    DenOfIz_RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource   = renderTarget;
    attachmentDesc.ClearColor = { 0.1f, 0.1f, 0.2f, 1.0f };

    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements            = &attachmentDesc;
    renderingDesc.RTAttachments.NumElements         = 1;
    renderingDesc.NumLayers                         = 1;
    renderingDesc.DepthAttachment.Resource          = depthTarget;
    renderingDesc.DepthAttachment.ClearDepthStencil = { 1.0f, 0.0f };

    DenOfIz_CommandList_BeginRendering( commandList, &renderingDesc );

    DenOfIz_CommandList_BindViewport( commandList, 0.0f, 0.0f, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindScissorRect( commandList, 0.0f, 0.0f, viewport->Width, viewport->Height );
    DenOfIz_CommandList_BindPipeline( commandList, m_skinnedMeshPipeline );
    DenOfIz_CommandList_BindGroup( commandList, m_bindGroup );
    DenOfIz_CommandList_BindVertexBuffer( commandList, m_vertexBuffer, 0, sizeof( SkinnedVertex ), 0 );
    DenOfIz_CommandList_BindIndexBuffer( commandList, m_indexBuffer, DENOFIZ_INDEX_TYPE_UINT32, 0 );
    DenOfIz_CommandList_DrawIndexed( commandList, static_cast<uint32_t>( m_indices.size( ) ), 1, 0, 0, 0 );
    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, renderTarget, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
}

void AnimatedFoxExample::HandleEvent( DenOfIz_Event &event )
{
    switch ( event.Type )
    {
    case DENOFIZ_EVENT_TYPE_KEY_DOWN:
        {
            switch ( event.Key.KeyCode )
            {
            case DENOFIZ_KEY_CODE_W:
                m_activeContext  = m_walkContext;
                m_currentAnim    = "Walk";
                m_animationRatio = 0.0f;
                break;
            case DENOFIZ_KEY_CODE_R:
                m_activeContext  = m_runContext;
                m_currentAnim    = "Run";
                m_animationRatio = 0.0f;
                break;
            case DENOFIZ_KEY_CODE_SPACE:
                m_animPlaying = !m_animPlaying;
                break;
            case DENOFIZ_KEY_CODE_UP:
                m_animSpeed += 0.1f;
                break;
            case DENOFIZ_KEY_CODE_DOWN:
                m_animSpeed = std::max( 0.1f, m_animSpeed - 0.1f );
                break;
            default:
                break;
            }
            break;
        }
    default:
        break;
    }

    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

void AnimatedFoxExample::OnResize( uint32_t width, uint32_t height )
{
    IExample::OnResize( width, height );
}

void AnimatedFoxExample::CreateTexture( )
{
    DenOfIz_StringView texturePath = DENOFIZ_STRING( "Assets/Models/Texture.png" );

    if ( !FileIO::FileExists( texturePath ) )
    {
        spdlog::warn( "Texture not found, creating fallback texture" );
        DenOfIz_TextureDesc textureDesc{ };
        textureDesc.Width     = 1;
        textureDesc.Height    = 1;
        textureDesc.Depth     = 1;
        textureDesc.ArraySize = 1;
        textureDesc.MipLevels = 1;
        textureDesc.Format    = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Usage     = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_COPY_DST_BIT;
        textureDesc.DebugName = DENOFIZ_STRING( "FoxMesh_FallbackTexture" );
        DenOfIz_LogicalDevice_CreateTexture( m_logicalDevice, &textureDesc, &m_texture );
        return;
    }

    DenOfIz_BatchResourceCopy     batchCopy;
    DenOfIz_BatchResourceCopyDesc batchDesc{ };
    batchDesc.Device = m_logicalDevice;
    DenOfIz_BatchResourceCopy_Create( &batchDesc, &batchCopy );
    DenOfIz_BatchResourceCopy_Begin( batchCopy );

    m_texture = DenOfIz_BatchResourceCopy_CreateAndLoadTexture( batchCopy, texturePath );

    DenOfIz_BatchResourceCopy_Submit( batchCopy, DENOFIZ_NULL_HANDLE );
    DenOfIz_BatchResourceCopy_Destroy( batchCopy );

    spdlog::info( "Loaded texture from: {}", std::string( texturePath.Chars, texturePath.NumChars ) );
}

void AnimatedFoxExample::CreateDefaultSampler( )
{
    DenOfIz_SamplerDesc samplerDesc{ };
    DenOfIz_LogicalDevice_CreateSampler( m_logicalDevice, &samplerDesc, &m_defaultSampler );
}
