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

#include "DenOfIzGraphicsInternal/Assets/Shaders/SPIRVToWGSL.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

void SPIRVToWGSL::Convert( const SPIRVToWGSLDesc &desc ) const
{
    if ( !desc.OutWGSLShaders )
    {
        spdlog::error( "Output WGSL shaders array is null" );
        return;
    }
    if ( desc.SPIRVShaders.NumElements != desc.OutWGSLShaders->NumElements )
    {
        spdlog::error( "Mismatch between input SPIRV shaders and output WGSL shaders array size" );
        return;
    }
    for ( size_t i = 0; i < desc.SPIRVShaders.NumElements; ++i )
    {
        const DenOfIz_CompiledShaderStage *spirvShader = desc.SPIRVShaders.Elements[ i ];
        if ( !spirvShader || spirvShader->SPIRV.NumElements == 0 )
        {
            spdlog::warn( "Skipping shader {} - no SPIRV data", i );
            desc.OutWGSLShaders->Elements[ i ] = DenOfIz_ByteArray{ };
            continue;
        }
        desc.OutWGSLShaders->Elements[ i ] = ConvertSPIRVToWGSL( spirvShader->SPIRV, spirvShader->Stage );
    }
}

DenOfIz_ByteArray SPIRVToWGSL::ConvertSPIRVToWGSL( const DenOfIz_ByteArray &spirv, DenOfIz_ShaderStageFlags stage ) const
{
    DenOfIz_ByteArray result{ };
    std::string       tempDir    = std::filesystem::temp_directory_path( ).string( );
    std::string       uniqueId   = std::to_string( reinterpret_cast<uintptr_t>( spirv.Elements ) ) + "_" + std::to_string( std::hash<std::thread::id>{ }( std::this_thread::get_id( ) ) );
    std::string       inputPath  = tempDir + "/shader_input_" + uniqueId + ".spv";
    std::string       outputPath = tempDir + "/shader_output_" + uniqueId + ".wgsl";

    std::ofstream inputFile( inputPath, std::ios::binary );
    if ( !inputFile )
    {
        spdlog::error( "Failed to create temporary SPIRV file" );
        return result;
    }
    inputFile.write( reinterpret_cast<const char *>( spirv.Elements ), spirv.NumElements );
    inputFile.close( );

    std::stringstream command;
    command << "naga " << inputPath << " " << outputPath << " --keep-coordinate-space 2>&1";
    int exitCode = std::system( command.str( ).c_str( ) );
    if ( exitCode != 0 )
    {
        spdlog::warn( "Naga conversion failed with exit code: {}", exitCode );
        std::filesystem::remove( inputPath );
        return result;
    }

    std::ifstream outputFile( outputPath, std::ios::binary | std::ios::ate );
    if ( !outputFile )
    {
        spdlog::error( "Failed to open naga output file" );
        std::filesystem::remove( inputPath );
        return result;
    }

    std::streamsize size = outputFile.tellg( );
    outputFile.seekg( 0, std::ios::beg );

    result.NumElements = static_cast<size_t>( size );
    result.Elements    = static_cast<Byte *>( std::malloc( result.NumElements ) );
    if ( !outputFile.read( reinterpret_cast<char *>( result.Elements ), size ) )
    {
        spdlog::error( "Failed to read naga output" );
        std::free( result.Elements );
        result.Elements    = nullptr;
        result.NumElements = 0;
    }

    outputFile.close( );
    std::filesystem::remove( inputPath );
    std::filesystem::remove( outputPath );
    return result;
}

SPIRVToWGSL::SPIRVToWGSL( )  = default;
SPIRVToWGSL::~SPIRVToWGSL( ) = default;
