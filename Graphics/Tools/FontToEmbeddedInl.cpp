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

/**
 * Tool to convert font files to compressed embedded C++ data
 * Usage: FontToEmbeddedInl <input.ttf> <output.inl> <array_name>
 */

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <miniz/miniz.h>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include "DenOfIzGraphics/Assets/Import/FontImporter.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"
#include "GoogleFontsDownloader.h"

using namespace DenOfIz;

std::string GetRepositoryRoot( )
{
    const std::string tempFile = std::filesystem::temp_directory_path( ).string( ) + "/git_root.tmp";
    const std::string gitCmd   = "git rev-parse --show-toplevel > \"" + tempFile + "\" 2>nul";

    const int result = std::system( gitCmd.c_str( ) );
    if ( result != 0 )
    {
        std::cerr << "Failed to find git repository root" << std::endl;
        return "";
    }

    std::ifstream file( tempFile );
    std::string   root;
    if ( file && std::getline( file, root ) )
    {
        file.close( );
        std::filesystem::remove( tempFile );

#ifdef _WIN32
        std::ranges::replace( root, '/', '\\' );
#endif
        return root;
    }

    std::filesystem::remove( tempFile );
    return "";
}

std::string ToCamelCase( const std::string &input )
{
    std::string result;
    bool        capitalizeNext = true;

    for ( const char c : input )
    {
        if ( std::isalnum( c ) )
        {
            if ( capitalizeNext )
            {
                result += std::toupper( c );
                capitalizeNext = false;
            }
            else
            {
                result += std::tolower( c );
            }
        }
        else
        {
            capitalizeNext = true;
        }
    }
    return result;
}

void UpdateEmbeddedFontsHeader( const std::string &fontName, const std::string &headerPath )
{
    std::ifstream     file( headerPath );
    std::stringstream buffer;
    buffer << file.rdbuf( );
    file.close( );

    std::string content   = buffer.str( );
    std::string camelName = ToCamelCase( fontName );

    // Check if function already exists
    std::string functionSignature = "static FontAsset *Get" + camelName + "( )";
    if ( content.find( functionSignature ) != std::string::npos )
    {
        std::cout << "Function Get" << camelName << "() already exists in EmbeddedFonts.h, skipping" << std::endl;
        return;
    }

    // Add convenience function before the private section
    std::string newFunction =
        "\n        DZ_API static FontAsset *Get" + camelName + "( )\n" + "        {\n" + "            return GetEmbeddedFont( \"" + camelName + "\" );\n" + "        }\n";

    size_t privatePos = content.find( "    private:" );
    if ( privatePos != std::string::npos )
    {
        content.insert( privatePos, newFunction );
    }

    std::ofstream outFile( headerPath );
    outFile << content;
    outFile.close( );

    std::cout << "Updated EmbeddedFonts.h with " << camelName << " function" << std::endl;
}

void UpdateEmbeddedFontsCpp( const std::string &fontName, const std::string &cppPath, const std::string &arrayName )
{
    std::ifstream     file( cppPath );
    std::stringstream buffer;
    buffer << file.rdbuf( );
    file.close( );

    std::string content   = buffer.str( );
    std::string camelName = ToCamelCase( fontName );

    // Check if extern declaration already exists
    std::string externCheck = "extern const uint8_t " + arrayName + "Compressed[];";
    if ( content.find( externCheck ) == std::string::npos )
    {
        // Add extern declaration
        std::string externDecl = "extern const uint8_t " + arrayName + "Compressed[];\n" + "extern const size_t  " + arrayName + "CompressedSize;\n";

        size_t externPos = content.find( "extern const size_t  g_InterFontCompressedSize;" );
        if ( externPos != std::string::npos )
        {
            size_t lineEnd = content.find( '\n', externPos ) + 1;
            content.insert( lineEnd, externDecl );
        }
    }
    else
    {
        std::cout << "Extern declarations for " << arrayName << " already exist, skipping" << std::endl;
    }

    // Check if registry entry already exists
    std::string registryCheck = "registry[ \"" + camelName + "\" ]";
    if ( content.find( registryCheck ) == std::string::npos )
    {
        // Add registry entry
        std::string registryEntry = "        registry[ \"" + camelName + "\" ] = FontData{ " + arrayName + "Compressed, " + arrayName + "CompressedSize };\n";

        size_t registryPos = content.find( "registry[ \"Inter\" ]" );
        if ( registryPos != std::string::npos )
        {
            size_t lineEnd = content.find( '\n', registryPos ) + 1;
            content.insert( lineEnd, registryEntry );
        }
    }
    else
    {
        std::cout << "Registry entry for " << camelName << " already exists, skipping" << std::endl;
    }

    // Check if include already exists
    std::string includeCheck = "#include \"DenOfIzGraphicsInternal/Assets/Font/Embedded/" + camelName + "Compressed.inl\"";
    if ( content.find( includeCheck ) == std::string::npos )
    {
        // Add include for the new .inl file
        std::string includeStr = includeCheck + "\n";
        size_t      includePos = content.find( "#include \"DenOfIzGraphicsInternal/Assets/Font/Embedded/JetbrainsMonoCompressed.inl\"" );
        if ( includePos != std::string::npos )
        {
            size_t lineEnd = content.find( '\n', includePos ) + 1;
            content.insert( lineEnd, includeStr );
        }
    }
    else
    {
        std::cout << "Include for " << camelName << "Compressed.inl already exists, skipping" << std::endl;
    }

    std::ofstream outFile( cppPath );
    outFile << content;
    outFile.close( );

    std::cout << "Updated EmbeddedFonts.cpp (checked for duplicates)" << std::endl;
}

