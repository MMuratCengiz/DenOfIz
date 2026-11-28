/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <miniz/miniz.h>
#include <sstream>
#include <string>
#include <vector>
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"

static auto TextVertexShaderSource = R"(
// ===================================================
// SlugDilate from the reference vertex shader for the Slug algorithm.
// SPDX-License-Identifier: MIT OR Apache-2.0
// Copyright 2017, by Eric Lengyel.
// ===================================================

float2 SlugDilate(float4 pos, float4 tex, float4 jac, float4 m0, float4 m1, float4 m3, float2 dim, out float2 vpos)
{
    float2 n = normalize(pos.zw);
    float s = dot(m3.xy, pos.xy) + m3.w;
    float t = dot(m3.xy, n);

    float u = (s * dot(m0.xy, n) - t * (dot(m0.xy, pos.xy) + m0.w)) * dim.x;
    float v = (s * dot(m1.xy, n) - t * (dot(m1.xy, pos.xy) + m1.w)) * dim.y;

    float s2 = s * s;
    float st = s * t;
    float uv = u * u + v * v;
    float2 d = pos.zw * (s2 * (st + sqrt(uv)) / (uv - st * st));

    vpos = pos.xy + d;
    return (float2(tex.x + dot(d, jac.xy), tex.y + dot(d, jac.zw)));
}

struct TextQuad
{
    float4 Rect;
    float4 Bounds;
    float4 Color;
    float2 EmsPerUnit;
    uint Glyph;
    uint Padding;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    nointerpolation float4 Banding : BANDING;
    nointerpolation uint4 Glyph : GLYPH;
};

struct SlugGlyph
{
    float4 Bounds;
    float4 BandTransform;
    uint4 Data;
};

cbuffer TextUniforms : register(b0)
{
    float4x4 Projection;
    float4 Viewport;
};

StructuredBuffer<SlugGlyph> SlugGlyphs : register(t2);
StructuredBuffer<TextQuad> Quads : register(t4);

static const uint ColourGlyphBit = 0x80000000u;

VSOutput main(uint vertexId : SV_VertexID)
{
    TextQuad quad = Quads[vertexId >> 2];
    bool right = ((vertexId + 1u) & 2u) != 0u;
    bool bottom = (vertexId & 2u) != 0u;
    float4 position = float4(right ? quad.Rect.z : quad.Rect.x, bottom ? quad.Rect.w : quad.Rect.y, right ? 1.0 : -1.0, bottom ? 1.0 : -1.0);
    float4 texCoord = float4(right ? quad.Bounds.z : quad.Bounds.x, bottom ? quad.Bounds.w : quad.Bounds.y, quad.EmsPerUnit);

    VSOutput output;
    output.Color = quad.Color;
    if ((quad.Glyph & ColourGlyphBit) != 0u)
    {
        output.Position = mul(float4(position.xy, 0.0, 1.0), Projection);
        output.TexCoord = texCoord.xy;
        output.Banding = float4(0.0, 0.0, 0.0, 0.0);
        output.Glyph = uint4(quad.Glyph & ~ColourGlyphBit, 0u, 0u, ColourGlyphBit);
        return output;
    }
    float4x4 m = transpose(Projection);
    float2 dilated;
    output.TexCoord = SlugDilate(position, texCoord, float4(texCoord.z, 0.0, 0.0, texCoord.w), m[0], m[1], m[3], Viewport.xy, dilated);
    output.Position = mul(float4(dilated, 0.0, 1.0), Projection);
    SlugGlyph glyph = SlugGlyphs[quad.Glyph];
    output.Banding = glyph.BandTransform;
    output.Glyph = glyph.Data;
    return output;
})";

static auto TextPixelShaderCommon = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    nointerpolation float4 Banding : BANDING;
    nointerpolation uint4 Glyph : GLYPH;
};

StructuredBuffer<float4> SlugCurves : register(t0);
StructuredBuffer<uint2> SlugBands : register(t1);
StructuredBuffer<uint> ColourGlyphs : register(t3);
)";

static auto TextPixelShaderMain = R"(
float4 main(PSInput input) : SV_TARGET
{
    if (input.Glyph.w == 0x80000000u)
    {
        return SampleColourGlyph(input.Glyph.x, input.TexCoord) * input.Color;
    }
    float coverage = SlugRender(input.TexCoord, fwidth(input.TexCoord), input.Banding, input.Glyph);
    return float4(input.Color.rgb, input.Color.a * coverage);
}
)";

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

static auto ImGuiPixelShaderSource = R"(
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

