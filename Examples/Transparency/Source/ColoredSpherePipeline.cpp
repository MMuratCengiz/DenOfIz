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

#include "DenOfIzExamples/ColoredSpherePipeline.h"

using namespace DenOfIz;
using namespace DirectX;

uint32_t Align( const uint32_t value, const uint32_t alignment )
{
    return ( value + alignment - 1 ) & ~( alignment - 1 );
}

ColoredSpherePipeline::ColoredSpherePipeline( const GraphicsApi *graphicsApi, ILogicalDevice *device, bool isTransparent, uint32_t numSpheres ) :
    m_device( device ), m_isTransparent( isTransparent ), m_numSpheres( numSpheres )
{
    InteropArray<ShaderStageDesc> shaderStages{ };
    ShaderStageDesc              &vertexShaderDesc = shaderStages.EmplaceElement( );
    vertexShaderDesc.Path                          = "Assets/Shaders/ColoredSphere.vs.hlsl";
    vertexShaderDesc.Stage                         = ShaderStage::Vertex;

    ShaderStageDesc &pixelShaderDesc = shaderStages.EmplaceElement( );
    if ( isTransparent )
    {
        pixelShaderDesc.Path = "Assets/Shaders/TransparentGlassSphere.ps.hlsl";
    }
    else
    {
        pixelShaderDesc.Path = "Assets/Shaders/OpaqueColoredSphere.ps.hlsl";
    }
    pixelShaderDesc.Stage = ShaderStage::Pixel;

    m_program              = std::make_unique<ShaderProgram>( ShaderProgramDesc{ .ShaderStages = shaderStages } );
    auto programReflection = m_program->Reflect( );

    m_rootSignature = std::unique_ptr<IRootSignature>( device->CreateRootSignature( programReflection.RootSignature ) );
    m_inputLayout   = std::unique_ptr<IInputLayout>( device->CreateInputLayout( programReflection.InputLayout ) );

    PipelineDesc pipelineDesc{ };
    pipelineDesc.InputLayout       = m_inputLayout.get( );
    pipelineDesc.RootSignature     = m_rootSignature.get( );
    pipelineDesc.ShaderProgram     = m_program.get( );
    pipelineDesc.Graphics.CullMode = CullMode::BackFace;

    pipelineDesc.Graphics.DepthTest.Enable             = true;
    pipelineDesc.Graphics.DepthTest.CompareOp          = CompareOp::LessOrEqual;
    pipelineDesc.Graphics.DepthStencilAttachmentFormat = Format::D32Float;

    if ( isTransparent )
    {
        // Set up alpha blending for transparent objects
        RenderTargetDesc rtDesc;
        rtDesc.Format                      = Format::B8G8R8A8Unorm;
        rtDesc.Blend.Enable                = true;
        rtDesc.Blend.SrcBlend              = Blend::SrcAlpha;
        rtDesc.Blend.DstBlend              = Blend::InvSrcAlpha;
        rtDesc.Blend.BlendOp               = BlendOp::Add;
        rtDesc.Blend.SrcBlendAlpha         = Blend::One;
        rtDesc.Blend.DstBlendAlpha         = Blend::Zero;
        rtDesc.Blend.BlendOpAlpha          = BlendOp::Add;
        rtDesc.Blend.RenderTargetWriteMask = 0x0F;
        pipelineDesc.Graphics.RenderTargets.AddElement( rtDesc );

        // For transparent objects: enable depth testing but disable depth writing
        // This allows transparent objects to be properly depth-sorted against opaque objects
        // without affecting subsequent transparent objects
        pipelineDesc.Graphics.DepthTest.Write = false;
    }
    else
    {
        pipelineDesc.Graphics.RenderTargets.AddElement( { .Format = Format::B8G8R8A8Unorm } );
        pipelineDesc.Graphics.DepthTest.Write = true;
    }

    m_pipeline = std::unique_ptr<IPipeline>( device->CreatePipeline( pipelineDesc ) );

    {
        BufferDesc viewProjBufferDesc{ };
        viewProjBufferDesc.NumBytes   = sizeof( ViewProjectionData );
        viewProjBufferDesc.Descriptor = BitSet( ResourceDescriptor::UniformBuffer );
        viewProjBufferDesc.HeapType   = HeapType::CPU_GPU;
        viewProjBufferDesc.DebugName  = "ViewProjectionBuffer";
        m_viewProjBuffer              = std::unique_ptr<IBufferResource>( device->CreateBufferResource( viewProjBufferDesc ) );
        m_viewProjMappedData          = static_cast<ViewProjectionData *>( m_viewProjBuffer->MapMemory( ) );

        XMStoreFloat4x4( &m_viewProjMappedData->viewProjection, XMMatrixIdentity( ) );

        ResourceBindGroupDesc bindGroupDesc{ };
        bindGroupDesc.RegisterSpace = 0; // Space 0 for view/projection
        bindGroupDesc.RootSignature = m_rootSignature.get( );
        m_viewProjBindGroup         = std::unique_ptr<IResourceBindGroup>( device->CreateResourceBindGroup( bindGroupDesc ) );
        m_viewProjBindGroup->BeginUpdate( );
        m_viewProjBindGroup->Cbv( 0, m_viewProjBuffer.get( ) );
        m_viewProjBindGroup->EndUpdate( );
    }

    {
        uint32_t   alignedNumBytes = Align( sizeof( ModelMatrixData ), m_device->DeviceInfo( ).Constants.ConstantBufferAlignment );
        BufferDesc modelBufferDesc{ };
        modelBufferDesc.NumBytes   = alignedNumBytes * m_numSpheres;
        modelBufferDesc.Descriptor = BitSet( ResourceDescriptor::UniformBuffer );
        modelBufferDesc.HeapType   = HeapType::CPU_GPU;
        modelBufferDesc.DebugName  = "ModelMatrixBuffer";
        m_modelBuffer              = std::unique_ptr<IBufferResource>( device->CreateBufferResource( modelBufferDesc ) );
        m_modelMappedData          = static_cast<Byte *>( m_modelBuffer->MapMemory( ) );

        m_modelBindGroups.resize( m_numSpheres );
        for ( uint32_t i = 0; i < m_numSpheres; ++i )
        {
            XMFLOAT4X4 modelMat{ };
            XMStoreFloat4x4( &modelMat, XMMatrixIdentity( ) );
            memcpy( m_modelMappedData + alignedNumBytes * i, &modelMat, sizeof( XMFLOAT4X4 ) );

            auto &modelBindGroup = m_modelBindGroups[ i ];

            ResourceBindGroupDesc bindGroupDesc{ };
            bindGroupDesc.RegisterSpace = 30; // Space 30 for model matrix
            bindGroupDesc.RootSignature = m_rootSignature.get( );
            modelBindGroup              = std::unique_ptr<IResourceBindGroup>( device->CreateResourceBindGroup( bindGroupDesc ) );
            modelBindGroup->BeginUpdate( );
            BindBufferDesc bindBufferDesc{ };
            bindBufferDesc.Binding        = 0;
            bindBufferDesc.Resource       = m_modelBuffer.get( );
            bindBufferDesc.ResourceOffset = alignedNumBytes * i;
            modelBindGroup->Cbv( bindBufferDesc );
            modelBindGroup->EndUpdate( );
        }
    }

    {
        uint32_t   alignedNumBytes = Align( sizeof( SphereMaterialData ), m_device->DeviceInfo( ).Constants.ConstantBufferAlignment );
        BufferDesc materialBufferDesc{ };
        materialBufferDesc.NumBytes   = alignedNumBytes * m_numSpheres;
        materialBufferDesc.Descriptor = BitSet( ResourceDescriptor::UniformBuffer );
        materialBufferDesc.HeapType   = HeapType::CPU_GPU;
        materialBufferDesc.DebugName  = isTransparent ? "TransparentMaterialBuffer" : "OpaqueMaterialBuffer";
        m_materialBuffer              = std::unique_ptr<IBufferResource>( device->CreateBufferResource( materialBufferDesc ) );
        m_materialMappedData          = static_cast<Byte *>( m_materialBuffer->MapMemory( ) );

        uint32_t alignedAlphaNumBytes = Align( sizeof( AlphaData ), m_device->DeviceInfo( ).Constants.ConstantBufferAlignment );
        if ( isTransparent )
        {
            BufferDesc alphaBufferDesc{ };
            alphaBufferDesc.NumBytes   = sizeof( AlphaData ) * m_numSpheres;
            alphaBufferDesc.Descriptor = BitSet( ResourceDescriptor::UniformBuffer );
            alphaBufferDesc.HeapType   = HeapType::CPU_GPU;
            alphaBufferDesc.DebugName  = "AlphaAnimationBuffer";
            m_alphaBuffer              = std::unique_ptr<IBufferResource>( device->CreateBufferResource( alphaBufferDesc ) );
            m_alphaMappedData          = static_cast<Byte *>( m_alphaBuffer->MapMemory( ) );
        }

        m_materialBindGroups.resize( m_numSpheres );
        for ( int i = 0; i < m_numSpheres; ++i )
        {
            SphereMaterialData materialData{};
            materialData.color              = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
            materialData.refractionIndex    = isTransparent ? 1.5f : 1.0f;
            materialData.fresnelPower       = isTransparent ? 3.0f : 1.0f;
            memcpy( m_modelMappedData + alignedNumBytes * i, &materialData, sizeof( SphereMaterialData ) );

            auto                 &materialBindGroup = m_materialBindGroups[ i ];
            ResourceBindGroupDesc bindGroupDesc{ };
            bindGroupDesc.RegisterSpace = 1; // Space 1 for material data
            bindGroupDesc.RootSignature = m_rootSignature.get( );
            materialBindGroup           = std::unique_ptr<IResourceBindGroup>( device->CreateResourceBindGroup( bindGroupDesc ) );
            materialBindGroup->BeginUpdate( );
            BindBufferDesc bindBufferDesc{ };
            bindBufferDesc.Binding        = 0;
            bindBufferDesc.Resource       = m_materialBuffer.get( );
            bindBufferDesc.ResourceOffset = alignedNumBytes * i;
            materialBindGroup->Cbv( bindBufferDesc );

            // For transparent objects, also create alpha animation buffer
            if ( isTransparent )
            {
                memcpy( m_modelMappedData + alignedAlphaNumBytes * i, &m_alphaMappedData[ i ], sizeof( AlphaData ) );

                BindBufferDesc bindAlphaBufferDesc{ };
                bindAlphaBufferDesc.Binding        = 1;
                bindAlphaBufferDesc.Resource       = m_alphaBuffer.get( );
                bindAlphaBufferDesc.ResourceOffset = alignedAlphaNumBytes * i;
                materialBindGroup->Cbv( bindAlphaBufferDesc );
            }

            materialBindGroup->EndUpdate( );
        }
    }
}

