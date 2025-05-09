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

#include <DenOfIzExamples/AnimatedFoxExample.h>
#include <DenOfIzGraphics/Animation/AnimationStateManager.h>
#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetReader.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryReader.h>
#include <DenOfIzGraphics/Data/BatchResourceCopy.h>

#include "DenOfIzGraphics/Utilities/InteropMathConverter.h"

using namespace DirectX;
using namespace DenOfIz;

void AnimatedFoxExample::Init( )
{
    LoadFoxAssets( );
    SetupAnimation( );
    CreateBuffers( );
    CreateShaders( );

    m_camera->SetPosition( XMVECTOR{ 0.0f, 1.0f, -10.0f, 1.0f } );
    m_camera->SetFront( XMVECTOR{ 0.0f, 0.0f, 1.0f, 0.0f } );

    m_timer.OnEachSecond = []( const uint32_t fps ) { LOG( WARNING ) << "FPS: " << fps; };
}

void AnimatedFoxExample::ModifyApiPreferences( APIPreference &apiPreference )
{
    apiPreference.Windows = APIPreferenceWindows::DirectX12;
}

void AnimatedFoxExample::LoadFoxAssets( )
{
    InteropString meshPath     = "Assets/Models/Fox_Fox_Mesh.dzmesh";
    InteropString texturePath  = "Assets/Models/Fox_Texture_Texture.dztex";
    InteropString skeletonPath = "Assets/Models/Fox_Fox_Skeleton.dzskel";
    InteropString walkAnimPath = "Assets/Models/Fox_Walk_Animation.dzanim";
    InteropString runAnimPath  = "Assets/Models/Fox_Run_Animation.dzanim";

    if ( bool assetsExist = FileIO::FileExists( meshPath ) && FileIO::FileExists( skeletonPath ) && FileIO::FileExists( walkAnimPath ) && FileIO::FileExists( runAnimPath );
         !assetsExist )
    {
        LOG( WARNING ) << "One or more fox assets are missing. Attempting to import the model...";
        if ( InteropString sourceGltfPath = "Assets/Models/Fox.gltf"; !ImportFoxModel( sourceGltfPath ) )
        {
            LOG( FATAL ) << "Failed to import fox model!";
            return;
        }

        assetsExist = FileIO::FileExists( meshPath ) && FileIO::FileExists( skeletonPath ) && FileIO::FileExists( walkAnimPath ) && FileIO::FileExists( runAnimPath );
        if ( !assetsExist )
        {
            LOG( FATAL ) << "Import completed but some assets are still missing. Using fallback quad mesh.";
            return;
        }

        LOG( INFO ) << "Successfully imported fox model.";
    }

    LOG( INFO ) << "Loading mesh from: " << meshPath.Get( );
    BinaryReader        meshReader( meshPath );
    MeshAssetReaderDesc meshReaderDesc{ };
    meshReaderDesc.Reader = &meshReader;
    MeshAssetReader meshAssetReader( meshReaderDesc );
    m_foxMesh = meshAssetReader.Read( );

    LOG( INFO ) << "Loading texture from: " << texturePath.Get( );
    m_textureAssetBinaryReader = std::make_unique<BinaryReader>( texturePath );
    TextureAssetReaderDesc textureReaderDesc{ };
    textureReaderDesc.Reader = m_textureAssetBinaryReader.get( );
    m_textureAssetReader     = std::make_unique<TextureAssetReader>( textureReaderDesc );

    LOG( INFO ) << "Loading skeleton from: " << skeletonPath.Get( );
    BinaryReader            skeletonReader( skeletonPath );
    SkeletonAssetReaderDesc skeletonReaderDesc{ };
    skeletonReaderDesc.Reader = &skeletonReader;
    SkeletonAssetReader skeletonAssetReader( skeletonReaderDesc );
    m_foxSkeleton = skeletonAssetReader.Read( );

    LOG( INFO ) << "Loading animations from: " << walkAnimPath.Get( ) << " and " << runAnimPath.Get( );
    BinaryReader             walkAnimReader( walkAnimPath );
    AnimationAssetReaderDesc walkAnimReaderDesc{ };
    walkAnimReaderDesc.Reader = &walkAnimReader;
    AnimationAssetReader walkAnimAssetReader( walkAnimReaderDesc );
    m_walkAnimation = walkAnimAssetReader.Read( );

    BinaryReader             runAnimReader( runAnimPath );
    AnimationAssetReaderDesc runAnimReaderDesc{ };
    runAnimReaderDesc.Reader = &runAnimReader;
    AnimationAssetReader runAnimAssetReader( runAnimReaderDesc );
    m_runAnimation = runAnimAssetReader.Read( );

    const auto &subMeshes = m_foxMesh.SubMeshes;
    if ( subMeshes.NumElements( ) == 0 )
    {
        LOG( FATAL ) << "Fox mesh has no sub-meshes.";
        return;
    }

    const auto &subMesh = subMeshes.GetElement( 0 );

    m_vertices.clear( );
    m_indices.Clear( );

    LOG( INFO ) << "Reading vertex data from mesh asset...";

    InteropArray<MeshVertex> meshVertices = meshAssetReader.ReadVertices( subMesh.VertexStream );
    LOG( INFO ) << "Read " << meshVertices.NumElements( ) << " vertices from mesh asset";

    m_vertices.resize( meshVertices.NumElements( ) );
    for ( size_t i = 0; i < meshVertices.NumElements( ); ++i )
    {
        const MeshVertex &mv            = meshVertices.GetElement( i );
        SkinnedVertex    &skinnedVertex = m_vertices[ i ];

        skinnedVertex.Position.X = mv.Position.X;
        skinnedVertex.Position.Y = mv.Position.Y;
        skinnedVertex.Position.Z = mv.Position.Z;
        skinnedVertex.Normal.X   = mv.Normal.X;
        skinnedVertex.Normal.Y   = mv.Normal.Y;
        skinnedVertex.Normal.Z   = mv.Normal.Z;
        if ( mv.UVs.NumElements( ) > 0 )
        {
            const Float_2 &uv      = mv.UVs.GetElement( 0 );
            skinnedVertex.TexCoord = { uv.X, uv.Y };
        }
        else
        {
            skinnedVertex.TexCoord = { 0.0f, 0.0f };
        }

        skinnedVertex.Tangent      = { mv.Tangent.X, mv.Tangent.Y, mv.Tangent.Z, mv.Tangent.W };
        skinnedVertex.BlendIndices = { mv.BlendIndices.X, mv.BlendIndices.Y, mv.BlendIndices.Z, mv.BlendIndices.W };
        skinnedVertex.BoneWeights  = { mv.BoneWeights.X, mv.BoneWeights.Y, mv.BoneWeights.Z, mv.BoneWeights.W };
    }

    LOG( INFO ) << "Reading index data from mesh asset...";
    m_indices = meshAssetReader.ReadIndices32( subMesh.IndexStream );
    LOG( INFO ) << "Read " << m_indices.NumElements( ) << " 32-bit indices from mesh asset";
}