static auto UIVertexShaderSource = R"(
struct UIVertex
{
    float4 Color;
    float3 Position;
    uint TextureIndex;
    float2 TexCoord;
    uint Glyph;
    uint Padding;
};

struct UIQuad
{
    float4 Rect;
    float4 UV;
    float4 Color;
    uint TextureIndex;
    uint Glyph;
    uint2 Padding;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    uint TextureIndex : TEXINDEX;
    nointerpolation float4 Banding : BANDING;
    nointerpolation uint4 Glyph : GLYPH;
};

struct SlugGlyph
{
    float4 Bounds;
    float4 BandTransform;
    uint4 Data;
};

cbuffer UIUniforms : register(b0, space1)
{
    float4x4 Projection;
    float4 ScreenSize;
};

StructuredBuffer<SlugGlyph> SlugGlyphs : register(t3, space1);
StructuredBuffer<UIVertex> Vertices : register(t5, space1);
StructuredBuffer<UIQuad> Quads : register(t6, space1);

static const uint QuadIndexBit = 0x800000u;

VSOutput main(uint vertexId : SV_VertexID)
{
    UIVertex input;
    if ((vertexId & QuadIndexBit) != 0u)
    {
        uint index = vertexId & (QuadIndexBit - 1u);
        UIQuad quad = Quads[index >> 2];
        bool right = ((index + 1u) & 2u) != 0u;
        bool bottom = (index & 2u) != 0u;
        input.Position = float3(right ? quad.Rect.z : quad.Rect.x, bottom ? quad.Rect.w : quad.Rect.y, 0.0);
        input.TexCoord = float2(right ? quad.UV.z : quad.UV.x, bottom ? quad.UV.w : quad.UV.y);
        input.Color = quad.Color;
        input.TextureIndex = quad.TextureIndex;
        input.Glyph = quad.Glyph;
    }
    else
    {
        input = Vertices[vertexId];
    }

    VSOutput output;
    output.Position = mul(float4(input.Position, 1.0), Projection);
    output.TexCoord = input.TexCoord;
    output.Color = input.Color;
    output.TextureIndex = input.TextureIndex;
    output.Banding = float4(0.0, 0.0, 0.0, 0.0);
    output.Glyph = uint4(0, 0, 0, 0);
    uint kind = input.TextureIndex & 127u;
    if (kind == 1u)
    {
        SlugGlyph glyph = SlugGlyphs[input.Glyph];
        output.Banding = glyph.BandTransform;
        output.Glyph = glyph.Data;
    }
    else if (kind == 2u)
    {
        output.Glyph = uint4(input.Glyph, 0, 0, 0);
    }
    return output;
})";

static auto UIPixelShaderCommon = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    uint TextureIndex : TEXINDEX;
    nointerpolation float4 Banding : BANDING;
    nointerpolation uint4 Glyph : GLYPH;
};

struct UIState
{
    float4 A;
    float4 B;
    float4 C;
};

StructuredBuffer<UIState> States : register(t0, space1);
StructuredBuffer<float4> SlugCurves : register(t1, space1);
StructuredBuffer<uint2> SlugBands : register(t2, space1);
StructuredBuffer<uint> ColourGlyphs : register(t4, space1);

float clipDistance(float2 p, float4 rect, float4 radius)
{
    float d = max(max(rect.x - p.x, p.x - rect.z), max(rect.y - p.y, p.y - rect.w));
    float2 c = rect.xy + radius.x;
    if (radius.x > 0.0 && p.x < c.x && p.y < c.y)
    {
        d = max(d, length(p - c) - radius.x);
    }
    c = float2(rect.z - radius.y, rect.y + radius.y);
    if (radius.y > 0.0 && p.x > c.x && p.y < c.y)
    {
        d = max(d, length(p - c) - radius.y);
    }
    c = rect.zw - radius.z;
    if (radius.z > 0.0 && p.x > c.x && p.y > c.y)
    {
        d = max(d, length(p - c) - radius.z);
    }
    c = float2(rect.x + radius.w, rect.w - radius.w);
    if (radius.w > 0.0 && p.x < c.x && p.y > c.y)
    {
        d = max(d, length(p - c) - radius.w);
    }
    return d;
}

float4 applyState(float4 color, uint state, float2 position)
{
    UIState s = States[state];
    color.rgb = color.rgb * s.A.w + s.A.xyz;
    uint count = min((uint)s.B.y, 4u);
    float coverage = s.B.x;
    for (uint i = 0; i < count; ++i)
    {
        UIState clip = States[(uint)s.C[i]];
        coverage *= saturate(0.5 - clipDistance(position, clip.A, clip.B));
    }
    color.a *= coverage;
    return color;
}
)";

