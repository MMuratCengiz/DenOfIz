/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#ifdef _WIN32

#include <windows.h>

#include <SheenBidi/SheenBidi.h>
#include <cstdint>
#include <dwrite_2.h>
#include <string>
#include <utility>
#include <vector>
#include <wrl/client.h>
#include "DenOfIzGraphicsInternal/Assets/Font/SystemFonts.h"
#include "DenOfIzGraphicsInternal/Assets/Font/TextShaper.h"

using Microsoft::WRL::ComPtr;

namespace
{
    // MapCharacters reads the text through this interface. It lives on the stack for one call, so it is not reference counted.
    class TextSource final : public IDWriteTextAnalysisSource
    {
        const WCHAR *m_text;
        UINT32       m_length;
        const WCHAR *m_locale;

    public:
        TextSource( const WCHAR *text, const UINT32 length, const WCHAR *locale ) : m_text( text ), m_length( length ), m_locale( locale )
        {
        }

        HRESULT STDMETHODCALLTYPE QueryInterface( REFIID id, void **object ) override
        {
            if ( id == __uuidof( IUnknown ) || id == __uuidof( IDWriteTextAnalysisSource ) )
            {
                *object = this;
                return S_OK;
            }
            *object = nullptr;
            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef( ) override
        {
            return 1;
        }

        ULONG STDMETHODCALLTYPE Release( ) override
        {
            return 1;
        }

        HRESULT STDMETHODCALLTYPE GetTextAtPosition( const UINT32 position, const WCHAR **text, UINT32 *length ) override
        {
            *text   = position < m_length ? m_text + position : nullptr;
            *length = position < m_length ? m_length - position : 0;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE GetTextBeforePosition( const UINT32 position, const WCHAR **text, UINT32 *length ) override
        {
            *text   = position > 0 && position <= m_length ? m_text : nullptr;
            *length = position > 0 && position <= m_length ? position : 0;
            return S_OK;
        }

        DWRITE_READING_DIRECTION STDMETHODCALLTYPE GetParagraphReadingDirection( ) override
        {
            return DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
        }

        HRESULT STDMETHODCALLTYPE GetLocaleName( const UINT32 position, UINT32 *length, const WCHAR **locale ) override
        {
            *locale = m_locale;
            *length = position < m_length ? m_length - position : 0;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE GetNumberSubstitution( const UINT32 position, UINT32 *length, IDWriteNumberSubstitution **substitution ) override
        {
            *substitution = nullptr;
            *length       = position < m_length ? m_length - position : 0;
            return S_OK;
        }
    };

    // Kept for the process, SystemFallbackFont serialises the calls.
    struct DirectWrite
    {
        IDWriteFactory2     *Factory  = nullptr;
        IDWriteFontFallback *Fallback = nullptr;
        WCHAR                Locale[ LOCALE_NAME_MAX_LENGTH ]{ };

        DirectWrite( )
        {
            if ( SUCCEEDED( DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory2 ), reinterpret_cast<IUnknown **>( &Factory ) ) ) )
            {
                Factory->GetSystemFontFallback( &Fallback );
            }
            if ( GetUserDefaultLocaleName( Locale, LOCALE_NAME_MAX_LENGTH ) == 0 )
            {
                wcscpy_s( Locale, L"en-us" );
            }
        }
    };

    DirectWrite &Instance( )
    {
        static DirectWrite directWrite;
        return directWrite;
    }

    bool LocalPath( IDWriteFontFile *file, std::string &path )
    {
        const void                        *key     = nullptr;
        UINT32                             keySize = 0;
        UINT32                             length  = 0;
        ComPtr<IDWriteFontFileLoader>      loader;
        ComPtr<IDWriteLocalFontFileLoader> local;
        if ( FAILED( file->GetReferenceKey( &key, &keySize ) ) || FAILED( file->GetLoader( &loader ) ) || FAILED( loader.As( &local ) ) ||
             FAILED( local->GetFilePathLengthFromKey( key, keySize, &length ) ) )
        {
            return false;
        }
        std::wstring wide( length + 1, L'\0' );
        if ( FAILED( local->GetFilePathFromKey( key, keySize, wide.data( ), length + 1 ) ) )
        {
            return false;
        }
        const int size = WideCharToMultiByte( CP_UTF8, 0, wide.data( ), static_cast<int>( length ), nullptr, 0, nullptr, nullptr );
        path.resize( size > 0 ? size : 0 );
        return size > 0 && WideCharToMultiByte( CP_UTF8, 0, wide.data( ), static_cast<int>( length ), path.data( ), size, nullptr, nullptr ) == size;
    }