void WriteCompressedData( std::ofstream &out, const std::vector<uint8_t> &compressed, const std::string &arrayName )
{
    out << "// Auto-generated compressed font data\n";
    out << "// DO NOT EDIT - Generated by FontToEmbeddedInl tool\n\n";

    out << "const uint8_t " << arrayName << "Compressed[] = {\n\t";

    constexpr int bytesPerLine = 16;
    for ( size_t i = 0; i < compressed.size( ); ++i )
    {
        if ( i > 0 && i % bytesPerLine == 0 )
        {
            out << "\n\t";
        }
        out << "0x" << std::hex << std::setw( 2 ) << std::setfill( '0' ) << static_cast<int>( compressed[ i ] );
        if ( i < compressed.size( ) - 1 )
        {
            out << ", ";
        }
    }

    out << "\n};\n\n";
    out << "const size_t " << arrayName << "CompressedSize = " << std::dec << compressed.size( ) << ";\n";
}

int main( int argc, char *argv[] )
{
    if ( argc < 2 )
    {
        std::cerr << "Usage (automatic): " << argv[ 0 ] << " --auto <font_name> [font_size]\n";
        std::cerr << "Usage (manual): " << argv[ 0 ] << " <input.ttf> <output.inl> <array_name> [font_size]\n";
        std::cerr << "Example (auto): " << argv[ 0 ] << " --auto \"Roboto\" 36\n";
        std::cerr << "Example (manual): " << argv[ 0 ] << " Inter.ttf InterCompressed.inl g_InterFont 24\n";
        return 1;
    }

    bool isAutoMode = ( argc >= 3 && std::string( argv[ 1 ] ) == "--auto" );

    std::string inputPath;
    std::string outputPath;
    std::string arrayName;
    std::string fontName;
    int         fontSize = 64;

    if ( isAutoMode )
    {
        if ( argc < 3 )
        {
            std::cerr << "Auto mode requires font name\n";
            return 1;
        }

        fontName = argv[ 2 ];
        if ( argc >= 4 )
        {
            fontSize = std::atoi( argv[ 3 ] );
            if ( fontSize <= 0 || fontSize > 72 )
            {
                std::cerr << "Invalid font size: " << argv[ 3 ] << ". Using default size 64.\n";
                fontSize = 64;
            }
        }

        // Setup paths for auto mode
        std::string camelName = ToCamelCase( fontName );
        std::string tempDir   = std::filesystem::temp_directory_path( ).string( );

        // Download font
        if ( !GoogleFontsDownloader::DownloadFont( fontName, tempDir, inputPath ) )
        {
            std::cerr << "Failed to download font: " << fontName << std::endl;
            return 1;
        }

        outputPath = camelName + "Compressed.inl";
        arrayName  = "g_" + camelName + "Font";
    }
    else
    {
        if ( argc < 4 )
        {
            std::cerr << "Manual mode requires: <input.ttf> <output.inl> <array_name>\n";
            return 1;
        }

        inputPath  = argv[ 1 ];
        outputPath = argv[ 2 ];
        arrayName  = argv[ 3 ];

        if ( argc >= 5 )
        {
            fontSize = std::atoi( argv[ 4 ] );
            if ( fontSize <= 0 || fontSize > 72 )
            {
                std::cerr << "Invalid font size: " << argv[ 4 ] << ". Using default size 64.\n";
                fontSize = 64;
            }
        }
    }

    std::filesystem::path outputFilePath( outputPath );
    std::filesystem::path outputDir = outputFilePath.parent_path( );
    if ( !outputDir.empty( ) && !std::filesystem::exists( outputDir ) )
    {
        std::filesystem::create_directories( outputDir );
    }

    try
    {
        std::filesystem::path importDir    = outputDir.empty( ) ? std::filesystem::current_path( ) : outputDir;
        std::string           importDirStr = importDir.make_preferred( ).string( );

        DenOfIz_FontImportDesc fontDesc{ };
        fontDesc.AtlasWidth      = 512;
        fontDesc.AtlasHeight     = 512;
        fontDesc.SourceFilePath  = DENOFIZ_STRING( inputPath.c_str( ) );
        fontDesc.TargetDirectory = DENOFIZ_STRING( importDirStr.c_str( ) );
        fontDesc.InitialFontSize = fontSize;

        std::cout << "Importing font: " << inputPath << " at size " << fontSize << "px" << std::endl;

        DenOfIz_FontImporter   importer = DenOfIz_FontImporter_Create( );
        DenOfIz_ImporterResult result   = DenOfIz_FontImporter_Import( importer, &fontDesc );
        DenOfIz_FontImporter_Destroy( importer );

        if ( result.ResultCode != DENOFIZ_IMPORTER_RESULT_SUCCESS )
        {
            auto errorMsg = std::string( result.ErrorMessage.Chars, result.ErrorMessage.NumChars );
            std::cerr << "Font import failed: " << errorMsg << std::endl;
            return 1;
        }

        std::string fontAssetPath;
        for ( size_t i = 0; i < result.CreatedAssets.NumElements; ++i )
        {
            const DenOfIz_StringView createdAsset = result.CreatedAssets.Elements[ i ];
            std::string              path         = createdAsset.Chars != nullptr ? std::string( createdAsset.Chars, createdAsset.NumChars ) : std::string( );
            if ( path.find( ".dzfont" ) != std::string::npos )
            {
                fontAssetPath = path;
                break;
            }
        }

        if ( fontAssetPath.empty( ) )
        {
            std::cerr << "No .dzfont file was created during import" << std::endl;
            return 1;
        }

        std::cout << "Font imported to: " << fontAssetPath << std::endl;

        std::filesystem::path normalizedPath( fontAssetPath );
        std::string           normalizedPathStr = normalizedPath.make_preferred( ).string( );
        std::cout << "Normalized path: " << normalizedPathStr << std::endl;

        std::ifstream file( normalizedPathStr, std::ios::binary | std::ios::ate );
        if ( !file.is_open( ) )
        {
            std::cerr << "Failed to open font asset file: " << normalizedPathStr << std::endl;
            return 1;
        }

        std::streamsize fileSize = file.tellg( );
        file.seekg( 0, std::ios::beg );

        std::vector<uint8_t> fileData( fileSize );
        if ( !file.read( reinterpret_cast<char *>( fileData.data( ) ), fileSize ) )
        {
            std::cerr << "Failed to read font asset file: " << normalizedPathStr << std::endl;
            return 1;
        }
        file.close( );

        size_t uncompressedSize = fileData.size( );
        std::cout << "Uncompressed size: " << uncompressedSize << " bytes" << std::endl;

        mz_ulong             compressedBound = mz_compressBound( uncompressedSize );
        std::vector<uint8_t> compressedBuffer( compressedBound + sizeof( mz_ulong ) );

        memcpy( compressedBuffer.data( ), &uncompressedSize, sizeof( mz_ulong ) );
        mz_ulong compressedSize = compressedBound;
        int      compressResult = mz_compress2( compressedBuffer.data( ) + sizeof( mz_ulong ), &compressedSize, fileData.data( ), uncompressedSize, MZ_BEST_COMPRESSION );

        if ( compressResult != MZ_OK )
        {
            std::cerr << "Compression failed with error: " << compressResult << std::endl;
            return 1;
        }

        compressedBuffer.resize( compressedSize + sizeof( mz_ulong ) );

        std::cout << "Compressed size: " << compressedBuffer.size( ) << " bytes" << std::endl;
        std::cout << "Compression ratio: " << std::fixed << std::setprecision( 2 ) << ( 100.0 * compressedBuffer.size( ) / uncompressedSize ) << "%" << std::endl;

        std::vector<uint8_t> testBuffer( uncompressedSize );
        mz_ulong             testSize         = uncompressedSize;
        int                  decompressResult = mz_uncompress( testBuffer.data( ), &testSize, compressedBuffer.data( ) + sizeof( mz_ulong ), compressedSize );

        if ( decompressResult != MZ_OK || testSize != uncompressedSize )
        {
            std::cerr << "Decompression test failed! Result: " << decompressResult << std::endl;
            return 1;
        }

        std::cout << "Decompression test passed!" << std::endl;
        std::ofstream outFile( outputPath );
        if ( !outFile.is_open( ) )
        {
            std::cerr << "Failed to open output file: " << outputPath << std::endl;
            return 1;
        }

        WriteCompressedData( outFile, compressedBuffer, arrayName );
        outFile.close( );

        std::cout << "Successfully generated: " << outputPath << std::endl;
        std::filesystem::remove( fontAssetPath );

        std::string repoRoot = GetRepositoryRoot( );
        if ( !repoRoot.empty( ) )
        {
            std::string camelName;
            if ( isAutoMode )
            {
                camelName = ToCamelCase( fontName );
            }
            else
            {
                std::filesystem::path inlPath( outputPath );
                std::string           inlBasename = inlPath.stem( ).string( );
                if ( inlBasename.find( "Compressed" ) != std::string::npos )
                {
                    inlBasename = inlBasename.substr( 0, inlBasename.find( "Compressed" ) );
                }
                camelName = inlBasename;
            }

            std::cout << "Repository root: " << repoRoot << std::endl;

            std::string embeddedDir   = repoRoot + "/Graphics/Internal/DenOfIzGraphicsInternal/Assets/Font/Embedded/";
            std::string targetInlPath = embeddedDir + camelName + "Compressed.inl";

            std::filesystem::create_directories( embeddedDir );
            std::filesystem::copy_file( outputPath, targetInlPath, std::filesystem::copy_options::overwrite_existing );
            std::cout << "Copied to: " << targetInlPath << std::endl;

            std::string headerPath = repoRoot + "/Graphics/Include/DenOfIzGraphics/Assets/Font/Embedded/EmbeddedFonts.h";
            std::string cppPath    = repoRoot + "/Graphics/Source/Assets/Font/EmbeddedFonts.cpp";

            try
            {
                UpdateEmbeddedFontsHeader( fontName.empty( ) ? camelName : fontName, headerPath );
                UpdateEmbeddedFontsCpp( fontName.empty( ) ? camelName : fontName, cppPath, arrayName );

                std::cout << "\n=== SETUP COMPLETE ===" << std::endl;
                std::cout << "Font '" << camelName << "' has been fully integrated!" << std::endl;
                std::cout << "Use EmbeddedFonts::Get" << camelName << "() to access it." << std::endl;
                std::cout << "Don't forget to rebuild your project!" << std::endl;
            }
            catch ( const std::exception &e )
            {
                std::cerr << "Warning: Failed to update source files: " << e.what( ) << std::endl;
                std::cerr << "You may need to manually update EmbeddedFonts.h and EmbeddedFonts.cpp" << std::endl;
            }

            if ( !isAutoMode )
            {
                std::filesystem::remove( outputPath );
                std::cout << "Cleaned up local .inl file (moved to embedded directory)" << std::endl;
            }

            if ( isAutoMode )
            {
                if ( std::filesystem::exists( inputPath ) )
                {
                    std::filesystem::remove( inputPath );
                    std::cout << "Cleaned up temporary downloaded font file" << std::endl;
                }
            }
        }
        else if ( isAutoMode )
        {
            std::cerr << "Error: Could not find git repository root. Make sure you're running from within the git repository." << std::endl;
            return 1;
        }

        return 0;
    }
    catch ( const std::exception &e )
    {
        std::cerr << "Error: " << e.what( ) << std::endl;
        return 1;
    }
}
