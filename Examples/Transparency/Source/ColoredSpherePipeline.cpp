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

#include <array>

using namespace DenOfIz;
using namespace DirectX;

uint32_t Align( const uint32_t value, const uint32_t alignment )
{
    return ( value + alignment - 1 ) & ~( alignment - 1 );
}

ColoredSpherePipeline::ColoredSpherePipeline( DenOfIz_GraphicsApi graphicsApi, DenOfIz_LogicalDevice device, bool isTransparent, uint32_t numSpheres ) :
    m_device( device ), m_isTransparent( isTransparent ), m_numSpheres( numSpheres )
{
    std::array<DenOfIz_ShaderStageDesc, 2> shaderStages( { } );
    DenOfIz_ShaderStageDesc               &vertexShaderDesc = shaderStages[ 0 ];
    vertexShaderDesc.Path                                   = DENOFIZ_STRING( "Assets/Shaders/ColoredSphere.vs.hlsl" );
    vertexShaderDesc.EntryPoint                             = DENOFIZ_STRING( "main" );
    vertexShaderDesc.Stage                                  = DENOFIZ_SHADER_STAGE_VERTEX_BIT;

    DenOfIz_ShaderStageDesc &pixelShaderDesc = shaderStages[ 1 ];
    if ( isTransparent )
    {
        pixelShaderDesc.Path = DENOFIZ_STRING( "Assets/Shaders/TransparentGlassSphere.ps.hlsl" );
    }
    else
    {
        pixelShaderDesc.Path = DENOFIZ_STRING( "Assets/Shaders/OpaqueColoredSphere.ps.hlsl" );
    }
    pixelShaderDesc.EntryPoint = DENOFIZ_STRING( "main" );
    pixelShaderDesc.Stage      = DENOFIZ_SHADER_STAGE_PIXEL_BIT;

    DenOfIz_ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.Elements    = shaderStages.data( );
    programDesc.ShaderStages.NumElements = shaderStages.size( );
    m_program                            = DenOfIz_ShaderProgram_Create( &programDesc );

    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( m_program, &reflectDesc );

    m_bindGroupLayouts.resize( reflectDesc.BindGroupLayouts.NumElements );
    for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
    {
        m_spaceToLayoutIndex[ reflectDesc.BindGroupLayouts.Elements[ i ].RegisterSpace ] = i;
        DenOfIz_LogicalDevice_CreateBindGroupLayout( device, &reflectDesc.BindGroupLayouts.Elements[ i ], &m_bindGroupLayouts[ i ] );
    }

    DenOfIz_RootSignatureDesc rootSigDesc{ };
    rootSigDesc.BindGroupLayouts.Elements    = m_bindGroupLayouts.data( );
    rootSigDesc.BindGroupLayouts.NumElements = m_bindGroupLayouts.size( );
    rootSigDesc.RootConstants                = reflectDesc.RootConstants;
    DenOfIz_LogicalDevice_CreateRootSignature( device, &rootSigDesc, &m_rootSignature );
    DenOfIz_LogicalDevice_CreateInputLayout( device, &reflectDesc.InputLayout, &m_inputLayout );

    DenOfIz_PipelineDesc pipelineDesc{ };
    pipelineDesc.InputLayout                = m_inputLayout;
    pipelineDesc.RootSignature              = m_rootSignature;
    pipelineDesc.ShaderProgram              = m_program;
    pipelineDesc.Graphics.PrimitiveTopology = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
    pipelineDesc.Graphics.CullMode          = DENOFIZ_CULL_MODE_BACK_FACE;

    pipelineDesc.Graphics.DepthTest.Enable             = true;
    pipelineDesc.Graphics.DepthTest.CompareOp          = DENOFIZ_COMPARE_OP_LESS_OR_EQUAL;
    pipelineDesc.Graphics.DepthStencilAttachmentFormat = DENOFIZ_FORMAT_D32_FLOAT;

    DenOfIz_RenderTargetDesc rtDesc{ };
    rtDesc.Format                      = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    rtDesc.Blend.RenderTargetWriteMask = 0x0F;

    if ( isTransparent )
    {
        rtDesc.Blend.Enable                             = true;
        rtDesc.Blend.SrcBlend                           = DENOFIZ_BLEND_SRC_ALPHA;
        rtDesc.Blend.DstBlend                           = DENOFIZ_BLEND_INV_SRC_ALPHA;
        rtDesc.Blend.BlendOp                            = DENOFIZ_BLEND_OP_ADD;
        rtDesc.Blend.SrcBlendAlpha                      = DENOFIZ_BLEND_ONE;
        rtDesc.Blend.DstBlendAlpha                      = DENOFIZ_BLEND_ZERO;
        rtDesc.Blend.BlendOpAlpha                       = DENOFIZ_BLEND_OP_ADD;
        pipelineDesc.Graphics.RenderTargets.Elements    = &rtDesc;
        pipelineDesc.Graphics.RenderTargets.NumElements = 1;
        pipelineDesc.Graphics.DepthTest.Write           = false;
    }
    else
    {
        pipelineDesc.Graphics.RenderTargets.Elements    = &rtDesc;
        pipelineDesc.Graphics.RenderTargets.NumElements = 1;
        pipelineDesc.Graphics.DepthTest.Write           = true;
    }

    DenOfIz_LogicalDevice_CreatePipeline( device, &pipelineDesc, &m_pipeline );

    DenOfIz_PhysicalDevice deviceInfo{ };
    DenOfIz_LogicalDevice_DeviceInfo( device, &deviceInfo );

    {
        DenOfIz_BufferDesc viewProjBufferDesc{ };
        viewProjBufferDesc.NumBytes  = sizeof( ViewProjectionData );
        viewProjBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
        viewProjBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
        viewProjBufferDesc.DebugName = DENOFIZ_STRING( "ViewProjectionBuffer" );
        DenOfIz_LogicalDevice_CreateBuffer( device, &viewProjBufferDesc, &m_viewProjBuffer );

        void *mappedData = nullptr;
        DenOfIz_Buffer_MapMemory( m_viewProjBuffer, &mappedData );
        m_viewProjMappedData = static_cast<ViewProjectionData *>( mappedData );

        XMStoreFloat4x4( &m_viewProjMappedData->viewProjection, XMMatrixIdentity( ) );

        DenOfIz_BindGroupDesc bindGroupDesc{ };
        bindGroupDesc.Layout = m_bindGroupLayouts[ m_spaceToLayoutIndex[ 0 ] ];
        DenOfIz_LogicalDevice_CreateBindGroup( device, &bindGroupDesc, &m_viewProjBindGroup );
        DenOfIz_BindGroup_BeginUpdate( m_viewProjBindGroup );
        DenOfIz_BindGroup_Cbv( m_viewProjBindGroup, 0, m_viewProjBuffer );
        DenOfIz_BindGroup_EndUpdate( m_viewProjBindGroup );
    }

    {
        uint32_t           alignedNumBytes = Align( sizeof( ModelMatrixData ), deviceInfo.Constants.ConstantBufferAlignment );
        DenOfIz_BufferDesc modelBufferDesc{ };
        modelBufferDesc.NumBytes  = alignedNumBytes * m_numSpheres;
        modelBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
        modelBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
        modelBufferDesc.DebugName = DENOFIZ_STRING( "ModelMatrixBuffer" );
        DenOfIz_LogicalDevice_CreateBuffer( device, &modelBufferDesc, &m_modelBuffer );

        void *mappedData = nullptr;
        DenOfIz_Buffer_MapMemory( m_modelBuffer, &mappedData );
        m_modelMappedData = static_cast<Byte *>( mappedData );

        m_modelBindGroups.resize( m_numSpheres );
        for ( uint32_t i = 0; i < m_numSpheres; ++i )
        {
            XMFLOAT4X4 modelMat{ };
            XMStoreFloat4x4( &modelMat, XMMatrixIdentity( ) );
            memcpy( m_modelMappedData + alignedNumBytes * i, &modelMat, sizeof( XMFLOAT4X4 ) );

            DenOfIz_BindGroupDesc bindGroupDesc{ };
            bindGroupDesc.Layout = m_bindGroupLayouts[ m_spaceToLayoutIndex[ 30 ] ];
            DenOfIz_LogicalDevice_CreateBindGroup( device, &bindGroupDesc, &m_modelBindGroups[ i ] );

            DenOfIz_BindGroup_BeginUpdate( m_modelBindGroups[ i ] );
            DenOfIz_BindBufferDesc bindBufferDesc{ };
            bindBufferDesc.Binding        = 0;
            bindBufferDesc.Resource       = m_modelBuffer;
            bindBufferDesc.ResourceOffset = alignedNumBytes * i;
            DenOfIz_BindGroup_CbvWithDesc( m_modelBindGroups[ i ], &bindBufferDesc );
            DenOfIz_BindGroup_EndUpdate( m_modelBindGroups[ i ] );
        }
    }

    {
        uint32_t           alignedNumBytes = Align( sizeof( SphereMaterialData ), deviceInfo.Constants.ConstantBufferAlignment );
        DenOfIz_BufferDesc materialBufferDesc{ };
        materialBufferDesc.NumBytes = alignedNumBytes * m_numSpheres;
        materialBufferDesc.Usage    = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
        materialBufferDesc.HeapType = DENOFIZ_HEAP_TYPE_CPU_GPU;
        if ( isTransparent )
        {
            materialBufferDesc.DebugName = DENOFIZ_STRING( "TransparentMaterialBuffer" );
        }
        else
        {
            materialBufferDesc.DebugName = DENOFIZ_STRING( "OpaqueMaterialBuffer" );
        }
        DenOfIz_LogicalDevice_CreateBuffer( device, &materialBufferDesc, &m_materialBuffer );

        void *mappedData = nullptr;
        DenOfIz_Buffer_MapMemory( m_materialBuffer, &mappedData );
        m_materialMappedData = static_cast<Byte *>( mappedData );

        uint32_t alignedAlphaNumBytes = Align( sizeof( AlphaData ), deviceInfo.Constants.ConstantBufferAlignment );
        if ( isTransparent )
        {
            DenOfIz_BufferDesc alphaBufferDesc{ };
            alphaBufferDesc.NumBytes  = sizeof( AlphaData ) * m_numSpheres;
            alphaBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
            alphaBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
            alphaBufferDesc.DebugName = DENOFIZ_STRING( "AlphaAnimationBuffer" );
            DenOfIz_LogicalDevice_CreateBuffer( device, &alphaBufferDesc, &m_alphaBuffer );

            void *alphaMappedData = nullptr;
            DenOfIz_Buffer_MapMemory( m_alphaBuffer, &alphaMappedData );
            m_alphaMappedData = static_cast<Byte *>( alphaMappedData );
        }

        m_materialBindGroups.resize( m_numSpheres );
        for ( uint32_t i = 0; i < m_numSpheres; ++i )
        {
            SphereMaterialData materialData{ };
            materialData.color           = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
            materialData.refractionIndex = isTransparent ? 1.5f : 1.0f;
            materialData.fresnelPower    = isTransparent ? 3.0f : 1.0f;
            memcpy( m_materialMappedData + alignedNumBytes * i, &materialData, sizeof( SphereMaterialData ) );

            DenOfIz_BindGroupDesc bindGroupDesc{ };
            bindGroupDesc.Layout = m_bindGroupLayouts[ m_spaceToLayoutIndex[ 1 ] ];
            DenOfIz_LogicalDevice_CreateBindGroup( device, &bindGroupDesc, &m_materialBindGroups[ i ] );

            DenOfIz_BindGroup_BeginUpdate( m_materialBindGroups[ i ] );
            DenOfIz_BindBufferDesc bindBufferDesc{ };
            bindBufferDesc.Binding        = 0;
            bindBufferDesc.Resource       = m_materialBuffer;
            bindBufferDesc.ResourceOffset = alignedNumBytes * i;
            DenOfIz_BindGroup_CbvWithDesc( m_materialBindGroups[ i ], &bindBufferDesc );

            if ( isTransparent )
            {
                AlphaData alphaData{ };
                alphaData.alphaValue = 0.5f;
                memcpy( m_alphaMappedData + alignedAlphaNumBytes * i, &alphaData, sizeof( AlphaData ) );

                DenOfIz_BindBufferDesc bindAlphaBufferDesc{ };
                bindAlphaBufferDesc.Binding        = 1;
                bindAlphaBufferDesc.Resource       = m_alphaBuffer;
                bindAlphaBufferDesc.ResourceOffset = alignedAlphaNumBytes * i;
                DenOfIz_BindGroup_CbvWithDesc( m_materialBindGroups[ i ], &bindAlphaBufferDesc );
            }

            DenOfIz_BindGroup_EndUpdate( m_materialBindGroups[ i ] );
        }
    }
}

