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

#include "GoogleFontsDownloader.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

using namespace DenOfIz;

bool GoogleFontsDownloader::DownloadFile( const std::string &url, std::vector<uint8_t> &data )
{
    try
    {
        const std::string tempFile = std::filesystem::temp_directory_path( ).string( ) + "/font_download.tmp";
        const std::string curlCmd  = std::string( CURL_EXECUTABLE ) + " -L -k -v -o \"" + tempFile + "\" \"" + url + "\"";

        std::cout << "Downloading from: " << url << std::endl;
        std::cout << "Using curl: " << CURL_EXECUTABLE << std::endl;

        int result = std::system( curlCmd.c_str( ) );

        if ( result != 0 )
        {
            std::cerr << "curl command failed with exit code: " << result << std::endl;
            const std::string curlCmdWithUA = std::string( CURL_EXECUTABLE ) + " -L -k -v -H \"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\" -o \"" +
                                              tempFile + "\" \"" + url + "\"";
            std::cout << "Retrying with browser user-agent..." << std::endl;
            result = std::system( curlCmdWithUA.c_str( ) );

            if ( result != 0 )
            {
                std::cerr << "curl command failed again with exit code: " << result << std::endl;
                return false;
            }
        }

        if ( !std::filesystem::exists( tempFile ) )
        {
            std::cerr << "Downloaded file does not exist" << std::endl;
            return false;
        }

        const auto fileSize = std::filesystem::file_size( tempFile );
        if ( fileSize == 0 )
        {
            std::cerr << "Downloaded file is empty" << std::endl;
            std::filesystem::remove( tempFile );
            return false;
        }

        std::ifstream file( tempFile, std::ios::binary );
        if ( !file )
        {
            std::cerr << "Failed to open downloaded file" << std::endl;
            std::filesystem::remove( tempFile );
            return false;
        }

        data.resize( fileSize );
        file.read( reinterpret_cast<char *>( data.data( ) ), fileSize );
        file.close( );
        std::filesystem::remove( tempFile );

        std::cout << "Downloaded " << data.size( ) << " bytes" << std::endl;
        return true;
    }
    catch ( const std::exception &e )
    {
        std::cerr << "Download error: " << e.what( ) << std::endl;
        return false;
    }
}

std::string GoogleFontsDownloader::GetGoogleFontApiUrl( const std::string &fontName )
{
    std::string encodedName;
    for ( const char c : fontName )
    {
        if ( c == ' ' )
        {
            encodedName += "+";
        }
        else if ( std::isalnum( c ) || c == '-' || c == '_' )
        {
            encodedName += c;
        }
        else
        {
            std::ostringstream oss;
            oss << "%" << std::hex << std::uppercase << static_cast<unsigned char>( c );
            encodedName += oss.str( );
        }
    }
    return "https://fonts.googleapis.com/css2?family=" + encodedName + ":wght@400&display=swap";
}

bool GoogleFontsDownloader::DownloadFontFromCss( const std::string &cssUrl, std::string &fontUrl )
{
    std::vector<uint8_t> cssData;
    if ( !DownloadFile( cssUrl, cssData ) )
    {
        return false;
    }

    const std::string css( cssData.begin( ), cssData.end( ) );

    const std::regex fontUrlRegex( R"(url\(([^)]+\.(?:ttf|woff2))\))" );
    std::smatch      match;

    if ( std::regex_search( css, match, fontUrlRegex ) )
    {
        fontUrl = match[ 1 ].str( );
        if ( fontUrl.front( ) == '\'' || fontUrl.front( ) == '"' )
        {
            fontUrl = fontUrl.substr( 1, fontUrl.length( ) - 2 );
        }
        std::cout << "Found font URL: " << fontUrl << std::endl;
        return true;
    }

    std::cerr << "Could not find font URL in CSS" << std::endl;
    return false;
}

bool GoogleFontsDownloader::DownloadFont( const std::string &fontName, const std::string &outputDir, std::string &extractedFontPath )
{
    std::cout << "Downloading font: " << fontName << " from Google Fonts API..." << std::endl;

    const std::string cssUrl = GetGoogleFontApiUrl( fontName );
    std::string       fontUrl;

    if ( !DownloadFontFromCss( cssUrl, fontUrl ) )
    {
        std::cerr << "Failed to get font URL from Google Fonts API" << std::endl;
        return false;
    }

    std::vector<uint8_t> fontData;
    if ( !DownloadFile( fontUrl, fontData ) )
    {
        std::cerr << "Failed to download font from: " << fontUrl << std::endl;
        return false;
    }

    std::cout << "Downloaded font: " << fontData.size( ) << " bytes" << std::endl;

    std::string filename = fontName + ".ttf";
    for ( char &c : filename )
    {
        if ( !std::isalnum( c ) && c != '.' && c != '-' && c != '_' )
        {
            c = '_';
        }
    }

    extractedFontPath = outputDir + "/" + filename;

    std::ofstream fontFile( extractedFontPath, std::ios::binary );
    if ( !fontFile )
    {
        std::cerr << "Failed to create font file: " << extractedFontPath << std::endl;
        return false;
    }

    fontFile.write( reinterpret_cast<const char *>( fontData.data( ) ), fontData.size( ) );
    fontFile.close( );

    std::cout << "Saved font: " << extractedFontPath << std::endl;
    return true;
}
