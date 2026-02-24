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
 * Tool to convert HLSL shaders to compressed embedded C++ data
 * Usage: ShaderToEmbeddedInl --generate-text-renderer
 *    or: ShaderToEmbeddedInl --hlsl-vs <path> --hlsl-ps <path> --name <shader_name>
 */

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <miniz/miniz.h>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"

static auto FontVertexShaderSource = R"(
struct VSInput
{
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

cbuffer FontUniforms : register(b0)
{
    float4x4 Projection;
    float4 TextColor;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Position = mul(float4(input.Position, 0.0, 1.0), Projection);
    output.TexCoord = input.TexCoord;
    output.Color = input.Color * TextColor;
    return output;
})";

static auto FontPixelShaderSource = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

Texture2D FontAtlas : register(t0);
SamplerState FontSampler : register(s0);

cbuffer FontUniforms : register(b0)
{
    float4x4 Projection;
    float4 TextColor;
    float4 TextureSizeParams; // xy: texture size, z: pixel range, w: screen pixel range (precomputed)
};

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange(float2 texCoord, float pxRange, float2 textureSize)
{
    float2 unitRange = float2(pxRange, pxRange) / textureSize;
    float2 screenTexSize = float2(1.0, 1.0) / fwidth(texCoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float4 main(PSInput input) : SV_TARGET
{
    float4 mtsdf = FontAtlas.Sample(FontSampler, input.TexCoord);
    float3 msdf = mtsdf.rgb;
    float sd = median(msdf.r, msdf.g, msdf.b);

    float2 textureSize = TextureSizeParams.xy;
    float pxRange = TextureSizeParams.z;

    float screenPxRangeValue = max(screenPxRange(input.TexCoord, pxRange, textureSize), 1.0);
    float pxDist = screenPxRangeValue * clamp(sd - 0.5, mtsdf.a - 0.52, mtsdf.a - 0.48);
    float opacity = clamp(pxDist + 0.5, 0.0, 1.0);

    float4 finalColor = float4(input.Color.rgb, input.Color.a * opacity);
    if (finalColor.a < 0.001f) {
        discard;
    }
    return finalColor;
}
)";

// ImGui shaders
static auto ImGuiVertexShaderSource = R"(
struct VSInput
{
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    uint   Color : COLOR0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

cbuffer ImGuiUniforms : register(b0, space1)
{
    float4x4 Projection;
    float4 ScreenSize;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Position = mul(float4(input.Position, 0.0, 1.0), Projection);
    output.TexCoord = input.TexCoord;
    output.Color = float4(
        ((input.Color >> 0) & 0xFF) / 255.0f,
        ((input.Color >> 8) & 0xFF) / 255.0f,
        ((input.Color >> 16) & 0xFF) / 255.0f,
        ((input.Color >> 24) & 0xFF) / 255.0f
    );
    return output;
})";

static auto ImGuiPixelShaderSourceArray = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

cbuffer PixelConstants : register(b0, space2)
{
    uint TextureIndex;
    uint Padding;
};

Texture2D Textures[128] : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = Textures[TextureIndex].Sample(LinearSampler, input.TexCoord);
    return texColor * input.Color;
})";

static auto ImGuiPixelShaderSourceSingle = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

Texture2D Texture : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);

float4 main(PSInput input) : SV_TARGET
{
    float4 texColor = Texture.Sample(LinearSampler, input.TexCoord);
    return texColor * input.Color;
})";

// UI (Clay) shaders
static auto UIVertexShaderSource = R"(
struct VSInput
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    uint TextureIndex : TEXINDEX;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    uint TextureIndex : TEXINDEX;
};

cbuffer UIUniforms : register(b0, space1)
{
    float4x4 Projection;
    float4 ScreenSize; // xy: screen dimensions, zw: unused
    float4 FontParams; // x: atlas width, y: atlas height, z: pixel range, w: first non-font texture index
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Position = mul(float4(input.Position, 1.0), Projection);
    output.TexCoord = input.TexCoord;
    output.Color = input.Color;
    output.TextureIndex = input.TextureIndex;
    return output;
})";

static auto UIPixelShaderSourceArray = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    uint TextureIndex : TEXINDEX;
};

Texture2D Textures[128] : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);

cbuffer UIUniforms : register(b0, space1)
{
    float4x4 Projection;
    float4 ScreenSize; // xy: screen dimensions, zw: unused
    float4 FontParams; // x: atlas width, y: atlas height, z: pixel range, w: first non-font texture index
};

