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

#include "gtest/gtest.h"

#include "../../../../Internal/DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "../../TestComparators.h"
#include "DenOfIzGraphics/Assets/Serde/Material/MaterialAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Material/MaterialAssetReader.h"
#include "DenOfIzGraphics/Assets/Serde/Material/MaterialAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"

using namespace DenOfIz;

class MaterialAssetSerdeTest : public testing::Test
{
    std::unique_ptr<MaterialAsset> m_asset;

protected:
    MaterialAsset *CreateSampleMaterialAsset( )
    {
        using namespace DenOfIz;
        m_asset           = std::make_unique<MaterialAsset>( );
        m_asset->Name     = "TestMaterial";
        m_asset->Uri      = AssetUri::Create( "test/TestMaterial.dzmat" );
        m_asset->ShaderRef = "shaders/PBR.hlsl";

        m_asset->AlbedoMapRef            = AssetUri::Create( "textures/albedo.dztex" );
        m_asset->NormalMapRef            = AssetUri::Create( "textures/normal.dztex" );
        m_asset->MetallicRoughnessMapRef = AssetUri::Create( "textures/metalRough.dztex" );
        m_asset->EmissiveMapRef          = AssetUri::Create( "textures/emissive.dztex" );
        m_asset->OcclusionMapRef         = AssetUri::Create( "textures/occlusion.dztex" );

        m_asset->BaseColorFactor = { 0.9f, 0.8f, 0.7f, 1.0f };
        m_asset->MetallicFactor  = 0.7f;
        m_asset->RoughnessFactor = 0.3f;
        m_asset->EmissiveFactor  = { 0.1f, 0.2f, 0.3f };

        m_asset->AlphaBlend  = true;
        m_asset->DoubleSided = false;

        m_asset->_Arena.EnsureCapacity( 1024 );
        DZArenaArrayHelper<UserPropertyArray, UserProperty>::AllocateAndConstructArray( m_asset->_Arena, m_asset->Properties, 2 );

        UserProperty &textureScaleProp = m_asset->Properties.Elements[ 0 ];
        textureScaleProp.Name         = "TextureScale";
        textureScaleProp.PropertyType = UserProperty::Type::Float2;
        textureScaleProp.Vector2Value = { 2.0f, 2.0f };

        UserProperty &glossinessProp = m_asset->Properties.Elements[ 1 ];
        glossinessProp.Name         = "UseGlossiness";
        glossinessProp.PropertyType = UserProperty::Type::Bool;
        glossinessProp.BoolValue    = true;

        return m_asset.get( );
    }
};

TEST_F( MaterialAssetSerdeTest, WriteAndReadBack )
{
    using namespace DenOfIz;

    BinaryContainer container;
    auto            sampleAsset = std::unique_ptr<MaterialAsset>( CreateSampleMaterialAsset( ) );

    {
        BinaryWriter        binaryWriter( container );
        MaterialAssetWriter writer( MaterialAssetWriterDesc{ &binaryWriter } );
        writer.Write( *sampleAsset );
    }
    BinaryReader        reader( container );
    MaterialAssetReader materialReader( MaterialAssetReaderDesc{ &reader } );
    auto                readAsset = std::unique_ptr<MaterialAsset>( materialReader.Read( ) );

    ASSERT_EQ( readAsset->Magic, MaterialAsset{ }.Magic );
    ASSERT_EQ( readAsset->Version, MaterialAsset::Latest );
    ASSERT_STREQ( readAsset->Name.Get( ), sampleAsset->Name.Get( ) );
    ASSERT_STREQ( readAsset->Uri.ToInteropString( ).Get( ), sampleAsset->Uri.ToInteropString( ).Get( ) );
    ASSERT_STREQ( readAsset->ShaderRef.Get( ), sampleAsset->ShaderRef.Get( ) );

    ASSERT_STREQ( readAsset->AlbedoMapRef.ToInteropString( ).Get( ), sampleAsset->AlbedoMapRef.ToInteropString( ).Get( ) );
    ASSERT_STREQ( readAsset->NormalMapRef.ToInteropString( ).Get( ), sampleAsset->NormalMapRef.ToInteropString( ).Get( ) );
    ASSERT_STREQ( readAsset->MetallicRoughnessMapRef.ToInteropString( ).Get( ), sampleAsset->MetallicRoughnessMapRef.ToInteropString( ).Get( ) );
    ASSERT_STREQ( readAsset->EmissiveMapRef.ToInteropString( ).Get( ), sampleAsset->EmissiveMapRef.ToInteropString( ).Get( ) );
    ASSERT_STREQ( readAsset->OcclusionMapRef.ToInteropString( ).Get( ), sampleAsset->OcclusionMapRef.ToInteropString( ).Get( ) );

    ASSERT_TRUE( Float4Equals( readAsset->BaseColorFactor, sampleAsset->BaseColorFactor ) );
    ASSERT_FLOAT_EQ( readAsset->MetallicFactor, sampleAsset->MetallicFactor );
    ASSERT_FLOAT_EQ( readAsset->RoughnessFactor, sampleAsset->RoughnessFactor );
    ASSERT_TRUE( Vector3Equal( readAsset->EmissiveFactor, sampleAsset->EmissiveFactor ) );

    ASSERT_EQ( readAsset->AlphaBlend, sampleAsset->AlphaBlend );
    ASSERT_EQ( readAsset->DoubleSided, sampleAsset->DoubleSided );

    ASSERT_EQ( readAsset->Properties.NumElements, sampleAsset->Properties.NumElements );

    ASSERT_GE( readAsset->Properties.NumElements, 1 );
    ASSERT_GE( sampleAsset->Properties.NumElements, 1 );
    const UserProperty &readProp1   = readAsset->Properties.Elements[ 0 ];
    const UserProperty &sampleProp1 = sampleAsset->Properties.Elements[ 0 ];

    ASSERT_EQ( readProp1.PropertyType, sampleProp1.PropertyType );
    ASSERT_STREQ( readProp1.Name.Get( ), sampleProp1.Name.Get( ) );
    ASSERT_FLOAT_EQ( readProp1.Vector2Value.X, sampleProp1.Vector2Value.X );
    ASSERT_FLOAT_EQ( readProp1.Vector2Value.Y, sampleProp1.Vector2Value.Y );

    ASSERT_GE( readAsset->Properties.NumElements, 2 );
    ASSERT_GE( sampleAsset->Properties.NumElements, 2 );
    const UserProperty &readProp2   = readAsset->Properties.Elements[ 1 ];
    const UserProperty &sampleProp2 = sampleAsset->Properties.Elements[ 1 ];

    ASSERT_EQ( readProp2.PropertyType, sampleProp2.PropertyType );
    ASSERT_STREQ( readProp2.Name.Get( ), sampleProp2.Name.Get( ) );
    ASSERT_EQ( readProp2.BoolValue, sampleProp2.BoolValue );
}
