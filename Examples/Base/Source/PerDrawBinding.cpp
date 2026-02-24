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

#include "DenOfIzExamples/PerDrawBinding.h"
#include <DirectXMath.h>

using namespace DenOfIz;
using namespace DirectX;

PerDrawBinding::PerDrawBinding( DenOfIz_LogicalDevice device, DenOfIz_BindGroupLayout layout )
{
    DenOfIz_BindGroupDesc bindGroupDesc{ };
    bindGroupDesc.Layout = layout;

    DenOfIz_LogicalDevice_CreateBindGroup( device, &bindGroupDesc, &m_bindGroup );

    DenOfIz_BufferDesc modelBufferDesc{ };
    modelBufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    modelBufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_UNIFORM_BIT;
    modelBufferDesc.NumBytes  = sizeof( XMFLOAT4X4 );
    modelBufferDesc.DebugName = DENOFIZ_STRING( "modelMatrixBuffer" );
    DenOfIz_LogicalDevice_CreateBuffer( device, &modelBufferDesc, &m_modelMatrixBuffer );
    DenOfIz_Buffer_MapMemory( m_modelMatrixBuffer, &m_modelMatrixMappedData );

    DenOfIz_BindGroup_BeginUpdate( m_bindGroup );
    DenOfIz_BindGroup_Cbv( m_bindGroup, 0, m_modelMatrixBuffer );
    DenOfIz_BindGroup_EndUpdate( m_bindGroup );
}

void PerDrawBinding::Update( const XMFLOAT4X4 &modelMatrix ) const
{
    memcpy( m_modelMatrixMappedData, &modelMatrix, sizeof( XMFLOAT4X4 ) );
}

DenOfIz_BindGroup PerDrawBinding::BindGroup( ) const
{
    return m_bindGroup;
}

PerDrawBinding::~PerDrawBinding( )
{
    if ( DENOFIZ_HANDLE_IS_VALID( m_modelMatrixBuffer ) )
    {
        DenOfIz_Buffer_UnmapMemory( m_modelMatrixBuffer );
        DenOfIz_Buffer_Destroy( m_modelMatrixBuffer );
    }
    if ( DENOFIZ_HANDLE_IS_VALID( m_bindGroup ) )
    {
        DenOfIz_BindGroup_Destroy( m_bindGroup );
    }
}