    bool FileOf( IDWriteFont *font, DenOfIz::SystemFontFile &out )
    {
        ComPtr<IDWriteFontFace> face;
        UINT32                  count = 0;
        if ( FAILED( font->CreateFontFace( &face ) ) || FAILED( face->GetFiles( &count, nullptr ) ) || count == 0 )
        {
            return false;
        }
        std::vector<IDWriteFontFile *> files( count, nullptr );
        const bool                     found = SUCCEEDED( face->GetFiles( &count, files.data( ) ) ) && files[ 0 ] != nullptr && LocalPath( files[ 0 ], out.Path );
        for ( IDWriteFontFile *file : files )
        {
            if ( file != nullptr )
            {
                file->Release( );
            }
        }
        out.PostScriptName.clear( );
        out.FaceIndex = face->GetIndex( );
        return found;
    }

    // Like Font::Covers: default ignorables other than tags may be missing.
    bool Covers( IDWriteFont *font, const char *text, const uint32_t length )
    {
        for ( SBUInteger i = 0; i < length; )
        {
            const SBCodepoint codePoint = SBCodepointDecodeNextFromUTF8( reinterpret_cast<const SBUInt8 *>( text ), length, &i );
            const bool        ignored   = DenOfIz::TextIsDefaultIgnorable( codePoint ) && ( codePoint < 0xE0020 || codePoint > 0xE007F );
            BOOL              exists    = FALSE;
            if ( !ignored && ( FAILED( font->HasCharacter( codePoint, &exists ) ) || !exists ) )
            {
                return false;
            }
        }
        return true;
    }

    // Up to limit fonts of the system collection covering the text. Simulated bold and oblique fonts share a real font's file and are skipped.
    void Search( const char *text, const uint32_t length, const size_t limit, std::vector<DenOfIz::SystemFontFile> &out )
    {
        const DirectWrite            &directWrite = Instance( );
        ComPtr<IDWriteFontCollection> collection;
        if ( directWrite.Factory == nullptr || FAILED( directWrite.Factory->GetSystemFontCollection( &collection, FALSE ) ) )
        {
            return;
        }
        const size_t first = out.size( );
        for ( UINT32 i = 0; i < collection->GetFontFamilyCount( ) && out.size( ) - first < limit; ++i )
        {
            ComPtr<IDWriteFontFamily> family;
            if ( FAILED( collection->GetFontFamily( i, &family ) ) )
            {
                continue;
            }
            for ( UINT32 j = 0; j < family->GetFontCount( ) && out.size( ) - first < limit; ++j )
            {
                ComPtr<IDWriteFont>     font;
                DenOfIz::SystemFontFile file;
                if ( SUCCEEDED( family->GetFont( j, &font ) ) && font->GetSimulations( ) == DWRITE_FONT_SIMULATIONS_NONE && Covers( font.Get( ), text, length ) &&
                     FileOf( font.Get( ), file ) )
                {
                    out.push_back( std::move( file ) );
                }
            }
        }
    }
} // namespace

bool DenOfIz::FindSystemFont( const char *text, const uint32_t length, SystemFontFile &out )
{
    const DirectWrite &directWrite = Instance( );
    const int          wideLength  = MultiByteToWideChar( CP_UTF8, 0, text, static_cast<int>( length ), nullptr, 0 );
    if ( directWrite.Fallback == nullptr || wideLength <= 0 )
    {
        return false;
    }
    std::wstring wide( wideLength, L'\0' );
    MultiByteToWideChar( CP_UTF8, 0, text, static_cast<int>( length ), wide.data( ), wideLength );

    TextSource          source( wide.data( ), static_cast<UINT32>( wideLength ), directWrite.Locale );
    UINT32              mappedLength = 0;
    FLOAT               scale        = 1.0f;
    ComPtr<IDWriteFont> font;
    if ( FAILED( directWrite.Fallback->MapCharacters( &source, 0, static_cast<UINT32>( wideLength ), nullptr, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
                                                      DWRITE_FONT_STRETCH_NORMAL, &mappedLength, &font, &scale ) ) )
    {
        return false;
    }
    if ( font.Get( ) != nullptr && ( mappedLength >= static_cast<UINT32>( wideLength ) || Covers( font.Get( ), text, length ) ) )
    {
        return FileOf( font.Get( ), out );
    }
    // The fallback mapped only the start of the cluster, or nothing because none of the fonts it lists has it: any installed font covering it all.
    std::vector<SystemFontFile> found;
    Search( text, length, 1, found );
    if ( found.empty( ) )
    {
        return false;
    }
    out = std::move( found.front( ) );
    return true;
}

void DenOfIz::FindSystemFonts( const char *text, const uint32_t length, std::vector<SystemFontFile> &out )
{
    Search( text, length, SIZE_MAX, out );
}

#endif