void AnimatedFoxExample::SetupAnimation( )
{
    AnimationStateManagerDesc animManagerDesc;
    animManagerDesc.Skeleton = &m_foxSkeleton;
    m_animationManager       = std::make_unique<AnimationStateManager>( animManagerDesc );
    m_animationManager->AddAnimation( m_walkAnimation );
    m_animationManager->AddAnimation( m_runAnimation );
    m_animationManager->Play( "Walk", true );
}

void AnimatedFoxExample::CreateBuffers( )
{
    BufferDesc vbDesc;
    vbDesc.Descriptor = ResourceDescriptor::VertexBuffer;
    vbDesc.NumBytes   = m_vertices.size( ) * sizeof( SkinnedVertex );
    vbDesc.DebugName  = "FoxMesh_VertexBuffer";
    m_vertexBuffer    = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( vbDesc ) );

    BufferDesc ibDesc;
    ibDesc.Descriptor = ResourceDescriptor::IndexBuffer;
    ibDesc.NumBytes   = m_indices.NumElements( ) * sizeof( uint32_t );
    ibDesc.DebugName  = "FoxMesh_IndexBuffer";
    m_indexBuffer     = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( ibDesc ) );

    BufferDesc boneBufferDesc;
    boneBufferDesc.Descriptor = ResourceDescriptor::UniformBuffer;
    boneBufferDesc.HeapType   = HeapType::CPU_GPU;
    boneBufferDesc.NumBytes   = sizeof( SkinnedModelConstantBuffer );
    boneBufferDesc.DebugName  = "FoxMesh_BoneTransformsBuffer";
    m_boneTransformsBuffer    = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( boneBufferDesc ) );
    m_boneTransformsData      = static_cast<SkinnedModelConstantBuffer *>( m_boneTransformsBuffer->MapMemory( ) );

    BufferDesc perFrameBufferDesc;
    perFrameBufferDesc.Descriptor  = ResourceDescriptor::UniformBuffer;
    perFrameBufferDesc.HeapType    = HeapType::CPU_GPU;
    perFrameBufferDesc.NumBytes    = sizeof( PerFrameConstantBuffer );
    perFrameBufferDesc.DebugName   = "FoxMesh_PerFrameBuffer";
    m_perFrameBuffer               = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( perFrameBufferDesc ) );
    m_perFrameData                 = static_cast<PerFrameConstantBuffer *>( m_perFrameBuffer->MapMemory( ) );
    m_perFrameData->ViewProjection = XMMatrixIdentity( );

    BatchResourceCopy batchCopy( m_logicalDevice );
    batchCopy.Begin( );

    size_t             vertexDataSize = m_vertices.size( ) * sizeof( SkinnedVertex );
    InteropArray<Byte> vertexData( vertexDataSize );
    memcpy( vertexData.Data( ), m_vertices.data( ), vertexDataSize );

    size_t             indexDataSize = m_indices.NumElements( ) * sizeof( uint32_t );
    InteropArray<Byte> indexData( indexDataSize );
    memcpy( indexData.Data( ), m_indices.Data( ), indexDataSize );

    CopyToGpuBufferDesc vertexCopyDesc;
    vertexCopyDesc.DstBuffer       = m_vertexBuffer.get( );
    vertexCopyDesc.DstBufferOffset = 0;
    vertexCopyDesc.Data            = vertexData;
    batchCopy.CopyToGPUBuffer( vertexCopyDesc );

    CopyToGpuBufferDesc indexCopyDesc;
    indexCopyDesc.DstBuffer       = m_indexBuffer.get( );
    indexCopyDesc.DstBufferOffset = 0;
    indexCopyDesc.Data            = indexData;
    batchCopy.CopyToGPUBuffer( indexCopyDesc );

    batchCopy.Submit( );
}

