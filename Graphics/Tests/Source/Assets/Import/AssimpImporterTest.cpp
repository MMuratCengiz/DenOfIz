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

#include <DenOfIzGraphics/Assets/Bundle/BundleManager.h> // If testing bundling
#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Assets/Import/AssimpImporter.h>
#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetReader.h> // For later tests
#include <DenOfIzGraphics/Assets/Serde/Material/MaterialAssetReader.h>   // For later tests
#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetReader.h>           // For later tests
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetReader.h>   // For later tests
#include <DenOfIzGraphics/Assets/Serde/Texture/TextureAssetReader.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryReader.h> // For later tests
#include <filesystem>                                   // Requires C++17
#include <fstream>                                      // For creating dummy files
#include "gtest/gtest.h"

using namespace DenOfIz;

const std::string TEST_OUTPUT_DIR = DZ_TEST_DATA_DEST_DIR;
// Define where test resource files (like Fox.gltf) are located relative to executable
const std::string TEST_RESOURCE_DIR = DZ_TEST_DATA_SRC_DIR; // Adjust as needed

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

    // Helper to find asset URI by type suffix (e.g., "_Mesh.dzmesh")
    static AssetUri FindAssetUriByType( const ImporterResult &result, const std::string &typeSuffix )
    {
        for ( size_t i = 0; i < result.CreatedAssets.NumElements( ); ++i )
        {
            if ( const AssetUri &uri = result.CreatedAssets.GetElement( i ); std::string( uri.Path.Get( ) ).ends_with( typeSuffix ) )
            {
                return uri;
            }
        }
        return AssetUri{ }; // Return empty/invalid URI if not found
    }
};

// --- Basic Initialization and Info Tests ---

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
    // Check for a few common extensions
    bool foundFbx  = false;
    bool foundGltf = false;
    bool foundObj  = false;
    for ( size_t i = 0; i < info.SupportedExtensions.NumElements( ); ++i )
    {
        const InteropString &ext = info.SupportedExtensions.GetElement( i );
        if ( ext.Equals( ".fbx" ) )
            foundFbx = true;
        if ( ext.Equals( ".gltf" ) )
            foundGltf = true;
        if ( ext.Equals( ".obj" ) )
            foundObj = true;
    }
    ASSERT_TRUE( foundFbx );
    ASSERT_TRUE( foundGltf );
    ASSERT_TRUE( foundObj );
}

TEST_F( AssimpImporterTest, CanProcessSupportedExtension )
{
    ASSERT_NE( importer, nullptr );
    ASSERT_TRUE( importer->CanProcessFileExtension( ".fbx" ) );
    ASSERT_TRUE( importer->CanProcessFileExtension( ".GLTF" ) ); // Case-insensitive check
    ASSERT_TRUE( importer->CanProcessFileExtension( ".obj" ) );
}