// MSDF rendering helper function to calculate median of 3 values
float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange(float2 texCoord, float pxRange, float2 textureSize)
{
    float2 unitRange = float2(pxRange, pxRange) / textureSize;
    float2 screenTexSize = float2(1.0, 1.0) / fwidth(texCoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float4 main(PSInput input) : SV_TARGET
{
    // TextureIndex 0 means solid color (no texture)
    if (input.TextureIndex == 0)
    {
        return input.Color;
    }

    float4 texColor = Textures[input.TextureIndex].Sample(LinearSampler, input.TexCoord);

    // Textures with index < FontParams.w are font textures (MTSDF)
    // Textures with index >= FontParams.w are regular image textures
    if (input.TextureIndex > 0 && input.TextureIndex < (uint)FontParams.w)
    {
        float4 mtsdf = texColor;
        float3 msdf = mtsdf.rgb;
        float sd = median(msdf.r, msdf.g, msdf.b);

        float2 textureSize = FontParams.xy;
        float pxRange = FontParams.z;

        float screenPxRangeValue = max(screenPxRange(input.TexCoord, pxRange, textureSize), 1.0);
        float pxDist = screenPxRangeValue * clamp(sd - 0.5, mtsdf.a - 0.52, mtsdf.a - 0.48);
        float opacity = clamp(pxDist + 0.5, 0.0, 1.0);

        float4 finalColor = float4(input.Color.rgb, input.Color.a * opacity);
        if (finalColor.a < 0.1f) {
            discard;
        }
        return finalColor;
    }
    else
    {
        // Regular image texture
        return texColor * input.Color;
    }
}
)";

static auto UIPixelShaderSourceSingle = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    uint TextureIndex : TEXINDEX;
};

Texture2D Texture : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);

cbuffer UIUniforms : register(b0, space1)
{
    float4x4 Projection;
    float4 ScreenSize; // xy: screen dimensions, zw: unused
    float4 FontParams; // x: atlas width, y: atlas height, z: pixel range, w: first non-font texture index
};

// MSDF rendering helper function to calculate median of 3 values
float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange(float2 texCoord, float pxRange, float2 textureSize)
{
    float2 unitRange = float2(pxRange, pxRange) / textureSize;
    float2 screenTexSize = float2(1.0, 1.0) / fwidth(texCoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float4 main(PSInput input) : SV_TARGET
{
    // TextureIndex 0 means solid color (no texture)
    if (input.TextureIndex == 0)
    {
        return input.Color;
    }

    float4 texColor = Texture.Sample(LinearSampler, input.TexCoord);

    // TextureIndex 1 = font texture (MTSDF), TextureIndex 2 = regular image
    if (input.TextureIndex == 1)
    {
        float4 mtsdf = texColor;
        float3 msdf = mtsdf.rgb;
        float sd = median(msdf.r, msdf.g, msdf.b);

        float2 textureSize = FontParams.xy;
        float pxRange = FontParams.z;

        float screenPxRangeValue = max(screenPxRange(input.TexCoord, pxRange, textureSize), 1.0);
        float pxDist = screenPxRangeValue * clamp(sd - 0.5, mtsdf.a - 0.52, mtsdf.a - 0.48);
        float opacity = clamp(pxDist + 0.5, 0.0, 1.0);

        float4 finalColor = float4(input.Color.rgb, input.Color.a * opacity);
        if (finalColor.a < 0.1f) {
            discard;
        }
        return finalColor;
    }
    else
    {
        // Regular image texture
        return texColor * input.Color;
    }
}
)";

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
                result += c;
            }
        }
        else
        {
            capitalizeNext = true;
        }
    }
    return result;
}

