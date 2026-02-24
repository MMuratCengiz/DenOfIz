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

#include "DenOfIzExamples/PerMaterialBinding.h"

using namespace DenOfIz;

PerMaterialBinding::PerMaterialBinding( DenOfIz_LogicalDevice device, DenOfIz_BindGroupLayout layout )
{
    m_nullTexture = std::make_unique<NullTexture>( device );

    DenOfIz_BindGroupDesc bindGroupDesc{ };
    bindGroupDesc.Layout = layout;

    DenOfIz_LogicalDevice_CreateBindGroup( device, &bindGroupDesc, &m_bindGroup );
}

PerMaterialBinding::~PerMaterialBinding( )
{
    if ( DENOFIZ_HANDLE_IS_VALID( m_bindGroup ) )
    {
        DenOfIz_BindGroup_Destroy( m_bindGroup );
    }
}

void PerMaterialBinding::Update( const MaterialData *materialData ) const
{
    DenOfIz_BindGroup_BeginUpdate( m_bindGroup );
    DenOfIz_BindGroup_Sampler( m_bindGroup, 0, materialData->Sampler( ) );
    DenOfIz_BindGroup_SrvTexture( m_bindGroup, 0, OrNull( materialData->AlbedoTexture( ) ) );
    DenOfIz_BindGroup_SrvTexture( m_bindGroup, 1, OrNull( materialData->NormalTexture( ) ) );
    DenOfIz_BindGroup_SrvTexture( m_bindGroup, 2, OrNull( materialData->HeightTexture( ) ) );
    DenOfIz_BindGroup_SrvTexture( m_bindGroup, 3, OrNull( materialData->RoughnessTexture( ) ) );
    DenOfIz_BindGroup_SrvTexture( m_bindGroup, 4, OrNull( materialData->AoTexture( ) ) );
    DenOfIz_BindGroup_EndUpdate( m_bindGroup );
}

DenOfIz_BindGroup PerMaterialBinding::BindGroup( ) const
{
    return m_bindGroup;
}

DenOfIz_Texture PerMaterialBinding::OrNull( DenOfIz_Texture texture ) const
{
    if ( DENOFIZ_HANDLE_IS_VALID( texture ) )
    {
        return texture;
    }
    return m_nullTexture->Texture( );
}