ColoredSpherePipeline::~ColoredSpherePipeline( )
{
    if ( m_viewProjMappedData )
    {
        DenOfIz_Buffer_UnmapMemory( m_viewProjBuffer );
    }

    if ( m_modelMappedData )
    {
        DenOfIz_Buffer_UnmapMemory( m_modelBuffer );
    }

    if ( m_materialMappedData )
    {
        DenOfIz_Buffer_UnmapMemory( m_materialBuffer );
    }

    if ( m_alphaMappedData )
    {
        DenOfIz_Buffer_UnmapMemory( m_alphaBuffer );
    }

    for ( auto &bindGroup : m_modelBindGroups )
    {
        DenOfIz_BindGroup_Destroy( bindGroup );
    }
    for ( auto &bindGroup : m_materialBindGroups )
    {
        DenOfIz_BindGroup_Destroy( bindGroup );
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_viewProjBindGroup ) )
    {
        DenOfIz_BindGroup_Destroy( m_viewProjBindGroup );
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_viewProjBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_viewProjBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_modelBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_modelBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_materialBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_materialBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_alphaBuffer ) )
    {
        DenOfIz_Buffer_Destroy( m_alphaBuffer );
    }

    if ( DENOFIZ_HANDLE_IS_VALID( m_pipeline ) )
    {
        DenOfIz_Pipeline_Destroy( m_pipeline );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_rootSignature ) )
    {
        DenOfIz_RootSignature_Destroy( m_rootSignature );
    }
    for ( auto &layout : m_bindGroupLayouts )
    {
        if ( DENOFIZ_HANDLE_IS_VALID( layout ) )
        {
            DenOfIz_BindGroupLayout_Destroy( layout );
        }
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_inputLayout ) )
    {
        DenOfIz_InputLayout_Destroy( m_inputLayout );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_program ) )
    {
        DenOfIz_ShaderProgram_Destroy( m_program );
    }
}