void WriteCompressedData( std::ofstream &out, const std::vector<uint8_t> &compressed, const std::string &arrayName )
{
    out << "// Auto-generated compressed shader data\n";
    out << "// DO NOT EDIT - Generated by ShaderToEmbeddedInl tool\n\n";

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

void UpdateEmbeddedShadersHeader( const std::string &shaderName, const std::string &headerPath )
{
    std::ifstream     file( headerPath );
    std::stringstream buffer;
    buffer << file.rdbuf( );
    file.close( );

    std::string content   = buffer.str( );
    std::string camelName = ToCamelCase( shaderName );

    std::string functionSignature = "static DenOfIz_ShaderProgram Get" + camelName + "ShaderProgram( )";
    if ( content.find( functionSignature ) != std::string::npos )
    {
        std::cout << "Function Get" << camelName << "ShaderProgram() already exists in EmbeddedShaders.h, skipping" << std::endl;
        return;
    }

    std::string newFunction = "\n        static DenOfIz_ShaderProgram Get" + camelName + "ShaderProgram( );\n";

    size_t classEndPos = content.rfind( "};" );
    if ( classEndPos != std::string::npos )
    {
        size_t insertPos = content.rfind( '\n', classEndPos );
        if ( insertPos != std::string::npos )
        {
            content.insert( insertPos, newFunction );
        }
    }

    std::ofstream outFile( headerPath );
    outFile << content;
    outFile.close( );

    std::cout << "Updated EmbeddedShaders.h with " << camelName << " function" << std::endl;
}

void UpdateEmbeddedShadersCpp( const std::string &shaderName, const std::string &cppPath, const std::string &arrayName )
{
    std::ifstream     file( cppPath );
    std::stringstream buffer;
    buffer << file.rdbuf( );
    file.close( );

    std::string content   = buffer.str( );
    std::string camelName = ToCamelCase( shaderName );

    std::string externCheck = "extern const uint8_t " + arrayName + "Compressed[];";
    if ( content.find( externCheck ) == std::string::npos )
    {
        std::string externDecl = "extern const uint8_t " + arrayName + "Compressed[];\n" + "extern const size_t  " + arrayName + "CompressedSize;\n";

        size_t usingPos = content.find( "using namespace DenOfIz;" );
        if ( usingPos != std::string::npos )
        {
            size_t lineEnd = content.find( '\n', usingPos ) + 1;
            content.insert( lineEnd, "\n" + externDecl );
        }
    }
    else
    {
        std::cout << "Extern declarations for " << arrayName << " already exist, skipping" << std::endl;
    }

    std::string registryCheck = "registry[ \"" + camelName + "\" ]";
    if ( content.find( registryCheck ) == std::string::npos )
    {
        std::string registryEntry = "        registry[ \"" + camelName + "\" ] = ShaderData{ " + arrayName + "Compressed, " + arrayName + "CompressedSize };\n";

        size_t registryPos = content.find( "return registry;" );
        if ( registryPos != std::string::npos )
        {
            content.insert( registryPos, registryEntry );
        }
    }
    else
    {
        std::cout << "Registry entry for " << camelName << " already exists, skipping" << std::endl;
    }

    std::string includeCheck = "#include \"DenOfIzGraphicsInternal/Assets/Shaders/Embedded/" + camelName + "ShadersCompressed.inl\"";
    if ( content.find( includeCheck ) == std::string::npos )
    {
        size_t lastIncludePos = content.rfind( "#include \"DenOfIzGraphicsInternal/Assets/Shaders/Embedded/" );
        if ( lastIncludePos != std::string::npos )
        {
            size_t lineEnd = content.find( '\n', lastIncludePos ) + 1;
            content.insert( lineEnd, includeCheck + "\n" );
        }
        else
        {
            content += "\n" + includeCheck + "\n";
        }
    }
    else
    {
        std::cout << "Include for " << camelName << "ShadersCompressed.inl already exists, skipping" << std::endl;
    }

    std::string functionCheck = "DenOfIz_ShaderProgram EmbeddedShaders::Get" + camelName + "ShaderProgram( )";
    if ( content.find( functionCheck ) == std::string::npos )
    {
        std::string newFunction = "\nDenOfIz_ShaderProgram EmbeddedShaders::Get" + camelName +
                                  "ShaderProgram( )\n"
                                  "{\n"
                                  "    return GetEmbeddedShaderProgram( \"" +
                                  camelName +
                                  "\" );\n"
                                  "}\n";

        size_t lastFunctionPos = content.rfind( "return GetEmbeddedShaderProgram(" );
        if ( lastFunctionPos != std::string::npos )
        {
            size_t funcEndPos = content.find( "}", lastFunctionPos );
            if ( funcEndPos != std::string::npos )
            {
                content.insert( funcEndPos + 1, newFunction );
            }
        }
    }
    else
    {
        std::cout << "Function Get" << camelName << "ShaderProgram() already exists, skipping" << std::endl;
    }

    std::ofstream outFile( cppPath );
    outFile << content;
    outFile.close( );

    std::cout << "Updated EmbeddedShaders.cpp" << std::endl;
}

bool GenerateTextRendererShaders( const std::string &repoRoot )
{
    std::cout << "Generating TextRenderer shaders..." << std::endl;

    std::vector<uint8_t> vsData( strlen( FontVertexShaderSource ) );
    std::memcpy( vsData.data( ), FontVertexShaderSource, vsData.size( ) );

    std::vector<uint8_t> psData( strlen( FontPixelShaderSource ) );
    std::memcpy( psData.data( ), FontPixelShaderSource, psData.size( ) );

    std::array<DenOfIz_ShaderStageDesc, 2> shaderStages( { } );
    DenOfIz_ShaderStageDesc               &vsDesc = shaderStages[ 0 ];
    vsDesc.Stage                                  = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
    vsDesc.EntryPoint                             = DENOFIZ_STRING( "main" );
    vsDesc.Data.Elements                          = vsData.data( );
    vsDesc.Data.NumElements                       = vsData.size( );

    DenOfIz_ShaderStageDesc &psDesc = shaderStages[ 1 ];
    psDesc.Stage                    = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
    psDesc.EntryPoint               = DENOFIZ_STRING( "main" );
    psDesc.Data.Elements            = psData.data( );
    psDesc.Data.NumElements         = psData.size( );

    DenOfIz_ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.NumElements = shaderStages.size( );
    programDesc.ShaderStages.Elements    = shaderStages.data( );

    std::cout << "Compiling shaders to all targets (DXIL, MSL, SPIRV, WGSL)..." << std::endl;
    DenOfIz_ShaderProgram shaderProgram = DenOfIz_ShaderProgram_Create( &programDesc );
    if ( !DENOFIZ_HANDLE_IS_VALID( shaderProgram ) )
    {
        std::cerr << "Failed to compile shaders" << std::endl;
        return false;
    }

    DenOfIz_CompiledShaderStageArray compiledStages{ };
    DenOfIz_ShaderProgram_CompiledShaders( shaderProgram, &compiledStages );

    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( shaderProgram, &reflectDesc );

    DenOfIz_CompiledShader compiledShader{ };
    compiledShader.Stages      = compiledStages;
    compiledShader.ReflectDesc = reflectDesc;
    compiledShader.RayTracing  = { };

    DenOfIz_ShaderAsset shaderAsset = DenOfIz_ShaderAssetWriter_CreateFromCompiledShader( &compiledShader );
    if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
    {
        std::cerr << "Failed to create shader asset" << std::endl;
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return false;
    }

    DenOfIz_ShaderAsset_SetPath( shaderAsset, DENOFIZ_STRING( "asset://TextRenderer" ) );

    DenOfIz_BinaryContainer container    = DenOfIz_BinaryContainer_Create( );
    DenOfIz_BinaryWriter    binaryWriter = DenOfIz_BinaryWriter_CreateFromContainer( container );

    DenOfIz_ShaderAssetWriterDesc assetWriterDesc{ };
    assetWriterDesc.Writer                = binaryWriter;
    DenOfIz_ShaderAssetWriter assetWriter = DenOfIz_ShaderAssetWriter_Create( &assetWriterDesc );

    DenOfIz_ShaderAssetWriter_Write( assetWriter, shaderAsset );
    DenOfIz_ShaderAssetWriter_End( assetWriter );

    uint64_t              writtenBytes  = DenOfIz_BinaryWriter_Position( binaryWriter );
    DenOfIz_ByteArrayView containerData = DenOfIz_BinaryContainer_GetData( container );

    std::vector<uint8_t> assetData( containerData.Elements, containerData.Elements + writtenBytes );

    std::cout << "Shader asset size: " << assetData.size( ) << " bytes" << std::endl;

    mz_ulong             compressedBound = mz_compressBound( assetData.size( ) );
    std::vector<uint8_t> compressedBuffer( compressedBound + sizeof( uint32_t ) );

    uint32_t uncompressedSize = static_cast<uint32_t>( assetData.size( ) );
    memcpy( compressedBuffer.data( ), &uncompressedSize, sizeof( uint32_t ) );

    mz_ulong  compressedSize = compressedBound;
    const int compressResult = mz_compress2( compressedBuffer.data( ) + sizeof( uint32_t ), &compressedSize, assetData.data( ), assetData.size( ), MZ_BEST_COMPRESSION );

    if ( compressResult != MZ_OK )
    {
        std::cerr << "Compression failed with error: " << compressResult << std::endl;
        DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
        DenOfIz_BinaryWriter_Destroy( binaryWriter );
        DenOfIz_BinaryContainer_Destroy( container );
        DenOfIz_ShaderAsset_Destroy( shaderAsset );
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return false;
    }

    compressedBuffer.resize( compressedSize + sizeof( uint32_t ) );
    std::cout << "Compressed size: " << compressedBuffer.size( ) << " bytes" << std::endl;
    std::cout << "Compression ratio: " << std::fixed << std::setprecision( 2 ) << ( 100.0 * compressedBuffer.size( ) / assetData.size( ) ) << "%" << std::endl;

    std::vector<uint8_t> testBuffer( uncompressedSize );
    mz_ulong             testSize         = uncompressedSize;
    int                  decompressResult = mz_uncompress( testBuffer.data( ), &testSize, compressedBuffer.data( ) + sizeof( uint32_t ), compressedSize );

    if ( decompressResult != MZ_OK || testSize != uncompressedSize )
    {
        std::cerr << "Decompression test failed! Result: " << decompressResult << std::endl;
        DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
        DenOfIz_BinaryWriter_Destroy( binaryWriter );
        DenOfIz_BinaryContainer_Destroy( container );
        DenOfIz_ShaderAsset_Destroy( shaderAsset );
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return false;
    }
    std::cout << "Decompression test passed!" << std::endl;

    std::string embeddedDir   = repoRoot + "/Graphics/Internal/DenOfIzGraphicsInternal/Assets/Shaders/Embedded/";
    std::string targetInlPath = embeddedDir + "TextRendererShadersCompressed.inl";

    std::filesystem::create_directories( embeddedDir );

    std::ofstream outFile( targetInlPath );
    if ( !outFile.is_open( ) )
    {
        std::cerr << "Failed to open output file: " << targetInlPath << std::endl;
        DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
        DenOfIz_BinaryWriter_Destroy( binaryWriter );
        DenOfIz_BinaryContainer_Destroy( container );
        DenOfIz_ShaderAsset_Destroy( shaderAsset );
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return false;
    }

    WriteCompressedData( outFile, compressedBuffer, "g_TextRendererShaders" );
    outFile.close( );

    std::cout << "Generated: " << targetInlPath << std::endl;

    DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
    DenOfIz_BinaryWriter_Destroy( binaryWriter );
    DenOfIz_BinaryContainer_Destroy( container );
    DenOfIz_ShaderAsset_Destroy( shaderAsset );
    DenOfIz_ShaderProgram_Destroy( shaderProgram );

    return true;
}

bool GenerateShaderFromSources( const std::string &repoRoot, const std::string &shaderName, const char *vsSource, const char *psSource, uint32_t maxTextureArraySize = 0 )
{
    std::cout << "Generating " << shaderName << " shaders..." << std::endl;

    std::vector<uint8_t> vsData( strlen( vsSource ) );
    std::memcpy( vsData.data( ), vsSource, vsData.size( ) );

    std::vector<uint8_t> psData( strlen( psSource ) );
    std::memcpy( psData.data( ), psSource, psData.size( ) );

    std::array<DenOfIz_ShaderStageDesc, 2> shaderStages( { } );
    DenOfIz_ShaderStageDesc               &vsDesc = shaderStages[ 0 ];
    vsDesc.Stage                                  = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
    vsDesc.EntryPoint                             = DENOFIZ_STRING( "main" );
    vsDesc.Data.Elements                          = vsData.data( );
    vsDesc.Data.NumElements                       = vsData.size( );

    DenOfIz_ShaderStageDesc &psDesc = shaderStages[ 1 ];
    psDesc.Stage                    = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
    psDesc.EntryPoint               = DENOFIZ_STRING( "main" );
    psDesc.Data.Elements            = psData.data( );
    psDesc.Data.NumElements         = psData.size( );

    if ( maxTextureArraySize > 0 )
    {
        std::array<DenOfIz_BindlessSlot, 1> bindlessSlots( { } );
        bindlessSlots[ 0 ].RegisterSpace = 0;
        bindlessSlots[ 0 ].Binding       = 0;
        bindlessSlots[ 0 ].MaxArraySize  = maxTextureArraySize;
        bindlessSlots[ 0 ].Descriptor    = DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT;

        psDesc.Bindless.BindlessArrays.Elements    = bindlessSlots.data( );
        psDesc.Bindless.BindlessArrays.NumElements = bindlessSlots.size( );
    }

    DenOfIz_ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.NumElements = shaderStages.size( );
    programDesc.ShaderStages.Elements    = shaderStages.data( );

    std::cout << "Compiling shaders to all targets (DXIL, MSL, SPIRV, WGSL)..." << std::endl;
    DenOfIz_ShaderProgram shaderProgram = DenOfIz_ShaderProgram_Create( &programDesc );
    if ( !DENOFIZ_HANDLE_IS_VALID( shaderProgram ) )
    {
        std::cerr << "Failed to compile shaders for " << shaderName << std::endl;
        return false;
    }

    DenOfIz_CompiledShaderStageArray compiledStages{ };
    DenOfIz_ShaderProgram_CompiledShaders( shaderProgram, &compiledStages );

    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( shaderProgram, &reflectDesc );

    DenOfIz_CompiledShader compiledShader{ };
    compiledShader.Stages      = compiledStages;
    compiledShader.ReflectDesc = reflectDesc;
    compiledShader.RayTracing  = { };

    DenOfIz_ShaderAsset shaderAsset = DenOfIz_ShaderAssetWriter_CreateFromCompiledShader( &compiledShader );
    if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
    {
        std::cerr << "Failed to create shader asset for " << shaderName << std::endl;
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return false;
    }

    std::string assetPath = "asset://" + shaderName;
    DenOfIz_ShaderAsset_SetPath( shaderAsset, DenOfIz_StringView{ assetPath.c_str( ), static_cast<uint32_t>( assetPath.size( ) ) } );

    DenOfIz_BinaryContainer container    = DenOfIz_BinaryContainer_Create( );
    DenOfIz_BinaryWriter    binaryWriter = DenOfIz_BinaryWriter_CreateFromContainer( container );

    DenOfIz_ShaderAssetWriterDesc assetWriterDesc{ };
    assetWriterDesc.Writer                = binaryWriter;
    DenOfIz_ShaderAssetWriter assetWriter = DenOfIz_ShaderAssetWriter_Create( &assetWriterDesc );

    DenOfIz_ShaderAssetWriter_Write( assetWriter, shaderAsset );
    DenOfIz_ShaderAssetWriter_End( assetWriter );

    uint64_t              writtenBytes  = DenOfIz_BinaryWriter_Position( binaryWriter );
    DenOfIz_ByteArrayView containerData = DenOfIz_BinaryContainer_GetData( container );

    std::vector<uint8_t> assetData( containerData.Elements, containerData.Elements + writtenBytes );

    std::cout << "Shader asset size: " << assetData.size( ) << " bytes" << std::endl;

    mz_ulong             compressedBound = mz_compressBound( assetData.size( ) );
    std::vector<uint8_t> compressedBuffer( compressedBound + sizeof( uint32_t ) );

    uint32_t uncompressedSize = static_cast<uint32_t>( assetData.size( ) );
    memcpy( compressedBuffer.data( ), &uncompressedSize, sizeof( uint32_t ) );

    mz_ulong  compressedSize = compressedBound;
    const int compressResult = mz_compress2( compressedBuffer.data( ) + sizeof( uint32_t ), &compressedSize, assetData.data( ), assetData.size( ), MZ_BEST_COMPRESSION );

    if ( compressResult != MZ_OK )
    {
        std::cerr << "Compression failed with error: " << compressResult << std::endl;
        DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
        DenOfIz_BinaryWriter_Destroy( binaryWriter );
        DenOfIz_BinaryContainer_Destroy( container );
        DenOfIz_ShaderAsset_Destroy( shaderAsset );
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return false;
    }

    compressedBuffer.resize( compressedSize + sizeof( uint32_t ) );
    std::cout << "Compressed size: " << compressedBuffer.size( ) << " bytes" << std::endl;
    std::cout << "Compression ratio: " << std::fixed << std::setprecision( 2 ) << ( 100.0 * compressedBuffer.size( ) / assetData.size( ) ) << "%" << std::endl;

    std::vector<uint8_t> testBuffer( uncompressedSize );
    mz_ulong             testSize         = uncompressedSize;
    int                  decompressResult = mz_uncompress( testBuffer.data( ), &testSize, compressedBuffer.data( ) + sizeof( uint32_t ), compressedSize );

    if ( decompressResult != MZ_OK || testSize != uncompressedSize )
    {
        std::cerr << "Decompression test failed! Result: " << decompressResult << std::endl;
        DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
        DenOfIz_BinaryWriter_Destroy( binaryWriter );
        DenOfIz_BinaryContainer_Destroy( container );
        DenOfIz_ShaderAsset_Destroy( shaderAsset );
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return false;
    }
    std::cout << "Decompression test passed!" << std::endl;

    std::string embeddedDir   = repoRoot + "/Graphics/Internal/DenOfIzGraphicsInternal/Assets/Shaders/Embedded/";
    std::string targetInlPath = embeddedDir + shaderName + "ShadersCompressed.inl";

    std::filesystem::create_directories( embeddedDir );

    std::ofstream outFile( targetInlPath );
    if ( !outFile.is_open( ) )
    {
        std::cerr << "Failed to open output file: " << targetInlPath << std::endl;
        DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
        DenOfIz_BinaryWriter_Destroy( binaryWriter );
        DenOfIz_BinaryContainer_Destroy( container );
        DenOfIz_ShaderAsset_Destroy( shaderAsset );
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return false;
    }

    std::string arrayName = "g_" + shaderName + "Shaders";
    WriteCompressedData( outFile, compressedBuffer, arrayName );
    outFile.close( );

    std::cout << "Generated: " << targetInlPath << std::endl;

    DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
    DenOfIz_BinaryWriter_Destroy( binaryWriter );
    DenOfIz_BinaryContainer_Destroy( container );
    DenOfIz_ShaderAsset_Destroy( shaderAsset );
    DenOfIz_ShaderProgram_Destroy( shaderProgram );

    return true;
}

bool GenerateShaderToPath( const std::string &outputDir, const std::string &shaderName, const char *vsSource, const char *psSource, uint32_t maxTextureArraySize = 0 )
{
    std::cout << "Generating " << shaderName << " shaders to " << outputDir << "..." << std::endl;

    std::vector<uint8_t> vsData( strlen( vsSource ) );
    std::memcpy( vsData.data( ), vsSource, vsData.size( ) );

    std::vector<uint8_t> psData( strlen( psSource ) );
    std::memcpy( psData.data( ), psSource, psData.size( ) );

    std::array<DenOfIz_ShaderStageDesc, 2> shaderStages( { } );
    DenOfIz_ShaderStageDesc               &vsDesc = shaderStages[ 0 ];
    vsDesc.Stage                                  = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
    vsDesc.EntryPoint                             = DENOFIZ_STRING( "main" );
    vsDesc.Data.Elements                          = vsData.data( );
    vsDesc.Data.NumElements                       = vsData.size( );

    DenOfIz_ShaderStageDesc &psDesc = shaderStages[ 1 ];
    psDesc.Stage                    = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
    psDesc.EntryPoint               = DENOFIZ_STRING( "main" );
    psDesc.Data.Elements            = psData.data( );
    psDesc.Data.NumElements         = psData.size( );

    if ( maxTextureArraySize > 0 )
    {
        std::array<DenOfIz_BindlessSlot, 1> bindlessSlots( { } );
        bindlessSlots[ 0 ].RegisterSpace = 0;
        bindlessSlots[ 0 ].Binding       = 0;
        bindlessSlots[ 0 ].MaxArraySize  = maxTextureArraySize;
        bindlessSlots[ 0 ].Descriptor    = DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT;

        psDesc.Bindless.BindlessArrays.Elements    = bindlessSlots.data( );
        psDesc.Bindless.BindlessArrays.NumElements = bindlessSlots.size( );
    }

    DenOfIz_ShaderProgramDesc programDesc{ };
    programDesc.ShaderStages.NumElements = shaderStages.size( );
    programDesc.ShaderStages.Elements    = shaderStages.data( );

    std::cout << "Compiling shaders to all targets (DXIL, MSL, SPIRV, WGSL)..." << std::endl;
    DenOfIz_ShaderProgram shaderProgram = DenOfIz_ShaderProgram_Create( &programDesc );
    if ( !DENOFIZ_HANDLE_IS_VALID( shaderProgram ) )
    {
        std::cerr << "Failed to compile shaders for " << shaderName << std::endl;
        return false;
    }

    DenOfIz_CompiledShaderStageArray compiledStages{ };
    DenOfIz_ShaderProgram_CompiledShaders( shaderProgram, &compiledStages );

    DenOfIz_ShaderReflectDesc reflectDesc{ };
    DenOfIz_ShaderProgram_Reflect( shaderProgram, &reflectDesc );

    DenOfIz_CompiledShader compiledShader{ };
    compiledShader.Stages      = compiledStages;
    compiledShader.ReflectDesc = reflectDesc;
    compiledShader.RayTracing  = { };

    DenOfIz_ShaderAsset shaderAsset = DenOfIz_ShaderAssetWriter_CreateFromCompiledShader( &compiledShader );
    if ( !DENOFIZ_HANDLE_IS_VALID( shaderAsset ) )
    {
        std::cerr << "Failed to create shader asset for " << shaderName << std::endl;
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return false;
    }

    std::string assetPath = "asset://" + shaderName;
    DenOfIz_ShaderAsset_SetPath( shaderAsset, DenOfIz_StringView{ assetPath.c_str( ), static_cast<uint32_t>( assetPath.size( ) ) } );

    DenOfIz_BinaryContainer container    = DenOfIz_BinaryContainer_Create( );
    DenOfIz_BinaryWriter    binaryWriter = DenOfIz_BinaryWriter_CreateFromContainer( container );

    DenOfIz_ShaderAssetWriterDesc assetWriterDesc{ };
    assetWriterDesc.Writer                = binaryWriter;
    DenOfIz_ShaderAssetWriter assetWriter = DenOfIz_ShaderAssetWriter_Create( &assetWriterDesc );

    DenOfIz_ShaderAssetWriter_Write( assetWriter, shaderAsset );
    DenOfIz_ShaderAssetWriter_End( assetWriter );

    uint64_t              writtenBytes  = DenOfIz_BinaryWriter_Position( binaryWriter );
    DenOfIz_ByteArrayView containerData = DenOfIz_BinaryContainer_GetData( container );

    std::vector<uint8_t> assetData( containerData.Elements, containerData.Elements + writtenBytes );

    std::cout << "Shader asset size: " << assetData.size( ) << " bytes" << std::endl;

    mz_ulong             compressedBound = mz_compressBound( assetData.size( ) );
    std::vector<uint8_t> compressedBuffer( compressedBound + sizeof( uint32_t ) );

    uint32_t uncompressedSize = static_cast<uint32_t>( assetData.size( ) );
    memcpy( compressedBuffer.data( ), &uncompressedSize, sizeof( uint32_t ) );

    mz_ulong  compressedSize = compressedBound;
    const int compressResult = mz_compress2( compressedBuffer.data( ) + sizeof( uint32_t ), &compressedSize, assetData.data( ), assetData.size( ), MZ_BEST_COMPRESSION );

    if ( compressResult != MZ_OK )
    {
        std::cerr << "Compression failed with error: " << compressResult << std::endl;
        DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
        DenOfIz_BinaryWriter_Destroy( binaryWriter );
        DenOfIz_BinaryContainer_Destroy( container );
        DenOfIz_ShaderAsset_Destroy( shaderAsset );
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return false;
    }

    compressedBuffer.resize( compressedSize + sizeof( uint32_t ) );
    std::cout << "Compressed size: " << compressedBuffer.size( ) << " bytes" << std::endl;
    std::cout << "Compression ratio: " << std::fixed << std::setprecision( 2 ) << ( 100.0 * compressedBuffer.size( ) / assetData.size( ) ) << "%" << std::endl;

    std::vector<uint8_t> testBuffer( uncompressedSize );
    mz_ulong             testSize         = uncompressedSize;
    int                  decompressResult = mz_uncompress( testBuffer.data( ), &testSize, compressedBuffer.data( ) + sizeof( uint32_t ), compressedSize );

    if ( decompressResult != MZ_OK || testSize != uncompressedSize )
    {
        std::cerr << "Decompression test failed! Result: " << decompressResult << std::endl;
        DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
        DenOfIz_BinaryWriter_Destroy( binaryWriter );
        DenOfIz_BinaryContainer_Destroy( container );
        DenOfIz_ShaderAsset_Destroy( shaderAsset );
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return false;
    }
    std::cout << "Decompression test passed!" << std::endl;

    std::string targetInlPath = outputDir + "/" + shaderName + "ShadersCompressed.inl";

    std::filesystem::create_directories( outputDir );

    std::ofstream outFile( targetInlPath );
    if ( !outFile.is_open( ) )
    {
        std::cerr << "Failed to open output file: " << targetInlPath << std::endl;
        DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
        DenOfIz_BinaryWriter_Destroy( binaryWriter );
        DenOfIz_BinaryContainer_Destroy( container );
        DenOfIz_ShaderAsset_Destroy( shaderAsset );
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return false;
    }

    std::string arrayName = "g_" + shaderName + "Shaders";
    WriteCompressedData( outFile, compressedBuffer, arrayName );
    outFile.close( );

    std::cout << "Generated: " << targetInlPath << std::endl;

    DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
    DenOfIz_BinaryWriter_Destroy( binaryWriter );
    DenOfIz_BinaryContainer_Destroy( container );
    DenOfIz_ShaderAsset_Destroy( shaderAsset );
    DenOfIz_ShaderProgram_Destroy( shaderProgram );

    return true;
}

bool GenerateImGuiShaders( const std::string &repoRoot )
{
    std::string imguiEmbeddedDir = repoRoot + "/ImGui/Embedded";
    bool        success          = true;

    success = success && GenerateShaderToPath( imguiEmbeddedDir, "ImGuiArray", ImGuiVertexShaderSource, ImGuiPixelShaderSourceArray, 128 );
    success = success && GenerateShaderToPath( imguiEmbeddedDir, "ImGuiSingle", ImGuiVertexShaderSource, ImGuiPixelShaderSourceSingle, 0 );

    return success;
}

bool GenerateUIShaders( const std::string &repoRoot )
{
    bool success = true;

    success = success && GenerateShaderFromSources( repoRoot, "UIArray", UIVertexShaderSource, UIPixelShaderSourceArray, 128 );
    success = success && GenerateShaderFromSources( repoRoot, "UISingle", UIVertexShaderSource, UIPixelShaderSourceSingle, 0 );

    return success;
}

int main( int argc, char *argv[] )
{
    if ( argc < 2 )
    {
        std::cerr << "Usage: " << argv[ 0 ] << " --generate-text-renderer\n";
        std::cerr << "       " << argv[ 0 ] << " --generate-imgui\n";
        std::cerr << "       " << argv[ 0 ] << " --generate-ui\n";
        std::cerr << "       " << argv[ 0 ] << " --generate-all\n";
        std::cerr << "   or: " << argv[ 0 ] << " --hlsl-vs <path> --hlsl-ps <path> --name <shader_name>\n";
        return 1;
    }

    std::string repoRoot = GetRepositoryRoot( );
    if ( repoRoot.empty( ) )
    {
        std::cerr << "Error: Could not find git repository root. Make sure you're running from within the git repository." << std::endl;
        return 1;
    }
    std::cout << "Repository root: " << repoRoot << std::endl;

    std::string arg1 = argv[ 1 ];

    if ( arg1 == "--generate-text-renderer" )
    {
        if ( !GenerateTextRendererShaders( repoRoot ) )
        {
            return 1;
        }

        std::cout << "\n=== SETUP COMPLETE ===" << std::endl;
        std::cout << "TextRenderer shaders have been embedded!" << std::endl;
        std::cout << "Don't forget to rebuild your project!" << std::endl;
        return 0;
    }

    if ( arg1 == "--generate-imgui" )
    {
        if ( !GenerateImGuiShaders( repoRoot ) )
        {
            return 1;
        }

        std::cout << "\n=== SETUP COMPLETE ===" << std::endl;
        std::cout << "ImGui shaders have been embedded!" << std::endl;
        std::cout << "Don't forget to rebuild your project!" << std::endl;
        return 0;
    }

    if ( arg1 == "--generate-ui" )
    {
        if ( !GenerateUIShaders( repoRoot ) )
        {
            return 1;
        }

        std::cout << "\n=== SETUP COMPLETE ===" << std::endl;
        std::cout << "UI (Clay) shaders have been embedded!" << std::endl;
        std::cout << "Don't forget to rebuild your project!" << std::endl;
        return 0;
    }

    if ( arg1 == "--generate-all" )
    {
        bool success = true;

        std::cout << "\n=== Generating TextRenderer Shaders ===" << std::endl;
        success = success && GenerateTextRendererShaders( repoRoot );

        std::cout << "\n=== Generating ImGui Shaders ===" << std::endl;
        success = success && GenerateImGuiShaders( repoRoot );

        std::cout << "\n=== Generating UI (Clay) Shaders ===" << std::endl;
        success = success && GenerateUIShaders( repoRoot );

        if ( !success )
        {
            std::cerr << "\nSome shaders failed to generate!" << std::endl;
            return 1;
        }

        std::cout << "\n=== ALL SHADERS GENERATED SUCCESSFULLY ===" << std::endl;
        std::cout << "Don't forget to rebuild your project!" << std::endl;
        return 0;
    }

    std::cerr << "Unknown option: " << arg1 << std::endl;
    return 1;
}
