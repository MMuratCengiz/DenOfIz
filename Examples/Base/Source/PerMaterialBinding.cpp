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

PerMaterialBinding::PerMaterialBinding( ILogicalDevice *device, IRootSignature *rootSignature )
{
    m_nullTexture = std::make_unique<NullTexture>( device );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RegisterSpace = RegisterSpace;
    bindGroupDesc.RootSignature = rootSignature;

    m_bindGroup = std::unique_ptr<IResourceBindGroup>( device->CreateResourceBindGroup( bindGroupDesc ) );
}

void PerMaterialBinding::Update( const MaterialData *materialData ) const
{
    m_bindGroup->BeginUpdate( );
    m_bindGroup->Sampler( 0, materialData->Sampler( ) );
    m_bindGroup->Srv( 0, OrNull( materialData->AlbedoTexture( ) ) );
    m_bindGroup->Srv( 1, OrNull( materialData->NormalTexture( ) ) );
    m_bindGroup->Srv( 2, OrNull( materialData->HeightTexture( ) ) );
    //    m_bindGroup->Srv( 3, OrNull( materialData->MetallicTexture( ) ) );
    m_bindGroup->Srv( 3, OrNull( materialData->RoughnessTexture( ) ) );
    m_bindGroup->Srv( 4, OrNull( materialData->AoTexture( ) ) );
    m_bindGroup->EndUpdate( );
}

IResourceBindGroup *PerMaterialBinding::BindGroup( ) const
{
    return m_bindGroup.get( );
}

ITextureResource *PerMaterialBinding::OrNull( ITextureResource *texture ) const
{
    if ( texture )
    {
        return texture;
    }
    return m_nullTexture->Texture( );
}