ColoredSpherePipeline::~ColoredSpherePipeline( )
{
    if ( m_viewProjMappedData )
    {
        m_viewProjBuffer->UnmapMemory( );
    }

    if ( m_modelMappedData )
    {
        m_modelBuffer->UnmapMemory( );
    }

    if ( m_materialMappedData )
    {
        m_materialBuffer->UnmapMemory( );
    }

    if ( m_alphaMappedData )
    {
        m_alphaBuffer->UnmapMemory( );
    }
}

void ColoredSpherePipeline::UpdateViewProjection( const Camera *camera ) const
{
    XMStoreFloat4x4( &m_viewProjMappedData->viewProjection, camera->ViewProjectionMatrix( ) );
}

void ColoredSpherePipeline::UpdateModel( const uint32_t sphereIndex, const XMFLOAT4X4 &modelMatrix ) const
{
    const uint32_t alignedNumBytes = Align( sizeof( ModelMatrixData ), m_device->DeviceInfo( ).Constants.ConstantBufferAlignment );
    memcpy( m_modelMappedData + alignedNumBytes * sphereIndex, &modelMatrix, sizeof( XMFLOAT4X4 ) );
}

void ColoredSpherePipeline::UpdateMaterialColor( const uint32_t sphereIndex, const XMFLOAT4 &color ) const
{
    SphereMaterialData materialData{ };
    materialData.color = color;
    if ( m_isTransparent )
    {
        materialData.refractionIndex = 1.5f; // Glass-like refraction
        materialData.fresnelPower    = 3.0f; // Strong fresnel effect for glass
    }
    else
    {
        materialData.refractionIndex = 1.0f;
        materialData.fresnelPower    = 1.0f;
    }

    const uint32_t alignedNumBytes = Align( sizeof( SphereMaterialData ), m_device->DeviceInfo( ).Constants.ConstantBufferAlignment );
    memcpy( m_materialMappedData + alignedNumBytes * sphereIndex, &materialData, sizeof( SphereMaterialData ) );
}

void ColoredSpherePipeline::UpdateAlphaValue( const uint32_t sphereIndex, const float alphaValue ) const
{
    if ( m_isTransparent && m_alphaMappedData )
    {
        memcpy( m_alphaMappedData + sizeof( AlphaData ) * sphereIndex, &alphaValue, sizeof( AlphaData ) );
    }
}

void ColoredSpherePipeline::Render( const uint32_t sphereIndex, ICommandList *commandList, const AssetData *assetData ) const
{
    commandList->BindPipeline( m_pipeline.get( ) );
    commandList->BindResourceGroup( m_viewProjBindGroup.get( ) );
    commandList->BindResourceGroup( m_modelBindGroups[ sphereIndex ].get( ) );
    commandList->BindResourceGroup( m_materialBindGroups[ sphereIndex ].get( ) );

    commandList->BindVertexBuffer( assetData->VertexBuffer( ) );
    commandList->BindIndexBuffer( assetData->IndexBuffer( ), IndexType::Uint32 );
    commandList->DrawIndexed( assetData->NumIndices( ), 1, 0, 0, 0 );
}
