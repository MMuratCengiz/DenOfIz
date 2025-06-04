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

#include "DenOfIzGraphics/Assets/Serde/Material/MaterialAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Material/MaterialAssetReader.h"
#include "DenOfIzGraphics/Assets/Serde/Material/MaterialAssetWriter.h"
#include "../../TestComparators.h"

using namespace DenOfIz;

class MaterialAssetSerdeTest : public testing::Test
{
protected:
    static MaterialAsset CreateSampleMaterialAsset( )
    {
        using namespace DenOfIz;
        MaterialAsset asset;
        asset.Name      = "TestMaterial";
        asset.Uri       = AssetUri::Create( "test/TestMaterial.dzmat" );
        asset.ShaderRef = "shaders/PBR.hlsl";

        asset.AlbedoMapRef            = AssetUri::Create( "textures/albedo.dztex" );
        asset.NormalMapRef            = AssetUri::Create( "textures/normal.dztex" );
        asset.MetallicRoughnessMapRef = AssetUri::Create( "textures/metalRough.dztex" );
        asset.EmissiveMapRef          = AssetUri::Create( "textures/emissive.dztex" );
        asset.OcclusionMapRef         = AssetUri::Create( "textures/occlusion.dztex" );

        asset.BaseColorFactor = { 0.9f, 0.8f, 0.7f, 1.0f };
        asset.MetallicFactor  = 0.7f;
        asset.RoughnessFactor = 0.3f;
        asset.EmissiveFactor  = { 0.1f, 0.2f, 0.3f };

        asset.AlphaBlend  = true;
        asset.DoubleSided = false;

        UserProperty textureScaleProp;
        textureScaleProp.Name         = "TextureScale";
        textureScaleProp.PropertyType = UserProperty::Type::Float2;
        textureScaleProp.Vector2Value = { 2.0f, 2.0f };
        asset.Properties.AddElement( textureScaleProp );

        UserProperty glossinessProp;
        glossinessProp.Name         = "UseGlossiness";
        glossinessProp.PropertyType = UserProperty::Type::Bool;
        glossinessProp.BoolValue    = true;
        asset.Properties.AddElement( glossinessProp );

        return asset;
    }
};

TEST_F( MaterialAssetSerdeTest, WriteAndReadBack )
{
    using namespace DenOfIz;

    BinaryContainer container;
    BinaryWriter    binaryWriter( container );
    MaterialAsset   sampleAsset = CreateSampleMaterialAsset( );

    {
        MaterialAssetWriter writer( MaterialAssetWriterDesc{ &binaryWriter } );
        writer.Write( sampleAsset );
    }

    BinaryReader        reader( container );
    MaterialAssetReader materialReader( MaterialAssetReaderDesc{ &reader } );

    MaterialAsset readAsset = materialReader.Read( );

    ASSERT_EQ( readAsset.Magic, MaterialAsset{ }.Magic );
    ASSERT_EQ( readAsset.Version, MaterialAsset::Latest );
    ASSERT_STREQ( readAsset.Name.Get( ), sampleAsset.Name.Get( ) );
    ASSERT_STREQ( readAsset.Uri.ToInteropString( ).Get( ), sampleAsset.Uri.ToInteropString( ).Get( ) );
    ASSERT_STREQ( readAsset.ShaderRef.Get( ), sampleAsset.ShaderRef.Get( ) );

    ASSERT_STREQ( readAsset.AlbedoMapRef.ToInteropString( ).Get( ), sampleAsset.AlbedoMapRef.ToInteropString( ).Get( ) );
    ASSERT_STREQ( readAsset.NormalMapRef.ToInteropString( ).Get( ), sampleAsset.NormalMapRef.ToInteropString( ).Get( ) );
    ASSERT_STREQ( readAsset.MetallicRoughnessMapRef.ToInteropString( ).Get( ), sampleAsset.MetallicRoughnessMapRef.ToInteropString( ).Get( ) );
    ASSERT_STREQ( readAsset.EmissiveMapRef.ToInteropString( ).Get( ), sampleAsset.EmissiveMapRef.ToInteropString( ).Get( ) );
    ASSERT_STREQ( readAsset.OcclusionMapRef.ToInteropString( ).Get( ), sampleAsset.OcclusionMapRef.ToInteropString( ).Get( ) );

    ASSERT_TRUE( Float4Equals( readAsset.BaseColorFactor, sampleAsset.BaseColorFactor ) );
    ASSERT_FLOAT_EQ( readAsset.MetallicFactor, sampleAsset.MetallicFactor );
    ASSERT_FLOAT_EQ( readAsset.RoughnessFactor, sampleAsset.RoughnessFactor );
    ASSERT_TRUE( Vector3Equal( readAsset.EmissiveFactor, sampleAsset.EmissiveFactor ) );

    ASSERT_EQ( readAsset.AlphaBlend, sampleAsset.AlphaBlend );
    ASSERT_EQ( readAsset.DoubleSided, sampleAsset.DoubleSided );

    ASSERT_EQ( readAsset.Properties.NumElements( ), sampleAsset.Properties.NumElements( ) );

    const UserProperty &readProp1   = readAsset.Properties.GetElement( 0 );
    const UserProperty &sampleProp1 = sampleAsset.Properties.GetElement( 0 );

    ASSERT_EQ( readProp1.PropertyType, sampleProp1.PropertyType );
    ASSERT_STREQ( readProp1.Name.Get( ), sampleProp1.Name.Get( ) );
    ASSERT_FLOAT_EQ( readProp1.Vector2Value.X, sampleProp1.Vector2Value.X );
    ASSERT_FLOAT_EQ( readProp1.Vector2Value.Y, sampleProp1.Vector2Value.Y );

    const UserProperty &readProp2   = readAsset.Properties.GetElement( 1 );
    const UserProperty &sampleProp2 = sampleAsset.Properties.GetElement( 1 );

    ASSERT_EQ( readProp2.PropertyType, sampleProp2.PropertyType );
    ASSERT_STREQ( readProp2.Name.Get( ), sampleProp2.Name.Get( ) );
    ASSERT_EQ( readProp2.BoolValue, sampleProp2.BoolValue );
}
