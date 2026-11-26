/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#ifdef __linux__

#include <SheenBidi/SheenBidi.h>
#include <dlfcn.h>
#include "DenOfIzGraphicsInternal/Assets/Font/SystemFonts.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextShaper.h"

namespace
{
    // fontconfig is loaded at run time with only the entry points used here: linking it would pull in FreeType, and a system without it simply has no fallback.
    struct FcPattern;
    struct FcCharSet;
    struct FcConfig;
    using FcBool                 = int;
    constexpr int FcResultMatch  = 0;
    constexpr int FcMatchPattern = 0;

    struct FcFontSet
    {
        int         nfont;
        int         sfont;
        FcPattern **fonts;
    };

    struct Fontconfig
    {
        FcPattern *( *PatternCreate )( )                                                    = nullptr;
        void ( *PatternDestroy )( FcPattern * )                                             = nullptr;
        FcBool ( *PatternAddCharSet )( FcPattern *, const char *, const FcCharSet * )       = nullptr;
        FcBool ( *PatternAddBool )( FcPattern *, const char *, FcBool )                     = nullptr;
        int ( *PatternGetString )( const FcPattern *, const char *, int, unsigned char ** ) = nullptr;
        int ( *PatternGetInteger )( const FcPattern *, const char *, int, int * )           = nullptr;
        int ( *PatternGetBool )( const FcPattern *, const char *, int, FcBool * )           = nullptr;
        int ( *PatternGetCharSet )( const FcPattern *, const char *, int, FcCharSet ** )    = nullptr;
        FcCharSet *( *CharSetCreate )( )                                                    = nullptr;
        FcBool ( *CharSetAddChar )( FcCharSet *, unsigned int )                             = nullptr;
        FcBool ( *CharSetIsSubset )( const FcCharSet *, const FcCharSet * )                 = nullptr;
        void ( *CharSetDestroy )( FcCharSet * )                                             = nullptr;
        FcBool ( *ConfigSubstitute )( FcConfig *, FcPattern *, int )                        = nullptr;
        void ( *DefaultSubstitute )( FcPattern * )                                          = nullptr;
        FcFontSet *( *FontSort )( FcConfig *, FcPattern *, FcBool, FcCharSet **, int * )    = nullptr;
        void ( *FontSetDestroy )( FcFontSet * )                                             = nullptr;
        bool Loaded                                                                         = false;

        template <typename Function>
        static bool Bind( void *library, const char *name, Function &function )
        {
            function = reinterpret_cast<Function>( dlsym( library, name ) );
            return function != nullptr;
        }

        Fontconfig( )
        {
            void *library = dlopen( "libfontconfig.so.1", RTLD_NOW | RTLD_LOCAL );
            Loaded        = library != nullptr && Bind( library, "FcPatternCreate", PatternCreate ) && Bind( library, "FcPatternDestroy", PatternDestroy ) &&
                     Bind( library, "FcPatternAddCharSet", PatternAddCharSet ) && Bind( library, "FcPatternAddBool", PatternAddBool ) &&
                     Bind( library, "FcPatternGetString", PatternGetString ) && Bind( library, "FcPatternGetInteger", PatternGetInteger ) &&
                     Bind( library, "FcPatternGetBool", PatternGetBool ) && Bind( library, "FcPatternGetCharSet", PatternGetCharSet ) &&
                     Bind( library, "FcCharSetCreate", CharSetCreate ) && Bind( library, "FcCharSetAddChar", CharSetAddChar ) &&
                     Bind( library, "FcCharSetIsSubset", CharSetIsSubset ) && Bind( library, "FcCharSetDestroy", CharSetDestroy ) &&
                     Bind( library, "FcConfigSubstitute", ConfigSubstitute ) && Bind( library, "FcDefaultSubstitute", DefaultSubstitute ) &&
                     Bind( library, "FcFontSort", FontSort ) && Bind( library, "FcFontSetDestroy", FontSetDestroy );
        }
    };

