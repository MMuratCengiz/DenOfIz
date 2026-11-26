/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#ifdef __APPLE__

#include <CoreText/CoreText.h>
#include <climits>
#include <cstring>
#include "DenOfIzGraphicsInternal/Assets/Font/SystemFonts.h"

namespace
{
    CFStringRef CreateString( const char *text, const uint32_t length )
    {
        return CFStringCreateWithBytes( kCFAllocatorDefault, reinterpret_cast<const UInt8 *>( text ), length, kCFStringEncodingUTF8, false );
    }

    // Releases url and name.
    bool FileOf( const CFURLRef url, const CFStringRef name, DenOfIz::SystemFontFile &out )
    {
        char path[ PATH_MAX ];
        char postScriptName[ 256 ];
        bool found = url != nullptr && name != nullptr && CFURLGetFileSystemRepresentation( url, true, reinterpret_cast<UInt8 *>( path ), sizeof( path ) ) &&
                     CFStringGetCString( name, postScriptName, sizeof( postScriptName ), kCFStringEncodingUTF8 );
        if ( url != nullptr )
        {
            CFRelease( url );
        }
        if ( name != nullptr )
        {
            CFRelease( name );
        }
        // LastResort maps every code point to a placeholder, cached for a script it would hide the real fonts.
        found = found && std::strcmp( postScriptName, "LastResort" ) != 0;
        if ( found )
        {
            out.Path           = path;
            out.PostScriptName = postScriptName;
            out.FaceIndex      = 0;
        }
        return found;
    }
} // namespace

bool DenOfIz::FindSystemFont( const char *text, const uint32_t length, SystemFontFile &out )
{
    static const CTFontRef base   = CTFontCreateUIFontForLanguage( kCTFontUIFontSystem, 0.0, nullptr );
    const CFStringRef      string = CreateString( text, length );
    if ( string == nullptr )
    {
        return false;
    }
    const CTFontRef font = base != nullptr ? CTFontCreateForString( base, string, CFRangeMake( 0, CFStringGetLength( string ) ) ) : nullptr;
    CFRelease( string );
    if ( font == nullptr )
    {
        return false;
    }
    const auto        url  = static_cast<CFURLRef>( CTFontCopyAttribute( font, kCTFontURLAttribute ) );
    const CFStringRef name = CTFontCopyPostScriptName( font );
    CFRelease( font );
    return FileOf( url, name, out );
}

void DenOfIz::FindSystemFonts( const char *text, const uint32_t length, std::vector<SystemFontFile> &out )
{
    const CFStringRef string = CreateString( text, length );
    if ( string == nullptr )
    {
        return;
    }
    const CFCharacterSetRef characters = CFCharacterSetCreateWithCharactersInString( kCFAllocatorDefault, string );
    CFRelease( string );
    const void           *keys[]     = { kCTFontCharacterSetAttribute };
    const void           *values[]   = { characters };
    const CFDictionaryRef attributes = CFDictionaryCreate( kCFAllocatorDefault, keys, values, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
    CFRelease( characters );
    const CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes( attributes );
    CFRelease( attributes );
    const CFArrayRef matches = CTFontDescriptorCreateMatchingFontDescriptors( descriptor, nullptr );
    CFRelease( descriptor );
    if ( matches == nullptr )
    {
        return;
    }
    for ( CFIndex i = 0; i < CFArrayGetCount( matches ); ++i )
    {
        const auto     match = static_cast<CTFontDescriptorRef>( CFArrayGetValueAtIndex( matches, i ) );
        SystemFontFile file;
        if ( FileOf( static_cast<CFURLRef>( CTFontDescriptorCopyAttribute( match, kCTFontURLAttribute ) ),
                     static_cast<CFStringRef>( CTFontDescriptorCopyAttribute( match, kCTFontNameAttribute ) ), file ) )
        {
            out.push_back( std::move( file ) );
        }
    }
    CFRelease( matches );
}

#endif