void ColoredSpherePipeline::UpdateViewProjection( const FreeCamera *camera ) const
{
    XMStoreFloat4x4( &m_viewProjMappedData->viewProjection, camera->ViewProjectionMatrix( ) );
}

void ColoredSpherePipeline::UpdateModel( const uint32_t sphereIndex, const XMFLOAT4X4 &modelMatrix ) const
{
    DenOfIz_PhysicalDevice deviceInfo{ };
    DenOfIz_LogicalDevice_DeviceInfo( m_device, &deviceInfo );
    const uint32_t alignedNumBytes = Align( sizeof( ModelMatrixData ), deviceInfo.Constants.ConstantBufferAlignment );
    memcpy( m_modelMappedData + alignedNumBytes * sphereIndex, &modelMatrix, sizeof( XMFLOAT4X4 ) );
}

void ColoredSpherePipeline::UpdateMaterialColor( const uint32_t sphereIndex, const XMFLOAT4 &color ) const
{
    SphereMaterialData materialData{ };
    materialData.color = color;
    if ( m_isTransparent )
    {
        materialData.refractionIndex = 1.5f;
        materialData.fresnelPower    = 3.0f;
    }
    else
    {
        materialData.refractionIndex = 1.0f;
        materialData.fresnelPower    = 1.0f;
    }

    DenOfIz_PhysicalDevice deviceInfo{ };
    DenOfIz_LogicalDevice_DeviceInfo( m_device, &deviceInfo );
    const uint32_t alignedNumBytes = Align( sizeof( SphereMaterialData ), deviceInfo.Constants.ConstantBufferAlignment );
    memcpy( m_materialMappedData + alignedNumBytes * sphereIndex, &materialData, sizeof( SphereMaterialData ) );
}

