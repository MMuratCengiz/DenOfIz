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

#include <DenOfIzExamples/PerFrameBinding.h>

using namespace DenOfIz;
using namespace DirectX;

PerFrameBinding::PerFrameBinding( ILogicalDevice *device, IRootSignature *rootSignature )
{
    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RegisterSpace = RegisterSpace;
    bindGroupDesc.RootSignature = rootSignature;

    m_bindGroup = device->CreateResourceBindGroup( bindGroupDesc );

    BufferDesc deltaTimeBufferDesc{ };
    deltaTimeBufferDesc.HeapType   = HeapType::CPU_GPU;
    deltaTimeBufferDesc.Descriptor = ResourceDescriptor::UniformBuffer;
    deltaTimeBufferDesc.NumBytes   = sizeof( float );
    deltaTimeBufferDesc.DebugName  = "deltaTimeBuffer";
    float timePassed               = 1.0f;
    m_deltaTimeBuffer              = device->CreateBufferResource( deltaTimeBufferDesc );
    m_deltaTimeMappedData          = m_deltaTimeBuffer->MapMemory( );
    memcpy( m_deltaTimeMappedData, &timePassed, sizeof( float ) );

    BufferDesc viewProjectionBufferDesc{ };
    deltaTimeBufferDesc.HeapType   = HeapType::CPU_GPU;
    deltaTimeBufferDesc.Descriptor = ResourceDescriptor::UniformBuffer;
    deltaTimeBufferDesc.NumBytes   = sizeof( XMFLOAT4X4 );
    deltaTimeBufferDesc.DebugName  = "viewProjectionBuffer";
    m_viewProjectionBuffer         = device->CreateBufferResource( deltaTimeBufferDesc );
    m_viewProjectionMappedData     = m_viewProjectionBuffer->MapMemory( );
    XMFLOAT4X4 viewProjectionMatrix;
    XMStoreFloat4x4( &viewProjectionMatrix, XMMatrixIdentity( ) );
    memcpy( m_viewProjectionMappedData, &viewProjectionMatrix, sizeof( XMFLOAT4X4 ) );

    m_bindGroup->Update( UpdateDesc( 0 ).Cbv( 0, m_viewProjectionBuffer.get( ) ).Cbv( 1, m_deltaTimeBuffer.get( ) ) );
}

void PerFrameBinding::Update( Camera *camera, float deltaTime )
{
    float deltaTimeTemp = 1.0f; // deltaTime is not quite used yet, maybe not even required
    memcpy( m_deltaTimeMappedData, &deltaTimeTemp, sizeof( float ) );
    XMFLOAT4X4 viewProjectionMatrix;
    XMStoreFloat4x4( &viewProjectionMatrix, camera->ViewProjectionMatrix( ) );
    memcpy( m_viewProjectionMappedData, &viewProjectionMatrix, sizeof( XMFLOAT4X4 ) );
}

IResourceBindGroup *PerFrameBinding::BindGroup( ) const
{
    return m_bindGroup.get( );
}

PerFrameBinding::~PerFrameBinding( )
{
    m_deltaTimeBuffer->UnmapMemory( );
    m_viewProjectionBuffer->UnmapMemory( );
}
