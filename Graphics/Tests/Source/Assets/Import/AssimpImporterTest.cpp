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

#include <DenOfIzGraphics/Assets/Bundle/BundleManager.h>
#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Assets/Import/AssimpImporter.h>
#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Material/MaterialAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetReader.h>
#include <DenOfIzGraphics/Assets/Serde/Texture/TextureAssetReader.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryReader.h>
#include <filesystem>
#include <fstream>
#include "gtest/gtest.h"

using namespace DenOfIz;

const std::string TEST_OUTPUT_DIR   = DZ_TEST_DATA_DEST_DIR;
const std::string TEST_RESOURCE_DIR = DZ_TEST_DATA_SRC_DIR;

class AssimpImporterTest : public testing::Test
{
protected:
    AssimpImporter    *importer = nullptr;
    AssimpImporterDesc importerDesc;
    // BundleManager* bundleManager = nullptr;

    void SetUp( ) override
    {
        std::filesystem::create_directory( TEST_OUTPUT_DIR );
        importer = new AssimpImporter( importerDesc );
    }

    void TearDown( ) override
    {
        delete importer;
        importer = nullptr;
        try
        {
            if ( std::filesystem::exists( TEST_OUTPUT_DIR ) )
            {
                std::filesystem::remove_all( TEST_OUTPUT_DIR );
            }
        }
        catch ( const std::filesystem::filesystem_error &e )
        {
            std::cerr << "Error removing test directory: " << e.what( ) << std::endl;
        }
    }

    static std::string CreateDummyFile( const std::string &filename, const std::string &content = "dummy content" )
    {
        const std::string fullPath = TEST_OUTPUT_DIR + "/" + filename;
        std::ofstream     outFile( fullPath );
        if ( outFile )
        {
            outFile << content;
            outFile.close( );
            return fullPath;
        }
        throw std::runtime_error( "Failed to create dummy file: " + fullPath );
    }

    static AssetUri FindAssetUriByType( const ImporterResult &result, const std::string &typeSuffix )
    {
        for ( size_t i = 0; i < result.CreatedAssets.NumElements( ); ++i )
        {
            if ( const AssetUri &uri = result.CreatedAssets.GetElement( i ); std::string( uri.Path.Get( ) ).ends_with( typeSuffix ) )
            {
                return uri;
            }
        }
        return AssetUri{ };
    }
};

TEST_F( AssimpImporterTest, GetImporterInfoReturnsCorrectName )
{
    ASSERT_NE( importer, nullptr );
    const ImporterDesc info = importer->GetImporterInfo( );
    ASSERT_STREQ( info.Name.Get( ), "Assimp Importer" );
}

TEST_F( AssimpImporterTest, GetImporterInfoReturnsSupportedExtensions )
{
    ASSERT_NE( importer, nullptr );
    const ImporterDesc info = importer->GetImporterInfo( );
    ASSERT_GT( info.SupportedExtensions.NumElements( ), 0 );
    bool foundFbx  = false;
    bool foundGltf = false;
    bool foundObj  = false;
    for ( size_t i = 0; i < info.SupportedExtensions.NumElements( ); ++i )
    {
        const InteropString &ext = info.SupportedExtensions.GetElement( i );
        if ( ext.Equals( ".fbx" ) )
        {
            foundFbx = true;
        }
        if ( ext.Equals( ".gltf" ) )
        {
            foundGltf = true;
        }
        if ( ext.Equals( ".obj" ) )
        {
            foundObj = true;
        }
    }
    ASSERT_TRUE( foundFbx );
    ASSERT_TRUE( foundGltf );
    ASSERT_TRUE( foundObj );
}

TEST_F( AssimpImporterTest, CanProcessSupportedExtension )
{
    ASSERT_NE( importer, nullptr );
    ASSERT_TRUE( importer->CanProcessFileExtension( ".fbx" ) );
    ASSERT_TRUE( importer->CanProcessFileExtension( ".GLTF" ) );
    ASSERT_TRUE( importer->CanProcessFileExtension( ".obj" ) );
}

TEST_F( AssimpImporterTest, CannotProcessUnsupportedExtension )
{
    ASSERT_NE( importer, nullptr );
    ASSERT_FALSE( importer->CanProcessFileExtension( ".txt" ) );
    ASSERT_FALSE( importer->CanProcessFileExtension( ".png" ) );
    ASSERT_FALSE( importer->CanProcessFileExtension( ".dzmesh" ) );
}

TEST_F( AssimpImporterTest, ValidateFileSupportedExtension )
{
    ASSERT_NE( importer, nullptr );
    const std::string dummyFbxPath = CreateDummyFile( "dummy.fbx" );
    ASSERT_TRUE( importer->ValidateFile( dummyFbxPath.c_str( ) ) );
}