static auto SlugPixelShaderSource = R"(
// ===================================================
// Reference pixel shader for the Slug algorithm, ported from 4096-wide textures to structured buffers.
// SPDX-License-Identifier: MIT OR Apache-2.0
// Copyright 2017, by Eric Lengyel.
// ===================================================

uint CalcRootCode(float y1, float y2, float y3)
{
    uint i1 = asuint(y1) >> 31U;
    uint i2 = asuint(y2) >> 30U;
    uint i3 = asuint(y3) >> 29U;

    uint shift = (i2 & 2U) | (i1 & ~2U);
    shift = (i3 & 4U) | (shift & ~4U);

    return ((0x2E74U >> shift) & 0x0101U);
}

float2 SolveHorizPoly(float4 p12, float2 p3)
{
    float2 a = p12.xy - p12.zw * 2.0 + p3;
    float2 b = p12.xy - p12.zw;
    float ra = 1.0 / a.y;
    float rb = 0.5 / b.y;

    float d = sqrt(max(b.y * b.y - a.y * p12.y, 0.0));
    float t1 = (b.y - d) * ra;
    float t2 = (b.y + d) * ra;

    if (abs(a.y) < 1.0 / 65536.0) t1 = t2 = p12.y * rb;

    return (float2((a.x * t1 - b.x * 2.0) * t1 + p12.x, (a.x * t2 - b.x * 2.0) * t2 + p12.x));
}

float2 SolveVertPoly(float4 p12, float2 p3)
{
    float2 a = p12.xy - p12.zw * 2.0 + p3;
    float2 b = p12.xy - p12.zw;
    float ra = 1.0 / a.x;
    float rb = 0.5 / b.x;

    float d = sqrt(max(b.x * b.x - a.x * p12.x, 0.0));
    float t1 = (b.x - d) * ra;
    float t2 = (b.x + d) * ra;

    if (abs(a.x) < 1.0 / 65536.0) t1 = t2 = p12.x * rb;

    return (float2((a.y * t1 - b.y * 2.0) * t1 + p12.y, (a.y * t2 - b.y * 2.0) * t2 + p12.y));
}

float CalcCoverage(float xcov, float ycov, float xwgt, float ywgt, int flags)
{
    float coverage = max(abs(xcov * xwgt + ycov * ywgt) / max(xwgt + ywgt, 1.0 / 65536.0), min(abs(xcov), abs(ycov)));

    if ((flags & 0x1000) == 0)
    {
        coverage = saturate(coverage);
    }
    else
    {
        coverage = 1.0 - abs(1.0 - frac(coverage * 0.5) * 2.0);
    }

    return (coverage);
}

float SlugRender(float2 renderCoord, float2 emsPerPixel, float4 bandTransform, uint4 glyphData)
{
    uint curveIndex;

    renderCoord += 1.0 / 32768.0;

    float2 pixelsPerEm = 1.0 / emsPerPixel;

    int2 bandMax = int2(glyphData.zw);
    bandMax.y &= 0x00FF;

    int2 bandIndex = clamp(int2(renderCoord * bandTransform.xy + bandTransform.zw), int2(0, 0), bandMax);
    uint glyphLoc = glyphData.x;

    float xcov = 0.0;
    float xwgt = 0.0;

    uint2 hbandData = SlugBands[glyphLoc + uint(bandIndex.y)];
    uint hbandLoc = glyphLoc + hbandData.y;

    for (curveIndex = 0; curveIndex < hbandData.x; curveIndex++)
    {
        uint2 curveLocs = SlugBands[hbandLoc + (curveIndex >> 1)];
        uint curveLoc = (curveIndex & 1U) != 0U ? curveLocs.y : curveLocs.x;
        float4 p12 = SlugCurves[curveLoc] - float4(renderCoord, renderCoord);
        float2 p3 = SlugCurves[curveLoc + 1].xy - renderCoord;

        if (max(max(p12.x, p12.z), p3.x) * pixelsPerEm.x < -0.5) break;

        uint code = CalcRootCode(p12.y, p12.w, p3.y);
        if (code != 0U)
        {
            float2 r = SolveHorizPoly(p12, p3) * pixelsPerEm.x;

            if ((code & 1U) != 0U)
            {
                xcov += saturate(r.x + 0.5);
                xwgt = max(xwgt, saturate(1.0 - abs(r.x) * 2.0));
            }

            if (code > 1U)
            {
                xcov -= saturate(r.y + 0.5);
                xwgt = max(xwgt, saturate(1.0 - abs(r.y) * 2.0));
            }
        }
    }

    float ycov = 0.0;
    float ywgt = 0.0;

    uint2 vbandData = SlugBands[glyphLoc + uint(bandMax.y) + 1 + uint(bandIndex.x)];
    uint vbandLoc = glyphLoc + vbandData.y;

    for (curveIndex = 0; curveIndex < vbandData.x; curveIndex++)
    {
        uint2 curveLocs = SlugBands[vbandLoc + (curveIndex >> 1)];
        uint curveLoc = (curveIndex & 1U) != 0U ? curveLocs.y : curveLocs.x;
        float4 p12 = SlugCurves[curveLoc] - float4(renderCoord, renderCoord);
        float2 p3 = SlugCurves[curveLoc + 1].xy - renderCoord;

        if (max(max(p12.y, p12.w), p3.y) * pixelsPerEm.y < -0.5) break;

        uint code = CalcRootCode(p12.x, p12.z, p3.x);
        if (code != 0U)
        {
            float2 r = SolveVertPoly(p12, p3) * pixelsPerEm.y;

            if ((code & 1U) != 0U)
            {
                ycov -= saturate(r.x + 0.5);
                ywgt = max(ywgt, saturate(1.0 - abs(r.x) * 2.0));
            }

            if (code > 1U)
            {
                ycov += saturate(r.y + 0.5);
                ywgt = max(ywgt, saturate(1.0 - abs(r.y) * 2.0));
            }
        }
    }

    return (CalcCoverage(xcov, ycov, xwgt, ywgt, int(glyphData.w)));
}
)";

