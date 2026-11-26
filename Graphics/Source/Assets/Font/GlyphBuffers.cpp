/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Assets/Font/GlyphBuffers.h"

#include <algorithm>
#include <cstring>

using namespace DenOfIz;

namespace
{
    struct StreamInfo
    {
        uint32_t    Stride;
        uint32_t    MinCapacity;
        const char *Name;
    };

    constexpr std::array<StreamInfo, 4> Streams = { { { sizeof( DenOfIz_Float4 ), 8192, "Slug Curves" },
                                                      { sizeof( DenOfIz_UInt2 ), 16384, "Slug Bands" },
                                                      { sizeof( SlugGlyph ), 256, "Slug Glyphs" },
                                                      { sizeof( uint32_t ), 65536, "Colour Glyph Pixels" } } };
    constexpr uint32_t                  ColourStream = 3;
} // namespace

GlyphBuffers::GlyphBuffers( const DenOfIz_LogicalDevice device ) : m_device( device )
{
    for ( uint32_t i = 0; i < m_streams.size( ); ++i )
    {
        Create( i, Streams[ i ].MinCapacity );
    }
}

GlyphBuffers::~GlyphBuffers( )
{
    for ( uint32_t i = 0; i < m_streams.size( ); ++i )
    {
        Destroy( i );
    }
}

void GlyphBuffers::Write( SlugGlyphStore &glyphs, ColourGlyphStore &colourGlyphs )
{
    const bool repacked = colourGlyphs.Generation( ) != m_colourGeneration;
    if ( !glyphs.HasPendingUpload( ) && !colourGlyphs.HasPendingUpload( ) && !repacked )
    {
        return;
    }
    const SlugUploadMark             &mark    = glyphs.GetUploadMark( );
    const std::vector<uint32_t>      &pixels  = colourGlyphs.GetPixels( );
    const std::array<const void *, 4> sources = { glyphs.GetCurves( ).data( ), glyphs.GetBands( ).data( ), glyphs.GetGlyphs( ).data( ), pixels.data( ) };
    const std::array<size_t, 4>       counts  = { glyphs.GetCurves( ).size( ), glyphs.GetBands( ).size( ), glyphs.GetGlyphs( ).size( ), pixels.size( ) };
    const std::array<size_t, 4>       written = { mark.Curves, mark.Bands, mark.Glyphs, colourGlyphs.GetUploadMark( ) };
    for ( uint32_t i = 0; i < m_streams.size( ); ++i )
    {
        size_t     first   = written[ i ];
        const bool replace = i == ColourStream && repacked;
        if ( counts[ i ] > m_streams[ i ].Capacity || replace )
        {
            uint32_t capacity = replace ? Streams[ i ].MinCapacity : m_streams[ i ].Capacity;
            while ( capacity < counts[ i ] )
            {
                capacity *= 2;
            }
            Destroy( i );
            Create( i, capacity );
            first = 0;
            ++m_generation;
        }
        const size_t stride = Streams[ i ].Stride;
        std::memcpy( m_streams[ i ].Data + first * stride, static_cast<const uint8_t *>( sources[ i ] ) + first * stride, ( counts[ i ] - first ) * stride );
    }
    glyphs.MarkUploaded( );
    colourGlyphs.MarkUploaded( );
    m_colourGeneration = colourGlyphs.Generation( );
}

uint32_t GlyphBuffers::Generation( ) const
{
    return m_generation;
}

void GlyphBuffers::Bind( const DenOfIz_BindGroup bindGroup, const uint32_t firstBinding ) const
{
    for ( uint32_t i = 0; i < m_streams.size( ); ++i )
    {
        DenOfIz_BindGroup_SrvBuffer( bindGroup, firstBinding + i, m_streams[ i ].Buffer );
    }
}

void GlyphBuffers::Create( const uint32_t stream, const uint32_t capacity )
{
    const StreamInfo  &info   = Streams[ stream ];
    Stream            &target = m_streams[ stream ];
    DenOfIz_BufferDesc desc{ };
    desc.NumBytes                  = static_cast<size_t>( capacity ) * info.Stride;
    desc.HeapType                  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    desc.StructureDesc.NumElements = capacity;
    desc.StructureDesc.Stride      = info.Stride;
    desc.DebugName                 = DENOFIZ_STRING( info.Name );
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &desc, &target.Buffer );
    DenOfIz_Buffer_MapMemory( target.Buffer, reinterpret_cast<void **>( &target.Data ) );
    target.Capacity = capacity;
}

void GlyphBuffers::Destroy( const uint32_t stream )
{
    Stream &target = m_streams[ stream ];
    if ( DENOFIZ_HANDLE_IS_VALID( target.Buffer ) )
    {
        DenOfIz_Buffer_UnmapMemory( target.Buffer );
        DenOfIz_Buffer_Destroy( target.Buffer );
    }
    target = Stream{ };
}