void AnimatedFoxExample::CreateShaders( )
{
    InteropArray<ShaderStageDesc> shaderStages( 2 );

    ShaderStageDesc &vsDesc = shaderStages.GetElement( 0 );
    vsDesc.Stage            = ShaderStage::Vertex;
    vsDesc.Path             = "Assets/Shaders/SkinnedMesh.vs.hlsl";
    vsDesc.EntryPoint       = "main";

    ShaderStageDesc &psDesc = shaderStages.GetElement( 1 );
    psDesc.Stage            = ShaderStage::Pixel;
    psDesc.Path             = "Assets/Shaders/SkinnedMesh.ps.hlsl";
    psDesc.EntryPoint       = "main";

    ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages = shaderStages;
    auto skinnedMeshProgram  = std::make_unique<ShaderProgram>( programDesc );

    auto reflection            = skinnedMeshProgram->Reflect( );
    m_skinnedMeshRootSignature = std::unique_ptr<IRootSignature>( m_logicalDevice->CreateRootSignature( reflection.RootSignature ) );

    auto inputLayout = std::unique_ptr<IInputLayout>( m_logicalDevice->CreateInputLayout( reflection.InputLayout ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.InputLayout       = inputLayout.get( );
    pipelineDesc.RootSignature     = m_skinnedMeshRootSignature.get( );
    pipelineDesc.ShaderProgram     = skinnedMeshProgram.get( );
    pipelineDesc.Graphics.CullMode = CullMode::BackFace;
    pipelineDesc.Graphics.RenderTargets.AddElement( { .Format = Format::B8G8R8A8Unorm } );

    m_skinnedMeshPipeline = std::unique_ptr<IPipeline>( m_logicalDevice->CreatePipeline( pipelineDesc ) );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RootSignature = m_skinnedMeshRootSignature.get( );
    m_resourceBindGroup         = std::unique_ptr<IResourceBindGroup>( m_logicalDevice->CreateResourceBindGroup( bindGroupDesc ) );

    m_resourceBindGroup->BeginUpdate( );
    m_resourceBindGroup->Cbv( 0, m_boneTransformsBuffer.get( ) );
    m_resourceBindGroup->Cbv( 1, m_perFrameBuffer.get( ) );

    BufferDesc materialBufferDesc;
    materialBufferDesc.Descriptor = ResourceDescriptor::UniformBuffer;
    materialBufferDesc.HeapType   = HeapType::CPU_GPU;
    materialBufferDesc.NumBytes   = sizeof( MaterialConstantBuffer );
    materialBufferDesc.DebugName  = "FoxMesh_MaterialBuffer";
    m_materialBuffer              = std::unique_ptr<IBufferResource>( m_logicalDevice->CreateBufferResource( materialBufferDesc ) );
    m_materialData                = static_cast<MaterialConstantBuffer *>( m_materialBuffer->MapMemory( ) );

    m_materialData->DiffuseColor      = XMVECTOR{ 0.8f, 0.8f, 0.8f, 1.0f };
    m_materialData->AmbientColor      = XMVECTOR{ 0.2f, 0.2f, 0.2f, 1.0f };
    m_materialData->SpecularPower     = 32.0f;
    m_materialData->SpecularIntensity = 0.5f;

    m_resourceBindGroup->Cbv( 2, m_materialBuffer.get( ) );

    m_texture        = CreateTexture( );
    m_defaultSampler = CreateDefaultSampler( );

    m_resourceBindGroup->Srv( 0, m_texture.get( ) );
    m_resourceBindGroup->Sampler( 0, m_defaultSampler.get( ) );

    m_resourceBindGroup->EndUpdate( );
}

void AnimatedFoxExample::Update( )
{
    m_timer.Tick( );
    m_time.Tick( );
    const auto deltaTime = static_cast<float>( m_time.GetDeltaTime( ) );
    m_camera->Update( deltaTime );
    if ( m_perFrameData != nullptr )
    {
        m_perFrameData->ViewProjection = m_camera->ViewProjectionMatrix( );
        m_perFrameData->CameraPosition = m_camera->Position( );
        m_perFrameData->Time           = XMVectorSet( static_cast<float>( m_timer.GetElapsedSeconds( ) ), static_cast<float>( m_time.GetDeltaTime( ) ), 0.0f, 0.0f );
    }
    if ( m_animPlaying )
    {
        m_animationManager->Update( deltaTime );
        UpdateBoneTransforms( );
    }

    RenderAndPresentFrame( );
}

void AnimatedFoxExample::UpdateBoneTransforms( )
{
    InteropArray<Float_4x4> boneTransforms;
    m_animationManager->GetModelSpaceTransforms( boneTransforms );

    const size_t numBones = m_animationManager->GetNumJoints( );
    const size_t maxBones = std::min( numBones, static_cast<size_t>( 128 ) );

    for ( size_t i = 0; i < maxBones && i < boneTransforms.NumElements( ); ++i )
    {
        const Float_4x4 &modelMatrix = boneTransforms.GetElement( i );
        if ( i < m_foxSkeleton.Joints.NumElements( ) )
        {
            const Joint &joint = m_foxSkeleton.Joints.GetElement( i );

            auto           xmModelMatrix = InteropMathConverter::Float_4x4ToXMFLOAT4X4( modelMatrix );
            const XMMATRIX modelMat      = XMLoadFloat4x4( &xmModelMatrix );
            auto           xmInvBindMat  = InteropMathConverter::Float_4x4ToXMFLOAT4X4( joint.InverseBindMatrix );
            const XMMATRIX invBindMat    = XMLoadFloat4x4( &xmInvBindMat );
            const XMMATRIX finalMat      = XMMatrixMultiply( invBindMat, modelMat );
            XMFLOAT4X4     f4x4FinalMat{ };
            XMStoreFloat4x4( &f4x4FinalMat, finalMat );
            m_boneTransformsData->BoneTransforms[ i ] = InteropMathConverter::Float_4x4FromXMFLOAT4X4( f4x4FinalMat );
        }
        else
        {
            m_boneTransformsData->BoneTransforms[ i ] = modelMatrix;
        }
    }
}

void AnimatedFoxExample::Render( const uint32_t frameIndex, ICommandList *commandList )
{
    commandList->Begin( );

    ITextureResource *renderTarget = m_swapChain->GetRenderTarget( m_frameSync->AcquireNextImage( frameIndex ) );

    BatchTransitionDesc batchTransition( commandList );
    batchTransition.TransitionTexture( renderTarget, ResourceUsage::RenderTarget );
    m_resourceTracking.BatchTransition( batchTransition );

    const Viewport         &viewport = m_swapChain->GetViewport( );
    RenderingAttachmentDesc attachmentDesc{ };
    attachmentDesc.Resource = renderTarget;
    attachmentDesc.SetClearColor( 0.1f, 0.1f, 0.2f, 1.0f );

    RenderingDesc renderingDesc;
    renderingDesc.RTAttachments.AddElement( attachmentDesc );

    commandList->BeginRendering( renderingDesc );

    commandList->BindViewport( 0.0f, 0.0f, viewport.Width, viewport.Height );
    commandList->BindScissorRect( 0.0f, 0.0f, viewport.Width, viewport.Height );
    commandList->BindPipeline( m_skinnedMeshPipeline.get( ) );
    commandList->BindResourceGroup( m_resourceBindGroup.get( ) );
    commandList->BindVertexBuffer( m_vertexBuffer.get( ) );
    commandList->BindIndexBuffer( m_indexBuffer.get( ), IndexType::Uint32 );
    commandList->DrawIndexed( static_cast<uint32_t>( m_indices.NumElements( ) ), 1, 0, 0, 0 );
    commandList->EndRendering( );

    batchTransition = BatchTransitionDesc( commandList );
    batchTransition.TransitionTexture( renderTarget, ResourceUsage::Present );
    m_resourceTracking.BatchTransition( batchTransition );

    commandList->End( );
}

void AnimatedFoxExample::HandleEvent( SDL_Event &event )
{
    switch ( event.type )
    {
    case SDL_KEYDOWN:
        {
            switch ( event.key.keysym.sym )
            {
            case SDLK_w: // Switch to walk animation
                m_animationManager->Play( "Walk", true );
                m_currentAnim = "Walk";
                break;
            case SDLK_r: // Switch to run animation
                m_animationManager->Play( "Run", true );
                m_currentAnim = "Run";
                break;
            case SDLK_b:
                {
                    // Blend between animations
                    const std::string animationName = m_animationManager->GetCurrentAnimationName( ).Get( );
                    if ( animationName == "Walk" )
                    {
                        m_animationManager->BlendTo( "Run", 0.5f );
                        m_currentAnim = "Blending to Run";
                    }
                    else
                    {
                        m_animationManager->BlendTo( "Walk", 0.5f );
                        m_currentAnim = "Blending to Walk";
                    }
                    break;
                }
            case SDLK_SPACE: // Pause/Resume animation
                m_animPlaying = !m_animPlaying;
                if ( m_animPlaying )
                {
                    m_animationManager->Resume( );
                }
                else
                {
                    m_animationManager->Pause( );
                }
                break;
            case SDLK_UP: // Increase animation speed
                m_animSpeed += 0.1f;
                break;
            case SDLK_DOWN: // Decrease animation speed
                m_animSpeed = std::max( 0.1f, m_animSpeed - 0.1f );
                break;
            default:;
            }
            break;
        }
    default:
        break;
    }

    m_worldData.Camera->HandleEvent( event );
    IExample::HandleEvent( event );
}

bool AnimatedFoxExample::ImportFoxModel( const InteropString &gltfPath )
{
    if ( !FileIO::FileExists( gltfPath ) )
    {
        LOG( FATAL ) << "Source GLTF file not found at: " << gltfPath.Get( );
        return false;
    }

    AssimpImporter importer( { } );

    if ( !importer.ValidateFile( gltfPath ) )
    {
        LOG( ERROR ) << "AssimpImporter cannot process the file: " << gltfPath.Get( );
        return false;
    }

    ImportJobDesc importJobDesc;
    importJobDesc.SourceFilePath  = gltfPath;
    importJobDesc.TargetDirectory = "Assets/Models/";
    importJobDesc.AssetNamePrefix = "Fox";

    AssimpImportDesc assimpDesc;
    assimpDesc.ImportMaterials         = true;
    assimpDesc.ImportTextures          = true;
    assimpDesc.ImportSkeletons         = true;
    assimpDesc.ImportAnimations        = true;
    assimpDesc.LimitBoneWeights        = true;
    assimpDesc.MaxBoneWeightsPerVertex = 4;
    assimpDesc.ScaleFactor             = 0.01f;

    importJobDesc.Desc = &assimpDesc;

    ImporterResult result = importer.Import( importJobDesc );
    if ( result.ResultCode != ImporterResultCode::Success )
    {
        LOG( ERROR ) << "Import failed: " << result.ErrorMessage.Get( );
        return false;
    }

    for ( size_t i = 0; i < result.CreatedAssets.NumElements( ); ++i )
    {
        AssetUri uri = result.CreatedAssets.GetElement( i );
        LOG( INFO ) << "Created asset: " << uri.Path.Get( );
    }

    return true;
}

std::unique_ptr<ITextureResource> AnimatedFoxExample::CreateTexture( ) const
{
    BatchResourceCopy batchCopy( m_logicalDevice );
    batchCopy.Begin( );

    CreateAssetTextureDesc createDesc{ };
    createDesc.Reader    = m_textureAssetReader.get( );
    createDesc.DebugName = "FoxMesh_Texture";
    auto texture         = std::unique_ptr<ITextureResource>( batchCopy.CreateAndLoadAssetTexture( createDesc ) );
    batchCopy.Submit( );

    return texture;
}

std::unique_ptr<ISampler> AnimatedFoxExample::CreateDefaultSampler( ) const
{
    return std::unique_ptr<ISampler>( m_logicalDevice->CreateSampler( { } ) );
}
