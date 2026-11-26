// ===================================================
// CPU port of the reference pixel shader for the Slug algorithm, reading the
// DenOfIz StructuredBuffer layout (SlugGlyphStore) instead of 4096-wide textures.
// SPDX-License-Identifier: MIT OR Apache-2.0
// Copyright 2017, by Eric Lengyel.
// ===================================================

#pragma once

#include <algorithm>
#include <bit>
#include <cmath>
#include <vector>
#include "DenOfIzGraphicsInternal/Assets/Font/SlugGlyphStore.h"

namespace SlugReference
{
    struct float2
    {
        float x;
        float y;
    };

    struct float4
    {
        float x;
        float y;
        float z;
        float w;
    };

    inline float saturate( const float v )
    {
        return v > 0.0f ? ( v < 1.0f ? v : 1.0f ) : 0.0f;
    }

    inline float frac( const float v )
    {
        return v - std::floor( v );
    }

    inline uint32_t CalcRootCode( const float y1, const float y2, const float y3 )
    {
        const uint32_t i1 = std::bit_cast<uint32_t>( y1 ) >> 31U;
        const uint32_t i2 = std::bit_cast<uint32_t>( y2 ) >> 30U;
        const uint32_t i3 = std::bit_cast<uint32_t>( y3 ) >> 29U;

        uint32_t shift = ( i2 & 2U ) | ( i1 & ~2U );
        shift          = ( i3 & 4U ) | ( shift & ~4U );

        return ( 0x2E74U >> shift ) & 0x0101U;
    }

    inline float2 SolveHorizPoly( const float4 p12, const float2 p3 )
    {
        const float2 a  = { p12.x - p12.z * 2.0f + p3.x, p12.y - p12.w * 2.0f + p3.y };
        const float2 b  = { p12.x - p12.z, p12.y - p12.w };
        const float  ra = 1.0f / a.y;
        const float  rb = 0.5f / b.y;

        const float d  = std::sqrt( std::fmax( b.y * b.y - a.y * p12.y, 0.0f ) );
        float       t1 = ( b.y - d ) * ra;
        float       t2 = ( b.y + d ) * ra;

        if ( std::fabs( a.y ) < 1.0f / 65536.0f )
        {
            t1 = t2 = p12.y * rb;
        }

        return { ( a.x * t1 - b.x * 2.0f ) * t1 + p12.x, ( a.x * t2 - b.x * 2.0f ) * t2 + p12.x };
    }

    inline float2 SolveVertPoly( const float4 p12, const float2 p3 )
    {
        const float2 a  = { p12.x - p12.z * 2.0f + p3.x, p12.y - p12.w * 2.0f + p3.y };
        const float2 b  = { p12.x - p12.z, p12.y - p12.w };
        const float  ra = 1.0f / a.x;
        const float  rb = 0.5f / b.x;

        const float d  = std::sqrt( std::fmax( b.x * b.x - a.x * p12.x, 0.0f ) );
        float       t1 = ( b.x - d ) * ra;
        float       t2 = ( b.x + d ) * ra;

        if ( std::fabs( a.x ) < 1.0f / 65536.0f )
        {
            t1 = t2 = p12.x * rb;
        }

        return { ( a.y * t1 - b.y * 2.0f ) * t1 + p12.y, ( a.y * t2 - b.y * 2.0f ) * t2 + p12.y };
    }

    inline float CalcCoverage( const float xcov, const float ycov, const float xwgt, const float ywgt, const int flags )
    {
        float coverage = std::fmax( std::fabs( xcov * xwgt + ycov * ywgt ) / std::fmax( xwgt + ywgt, 1.0f / 65536.0f ), std::fmin( std::fabs( xcov ), std::fabs( ycov ) ) );

        if ( ( flags & 0x1000 ) == 0 )
        {
            coverage = saturate( coverage );
        }
        else
        {
            coverage = 1.0f - std::fabs( 1.0f - frac( coverage * 0.5f ) * 2.0f );
        }

        return coverage;
    }

    inline float4 LoadCurve( const std::vector<DenOfIz_Float4> &curveData, const uint32_t loc, const float2 renderCoord )
    {
        const DenOfIz_Float4 &t = curveData[ loc ];
        return { t.X - renderCoord.x, t.Y - renderCoord.y, t.Z - renderCoord.x, t.W - renderCoord.y };
    }

    inline float2 LoadP3( const std::vector<DenOfIz_Float4> &curveData, const uint32_t loc, const float2 renderCoord )
    {
        const DenOfIz_Float4 &t = curveData[ loc + 1 ];
        return { t.X - renderCoord.x, t.Y - renderCoord.y };
    }

