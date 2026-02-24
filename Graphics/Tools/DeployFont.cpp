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

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "DenOfIzGraphics/Assets/Import/FontImporter.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetWriter.h"
#include "GoogleFontsDownloader.h"

using namespace DenOfIz;

std::string GetRepositoryRoot( )
{
    const std::string tempFile = std::filesystem::temp_directory_path( ).string( ) + "/git_root.tmp";

#ifdef _WIN32
    const std::string gitCmd = "git rev-parse --show-toplevel > \"" + tempFile + "\" 2>nul";
#else
    const std::string gitCmd = "git rev-parse --show-toplevel > \"" + tempFile + "\" 2>/dev/null";
#endif

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
        for ( char &c : root )
        {
            if ( c == '/' )
            {
                c = '\\';
            }
        }
#endif
        return root;
    }

    std::filesystem::remove( tempFile );
    return "";
}

bool ParseUnicodeRanges( const std::string &rangesStr, std::vector<UnicodeRange> &ranges )
{
    if ( rangesStr.empty( ) )
    {
        return true;
    }

    std::stringstream ss( rangesStr );
    std::string       rangeToken;

    while ( std::getline( ss, rangeToken, ',' ) )
    {
        size_t dashPos = rangeToken.find( '-' );
        if ( dashPos == std::string::npos )
        {
            std::cerr << "Invalid range format: " << rangeToken << " (expected format: START-END)" << std::endl;
            return false;
        }

        std::string startStr = rangeToken.substr( 0, dashPos );
        std::string endStr   = rangeToken.substr( dashPos + 1 );

        uint32_t start = 0, end = 0;
        try
        {
            start = std::stoul( startStr, nullptr, 0 );
            end   = std::stoul( endStr, nullptr, 0 );
        }
        catch ( ... )
        {
            std::cerr << "Failed to parse range: " << rangeToken << std::endl;
            return false;
        }

        if ( start > end )
        {
            std::cerr << "Invalid range: start (" << start << ") > end (" << end << ")" << std::endl;
            return false;
        }

        ranges.push_back( { start, end } );
    }

    return true;
}

int main( int argc, char *argv[] )
{
    if ( argc < 2 )
    {
        std::cerr << "Usage (auto mode - download from Google Fonts): " << argv[ 0 ] << " --auto <font_name> [font_size] [--unicode-ranges=RANGES]\n";
        std::cerr << "Usage (manual mode - local file): " << argv[ 0 ] << " <font_file_path> [font_size] [--unicode-ranges=RANGES]\n";
        std::cerr << "\nExamples:\n";
        std::cerr << "  " << argv[ 0 ] << " --auto Inter 36\n";
        std::cerr << "  " << argv[ 0 ] << " --auto Inter 36 --unicode-ranges=0x20-0x7F,0xA0-0xFF\n";
        std::cerr << "  " << argv[ 0 ] << " FontAwesome.otf 64 --unicode-ranges=0xF000-0xF8FF\n";
        std::cerr << "\nFor Font Awesome icons, use: --unicode-ranges=0xF000-0xF8FF\n";
        return 1;
    }

    const bool isAutoMode = ( std::string( argv[ 1 ] ) == "--auto" );

    std::string               inputPath;
    std::string               fontName;
    int                       fontSize = 64;
    std::string               rangesStr;
    std::vector<UnicodeRange> customRanges;

    if ( isAutoMode )
    {
        if ( argc < 3 )
        {
            std::cerr << "Auto mode requires font name" << std::endl;
            return 1;
        }
        fontName = argv[ 2 ];

        for ( int i = 3; i < argc; ++i )
        {
            std::string arg( argv[ i ] );
            if ( arg.find( "--unicode-ranges=" ) == 0 )
            {
                rangesStr = arg.substr( 17 );
            }
            else
            {
                fontSize = std::atoi( argv[ i ] );
                if ( fontSize <= 0 || fontSize > 128 )
                {
                    std::cerr << "Invalid font size. Using default size 64." << std::endl;
                    fontSize = 64;
                }
            }
        }
    }
    else
    {
        inputPath = argv[ 1 ];

        for ( int i = 2; i < argc; ++i )
        {
            std::string arg( argv[ i ] );
            if ( arg.find( "--unicode-ranges=" ) == 0 )
            {
                rangesStr = arg.substr( 17 );
            }
            else
            {
                fontSize = std::atoi( argv[ i ] );
                if ( fontSize <= 0 || fontSize > 128 )
                {
                    std::cerr << "Invalid font size. Using default size 64." << std::endl;
                    fontSize = 64;
                }
            }
        }

        if ( !std::filesystem::exists( inputPath ) )
        {
            std::cerr << "Font file does not exist: " << inputPath << std::endl;
            return 1;
        }
    }

    if ( !ParseUnicodeRanges( rangesStr, customRanges ) )
    {
        return 1;
    }

    if ( !customRanges.empty( ) )
    {
        std::cout << "Using custom Unicode ranges:" << std::endl;
        for ( const auto &range : customRanges )
        {
            std::cout << "  0x" << std::hex << range.Start << " - 0x" << range.End << std::dec << std::endl;
        }
    }

    const std::string repoRoot = GetRepositoryRoot( );
    if ( repoRoot.empty( ) )
    {
        std::cerr << "Error: Could not find git repository root. Make sure you're running from within the git repository." << std::endl;
        return 1;
    }

    const std::string fontsDir = repoRoot + "/Fonts";
    std::filesystem::create_directories( fontsDir );

    std::cout << "Repository root: " << repoRoot << std::endl;
    std::cout << "Fonts directory: " << fontsDir << std::endl;

    try
    {
        if ( isAutoMode )
        {
            const std::string tempDir = std::filesystem::temp_directory_path( ).string( );
            if ( !GoogleFontsDownloader::DownloadFont( fontName, tempDir, inputPath ) )
            {
                std::cerr << "Failed to download font: " << fontName << std::endl;
                return 1;
            }
        }

        DenOfIz_FontImportDesc fontDesc{ };
        fontDesc.AtlasWidth      = 512;
        fontDesc.AtlasHeight     = 512;
        fontDesc.SourceFilePath  = DENOFIZ_STRING( inputPath.c_str( ) );
        fontDesc.TargetDirectory = DENOFIZ_STRING( fontsDir.c_str( ) );
        fontDesc.InitialFontSize = fontSize;

        if ( !customRanges.empty( ) )
        {
            fontDesc.CustomRanges.Elements    = reinterpret_cast<const DenOfIz_UnicodeRange *>( customRanges.data( ) );
            fontDesc.CustomRanges.NumElements = customRanges.size( );
        }

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

        std::cout << "\n=== DEPLOYMENT COMPLETE ===" << std::endl;
        std::cout << "Font deployed to: " << fontsDir << std::endl;
        for ( size_t i = 0; i < result.CreatedAssets.NumElements; ++i )
        {
            const DenOfIz_StringView assetView = result.CreatedAssets.Elements[ i ];
            const std::string        asset     = assetView.Chars != nullptr ? std::string( assetView.Chars, assetView.NumChars ) : std::string( );
            std::cout << "  " << asset << std::endl;
        }

        if ( isAutoMode )
        {
            if ( std::filesystem::exists( inputPath ) )
            {
                std::filesystem::remove( inputPath );
                std::cout << "Cleaned up temporary downloaded font file" << std::endl;
            }
        }

        return 0;
    }
    catch ( const std::exception &e )
    {
        std::cerr << "Error: " << e.what( ) << std::endl;
        return 1;
    }
}