TEST_F( AssimpImporterTest, ValidateFileUnsupportedExtension )
{
    ASSERT_NE( importer, nullptr );
    const std::string dummyTxtPath = CreateDummyFile( "dummy.txt" );
    ASSERT_FALSE( importer->ValidateFile( dummyTxtPath.c_str( ) ) );
}

TEST_F( AssimpImporterTest, ValidateFileNotFound )
{
    ASSERT_NE( importer, nullptr );
    ASSERT_FALSE( importer->ValidateFile( "non_existent_file.fbx" ) );
}

TEST_F( AssimpImporterTest, ImportFileNotFound )
{
    ASSERT_NE( importer, nullptr );
    ImportJobDesc desc;
    desc.SourceFilePath  = "path/to/non/existent/file.fbx";
    desc.TargetDirectory = TEST_OUTPUT_DIR.c_str( );
    desc.AssetNamePrefix = "test";

    const ImporterResult result = importer->Import( desc );

    ASSERT_EQ( result.ResultCode, ImporterResultCode::FileNotFound );
    ASSERT_FALSE( result.ErrorMessage.IsEmpty( ) );
    ASSERT_EQ( result.CreatedAssets.NumElements( ), 0 );
}

TEST_F( AssimpImporterTest, ImportUnsupportedExtension )
{
    ASSERT_NE( importer, nullptr );
    const std::string dummyFilePath = CreateDummyFile( "test.unsupported" );

    ImportJobDesc desc;
    desc.SourceFilePath  = dummyFilePath.c_str( );
    desc.TargetDirectory = TEST_OUTPUT_DIR.c_str( );
    desc.AssetNamePrefix = "test_unsupported";

    const ImporterResult result = importer->Import( desc );

    ASSERT_EQ( result.ResultCode, ImporterResultCode::ImportFailed );
    ASSERT_FALSE( result.ErrorMessage.IsEmpty( ) );
    ASSERT_EQ( result.CreatedAssets.NumElements( ), 0 );
}

TEST_F( AssimpImporterTest, ImportInvalidFileContent )
{
    ASSERT_NE( importer, nullptr );
    const std::string invalidFbxPath = CreateDummyFile( "invalid_model.fbx", "This is not a valid FBX file." );

    ImportJobDesc desc;
    desc.SourceFilePath  = invalidFbxPath.c_str( );
    desc.TargetDirectory = TEST_OUTPUT_DIR.c_str( );
    desc.AssetNamePrefix = "test_invalid";

    const ImporterResult result = importer->Import( desc );

    ASSERT_EQ( result.ResultCode, ImporterResultCode::ImportFailed );
    ASSERT_FALSE( result.ErrorMessage.IsEmpty( ) ); // Assimp should provide an error
    ASSERT_EQ( result.CreatedAssets.NumElements( ), 0 );
}

TEST_F( AssimpImporterTest, ImportTargetDirectoryNotCreatable )
{
    ASSERT_NE( importer, nullptr );
    const std::string dummyFbxPath = CreateDummyFile( "dummy_for_dir_test.fbx" );

    ImportJobDesc desc;
    desc.SourceFilePath = dummyFbxPath.c_str( );
#ifdef _WIN32
    desc.TargetDirectory = "CON/InvalidDir";
#else
    desc.TargetDirectory = "/proc/InvalidDir";
#endif
    desc.AssetNamePrefix        = "test_dirfail";
    const ImporterResult result = importer->Import( desc );

    ASSERT_EQ( result.ResultCode, ImporterResultCode::WriteFailed );
    ASSERT_FALSE( result.ErrorMessage.IsEmpty( ) );
    ASSERT_TRUE( strstr( result.ErrorMessage.Get( ), "Failed to create target directory" ) != nullptr );
    ASSERT_EQ( result.CreatedAssets.NumElements( ), 0 );
}

