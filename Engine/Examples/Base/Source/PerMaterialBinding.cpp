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

#include <DenOfIzExamples/PerMaterialBinding.h>

using namespace DenOfIz;
using namespace DirectX;

PerMaterialBinding::PerMaterialBinding( ILogicalDevice *device, IRootSignature *rootSignature )
{
    m_nullTexture = std::make_unique<NullTexture>( device );

    ResourceBindGroupDesc bindGroupDesc{ };
    bindGroupDesc.RegisterSpace = RegisterSpace;
    bindGroupDesc.NumSamplers   = 1;
    bindGroupDesc.NumTextures   = 1; // TODO: 5
    bindGroupDesc.RootSignature = rootSignature;

    m_bindGroup = device->CreateResourceBindGroup( bindGroupDesc );
}

void PerMaterialBinding::Update( MaterialData *materialData )
{
    m_bindGroup->Update( UpdateDesc( RegisterSpace )
                             .Sampler( 0, materialData->Sampler( ) )
                             .Srv( 0, OrNull( materialData->AlbedoTexture( ) ) ) );
//                             TODO:
//                             .Srv( 1, OrNull( materialData->NormalTexture( ) ) )
//                             .Srv( 2, OrNull( materialData->HeightTexture( ) ) )
//                             .Srv( 3, OrNull( materialData->MetallicTexture( ) ) )
//                             .Srv( 4, OrNull( materialData->RoughnessTexture( ) ) )
//                             .Srv( 5, OrNull( materialData->AoTexture( ) ) ) );
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