    inline void AccumulateHoriz( const float4 p12, const float2 p3, const float pixelsPerEm, float &xcov, float &xwgt )
    {
        const uint32_t code = CalcRootCode( p12.y, p12.w, p3.y );
        if ( code != 0U )
        {
            const float2 s = SolveHorizPoly( p12, p3 );
            const float2 r = { s.x * pixelsPerEm, s.y * pixelsPerEm };

            if ( ( code & 1U ) != 0U )
            {
                xcov += saturate( r.x + 0.5f );
                xwgt = std::fmax( xwgt, saturate( 1.0f - std::fabs( r.x ) * 2.0f ) );
            }

            if ( code > 1U )
            {
                xcov -= saturate( r.y + 0.5f );
                xwgt = std::fmax( xwgt, saturate( 1.0f - std::fabs( r.y ) * 2.0f ) );
            }
        }
    }

    inline void AccumulateVert( const float4 p12, const float2 p3, const float pixelsPerEm, float &ycov, float &ywgt )
    {
        const uint32_t code = CalcRootCode( p12.x, p12.z, p3.x );
        if ( code != 0U )
        {
            const float2 s = SolveVertPoly( p12, p3 );
            const float2 r = { s.x * pixelsPerEm, s.y * pixelsPerEm };

            if ( ( code & 1U ) != 0U )
            {
                ycov -= saturate( r.x + 0.5f );
                ywgt = std::fmax( ywgt, saturate( 1.0f - std::fabs( r.x ) * 2.0f ) );
            }

            if ( code > 1U )
            {
                ycov += saturate( r.y + 0.5f );
                ywgt = std::fmax( ywgt, saturate( 1.0f - std::fabs( r.y ) * 2.0f ) );
            }
        }
    }

    constexpr float SampleBias = 1.0f / 32768.0f;

    inline float SlugRender( const std::vector<DenOfIz_Float4> &curveData, const std::vector<DenOfIz_UInt2> &bandData, const float2 sampleCoord, const float2 emsPerPixel,
                             const DenOfIz::SlugGlyph &glyph )
    {
        const float2 renderCoord = { sampleCoord.x + SampleBias, sampleCoord.y + SampleBias };
        const float2 pixelsPerEm = { 1.0f / emsPerPixel.x, 1.0f / emsPerPixel.y };

        const int bandMaxX = static_cast<int>( glyph.Data.Z );
        const int bandMaxY = static_cast<int>( glyph.Data.W & 0x00FFU );

        const DenOfIz_Float4 &bandTransform = glyph.BandTransform;
        const int             bandIndexX    = std::clamp( static_cast<int>( renderCoord.x * bandTransform.X + bandTransform.Z ), 0, bandMaxX );
        const int             bandIndexY    = std::clamp( static_cast<int>( renderCoord.y * bandTransform.Y + bandTransform.W ), 0, bandMaxY );
        const uint32_t        glyphLoc      = glyph.Data.X;

        float xcov = 0.0f;
        float xwgt = 0.0f;

        const DenOfIz_UInt2 hbandData = bandData[ glyphLoc + bandIndexY ];
        const uint32_t      hbandLoc  = glyphLoc + hbandData.Y;

        for ( uint32_t curveIndex = 0; curveIndex < hbandData.X; curveIndex++ )
        {
            const DenOfIz_UInt2 refs     = bandData[ hbandLoc + ( curveIndex >> 1 ) ];
            const uint32_t      curveLoc = ( curveIndex & 1U ) != 0U ? refs.Y : refs.X;
            const float4   p12      = LoadCurve( curveData, curveLoc, renderCoord );
            const float2   p3       = LoadP3( curveData, curveLoc, renderCoord );

            if ( std::fmax( std::fmax( p12.x, p12.z ), p3.x ) * pixelsPerEm.x < -0.5f )
            {
                break;
            }

            AccumulateHoriz( p12, p3, pixelsPerEm.x, xcov, xwgt );
        }

        float ycov = 0.0f;
        float ywgt = 0.0f;

        const DenOfIz_UInt2 vbandData = bandData[ glyphLoc + bandMaxY + 1 + bandIndexX ];
        const uint32_t      vbandLoc  = glyphLoc + vbandData.Y;

        for ( uint32_t curveIndex = 0; curveIndex < vbandData.X; curveIndex++ )
        {
            const DenOfIz_UInt2 refs     = bandData[ vbandLoc + ( curveIndex >> 1 ) ];
            const uint32_t      curveLoc = ( curveIndex & 1U ) != 0U ? refs.Y : refs.X;
            const float4   p12      = LoadCurve( curveData, curveLoc, renderCoord );
            const float2   p3       = LoadP3( curveData, curveLoc, renderCoord );

            if ( std::fmax( std::fmax( p12.y, p12.w ), p3.y ) * pixelsPerEm.y < -0.5f )
            {
                break;
            }

            AccumulateVert( p12, p3, pixelsPerEm.y, ycov, ywgt );
        }

        return CalcCoverage( xcov, ycov, xwgt, ywgt, static_cast<int>( glyph.Data.W ) );
    }
} // namespace SlugReference