static auto ColourGlyphShaderSource = R"(
float4 ColourTexel(uint offset, uint width, int2 texel)
{
    uint packed = ColourGlyphs[offset + 1u + uint(texel.y) * width + uint(texel.x)];
    return float4(float(packed & 255u), float((packed >> 8) & 255u), float((packed >> 16) & 255u), float(packed >> 24)) / 255.0;
}

float4 SampleColourGlyph(uint offset, float2 texCoord)
{
    uint header = ColourGlyphs[offset];
    int2 last = int2(int(header & 0xFFFFu), int(header >> 16)) - 1;
    float2 p = texCoord - 0.5;
    int2 i0 = int2(floor(p));
    float2 f = p - float2(i0);
    int2 i1 = clamp(i0 + 1, int2(0, 0), last);
    i0 = clamp(i0, int2(0, 0), last);
    uint width = uint(last.x + 1);
    float4 top = lerp(ColourTexel(offset, width, i0), ColourTexel(offset, width, int2(i1.x, i0.y)), f.x);
    float4 bottom = lerp(ColourTexel(offset, width, int2(i0.x, i1.y)), ColourTexel(offset, width, i1), f.x);
    float4 c = lerp(top, bottom, f.y);
    return c.a > 0.0 ? float4(c.rgb / c.a, c.a) : float4(0.0, 0.0, 0.0, 0.0);
}
)";

static auto UIPixelShaderMain = R"(
Texture2D Textures[128] : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);