    // A scalable font covering the code points, of the kind asked for unless colour < 0. Colour bitmap fonts are kept whatever their scalable flag says.
    bool Accept( const Fontconfig &fc, const FcPattern *font, const FcCharSet *codePoints, const int colour, DenOfIz::SystemFontFile &out )
    {
        FcBool         scalable = 0;
        FcBool         isColour = 0;
        FcCharSet     *coverage = nullptr;
        unsigned char *file     = nullptr;
        int            index    = 0;
        fc.PatternGetBool( font, "scalable", 0, &scalable );
        fc.PatternGetBool( font, "color", 0, &isColour );
        if ( ( !scalable && !isColour ) || ( colour >= 0 && ( isColour != 0 ) != ( colour != 0 ) ) || fc.PatternGetCharSet( font, "charset", 0, &coverage ) != FcResultMatch ||
             !fc.CharSetIsSubset( codePoints, coverage ) || fc.PatternGetString( font, "file", 0, &file ) != FcResultMatch || file == nullptr )
        {
            return false;
        }
        fc.PatternGetInteger( font, "index", 0, &index );
        out.Path = reinterpret_cast<const char *>( file );
        out.PostScriptName.clear( );
        // The high bits select a named instance of a variable font, the default instance is used.
        out.FaceIndex = static_cast<uint32_t>( index ) & 0xFFFF;
        return true;
    }

    // colour < 0 takes fontconfig's pick, otherwise it asks for a colour or an outline font.
    bool Match( const char *text, const uint32_t length, const int colour, DenOfIz::SystemFontFile &out )
    {
        static const Fontconfig fc;
        if ( !fc.Loaded )
        {
            return false;
        }
        // Like Font::Covers: default ignorables other than tags may be missing.
        FcCharSet *codePoints = fc.CharSetCreate( );
        for ( SBUInteger i = 0; i < length; )
        {
            const SBCodepoint codePoint = SBCodepointDecodeNextFromUTF8( reinterpret_cast<const SBUInt8 *>( text ), length, &i );
            if ( !DenOfIz::TextIsDefaultIgnorable( codePoint ) || ( codePoint >= 0xE0020 && codePoint <= 0xE007F ) )
            {
                fc.CharSetAddChar( codePoints, codePoint );
            }
        }
        FcPattern *pattern = fc.PatternCreate( );
        fc.PatternAddCharSet( pattern, "charset", codePoints );
        fc.PatternAddBool( pattern, "scalable", 1 );
        if ( colour >= 0 )
        {
            fc.PatternAddBool( pattern, "color", colour );
        }
        fc.ConfigSubstitute( nullptr, pattern, FcMatchPattern );
        fc.DefaultSubstitute( pattern );
        // The best match can miss part of the text or be of the other kind, the fonts are tried in fontconfig's order instead.
        int        result = FcResultMatch;
        FcFontSet *fonts  = fc.FontSort( nullptr, pattern, 0, nullptr, &result );
        bool       found  = false;
        for ( int i = 0; fonts != nullptr && !found && i < fonts->nfont; ++i )
        {
            found = Accept( fc, fonts->fonts[ i ], codePoints, colour, out );
        }
        if ( fonts != nullptr )
        {
            fc.FontSetDestroy( fonts );
        }
        fc.PatternDestroy( pattern );
        fc.CharSetDestroy( codePoints );
        return found;
    }
} // namespace

bool DenOfIz::FindSystemFont( const char *text, const uint32_t length, SystemFontFile &out )
{
    return Match( text, length, -1, out );
}

// The best colour and the best outline font covering the text.
void DenOfIz::FindSystemFonts( const char *text, const uint32_t length, std::vector<SystemFontFile> &out )
{
    for ( const int colour : { 1, 0 } )
    {
        SystemFontFile file;
        if ( Match( text, length, colour, file ) )
        {
            out.push_back( std::move( file ) );
        }
    }
}

#endif