void ColoredSpherePipeline::UpdateAlphaValue( const uint32_t sphereIndex, const float alphaValue ) const
{
    if ( m_isTransparent && m_alphaMappedData )
    {
        memcpy( m_alphaMappedData + sizeof( AlphaData ) * sphereIndex, &alphaValue, sizeof( AlphaData ) );
    }
}

void ColoredSpherePipeline::Render( const uint32_t sphereIndex, DenOfIz_CommandList commandList, const AssetData *assetData ) const
{
    DenOfIz_CommandList_BindPipeline( commandList, m_pipeline );
    DenOfIz_CommandList_BindGroup( commandList, m_viewProjBindGroup );
    DenOfIz_CommandList_BindGroup( commandList, m_modelBindGroups[ sphereIndex ] );
    DenOfIz_CommandList_BindGroup( commandList, m_materialBindGroups[ sphereIndex ] );

    DenOfIz_CommandList_BindVertexBuffer( commandList, assetData->VertexBuffer( ), 0, 0, 0 );
    DenOfIz_CommandList_BindIndexBuffer( commandList, assetData->IndexBuffer( ), DENOFIZ_INDEX_TYPE_UINT32, 0 );
    DenOfIz_CommandList_DrawIndexed( commandList, assetData->NumIndices( ), 1, 0, 0, 0 );
}