float4 main(PSInput input) : SV_TARGET
{
    uint slot = input.TextureIndex & 127u;
    uint state = input.TextureIndex >> 7;
    float2 emsPerPixel = fwidth(input.TexCoord);
    float4 color = input.Color;
    if (slot == 1)
    {
        color.a *= SlugRender(input.TexCoord, emsPerPixel, input.Banding, input.Glyph);
    }
    else if (slot == 2)
    {
        color = SampleColourGlyph(input.Glyph.x, input.TexCoord) * input.Color;
    }
    else if (slot != 0)
    {
        color = Textures[NonUniformResourceIndex(slot)].Sample(LinearSampler, input.TexCoord) * input.Color;
    }
    if (state != 0)
    {
        color = applyState(color, state, input.Position.xy);
    }
    return color;
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

void WriteCompressedData( std::ostream &out, const std::vector<uint8_t> &compressed, const std::string &arrayName )
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

static bool CheckOnly = false;

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

    std::array<DenOfIz_BindlessSlot, 1> bindlessSlots( { } );
    if ( maxTextureArraySize > 0 )
    {
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

    std::cout << "Compiling shaders to all targets (DXIL, MSL, SPIRV)..." << std::endl;
    DenOfIz_ShaderProgram shaderProgram = DenOfIz_ShaderProgram_Create( &programDesc );
    if ( !DENOFIZ_HANDLE_IS_VALID( shaderProgram ) )
    {
        std::cerr << "Failed to compile shaders for " << shaderName << std::endl;
        return false;
    }

    DenOfIz_CompiledShaderStageArray compiledStages{ };
    DenOfIz_ShaderProgram_CompiledShaders( shaderProgram, &compiledStages );

    if ( CheckOnly )
    {
        for ( size_t i = 0; i < compiledStages.NumElements; ++i )
        {
            const DenOfIz_CompiledShaderStage *stage = compiledStages.Elements[ i ];
            if ( stage->DXIL.NumElements == 0 || stage->SPIRV.NumElements == 0 || stage->MSL.NumElements == 0 )
            {
                std::cerr << shaderName << " stage " << i << " is incomplete: DXIL " << stage->DXIL.NumElements << " bytes, SPIRV " << stage->SPIRV.NumElements << " bytes, MSL "
                          << stage->MSL.NumElements << " bytes" << std::endl;
                DenOfIz_ShaderProgram_Destroy( shaderProgram );
                return false;
            }
        }
    }

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

    if ( CheckOnly )
    {
        std::ostringstream expected;
        WriteCompressedData( expected, compressedBuffer, "g_" + shaderName + "Shaders" );

        std::ifstream      inFile( targetInlPath );
        std::ostringstream actual;
        actual << inFile.rdbuf( );

        const bool upToDate = actual.str( ) == expected.str( );
        if ( upToDate )
        {
            std::cout << "Up to date: " << targetInlPath << std::endl;
        }
        else
        {
            std::cerr << "Out of date: " << targetInlPath << std::endl;
        }

        DenOfIz_ShaderAssetWriter_Destroy( assetWriter );
        DenOfIz_BinaryWriter_Destroy( binaryWriter );
        DenOfIz_BinaryContainer_Destroy( container );
        DenOfIz_ShaderAsset_Destroy( shaderAsset );
        DenOfIz_ShaderProgram_Destroy( shaderProgram );
        return upToDate;
    }

    std::filesystem::create_directories( outputDir );

    std::ofstream outFile( targetInlPath, std::ios::binary );
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

static auto EmbeddedShadersDir = "/Graphics/Internal/DenOfIzGraphicsInternal/Assets/Shaders/Embedded";

bool GenerateTextRendererShaders( const std::string &repoRoot )
{
    const std::string pixelSource = std::string( TextPixelShaderCommon ) + SlugPixelShaderSource + ColourGlyphShaderSource + TextPixelShaderMain;
    return GenerateShaderToPath( repoRoot + EmbeddedShadersDir, "TextRenderer", TextVertexShaderSource, pixelSource.c_str( ) );
}

bool GenerateImGuiShaders( const std::string &repoRoot )
{
    return GenerateShaderToPath( repoRoot + "/ImGui/Embedded", "ImGui", ImGuiVertexShaderSource, ImGuiPixelShaderSource, 128 );
}

bool GenerateUIShaders( const std::string &repoRoot )
{
    const std::string pixelSource = std::string( UIPixelShaderCommon ) + SlugPixelShaderSource + ColourGlyphShaderSource + UIPixelShaderMain;
    return GenerateShaderToPath( repoRoot + EmbeddedShadersDir, "UI", UIVertexShaderSource, pixelSource.c_str( ), 128 );
}

int main( int argc, char *argv[] )
{
    if ( argc < 2 )
    {
        std::cerr << "Usage: " << argv[ 0 ] << " --generate-text-renderer\n";
        std::cerr << "       " << argv[ 0 ] << " --generate-imgui\n";
        std::cerr << "       " << argv[ 0 ] << " --generate-ui\n";
        std::cerr << "       " << argv[ 0 ] << " --generate-all\n";
        std::cerr << "       " << argv[ 0 ] << " --check\n";
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
        std::cout << "UI shaders have been embedded!" << std::endl;
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

        std::cout << "\n=== Generating UI Shaders ===" << std::endl;
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

    if ( arg1 == "--check" )
    {
        CheckOnly = true;

        std::cout << "\n=== Checking TextRenderer Shaders ===" << std::endl;
        bool success = GenerateTextRendererShaders( repoRoot );

        std::cout << "\n=== Checking ImGui Shaders ===" << std::endl;
        success = GenerateImGuiShaders( repoRoot ) && success;

        std::cout << "\n=== Checking UI Shaders ===" << std::endl;
        success = GenerateUIShaders( repoRoot ) && success;

        if ( !success )
        {
            std::cerr << "\nEmbedded shaders are incomplete or out of date, regenerate them with --generate-all on Windows x64 or macOS!" << std::endl;
            return 1;
        }

        std::cout << "\n=== ALL EMBEDDED SHADERS ARE COMPLETE AND UP TO DATE ===" << std::endl;
        return 0;
    }

    std::cerr << "Unknown option: " << arg1 << std::endl;
    return 1;
}