TEST_F( AssimpImporterTest, ImportFoxGltf )
{
    ASSERT_NE( importer, nullptr );

    const std::string inputModelPath = TEST_RESOURCE_DIR + "/Models/Fox.gltf";
    if ( !FileIO::FileExists( inputModelPath.c_str( ) ) )
    {
        GTEST_SKIP( ) << "Skipping ImportFoxGltf test, required resource file not found: " << inputModelPath;
    }

    ImportJobDesc desc;
    desc.SourceFilePath  = inputModelPath.c_str( );
    desc.TargetDirectory = TEST_OUTPUT_DIR.c_str( );
    desc.AssetNamePrefix = "Fox"; // Use "Fox" as prefix

    const ImporterResult result = importer->Import( desc );

    ASSERT_EQ( result.ResultCode, ImporterResultCode::Success ) << "Import failed: " << result.ErrorMessage.Get( );
    ASSERT_EQ( result.CreatedAssets.NumElements( ), 7 ) << "Expected 7 assets to be created";

    AssetUri meshUri     = FindAssetUriByType( result, "_Mesh.dzmesh" );
    AssetUri materialUri = FindAssetUriByType( result, "_Material.dzmat" );
    AssetUri textureUri  = FindAssetUriByType( result, "_Texture.dztex" );
    AssetUri skeletonUri = FindAssetUriByType( result, "_Skeleton.dzskel" );

    AssetUri surveyAnimUri = FindAssetUriByType( result, "Survey_Animation.dzanim" );
    AssetUri walkAnimUri   = FindAssetUriByType( result, "Walk_Animation.dzanim" );
    AssetUri runAnimUri    = FindAssetUriByType( result, "Run_Animation.dzanim" );

    ASSERT_FALSE( meshUri.Path.IsEmpty( ) ) << "Mesh asset URI not found in results";
    ASSERT_FALSE( materialUri.Path.IsEmpty( ) ) << "Material asset URI not found in results";
    ASSERT_FALSE( textureUri.Path.IsEmpty( ) ) << "Texture asset URI not found in results";
    ASSERT_FALSE( skeletonUri.Path.IsEmpty( ) ) << "Skeleton asset URI not found in results";
    ASSERT_FALSE( surveyAnimUri.Path.IsEmpty( ) ) << "Survey animation asset URI not found in results";
    ASSERT_FALSE( walkAnimUri.Path.IsEmpty( ) ) << "Walk animation asset URI not found in results";
    ASSERT_FALSE( runAnimUri.Path.IsEmpty( ) ) << "Run animation asset URI not found in results";

    const std::string meshPath       = TEST_OUTPUT_DIR + "/" + meshUri.Path.Get( );
    const std::string materialPath   = TEST_OUTPUT_DIR + "/" + materialUri.Path.Get( );
    const std::string texturePath    = TEST_OUTPUT_DIR + "/" + textureUri.Path.Get( );
    const std::string skeletonPath   = TEST_OUTPUT_DIR + "/" + skeletonUri.Path.Get( );
    const std::string surveyAnimPath = TEST_OUTPUT_DIR + "/" + surveyAnimUri.Path.Get( );
    const std::string walkAnimPath   = TEST_OUTPUT_DIR + "/" + walkAnimUri.Path.Get( );
    const std::string runAnimPath    = TEST_OUTPUT_DIR + "/" + runAnimUri.Path.Get( );

    ASSERT_TRUE( FileIO::FileExists( meshPath.c_str( ) ) ) << "Mesh file not created: " << meshPath;
    ASSERT_TRUE( FileIO::FileExists( materialPath.c_str( ) ) ) << "Material file not created: " << materialPath;
    ASSERT_TRUE( FileIO::FileExists( texturePath.c_str( ) ) ) << "Texture file not created: " << texturePath;
    ASSERT_TRUE( FileIO::FileExists( skeletonPath.c_str( ) ) ) << "Skeleton file not created: " << skeletonPath;
    ASSERT_TRUE( FileIO::FileExists( surveyAnimPath.c_str( ) ) ) << "Survey animation file not created: " << surveyAnimPath;
    ASSERT_TRUE( FileIO::FileExists( walkAnimPath.c_str( ) ) ) << "Walk animation file not created: " << walkAnimPath;
    ASSERT_TRUE( FileIO::FileExists( runAnimPath.c_str( ) ) ) << "Run animation file not created: " << runAnimPath;

    BinaryReader    meshFileReader( meshPath.c_str( ) );
    MeshAssetReader meshReader( { &meshFileReader } );
    MeshAsset       readMesh = meshReader.Read( );
    ASSERT_STREQ( readMesh.Name.Get( ), "Fox" ); // Check sanitized name
    ASSERT_EQ( readMesh.SubMeshes.NumElements( ), 1 );
    ASSERT_TRUE( readMesh.SubMeshes.GetElement( 0 ).MaterialRef.Equals( materialUri ) );
    ASSERT_TRUE( readMesh.SkeletonRef.Equals( skeletonUri ) );
    ASSERT_EQ( readMesh.AnimationRefs.NumElements( ), 3 ) << "Expected 3 animation references";
    ASSERT_TRUE( readMesh.AnimationRefs.GetElement( 0 ).Equals( surveyAnimUri ) );
    ASSERT_TRUE( readMesh.AnimationRefs.GetElement( 1 ).Equals( walkAnimUri ) );
    ASSERT_TRUE( readMesh.AnimationRefs.GetElement( 2 ).Equals( runAnimUri ) );

    ASSERT_GT( readMesh.SubMeshes.GetElement( 0 ).NumVertices, 100 ); // Fox has many vertices
    ASSERT_GT( readMesh.SubMeshes.GetElement( 0 ).NumIndices, 100 );
    ASSERT_TRUE( readMesh.EnabledAttributes.Position );
    ASSERT_TRUE( readMesh.EnabledAttributes.Normal );
    ASSERT_TRUE( readMesh.EnabledAttributes.UV );
    ASSERT_TRUE( readMesh.EnabledAttributes.BlendIndices );
    ASSERT_TRUE( readMesh.EnabledAttributes.BlendWeights );

    BinaryReader        matFileReader( materialPath.c_str( ) );
    MaterialAssetReader matReader( { &matFileReader } );
    MaterialAsset       readMat = matReader.Read( );
    ASSERT_STREQ( readMat.Name.Get( ), "fox_material" );
    ASSERT_TRUE( readMat.AlbedoMapRef.Equals( textureUri ) );
    ASSERT_FLOAT_EQ( readMat.MetallicFactor, 0.0f );
    ASSERT_NEAR( readMat.RoughnessFactor, 0.0f, 0.0f );

    BinaryReader       texFileReader( texturePath.c_str( ) );
    TextureAssetReader texReader( { &texFileReader } );
    TextureAsset       readTex = texReader.Read( );
    ASSERT_STREQ( readTex.Name.Get( ), "Texture" );
    ASSERT_TRUE( std::string( readTex.SourcePath.Get( ) ).ends_with( "Texture.png" ) );

    BinaryReader        skelFileReader( skeletonPath.c_str( ) );
    SkeletonAssetReader skelReader( { &skelFileReader } );
    SkeletonAsset       readSkel = skelReader.Read( );
    ASSERT_STREQ( readSkel.Name.Get( ), "Fox" );
    ASSERT_EQ( readSkel.Joints.NumElements( ), 24 );
    bool rootFound = false;
    for ( size_t j = 0; j < readSkel.Joints.NumElements( ); ++j )
    {
        if ( readSkel.Joints.GetElement( j ).ParentIndex == -1 )
        {
            ASSERT_STREQ( readSkel.Joints.GetElement( j ).Name.Get( ), "_rootJoint" );
            rootFound = true;
            break;
        }
    }
    ASSERT_TRUE( rootFound ) << "Root joint not found";

    BinaryReader         surveyAnimReader( surveyAnimPath.c_str( ) );
    AnimationAssetReader surveyReader( { &surveyAnimReader } );
    AnimationAsset       surveyAnim = surveyReader.Read( );
    ASSERT_STREQ( surveyAnim.Name.Get( ), "Survey" );
    ASSERT_TRUE( surveyAnim.SkeletonRef.Equals( skeletonUri ) );
    ASSERT_EQ( surveyAnim.Animations.NumElements( ), 1 ); // One clip per animation

    const AnimationClip &surveyClip = surveyAnim.Animations.GetElement( 0 );
    ASSERT_STREQ( surveyClip.Name.Get( ), "Survey" );
    ASSERT_GT( surveyClip.Duration, 0.0f );
    ASSERT_GT( surveyClip.Tracks.NumElements( ), 10 ); // Expect tracks for multiple joints

    BinaryReader         walkAnimReader( walkAnimPath.c_str( ) );
    AnimationAssetReader walkReader( { &walkAnimReader } );
    AnimationAsset       walkAnim = walkReader.Read( );
    ASSERT_STREQ( walkAnim.Name.Get( ), "Walk" );
    ASSERT_TRUE( walkAnim.SkeletonRef.Equals( skeletonUri ) );
    ASSERT_EQ( walkAnim.Animations.NumElements( ), 1 );

    BinaryReader         runAnimReader( runAnimPath.c_str( ) );
    AnimationAssetReader runReader( { &runAnimReader } );
    AnimationAsset       runAnim = runReader.Read( );
    ASSERT_STREQ( runAnim.Name.Get( ), "Run" );
    ASSERT_TRUE( runAnim.SkeletonRef.Equals( skeletonUri ) );
    ASSERT_EQ( runAnim.Animations.NumElements( ), 1 );

    bool spineTrackFound = false;
    for ( size_t t = 0; t < surveyClip.Tracks.NumElements( ); ++t )
    {
        const JointAnimTrack &track = surveyClip.Tracks.GetElement( t );
        if ( track.JointName.Equals( "b_Spine02_03" ) )
        {
            ASSERT_GT( track.PositionKeys.NumElements( ), 0 );
            ASSERT_GT( track.RotationKeys.NumElements( ), 0 );
            ASSERT_GT( track.ScaleKeys.NumElements( ), 0 );
            ASSERT_NEAR( track.PositionKeys.GetElement( 0 ).Timestamp, 0.0f, 0.01f );
            spineTrackFound = true;
            break;
        }
    }
    ASSERT_TRUE( spineTrackFound ) << "Spine track not found in Survey animation";
}