TEST_F( AssimpImporterTest, CannotProcessUnsupportedExtension )
{
    ASSERT_NE( importer, nullptr );
    ASSERT_FALSE( importer->CanProcessFileExtension( ".txt" ) );
    ASSERT_FALSE( importer->CanProcessFileExtension( ".png" ) );
    ASSERT_FALSE( importer->CanProcessFileExtension( ".dzmesh" ) ); // Shouldn't process its own output
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

// --- Error Handling Tests ---

TEST_F( AssimpImporterTest, ImportFileNotFound )
{
    ASSERT_NE( importer, nullptr );
    ImportJobDesc desc;
    desc.SourceFilePath  = "path/to/non/existent/file.fbx";
    desc.TargetDirectory = TEST_OUTPUT_DIR.c_str( );
    desc.AssetNamePrefix = "test";
    // desc.Options can be default

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

    // Assimp's ReadFile might return fail, or IsExtensionSupported might catch it earlier.
    // The importer might not explicitly check CanProcessFileExtension inside Import.
    // We expect Assimp itself to fail.
    ASSERT_EQ( result.ResultCode, ImporterResultCode::ImportFailed );
    ASSERT_FALSE( result.ErrorMessage.IsEmpty( ) ); // Assimp should provide an error
    ASSERT_EQ( result.CreatedAssets.NumElements( ), 0 );
}

TEST_F( AssimpImporterTest, ImportInvalidFileContent )
{
    ASSERT_NE( importer, nullptr );
    // Create a text file but give it a supported extension
    const std::string invalidFbxPath = CreateDummyFile( "invalid_model.fbx", "This is not a valid FBX file." );

    ImportJobDesc desc;
    desc.SourceFilePath  = invalidFbxPath.c_str( );
    desc.TargetDirectory = TEST_OUTPUT_DIR.c_str( );
    desc.AssetNamePrefix = "test_invalid";

    const ImporterResult result = importer->Import( desc );

    // Assimp should fail to parse this file
    ASSERT_EQ( result.ResultCode, ImporterResultCode::ImportFailed );
    ASSERT_FALSE( result.ErrorMessage.IsEmpty( ) ); // Assimp should provide an error
    ASSERT_EQ( result.CreatedAssets.NumElements( ), 0 );
}

TEST_F( AssimpImporterTest, ImportTargetDirectoryNotCreatable )
{
    ASSERT_NE( importer, nullptr );
    // On some systems, creating a directory with an invalid name might fail.
    // Or, simulate lack of permissions (harder to do reliably in tests).
    // For simplicity, we'll test the check that happens *before* Assimp runs.
    const std::string dummyFbxPath = CreateDummyFile( "dummy_for_dir_test.fbx" );

    ImportJobDesc desc;
    desc.SourceFilePath = dummyFbxPath.c_str( );
// Use an invalid path component (may vary by OS)
#ifdef _WIN32
    desc.TargetDirectory = "CON/InvalidDir"; // CON is reserved on Windows
#else
    desc.TargetDirectory = "/proc/InvalidDir"; // /proc is often read-only
#endif
    desc.AssetNamePrefix = "test_dirfail";

    // We expect the CreateDirectories call within Import to fail early.
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

    // Use default import options initially, can customize if needed
    // AssimpImportOptions options = AssimpImportOptions::FromBase(desc.Options);
    // options.ImportMaterials = true; // etc.
    // desc.Options = options;

    // --- Import ---
    const ImporterResult result = importer->Import( desc );

    // --- Basic Assertions ---
    ASSERT_EQ( result.ResultCode, ImporterResultCode::Success ) << "Import failed: " << result.ErrorMessage.Get( );
    // Expected assets: 1 Mesh, 1 Material, 1 Texture (referenced), 1 Skeleton, 1 Animation (with 3 clips)
    ASSERT_EQ( result.CreatedAssets.NumElements( ), 5 ) << "Expected 5 assets to be created";

    // --- Find Asset URIs ---
    AssetUri meshUri      = FindAssetUriByType( result, "_Mesh.dzmesh" );
    AssetUri materialUri  = FindAssetUriByType( result, "_Material.dzmat" );
    AssetUri textureUri   = FindAssetUriByType( result, "_Texture.dztex" );
    AssetUri skeletonUri  = FindAssetUriByType( result, "_Skeleton.dzskel" );
    AssetUri animationUri = FindAssetUriByType( result, "_Animation.dzanim" );

    ASSERT_FALSE( meshUri.Path.IsEmpty( ) ) << "Mesh asset URI not found in results";
    ASSERT_FALSE( materialUri.Path.IsEmpty( ) ) << "Material asset URI not found in results";
    ASSERT_FALSE( textureUri.Path.IsEmpty( ) ) << "Texture asset URI not found in results";
    ASSERT_FALSE( skeletonUri.Path.IsEmpty( ) ) << "Skeleton asset URI not found in results";
    ASSERT_FALSE( animationUri.Path.IsEmpty( ) ) << "Animation asset URI not found in results";

    // --- Validate Created Files Exist ---
    const std::string meshPath      = TEST_OUTPUT_DIR + "/" + meshUri.Path.Get( );
    const std::string materialPath  = TEST_OUTPUT_DIR + "/" + materialUri.Path.Get( );
    const std::string texturePath   = TEST_OUTPUT_DIR + "/" + textureUri.Path.Get( );
    const std::string skeletonPath  = TEST_OUTPUT_DIR + "/" + skeletonUri.Path.Get( );
    const std::string animationPath = TEST_OUTPUT_DIR + "/" + animationUri.Path.Get( );

    ASSERT_TRUE( FileIO::FileExists( meshPath.c_str( ) ) ) << "Mesh file not created: " << meshPath;
    ASSERT_TRUE( FileIO::FileExists( materialPath.c_str( ) ) ) << "Material file not created: " << materialPath;
    ASSERT_TRUE( FileIO::FileExists( texturePath.c_str( ) ) ) << "Texture file not created: " << texturePath;
    ASSERT_TRUE( FileIO::FileExists( skeletonPath.c_str( ) ) ) << "Skeleton file not created: " << skeletonPath;
    ASSERT_TRUE( FileIO::FileExists( animationPath.c_str( ) ) ) << "Animation file not created: " << animationPath;

    // --- Validate Asset Contents (Using Readers) ---
    try
    {
        // Validate Mesh
        BinaryReader    meshFileReader( meshPath.c_str( ) );
        MeshAssetReader meshReader( { &meshFileReader } );
        MeshAsset       readMesh = meshReader.Read( );
        ASSERT_STREQ( readMesh.Name.Get( ), "Fox" ); // Check sanitized name
        ASSERT_EQ( readMesh.SubMeshes.NumElements( ), 1 );
        ASSERT_TRUE( readMesh.SubMeshes.GetElement( 0 ).MaterialRef.Equals( materialUri ) );
        ASSERT_TRUE( readMesh.SkeletonRef.Equals( skeletonUri ) );
        ASSERT_TRUE( readMesh.AnimationRef.Equals( animationUri ) );      // Check if linked
        ASSERT_GT( readMesh.SubMeshes.GetElement( 0 ).NumVertices, 100 ); // Fox has many vertices
        ASSERT_GT( readMesh.SubMeshes.GetElement( 0 ).NumIndices, 100 );
        ASSERT_TRUE( readMesh.EnabledAttributes.Position );
        ASSERT_TRUE( readMesh.EnabledAttributes.Normal );
        ASSERT_TRUE( readMesh.EnabledAttributes.UV );
        ASSERT_TRUE( readMesh.EnabledAttributes.BlendIndices );
        ASSERT_TRUE( readMesh.EnabledAttributes.BlendWeights );

        // Validate Material
        BinaryReader        matFileReader( materialPath.c_str( ) );
        MaterialAssetReader matReader( { &matFileReader } );
        MaterialAsset       readMat = matReader.Read( );
        ASSERT_STREQ( readMat.Name.Get( ), "Fox" );
        ASSERT_TRUE( readMat.AlbedoMapRef.Equals( textureUri ) );
        ASSERT_FLOAT_EQ( readMat.MetallicFactor, 0.0f );    // Fox is non-metallic
        ASSERT_NEAR( readMat.RoughnessFactor, 1.0f, 0.1f ); // Fox is rough

        // Validate Texture (Metadata only for external)
        BinaryReader       texFileReader( texturePath.c_str( ) );
        TextureAssetReader texReader( { &texFileReader } );
        TextureAsset       readTex = texReader.Read( );
        ASSERT_STREQ( readTex.Name.Get( ), "Texture_0" ); // Default name from glTF
        ASSERT_FALSE( readTex.SourcePath.IsEmpty( ) );    // Should point to original PNG
        ASSERT_TRUE( std::string( readTex.SourcePath.Get( ) ).ends_with( "Texture_0.png" ) );

        // Validate Skeleton
        BinaryReader        skelFileReader( skeletonPath.c_str( ) );
        SkeletonAssetReader skelReader( { &skelFileReader } );
        SkeletonAsset       readSkel = skelReader.Read( );
        ASSERT_STREQ( readSkel.Name.Get( ), "Fox" );
        ASSERT_GT( readSkel.Joints.NumElements( ), 40 ); // Fox has ~50-60 joints
        // Check root joint
        bool rootFound = false;
        for ( size_t j = 0; j < readSkel.Joints.NumElements( ); ++j )
        {
            if ( readSkel.Joints.GetElement( j ).ParentIndex == -1 )
            {
                ASSERT_STREQ( readSkel.Joints.GetElement( j ).Name.Get( ), "Root" );
                rootFound = true;
                break;
            }
        }
        ASSERT_TRUE( rootFound ) << "Root joint not found";

        // Validate Animation
        BinaryReader         animFileReader( animationPath.c_str( ) );
        AnimationAssetReader animReader( { &animFileReader } );
        AnimationAsset       readAnim = animReader.Read( );
        ASSERT_STREQ( readAnim.Name.Get( ), "Fox" );
        ASSERT_TRUE( readAnim.SkeletonRef.Equals( skeletonUri ) );
        ASSERT_EQ( readAnim.Animations.NumElements( ), 3 ); // Survey, Walk, Run

        // Check first clip ("Survey")
        const AnimationClip &clip0 = readAnim.Animations.GetElement( 0 );
        ASSERT_STREQ( clip0.Name.Get( ), "Survey" );
        ASSERT_GT( clip0.Duration, 0.0f );
        ASSERT_GT( clip0.Tracks.NumElements( ), 10 ); // Expect tracks for multiple joints

        // Check one track in the first clip
        bool spineTrackFound = false;
        for ( size_t t = 0; t < clip0.Tracks.NumElements( ); ++t )
        {
            const JointAnimTrack &track = clip0.Tracks.GetElement( t );
            if ( track.JointName.Equals( "Spine" ) )
            {
                ASSERT_GT( track.PositionKeys.NumElements( ), 0 );
                ASSERT_GT( track.RotationKeys.NumElements( ), 0 );
                ASSERT_GT( track.ScaleKeys.NumElements( ), 0 );
                // Check first position key time/value roughly
                ASSERT_NEAR( track.PositionKeys.GetElement( 0 ).Timestamp, 0.0f, 0.01f );
                // ASSERT_NEAR(track.PositionKeys.GetElement(0).Value.X, ...); // Check against known value if needed
                spineTrackFound = true;
                break;
            }
        }
        ASSERT_TRUE( spineTrackFound ) << "Spine track not found in 'Survey' animation";
    }
    catch ( const std::exception &e )
    {
        FAIL( ) << "Exception during asset validation: " << e.what( );
    }
}
