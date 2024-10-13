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

#include <DenOfIzExamples/PerDrawBinding.h>
#include <DirectXMath.h>

using namespace DenOfIz;
using namespace DirectX;

const uint32_t PerDrawBinding::RegisterSpace = DZConfiguration::Instance( ).RootLevelBufferRegisterSpace;

PerDrawBinding::PerDrawBinding( ILogicalDevice *device, IRootSignature *rootSignature )
{
    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RegisterSpace = RegisterSpace;
    bindGroupDesc.RootSignature = rootSignature;

    m_bindGroup = std::unique_ptr<IResourceBindGroup>( device->CreateResourceBindGroup( bindGroupDesc ) );

    BufferDesc modelBufferDesc{ };
    modelBufferDesc.HeapType   = HeapType::CPU_GPU;
    modelBufferDesc.Descriptor = ResourceDescriptor::UniformBuffer;
    modelBufferDesc.NumBytes   = sizeof( XMFLOAT4X4 );
    modelBufferDesc.DebugName  = "modelMatrixBuffer";
    m_modelMatrixBuffer        = std::unique_ptr<IBufferResource>( device->CreateBufferResource( modelBufferDesc ) );
    m_modelMatrixMappedData    = m_modelMatrixBuffer->MapMemory( );

    m_bindGroup->Update( UpdateDesc( RegisterSpace ).Cbv( 0, m_modelMatrixBuffer.get( ) ) );
}

void PerDrawBinding::Update( const XMFLOAT4X4 &modelMatrix ) const
{
    memcpy( m_modelMatrixMappedData, &modelMatrix, sizeof( XMFLOAT4X4 ) );
}

IResourceBindGroup *PerDrawBinding::BindGroup( ) const
{
    return m_bindGroup.get( );
}

PerDrawBinding::~PerDrawBinding( )
{
    m_modelMatrixBuffer->UnmapMemory( );
}
