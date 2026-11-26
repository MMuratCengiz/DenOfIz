/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzExamples/PerFrameBinding.h"

using namespace DenOfIz;
using namespace DirectX;

PerFrameBinding::PerFrameBinding( DenOfIz_LogicalDevice device, DenOfIz_BindGroupLayout layout )
{
    DenOfIz_BindGroupDesc bindGroupDesc{ };
    bindGroupDesc.Layout = layout;

    DenOfIz_LogicalDevice_CreateBindGroup( device, &bindGroupDesc, &m_bindGroup );

    DenOfIz_BufferDesc deltaTimeBufferDesc{ };
    deltaTimeBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    deltaTimeBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    deltaTimeBufferDesc.NumBytes  = sizeof( float );
    deltaTimeBufferDesc.DebugName = DENOFIZ_STRING( "deltaTimeBuffer" );
    constexpr float timePassed    = 1.0f;
    DenOfIz_LogicalDevice_CreateBuffer( device, &deltaTimeBufferDesc, &m_deltaTimeBuffer );
    DenOfIz_Buffer_MapMemory( m_deltaTimeBuffer, &m_deltaTimeMappedData );
    memcpy( m_deltaTimeMappedData, &timePassed, sizeof( float ) );

    DenOfIz_BufferDesc viewProjectionBufferDesc{ };
    viewProjectionBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    viewProjectionBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    viewProjectionBufferDesc.NumBytes  = sizeof( XMFLOAT4X4 );
    viewProjectionBufferDesc.DebugName = DENOFIZ_STRING( "viewProjectionBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( device, &viewProjectionBufferDesc, &m_viewProjectionBuffer );
    DenOfIz_Buffer_MapMemory( m_viewProjectionBuffer, &m_viewProjectionMappedData );
    XMFLOAT4X4 viewProjectionMatrix{ };
    XMStoreFloat4x4( &viewProjectionMatrix, XMMatrixIdentity( ) );
    memcpy( m_viewProjectionMappedData, &viewProjectionMatrix, sizeof( XMFLOAT4X4 ) );

    DenOfIz_BindGroup_BeginUpdate( m_bindGroup );
    DenOfIz_BindGroup_Cbv( m_bindGroup, 0, m_viewProjectionBuffer );
    DenOfIz_BindGroup_Cbv( m_bindGroup, 1, m_deltaTimeBuffer );
    DenOfIz_BindGroup_EndUpdate( m_bindGroup );
}

void PerFrameBinding::Update( const FreeCamera *camera, float deltaTime ) const
{
    constexpr float deltaTimeTemp = 1.0f;
    memcpy( m_deltaTimeMappedData, &deltaTimeTemp, sizeof( float ) );
    XMFLOAT4X4 viewProjectionMatrix{ };
    XMStoreFloat4x4( &viewProjectionMatrix, camera->ViewProjectionMatrix( ) );
    memcpy( m_viewProjectionMappedData, &viewProjectionMatrix, sizeof( XMFLOAT4X4 ) );
}

DenOfIz_BindGroup PerFrameBinding::BindGroup( ) const
{
    return m_bindGroup;
}

PerFrameBinding::~PerFrameBinding( )
{
    if ( DENOFIZ_HANDLE_IS_VALID( m_deltaTimeBuffer ) )
    {
        DenOfIz_Buffer_UnmapMemory( m_deltaTimeBuffer );
        DenOfIz_Buffer_Destroy( m_deltaTimeBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_viewProjectionBuffer ) )
    {
        DenOfIz_Buffer_UnmapMemory( m_viewProjectionBuffer );
        DenOfIz_Buffer_Destroy( m_viewProjectionBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_bindGroup ) )
    {
        DenOfIz_BindGroup_Destroy( m_bindGroup );
    }
}
