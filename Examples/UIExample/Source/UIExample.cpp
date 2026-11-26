/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzExamples/UIExample.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <thread>
#include <utility>
#include "DenOfIzGraphics/Assets/Font/Text.h"
#include "DenOfIzGraphics/Input/InputSystem.h"
#ifndef _WIN32
#include <sys/wait.h>
#endif

using namespace DenOfIz;

namespace
{
    constexpr const char *PageNames[]  = { "Controls", "Text", "Fonts", "Scripts", "Media" };
    constexpr const char *Qualities[]  = { "Low", "Medium", "High", "Ultra" };
    constexpr uint32_t    NumQualities = 4;
    constexpr uint32_t    NumRows      = 40;
    constexpr float       KnobSize     = 16.0f;
    constexpr const char *Paragraph    = "Text is measured by the font and wrapped by the layout, resize the window to see it reflow. "
                                         "A word longer than the line, like Supercalifragilisticexpialidocious, overflows instead of breaking.";
    constexpr const char *Ligatures    = "-> => != == >= <= === !== <=> |> <| :: ... /* */ // www 0xFF";
    constexpr uint16_t    InterFont    = 1;
    constexpr uint16_t    BidiFont     = 2;
    constexpr uint16_t    IconFont     = 3;
    constexpr uint16_t    MaxFonts     = 64;
    constexpr uint16_t    NotoChain    = MaxFonts - 1;
    constexpr float       TextSizes[]  = { 9, 11, 13, 16, 24, 48, 96 };
    constexpr float       Waterfall[]  = { 12, 18, 28, 40 };
    constexpr float       MinSample    = 8.0f;
    constexpr float       MaxSample    = 400.0f;
    constexpr const char *Pangram      = "Sphinx of black quartz, judge my vow. Five boxing wizards jump quickly, 0123456789 & ?!";
    constexpr const char *Presets[]    = { "Roboto",     "Lobster",      "Playfair Display", "Pacifico", "Fira Code",    "JetBrains Mono", "Press Start 2P",
                                           "Noto Serif", "Merriweather", "Dancing Script",   "Nabla",    "Bungee Spice", "Honk",           "Noto Color Emoji" };

    struct ScriptSample
    {
        const char *Script;
        const char *Family;
        const char *Text;
        bool        Rtl;
    };

    constexpr ScriptSample Scripts[] = {
        { "Arabic, vocalised", "Noto Naskh Arabic", "اَلْخَطُّ الْعَرَبِيُّ فَنٌّ جَمِيلٌ", true },
        { "Urdu, Nastaliq", "Noto Nastaliq Urdu", "اردو نستعلیق میں لکھی جاتی ہے", true },
        { "Hebrew, niqqud", "Noto Sans Hebrew", "עִבְרִית הִיא שָׂפָה יָפָה", true },
        { "Devanagari", "Noto Sans Devanagari", "विद्यार्थी हिन्दी किताब पढ़ते हैं", false },
        { "Bengali", "Noto Sans Bengali", "আমি বাংলায় স্বপ্ন দেখি", false },
        { "Tamil", "Noto Sans Tamil", "தமிழ் இனிமையான மொழி", false },
        { "Thai", "Noto Sans Thai", "ผู้ใหญ่ปั่นจักรยานไปดื่มน้ำที่ญี่ปุ่น", false },
        { "Khmer", "Noto Sans Khmer", "ភាសាខ្មែរ ស្រស់ស្អាត", false },
        { "Myanmar", "Noto Sans Myanmar", "မြန်မာဘာသာစကား", false },
        { "Japanese", "Noto Sans JP", "日本語の文章は、漢字とひらがなとカタカナで書きます。", false },
        { "Korean", "Noto Sans KR", "한국어는 아름다운 언어입니다", false },
        { "Vietnamese", "Roboto", "Tiếng Việt có dấu: Ở Huế, chữ quốc ngữ thật đẹp", false },
        { "Code ligatures", "Fira Code", "-> => != === >= <= <=> |> :: /* */ www 0xFF &&", false },
    };

    struct BidiSample
    {
        const char *Name;
        const char *Text;
        bool        Rtl;
    };

    constexpr BidiSample BidiSamples[] = {
        { "Hebrew words in English", "The word שלום means peace, and עולם means world.", false },
        { "Arabic with a number and English", "قرأت 3 كتب في Book World اليوم", true },
        { "English in parentheses", "הוא אמר (hello world) ויצא", true },
        { "A price in Hebrew", "המחיר הוא 1,234.50 ₪ בלבד", true },
        { "Nested brackets", "Call (שלום [123]) now", false },
    };

    constexpr const char *FallbackLines[] = {
        "English · العربية · עברית · हिन्दी · ไทย · 日本語 · 한국어 · Ελληνικά · Русский · ქართული · አማርኛ",
        "हिन्दी · বাংলা · ਪੰਜਾਬੀ · ગુજરાતી · தமிழ் · తెలుగు · ಕನ್ನಡ · മലയാളം · සිංහල",
        "中文 · 日本語 · 한국어 · ລາວ · ខ្មែរ · မြန်မာ · Հայերեն · فارسی · ᏣᎳᎩ · ᓀᐦᐃᔭᐍᐏᐣ",
        "Save \xEF\x83\x87   Search \xEF\x80\x82   Settings \xEF\x80\x93   Delete \xEF\x87\xB8   Home \xEF\x80\x95",
    };

    constexpr const char *EmojiLines[] = {
        "Smileys 😀 😃 😄 😁 😆 😅 🤣 😂 🙂 😉 😊 😇 🥰 😍 🤩 🥳",
        "Skin tones 👍 👍🏻 👍🏼 👍🏽 👍🏾 👍🏿 ✌🏽 🧑🏾‍💻 👩🏻‍🚀",
        "ZWJ sequences 👨‍👩‍👧 👩‍👩‍👦‍👦 🧑‍🤝‍🧑 ❤️‍🔥 🏳️‍🌈 🏴‍☠️ 🐻‍❄️",
        "Flags 🇳🇱 🇹🇷 🇯🇵 🇧🇷 🇺🇦 🇨🇦 🇪🇺 🏴󠁧󠁢󠁥󠁮󠁧󠁿 🏴󠁧󠁢󠁳󠁣󠁴󠁿 · Keycaps 1️⃣ 2️⃣ 3️⃣ #️⃣ *️⃣ 🔟",
        "Presentation: ☺ and ☺️, ❤ and ❤️, ✔ and ✔️, ❤︎ asks for text with VS15",
        "Inline: the build passed ✅ and shipped 🚀 before lunch 🍜, then coffee ☕.",
        "مبروك 🎉 عيد ميلاد سعيد 🎂 · מזל טוב 🥂 לכולם",
    };
    constexpr const char *EmojiSample   = "😀 👍🏽 🇳🇱 👨‍👩‍👧 🦄 🌈 🔥";
    constexpr float       EmojiSizes[]  = { 16, 20, 24, 32, 48, 64, 96, 128 };
    constexpr const char *ColourFonts[] = { "Noto Color Emoji", "Nabla", "Bungee Spice", "Honk" };

    struct EdgeCase
    {
        const char        *Name;
        const char        *Text;
        float              Width;
        DenOfIz_UITextWrap Wrap;
    };

    constexpr EdgeCase EdgeCases[] = {
        { "Line boxes, selected", "Latin line\nမြန်မာဘာသာ line\nEmoji 😀 line\nLatin again", 190, DENOFIZ_UI_TEXT_WRAP_WORDS },
        { "Over-long Arabic word", "مستشفياتمستشفياتمستشفيات", 120, DENOFIZ_UI_TEXT_WRAP_ANYWHERE },
        { "Over-long kerned word", "AVAVAVAVAVAVAVAVAVAVAV", 120, DENOFIZ_UI_TEXT_WRAP_ANYWHERE },
        { "Controls and ill-formed UTF-8", "RLO a\u202Ebcd\u202Ce \u00B7 soft hy\u00ADphen, zw\u200Bsp, wj\u2060oin\nbad \xC0\xAF \xE2\x82 \xF0\x9F\x98 end", 190,
          DENOFIZ_UI_TEXT_WRAP_WORDS },
    };

    constexpr const char *BidiParagraphs[] = {
        "تلتف هذه الفقرة العربية داخل صندوق ضيق، ويبدأ كل سطر من اليمين. تبقى الأرقام مثل 2025 والكلمات الإنجليزية مثل Unicode بترتيبها الصحيح.",
        "פסקה בעברית שנשברת לשורות בתוך תיבה צרה. מילים באנגלית like these ומספרים כמו 42 נשארים במקומם.",
    };

    using Clock = std::chrono::steady_clock;

    constexpr DenOfIz_UIColor Hex( const uint32_t rgba )
    {
        return DenOfIz_UIColor{ static_cast<uint8_t>( rgba >> 24 ), static_cast<uint8_t>( rgba >> 16 ), static_cast<uint8_t>( rgba >> 8 ), static_cast<uint8_t>( rgba ) };
    }

    namespace Theme
    {
        constexpr DenOfIz_UIColor Background   = Hex( 0xF3F4F6FF );
        constexpr DenOfIz_UIColor Surface      = Hex( 0xFFFFFFFF );
        constexpr DenOfIz_UIColor Sidebar      = Hex( 0xEEF1F5FF );
        constexpr DenOfIz_UIColor RowAlt       = Hex( 0xEEF2F7FF );
        constexpr DenOfIz_UIColor Border       = Hex( 0xD9DEE5FF );
        constexpr DenOfIz_UIColor BorderStrong = Hex( 0xAEB6C2FF );
        constexpr DenOfIz_UIColor Text         = Hex( 0x1F2937FF );
        constexpr DenOfIz_UIColor Muted        = Hex( 0x6B7280FF );
        constexpr DenOfIz_UIColor OnAccent     = Hex( 0xFFFFFFFF );
        constexpr DenOfIz_UIColor Accent       = Hex( 0x2563EBFF );
        constexpr DenOfIz_UIColor AccentHover  = Hex( 0x1D4ED8FF );
        constexpr DenOfIz_UIColor AccentDown   = Hex( 0x1E40AFFF );
        constexpr DenOfIz_UIColor AccentSoft   = Hex( 0xDBEAFEFF );
        constexpr DenOfIz_UIColor Hover        = Hex( 0xE5E9F0FF );
        constexpr DenOfIz_UIColor Pressed      = Hex( 0xD5DBE4FF );
        constexpr DenOfIz_UIColor Disabled     = Hex( 0xE5E7EBFF );
        constexpr DenOfIz_UIColor DisabledText = Hex( 0x9CA3AFFF );
        constexpr DenOfIz_UIColor Dark         = Hex( 0x111827FF );
        constexpr DenOfIz_UIColor Selection    = Hex( 0x2563EB59 );
        constexpr DenOfIz_UIColor Backdrop     = Hex( 0x0F172A73 );
        constexpr DenOfIz_UIColor Danger       = Hex( 0xDC2626FF );
        constexpr DenOfIz_UIColor Clear        = Hex( 0x00000000 );
    } // namespace Theme

    DenOfIz_StringView View( const char *text )
    {
        return DenOfIz_StringView{ text, std::strlen( text ) };
    }

    bool Has( const uint32_t flags, const uint32_t bits )
    {
        return ( flags & bits ) != 0;
    }

    bool Shift( const uint32_t mod )
    {
        return Has( mod, DENOFIZ_KEY_MOD_SHIFT | DENOFIZ_KEY_MOD_LSHIFT | DENOFIZ_KEY_MOD_RSHIFT );
    }

    bool Shortcut( const uint32_t mod )
    {
        return Has( mod, DENOFIZ_KEY_MOD_GUI | DENOFIZ_KEY_MOD_LGUI | DENOFIZ_KEY_MOD_RGUI | DENOFIZ_KEY_MOD_CTRL | DENOFIZ_KEY_MOD_LCTRL | DENOFIZ_KEY_MOD_RCTRL );
    }

    DenOfIz_UIElementDesc Row( const float gap = 0.0f )
    {
        DenOfIz_UIElementDesc desc{ };
        desc.Layout.AlignItems = DENOFIZ_UI_ALIGN_CENTER;
        desc.Layout.Gap        = DenOfIz_UILength_Points( gap );
        return desc;
    }

    DenOfIz_UIElementDesc Column( const float gap = 0.0f )
    {
        DenOfIz_UIElementDesc desc{ };
        desc.Layout.Direction = DENOFIZ_UI_DIRECTION_COLUMN;
        desc.Layout.Gap       = DenOfIz_UILength_Points( gap );
        return desc;
    }

    void Fill( DenOfIz_UILayoutDesc &layout )
    {
        layout.Grow   = 1.0f;
        layout.Shrink = 1.0f;
        layout.Basis  = DenOfIz_UIValue_Points( 0.0f );
    }

    DenOfIz_UITransitionDesc ColorTransition( const DenOfIz_UITransitionPropertyFlags properties = DENOFIZ_UI_TRANSITION_PROPERTY_COLORS_BIT )
    {
        DenOfIz_UITransitionDesc transition{ };
        transition.Duration   = 0.12f;
        transition.Properties = properties;
        return transition;
    }

    DenOfIz_UITransitionDesc FadeIn( const float duration, const DenOfIz_Float2 offset )
    {
        DenOfIz_UITransitionDesc transition{ };
        transition.Duration      = duration;
        transition.Enter.Flags   = DENOFIZ_UI_TRANSITION_STATE_OPACITY_BIT | DENOFIZ_UI_TRANSITION_STATE_OFFSET_BIT;
        transition.Enter.Opacity = 0.0f;
        transition.Enter.Offset  = offset;
        transition.Exit.Flags    = DENOFIZ_UI_TRANSITION_STATE_OPACITY_BIT;
        transition.Exit.Opacity  = 0.0f;
        return transition;
    }

    double MsSince( const Clock::time_point start )
    {
        return std::chrono::duration<double, std::milli>( Clock::now( ) - start ).count( );
    }

    std::string Trim( const std::string &text )
    {
        const size_t begin = text.find_first_not_of( ' ' );
        return begin == std::string::npos ? std::string( ) : text.substr( begin, text.find_last_not_of( ' ' ) - begin + 1 );
    }

    bool IsFamilyName( const std::string &family )
    {
        return !family.empty( ) && std::all_of( family.begin( ), family.end( ), []( const char c ) { return std::isalnum( static_cast<unsigned char>( c ) ) || c == ' '; } );
    }

    bool IsFontUrl( const std::string &url )
    {
        constexpr std::string_view allowed = "/:._-";
        return url.starts_with( "https://fonts.gstatic.com/" ) &&
               std::all_of( url.begin( ), url.end( ),
                            [ & ]( const char c ) { return std::isalnum( static_cast<unsigned char>( c ) ) || allowed.find( c ) != std::string_view::npos; } );
    }

    int Curl( const std::string &url, const char *flags, std::vector<uint8_t> &out )
    {
        const std::string command = std::string( "curl " ) + flags + " --max-time 30 \"" + url + "\"";
#ifdef _WIN32
        FILE *pipe = _popen( command.c_str( ), "rb" );
#else
        FILE *pipe = popen( command.c_str( ), "r" );
#endif
        if ( pipe == nullptr )
        {
            return -1;
        }
        std::array<uint8_t, 1 << 16> chunk{ };
        for ( size_t read; ( read = std::fread( chunk.data( ), 1, chunk.size( ), pipe ) ) > 0; )
        {
            out.insert( out.end( ), chunk.begin( ), chunk.begin( ) + read );
        }
#ifdef _WIN32
        return _pclose( pipe );
#else
        const int status = pclose( pipe );
        return WIFEXITED( status ) ? WEXITSTATUS( status ) : -1;
#endif
    }

    std::string CurlError( const int code )
    {
        switch ( code )
        {
        case 6:
            return "Could not resolve the host, is the network up?";
        case 7:
            return "Could not connect to the server";
        case 22:
        case 56:
            return "The server answered with an HTTP error";
        case 28:
            return "Timed out after 30 seconds";
        case 127:
        case 9009:
            return "curl was not found on the PATH";
        default:
            return "curl failed with exit code " + std::to_string( code );
        }
    }

    std::string FetchGoogleFont( const std::string &family, std::vector<uint8_t> &bytes )
    {
        std::string query = family;
        std::replace( query.begin( ), query.end( ), ' ', '+' );
        std::vector<uint8_t> css;
        if ( const int code = Curl( "https://fonts.googleapis.com/css2?family=" + query, "-sSL", css ); code != 0 )
        {
            return CurlError( code );
        }
        const std::string text( css.begin( ), css.end( ) );
        const size_t      begin = text.find( "url(" );
        const size_t      end   = text.find( ')', begin );
        if ( begin == std::string::npos || end == std::string::npos )
        {
            return "Google Fonts has no family named \"" + family + "\", names are case sensitive";
        }
        const std::string url = text.substr( begin + 4, end - begin - 4 );
        if ( !IsFontUrl( url ) )
        {
            return "Unexpected font URL in the stylesheet";
        }
        if ( const int code = Curl( url, "-sSfL", bytes ); code != 0 )
        {
            return CurlError( code );
        }
        return { };
    }
} // namespace

void UIExample::Init( )
{
    DenOfIz_UIDesc uiDesc{ };
    uiDesc.LogicalDevice      = m_logicalDevice;
    uiDesc.ResourceTracking   = m_resourceTracking;
    uiDesc.Window             = m_window;
    uiDesc.RenderTargetFormat = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    uiDesc.NumFrames          = NumSwapChainBuffers;
    uiDesc.Width              = m_pixelWidth;
    uiDesc.Height             = m_pixelHeight;
    m_ui                      = DenOfIz_UI_Create( &uiDesc );

    m_interFont = DenOfIz_Font_CreateFromPath( DENOFIZ_STRING( "Assets/Fonts/Inter-Regular.ttf" ), 0 );
    DenOfIz_UI_AddFont( m_ui, InterFont, m_interFont );
    m_iconFont = DenOfIz_Font_CreateFromPath( DENOFIZ_STRING( "Assets/Fonts/FontAwesome7.otf" ), 0 );
    DenOfIz_UI_AddFont( m_ui, IconFont, m_iconFont );
    uint16_t            interFallbacks[] = { IconFont };
    DenOfIz_UInt16Array fallbacks{ interFallbacks, 1 };
    DenOfIz_UI_SetFontFallbacks( m_ui, InterFont, &fallbacks );
#ifdef __APPLE__
    m_bidiFont = DenOfIz_Font_CreateFromPath( DENOFIZ_STRING( "/System/Library/Fonts/Supplemental/Arial Unicode.ttf" ), 0 );
    if ( DENOFIZ_HANDLE_IS_VALID( m_bidiFont ) )
    {
        DenOfIz_UI_AddFont( m_ui, BidiFont, m_bidiFont );
    }
#endif
    m_bidi.FontId = BidiFont;

    m_tabsId    = Id( "Tabs" );
    m_pagesId   = Id( "Pages" );
    m_clickId   = Id( "ClickMe" );
    m_qualityId = Id( "Quality" );
    m_listId    = Id( "List" );
    m_dialogId  = Id( "Dialog" );
    for ( TextEdit *edit : { &m_field, &m_family, &m_bidi } )
    {
        edit->Id    = Id( edit == &m_field ? "TextField" : edit == &m_family ? "FamilyField" : "BidiField" );
        edit->Caret = edit->Anchor = static_cast<uint32_t>( edit->Text.size( ) );
    }

    m_cube = std::make_unique<Spinning3DCubeWidget>( Id( "Cube" ) );
    m_cube->SetCubeColor( DirectX::XMFLOAT4( 0.2f, 0.6f, 1.0f, 1.0f ) );
    m_cube->InitializeRenderResources( m_logicalDevice, m_graphicsQueue, 512, 512 );
}

void UIExample::Update( )
{
    m_deltaTime = static_cast<float>( DenOfIz_StepTimer_GetDeltaTime( m_stepTimer ) );
    m_blinkTimer += m_deltaTime;
    m_cube->Update( m_deltaTime );

    uint32_t frameIndex = 0;
    if ( !BeginFrame( frameIndex ) )
    {
        return;
    }
    m_cube->RenderToTexture( frameIndex );
    LoadDownloadedFont( );
    const auto start = Clock::now( );
    BuildUI( frameIndex );

    const DenOfIz_SystemCursor cursor = DenOfIz_UI_GetCursor( m_ui );
    if ( cursor != m_cursor )
    {
        DenOfIz_InputSystem_SetCursor( cursor );
        m_cursor = cursor;
    }

    Render( frameIndex, m_frame.CommandList );
    const double frameMs = MsSince( start );
    m_frameMs            = m_frameMs == 0.0 ? frameMs : m_frameMs + ( frameMs - m_frameMs ) * 0.05;
    for ( const auto &font : m_webFonts )
    {
        if ( std::exchange( font->Drawn, false ) )
        {
            font->FirstFrameMs = frameMs;
        }
    }

    DenOfIz_Semaphore      cubeSemaphore = m_cube->GetRenderSemaphore( );
    DenOfIz_SemaphoreArray waitSemaphores{ };
    waitSemaphores.Elements    = &cubeSemaphore;
    waitSemaphores.NumElements = 1;
    EndFrame( &waitSemaphores );
}

void UIExample::BuildUI( const uint32_t frameIndex )
{
    DenOfIz_UI_BeginFrame( m_ui, m_deltaTime );
    m_focusId            = DenOfIz_UI_GetFocus( m_ui );
    const bool wantsText = Editing( ) != nullptr;
    if ( wantsText && !m_textInput )
    {
        DenOfIz_InputSystem_StartTextInput( );
        m_blinkTimer = 0.0f;
    }
    else if ( !wantsText && m_textInput )
    {
        DenOfIz_InputSystem_StopTextInput( );
    }
    m_textInput = wantsText;

    DenOfIz_UIElementDesc root = Column( );
    root.Layout.Grow           = 1.0f;
    root.Style.BackgroundColor = Theme::Background;
    Open( root );
    {
        TopBar( );

        DenOfIz_UIElementDesc body{ };
        Fill( body.Layout );
        Open( body );
        {
            Sidebar( );
            ControlsPage( );
            TextPage( );
            FontsPage( );
            ScriptsPage( );
            MediaPage( frameIndex );
        }
        Close( );
    }
    Close( );

    QualityPopup( );
    Tooltip( );
    Dialog( );
    DenOfIz_UI_EndFrame( m_ui );
    m_keyActivate = false;
}

void UIExample::TopBar( )
{
    DenOfIz_UIElementDesc bar    = Row( 10 );
    bar.Layout.Height            = DenOfIz_UIValue_Points( 52 );
    bar.Layout.Padding           = DenOfIz_UILengthEdges_Symmetric( 16, 0 );
    bar.Style.BackgroundColor    = Theme::Surface;
    bar.Style.BorderColor        = Theme::Border;
    bar.Style.BorderWidth.Bottom = 1.0f;
    Open( bar );
    {
        DenOfIz_UIElementDesc logo = Row( );
        logo.Layout.Width          = DenOfIz_UIValue_Points( 26 );
        logo.Layout.Height         = DenOfIz_UIValue_Points( 26 );
        logo.Layout.Justify        = DENOFIZ_UI_JUSTIFY_CENTER;
        logo.Style.BackgroundColor = Theme::Accent;
        logo.Style.CornerRadius    = DenOfIz_UICorners_All( 7 );
        Open( logo );
        Label( "Dz", 12, Theme::OnAccent );
        Close( );
        Label( "DenOfIz UI", 17, Theme::Text );

        DenOfIz_UIElementDesc pushRight{ };
        pushRight.Layout.Margin.Left = DenOfIz_UILength_Auto( );
        Open( pushRight );
        if ( Button( m_debug ? "Outlines: on" : "Outlines: off", Id( "Outlines" ), false ) )
        {
            ToggleDebug( );
        }
        Close( );
    }
    Close( );
}

void UIExample::Sidebar( )
{
    DenOfIz_UIElementDesc side   = Column( 4 );
    side.Layout.Width            = DenOfIz_UIValue_Points( 200 );
    side.Layout.Padding          = DenOfIz_UILengthEdges_All( 12 );
    side.Style.BackgroundColor   = Theme::Sidebar;
    side.Style.BorderColor       = Theme::Border;
    side.Style.BorderWidth.Right = 1.0f;
    Open( side );
    for ( uint32_t i = 0; i < static_cast<uint32_t>( Page::Count ); ++i )
    {
        const uint32_t              id          = DenOfIz_UI_CombineId( m_tabsId, i );
        const DenOfIz_UIInteraction interaction = Interaction( id );
        if ( Activated( id, interaction ) && m_page != static_cast<Page>( i ) )
        {
            m_page        = static_cast<Page>( i );
            m_qualityOpen = false;
        }
        const bool selected = m_page == static_cast<Page>( i );
        const bool hovered  = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );

        DenOfIz_UIElementDesc tab = Row( );
        tab.Id                    = id;
        tab.Flags                 = DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT;
        tab.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
        tab.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 12, 8 );
        tab.Style.BackgroundColor = selected ? Theme::AccentSoft : hovered ? Theme::Hover : Theme::Clear;
        tab.Style.BorderColor     = Theme::Clear;
        tab.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
        tab.Style.CornerRadius    = DenOfIz_UICorners_All( 6 );
        tab.Transition            = ColorTransition( );
        FocusRing( tab );
        Open( tab );
        Label( PageNames[ i ], 14, selected ? Theme::Accent : Theme::Text );
        Close( );
    }

    DenOfIz_UIElementDesc hint{ };
    hint.Layout.Margin.Top = DenOfIz_UILength_Auto( );
    hint.Layout.Padding    = DenOfIz_UILengthEdges_Symmetric( 4, 0 );
    Open( hint );
    Label( "Tab focus, F11 outlines, Esc quits", 12, Theme::Muted );
    Close( );
    Close( );
}

void UIExample::ToggleDebug( )
{
    m_debug = !m_debug;
    DenOfIz_UI_SetDebugFlags( m_ui, m_debug ? DENOFIZ_UI_DEBUG_OUTLINES_BIT | DENOFIZ_UI_DEBUG_HOVERED_BIT : DENOFIZ_UI_DEBUG_NONE_BIT );
}

void UIExample::BeginCard( const char *title, const char *description, const float basis ) const
{
    DenOfIz_UIElementDesc card = Column( 12 );
    if ( basis > 0.0f )
    {
        card.Layout.Grow   = 1.0f;
        card.Layout.Shrink = 1.0f;
        card.Layout.Basis  = DenOfIz_UIValue_Points( basis );
    }
    card.Layout.Padding        = DenOfIz_UILengthEdges_All( 16 );
    card.Style.BackgroundColor = Theme::Surface;
    card.Style.BorderColor     = Theme::Border;
    card.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    card.Style.CornerRadius    = DenOfIz_UICorners_All( 10 );
    Open( card );
    Label( title, 16, Theme::Text );
    Label( description, 13, Theme::Muted );
}

void UIExample::BeginPage( const Page page ) const
{
    DenOfIz_UIElementDesc desc = Column( 16 );
    desc.Id                    = DenOfIz_UI_CombineId( m_pagesId, static_cast<uint32_t>( page ) );
    desc.Layout.Display        = m_page == page ? DENOFIZ_UI_DISPLAY_FLEX : DENOFIZ_UI_DISPLAY_NONE;
    desc.Layout.Padding        = DenOfIz_UILengthEdges_All( 24 );
    desc.Scroll.OverflowY      = DENOFIZ_UI_OVERFLOW_SCROLL;
    Fill( desc.Layout );
    Open( desc );
}

void UIExample::ControlsPage( )
{
    BeginPage( Page::Controls );

    DenOfIz_UIElementDesc grid = { };
    grid.Layout.Wrap           = DENOFIZ_UI_FLEX_WRAP_WRAP;
    grid.Layout.Gap            = DenOfIz_UILength_Points( 16 );
    grid.Layout.LineGap        = DenOfIz_UILength_Points( 16 );
    Open( grid );

    BeginCard( "Buttons", "Hover, press and focus feed back through color transitions.", 320 );
    {
        Open( Row( 8 ) );
        if ( Button( "Click me", m_clickId ) )
        {
            ++m_clicks;
        }
        if ( Button( "Reset", Id( "Reset" ), false, m_clicks == 0 ) )
        {
            m_clicks = 0;
        }
        Close( );
        m_showTooltip = Has( Interaction( m_clickId ).Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );
        Label( Format( "Clicked %u times", m_clicks ), 14, Theme::Text );
    }
    Close( );

    BeginCard( "Settings", "A toggle, a slider and a dropdown popup.", 320 );
    {
        Toggle( Id( "Notifications" ), m_notifications, "Notifications" );

        Open( Row( 12 ) );
        Label( "Volume", 14, Theme::Text );
        Slider( Id( "Volume" ), m_volume );
        Label( Format( "%.0f%%", m_volume * 100.0f ), 14, Theme::Muted );
        Close( );

        Open( Row( 12 ) );
        Label( "Quality", 14, Theme::Text );
        Dropdown( );
        Close( );
    }
    Close( );

    BeginCard( "Dialog", "A modal with enter and exit transitions. Click the backdrop or press Esc to dismiss it.", 320 );
    {
        Open( Row( 12 ) );
        if ( Button( "Open dialog", Id( "OpenDialog" ) ) )
        {
            m_dialogOpen = true;
        }
        Label( Format( "Last result: %s", m_dialogResult ), 14, Theme::Muted );
        Close( );
    }
    Close( );

    Close( );
    Close( );
}

void UIExample::TextPage( )
{
    BeginPage( Page::Text );

    BeginCard( "Text field", "Click to place the caret, drag or Shift+arrows to select, double click selects everything." );
    TextField( m_field );
    Label( Format( "Caret %u, selection %u..%u", m_field.Caret, std::min( m_field.Caret, m_field.Anchor ), std::max( m_field.Caret, m_field.Anchor ) ), 12, Theme::Muted );
    Close( );

    BeginCard( "Glyphs", "Outlines are rendered from the font's curves, so every size stays sharp. The built in font draws its programming ligatures." );
    for ( const float size : TextSizes )
    {
        Label( Format( "%.0fpx Sphinx of black quartz, judge my vow", size ), size, Theme::Text );
    }
    Label( Ligatures, 18, Theme::Text );
    for ( const float size : { 11.0f, 14.0f, 20.0f, 32.0f } )
    {
        Label( Format( "Inter %.0fpx The quick brown fox -> jumps over 1/2 the lazy dog", size ), size, Theme::Text, InterFont );
    }
    {
        DenOfIz_UIElementDesc clip = Row( );
        clip.Layout.Height         = DenOfIz_UIValue_Points( 64 );
        clip.Scroll.OverflowX      = DENOFIZ_UI_OVERFLOW_CLIP;
        clip.Scroll.OverflowY      = DENOFIZ_UI_OVERFLOW_CLIP;
        clip.Style.BackgroundColor = Theme::AccentSoft;
        clip.Style.CornerRadius    = DenOfIz_UICorners_All( 28 );
        Open( clip );
        Label( "Clipped by rounded corners", 72, Theme::Accent, InterFont );
        Close( );
    }
    Close( );

    BeginCard( "Wrapping", "Each column gets an equal share of the row and wraps its text to that width." );
    {
        DenOfIz_UIElementDesc columns = Row( 16 );
        columns.Layout.AlignItems     = DENOFIZ_UI_ALIGN_START;
        Open( columns );

        DenOfIz_UIElementDesc column = Column( 6 );
        Fill( column.Layout );

        DenOfIz_UITextDesc text{ };
        text.Color    = Theme::Text;
        text.FontSize = 14;

        Open( column );
        Label( "Words", 12, Theme::Muted );
        DenOfIz_UI_Text( m_ui, View( Paragraph ), &text );
        Close( );

        Open( column );
        Label( "Two lines, ellipsis", 12, Theme::Muted );
        DenOfIz_UITextDesc clamped = text;
        clamped.MaxLines           = 2;
        clamped.Overflow           = DENOFIZ_UI_TEXT_OVERFLOW_ELLIPSIS;
        DenOfIz_UI_Text( m_ui, View( Paragraph ), &clamped );
        Close( );

        Open( column );
        Label( "Centered", 12, Theme::Muted );
        DenOfIz_UITextDesc centered = text;
        centered.Align              = DENOFIZ_UI_TEXT_ALIGN_CENTER;
        DenOfIz_UI_Text( m_ui, View( Paragraph ), &centered );
        Close( );

        Close( );
    }
    Close( );
    Close( );
}

void UIExample::FontsPage( )
{
    BeginPage( Page::Fonts );

    BeginCard( "Google Fonts", "Type a family name or pick one below. A worker thread fetches the stylesheet and the TrueType file from "
                               "fonts.googleapis.com with curl, the same file a browser downloads, and the text is drawn straight from the outlines." );
    {
        auto show = [ this ]( const std::string &family )
        {
            WebFont &font = RequestFont( family );
            font.Listed   = true;
            m_sizeFont    = &font;
        };

        Open( Row( 8 ) );
        DenOfIz_UIElementDesc field = Column( );
        Fill( field.Layout );
        Open( field );
        TextField( m_family );
        Close( );
        const bool submit = std::exchange( m_submitFamily, false );
        if ( Button( "Load", Id( "LoadFamily" ) ) || submit )
        {
            const std::string family = Trim( m_family.Text );
            m_fontsStatus            = IsFamilyName( family ) ? std::string( ) : "Family names are letters, digits and spaces, like Open Sans";
            if ( m_fontsStatus.empty( ) )
            {
                show( family );
            }
        }
        Close( );

        DenOfIz_UIElementDesc presets = Row( 8 );
        presets.Layout.Wrap           = DENOFIZ_UI_FLEX_WRAP_WRAP;
        presets.Layout.LineGap        = DenOfIz_UILength_Points( 8 );
        Open( presets );
        for ( uint32_t i = 0; i < std::size( Presets ); ++i )
        {
            if ( Button( Presets[ i ], DenOfIz_UI_CombineId( Id( "Preset" ), i ), false ) )
            {
                show( Presets[ i ] );
            }
        }
        if ( Button( "Load all", Id( "LoadAll" ) ) )
        {
            std::for_each( std::begin( Presets ), std::end( Presets ), show );
        }
        Close( );

        uint32_t loaded = 0, pending = 0;
        for ( const auto &font : m_webFonts )
        {
            loaded += font->FontId != 0;
            pending += !font->Done.load( std::memory_order_acquire );
        }
        if ( m_fontsStatus.empty( ) )
        {
            Label( Format( "%u fonts loaded, %u downloading. A frame takes %.1f ms on the CPU.", loaded, pending, m_frameMs ), 12, Theme::Muted );
        }
        else
        {
            Label( m_fontsStatus.c_str( ), 12, Theme::Danger );
        }
    }
    Close( );

    BeginCard( "Any size", "One outline for every size: coverage is solved from the curves per pixel, nothing is rasterised ahead of time." );
    {
        const float size = MinSample * std::pow( MaxSample / MinSample, m_sizeValue );
        Open( Row( 12 ) );
        Label( m_sizeFont != nullptr ? m_sizeFont->Family.c_str( ) : "Inter", 14, Theme::Text );
        Slider( Id( "SampleSize" ), m_sizeValue );
        Label( Format( "%.0f px", size ), 14, Theme::Muted );
        Close( );

        if ( m_sizeFont != nullptr && m_sizeFont->FontId == 0 )
        {
            FontStatus( *m_sizeFont );
        }
        else
        {
            DenOfIz_UIElementDesc view = Row( );
            view.Scroll.OverflowX      = DENOFIZ_UI_OVERFLOW_SCROLL;
            view.Scroll.OverflowY      = DENOFIZ_UI_OVERFLOW_CLIP;
            Open( view );
            DenOfIz_UITextDesc text{ };
            text.FontId   = m_sizeFont != nullptr ? Use( *m_sizeFont, Page::Fonts ) : InterFont;
            text.FontSize = size;
            text.Color    = Theme::Text;
            text.Wrap     = DENOFIZ_UI_TEXT_WRAP_NONE;
            DenOfIz_UI_Text( m_ui, View( "Hamburgefonstiv" ), &text );
            Close( );
        }
    }
    Close( );

    DenOfIz_UIElementDesc grid = { };
    grid.Layout.Wrap           = DENOFIZ_UI_FLEX_WRAP_WRAP;
    grid.Layout.Gap            = DenOfIz_UILength_Points( 16 );
    grid.Layout.LineGap        = DenOfIz_UILength_Points( 16 );
    Open( grid );
    for ( uint32_t i = 0; i < m_webFonts.size( ); ++i )
    {
        if ( m_webFonts[ i ]->Listed )
        {
            FontCard( *m_webFonts[ i ], i );
        }
    }
    Close( );

    Close( );
}

void UIExample::FontCard( WebFont &font, const uint32_t index )
{
    const uint32_t              id          = DenOfIz_UI_CombineId( Id( "FontCard" ), index );
    const DenOfIz_UIInteraction interaction = Interaction( id );
    if ( font.FontId != 0 && Activated( id, interaction ) )
    {
        m_sizeFont = &font;
    }
    const bool hovered = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );

    DenOfIz_UIElementDesc card = Column( 10 );
    card.Id                    = id;
    card.Flags                 = DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT;
    card.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
    card.Layout.Padding        = DenOfIz_UILengthEdges_All( 16 );
    card.Style.BackgroundColor = Theme::Surface;
    card.Style.BorderColor     = m_sizeFont == &font ? Theme::Accent : hovered ? Theme::BorderStrong : Theme::Border;
    card.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    card.Style.CornerRadius    = DenOfIz_UICorners_All( 10 );
    card.Transition            = ColorTransition( );
    Fill( card.Layout );
    card.Layout.Basis = DenOfIz_UIValue_Points( 360 );
    FocusRing( card );
    Open( card );
    const uint16_t fontId = font.FontId != 0 ? Use( font, Page::Fonts ) : 0;
    Label( font.Family.c_str( ), 26, Theme::Text, fontId );
    FontStatus( font );
    if ( fontId != 0 )
    {
        DenOfIz_UITextDesc paragraph{ };
        paragraph.FontId     = fontId;
        paragraph.FontSize   = 16;
        paragraph.LineHeight = 24;
        paragraph.Color      = Theme::Text;
        DenOfIz_UI_Text( m_ui, View( Pangram ), &paragraph );
        for ( const float size : Waterfall )
        {
            Label( Format( "%.0f Handgloves", size ), size, Theme::Text, fontId );
        }
    }
    Close( );
}

void UIExample::ScriptsPage( )
{
    if ( m_page == Page::Scripts && m_scriptFonts.empty( ) )
    {
        for ( const ScriptSample &sample : Scripts )
        {
            m_scriptFonts.push_back( &RequestFont( sample.Family ) );
        }
        for ( const char *family : ColourFonts )
        {
            m_colourFonts.push_back( &RequestFont( family ) );
        }
    }
    BeginPage( Page::Scripts );
    FallbackCard( );
    EmojiCard( );
    BidiCard( );

    BeginCard( "Complex scripts", "Each line is one script, shaped by HarfBuzz with the font's OpenType rules and drawn from a Google font fetched when "
                                  "this page first opened. Right to left lines wrap and start on the right." );
    for ( uint32_t i = 0; i < m_scriptFonts.size( ); ++i )
    {
        const ScriptSample &sample = Scripts[ i ];
        WebFont            &font   = *m_scriptFonts[ i ];

        DenOfIz_UIElementDesc row = Row( 24 );
        row.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 0, 12 );
        row.Style.BorderColor     = Theme::Border;
        row.Style.BorderWidth.Top = 1.0f;
        Open( row );
        {
            DenOfIz_UIElementDesc info = Column( 3 );
            info.Layout.Width          = DenOfIz_UIValue_Points( 300 );
            info.Layout.Shrink         = 0.0f;
            Open( info );
            Label( sample.Script, 15, Theme::Text );
            Label( font.Family.c_str( ), 12, Theme::Muted );
            FontStatus( font );
            Close( );

            DenOfIz_UIElementDesc cell = Row( );
            cell.Layout.Justify        = sample.Rtl ? DENOFIZ_UI_JUSTIFY_END : DENOFIZ_UI_JUSTIFY_START;
            Fill( cell.Layout );
            Open( cell );
            if ( font.FontId != 0 )
            {
                DenOfIz_UITextDesc text{ };
                text.FontId   = Use( font, Page::Scripts );
                text.FontSize = 30;
                text.Color    = Theme::Text;
                DenOfIz_UI_Text( m_ui, View( sample.Text ), &text );
            }
            Close( );
        }
        Close( );
    }
    Close( );

    EdgeCaseCard( );
    Close( );
}

void UIExample::EdgeCaseCard( )
{
    BeginCard( "Edge cases", "A line box is as tall as the tallest font drawing on it, the selection shows each one. A word longer than its box breaks "
                             "between clusters and is shaped again on both sides of the cut: kerning stops there, Arabic keeps its joining forms. Bidi "
                             "controls and other default ignorables take no space, ill-formed UTF-8 shows one U+FFFD per maximal subpart." );
    DenOfIz_UIElementDesc boxes = Row( 16 );
    boxes.Layout.AlignItems     = DENOFIZ_UI_ALIGN_START;
    boxes.Layout.Wrap           = DENOFIZ_UI_FLEX_WRAP_WRAP;
    boxes.Layout.LineGap        = DenOfIz_UILength_Points( 16 );
    Open( boxes );
    for ( uint32_t i = 0; i < std::size( EdgeCases ); ++i )
    {
        const EdgeCase       &sample = EdgeCases[ i ];
        DenOfIz_UIElementDesc box    = Column( 8 );
        box.Layout.Width             = DenOfIz_UIValue_Points( sample.Width + 24 );
        box.Layout.Padding           = DenOfIz_UILengthEdges_All( 12 );
        box.Style.BackgroundColor    = Theme::RowAlt;
        box.Style.CornerRadius       = DenOfIz_UICorners_All( 8 );
        Open( box );
        Label( sample.Name, 12, Theme::Muted );
        DenOfIz_UITextDesc text{ };
        text.FontId   = InterFont;
        text.FontSize = 20;
        text.Color    = Theme::Text;
        text.Wrap     = sample.Wrap;
        if ( i == 0 )
        {
            text.Selection = DenOfIz_UITextSelection{ 0, static_cast<uint32_t>( std::strlen( sample.Text ) ), Theme::Selection };
        }
        DenOfIz_UI_Text( m_ui, View( sample.Text ), &text );
        Close( );
    }
    Close( );
    Close( );
}

void UIExample::FallbackCard( )
{
    BeginCard( "Fallback", "Every line is drawn with Inter, which covers Latin, Greek and Cyrillic. A grapheme cluster Inter lacks takes the first font of its "
                           "fallback list that covers it, here Font Awesome for the icons, else the font the OS picks for it (CoreText, DirectWrite or "
                           "fontconfig), mapped from its file once and cached for every later character of that script." );
    for ( const char *line : FallbackLines )
    {
        DenOfIz_UITextDesc text{ };
        text.FontId   = InterFont;
        text.FontSize = 22;
        text.Color    = Theme::Text;
        DenOfIz_UI_Text( m_ui, View( line ), &text );
    }
    Close( );
}

void UIExample::EmojiCard( )
{
    BeginCard( "Emoji and colour fonts", "Emoji presentation clusters (an emoji base, VS16, keycaps, skin tones, ZWJ sequences, flags) take the first colour font "
                                         "of the fallback chain, else the system's emoji font, VS15 asks for a text font. COLR v0 and v1 paint graphs and sbix or CBDT "
                                         "bitmaps are rasterised on the CPU once per size bucket and sampled from a GPU buffer, other glyphs stay Slug outlines." );
    for ( const char *line : EmojiLines )
    {
        DenOfIz_UITextDesc text{ };
        text.FontId   = InterFont;
        text.FontSize = 22;
        text.Color    = Theme::Text;
        DenOfIz_UI_Text( m_ui, View( line ), &text );
    }

    DenOfIz_UIElementDesc sizes = Row( 16 );
    sizes.Layout.AlignItems     = DENOFIZ_UI_ALIGN_END;
    sizes.Layout.Wrap           = DENOFIZ_UI_FLEX_WRAP_WRAP;
    Open( sizes );
    for ( const float size : EmojiSizes )
    {
        DenOfIz_UIElementDesc cell = Column( 4 );
        cell.Layout.AlignItems     = DENOFIZ_UI_ALIGN_CENTER;
        Open( cell );
        Label( "😀🦄", size, Theme::Text, InterFont );
        Label( Format( "%.0f px", size ), 11, Theme::Muted );
        Close( );
    }
    Close( );

    for ( uint32_t i = 0; i < m_colourFonts.size( ); ++i )
    {
        WebFont              &font = *m_colourFonts[ i ];
        DenOfIz_UIElementDesc row  = Row( 24 );
        row.Layout.Padding         = DenOfIz_UILengthEdges_Symmetric( 0, 10 );
        row.Style.BorderColor      = Theme::Border;
        row.Style.BorderWidth.Top  = 1.0f;
        Open( row );
        {
            DenOfIz_UIElementDesc info = Column( 3 );
            info.Layout.Width          = DenOfIz_UIValue_Points( 220 );
            info.Layout.Shrink         = 0.0f;
            Open( info );
            Label( font.Family.c_str( ), 15, Theme::Text );
            Label( i == 0 ? "COLRv1 build from Google Fonts as Inter's fallback, above the system emoji" : "COLRv1 gradients, transforms and composites", 12,
                   Theme::Muted );
            FontStatus( font );
            Close( );

            if ( font.FontId != 0 )
            {
                DenOfIz_UIElementDesc column = Column( 6 );
                Open( column );
                if ( i == 0 )
                {
                    uint16_t fallback[] = { Use( font, Page::Scripts ) };
                    if ( !m_notoChain )
                    {
                        DenOfIz_UInt16Array fallbacks{ fallback, 1 };
                        DenOfIz_UI_AddFont( m_ui, NotoChain, m_interFont );
                        DenOfIz_UI_SetFontFallbacks( m_ui, NotoChain, &fallbacks );
                        m_notoChain = true;
                    }
                    Label( EmojiSample, 48, Theme::Text, NotoChain );
                    Label( EmojiSample, 48, Theme::Text, InterFont );
                }
                else
                {
                    Label( "Colour 123", 80, Theme::Text, Use( font, Page::Scripts ) );
                }
                Close( );
            }
        }
        Close( );
    }
    Close( );
}

void UIExample::BidiCard( )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_bidiFont ) )
    {
        return;
    }
    BeginCard( "Bidi", "Mixed directions in one string. SheenBidi resolves the embedding levels (UAX #9), every run is shaped in its own direction and "
                       "each line is drawn in visual order, with brackets mirrored and numbers kept left to right. Lines break at libunibreak's "
                       "opportunities (UAX #14). Drawn with Arial Unicode from the system fonts." );
    for ( const BidiSample &sample : BidiSamples )
    {
        DenOfIz_UIElementDesc row = Row( 24 );
        row.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 0, 8 );
        row.Style.BorderColor     = Theme::Border;
        row.Style.BorderWidth.Top = 1.0f;
        Open( row );
        {
            DenOfIz_UIElementDesc info = Column( );
            info.Layout.Width          = DenOfIz_UIValue_Points( 240 );
            info.Layout.Shrink         = 0.0f;
            Open( info );
            Label( sample.Name, 13, Theme::Muted );
            Close( );

            DenOfIz_UIElementDesc cell = Row( );
            cell.Layout.Justify        = sample.Rtl ? DENOFIZ_UI_JUSTIFY_END : DENOFIZ_UI_JUSTIFY_START;
            Fill( cell.Layout );
            Open( cell );
            DenOfIz_UITextDesc text{ };
            text.FontId   = BidiFont;
            text.FontSize = 22;
            text.Color    = Theme::Text;
            DenOfIz_UI_Text( m_ui, View( sample.Text ), &text );
            Close( );
        }
        Close( );
    }

    DenOfIz_UIElementDesc boxes = Row( 16 );
    boxes.Layout.AlignItems     = DENOFIZ_UI_ALIGN_START;
    boxes.Layout.Wrap           = DENOFIZ_UI_FLEX_WRAP_WRAP;
    boxes.Layout.LineGap        = DenOfIz_UILength_Points( 16 );
    Open( boxes );
    for ( const char *paragraph : BidiParagraphs )
    {
        DenOfIz_UIElementDesc box = Column( );
        box.Layout.Width          = DenOfIz_UIValue_Points( 300 );
        box.Layout.Padding        = DenOfIz_UILengthEdges_All( 12 );
        box.Style.BackgroundColor = Theme::RowAlt;
        box.Style.CornerRadius    = DenOfIz_UICorners_All( 8 );
        Open( box );
        DenOfIz_UITextDesc text{ };
        text.FontId     = BidiFont;
        text.FontSize   = 18;
        text.LineHeight = 30;
        text.Color      = Theme::Text;
        DenOfIz_UI_Text( m_ui, View( paragraph ), &text );
        Close( );
    }
    Close( );

    Label( "The caret steps over grapheme clusters, a selection across a direction change is drawn as several rectangles.", 13, Theme::Muted );
    TextField( m_bidi );
    Label( Format( "Caret %u, selection %u..%u", m_bidi.Caret, std::min( m_bidi.Caret, m_bidi.Anchor ), std::max( m_bidi.Caret, m_bidi.Anchor ) ), 12, Theme::Muted );
    Close( );
}

UIExample::WebFont &UIExample::RequestFont( const std::string &family )
{
    auto it = std::find_if( m_webFonts.begin( ), m_webFonts.end( ), [ & ]( const auto &font ) { return font->Family == family; } );
    if ( it == m_webFonts.end( ) )
    {
        it              = m_webFonts.insert( it, std::make_shared<WebFont>( ) );
        ( *it )->Family = family;
    }
    else if ( !( *it )->Done.load( std::memory_order_acquire ) || ( *it )->Error.empty( ) )
    {
        return **it;
    }

    std::shared_ptr<WebFont> font = *it;
    font->Error.clear( );
    font->Done = false;
    std::thread(
        [ font ]
        {
            const auto start = Clock::now( );
            font->Error      = FetchGoogleFont( font->Family, font->Bytes );
            font->Size       = font->Bytes.size( );
            font->DownloadMs = MsSince( start );
            font->Done.store( true, std::memory_order_release );
        } )
        .detach( );
    return *font;
}

void UIExample::LoadDownloadedFont( )
{
    for ( const auto &entry : m_webFonts )
    {
        WebFont &font = *entry;
        if ( font.FontId != 0 || !font.Done.load( std::memory_order_acquire ) || !font.Error.empty( ) )
        {
            continue;
        }
        DenOfIz_FontDesc desc{ };
        desc.Data.Elements    = font.Bytes.data( );
        desc.Data.NumElements = font.Bytes.size( );
        const auto start      = Clock::now( );
        font.Font             = DenOfIz_Font_Create( &desc );
        font.CreateMs         = MsSince( start );
        font.Bytes            = { };
        if ( !DENOFIZ_HANDLE_IS_VALID( font.Font ) )
        {
            font.Error = "The download is not a font file";
        }
        else if ( m_nextFontId >= NotoChain )
        {
            DenOfIz_Font_Destroy( font.Font );
            font.Font  = DENOFIZ_NULL_HANDLE;
            font.Error = "Every UI font id is in use";
        }
        else
        {
            font.FontId = m_nextFontId++;
            DenOfIz_UI_AddFont( m_ui, font.FontId, font.Font );
        }
        return;
    }
}

void UIExample::FontStatus( const WebFont &font )
{
    if ( !font.Done.load( std::memory_order_acquire ) || ( font.FontId == 0 && font.Error.empty( ) ) )
    {
        Label( "Downloading...", 12, Theme::Muted );
        return;
    }
    if ( !font.Error.empty( ) )
    {
        Label( font.Error.c_str( ), 12, Theme::Danger );
        return;
    }
    const double kb = static_cast<double>( font.Size ) / 1024.0;
    Open( Column( 2 ) );
    Label( kb < 1024.0 ? Format( "%.0f KB, fetched in %.0f ms", kb, font.DownloadMs ) : Format( "%.1f MB, fetched in %.0f ms", kb / 1024.0, font.DownloadMs ), 12, Theme::Muted );
    Label( font.FirstFrameMs < 0.0 ? Format( "created in %.2f ms", font.CreateMs ) : Format( "created in %.2f ms, first frame %.1f ms", font.CreateMs, font.FirstFrameMs ), 12,
           Theme::Muted );
    Close( );
}

uint16_t UIExample::Use( WebFont &font, const Page page ) const
{
    font.Drawn = font.Drawn || ( m_page == page && font.FirstFrameMs < 0.0 );
    return font.FontId;
}

void UIExample::MediaPage( const uint32_t frameIndex )
{
    BeginPage( Page::Media );

    BeginCard( "Render target as an image", "The cube is rendered into a texture every frame and drawn with each fit mode." );
    {
        Open( Row( 12 ) );
        constexpr std::pair<DenOfIz_UIImageFit, const char *> fits[] = {
            { DENOFIZ_UI_IMAGE_FIT_FILL, "Fill" }, { DENOFIZ_UI_IMAGE_FIT_CONTAIN, "Contain" }, { DENOFIZ_UI_IMAGE_FIT_COVER, "Cover" } };
        for ( const auto &[ fit, name ] : fits )
        {
            DenOfIz_UIElementDesc column = Column( 6 );
            Fill( column.Layout );
            Open( column );

            DenOfIz_UIElementDesc image = { };
            image.Layout.Height         = DenOfIz_UIValue_Points( 150 );
            image.Style.BackgroundColor = Theme::Dark;
            image.Style.CornerRadius    = DenOfIz_UICorners_All( 8 );
            image.Image.Texture         = m_cube->GetRenderTarget( frameIndex );
            image.Image.Fit             = fit;
            Open( image );
            Close( );
            Label( name, 12, Theme::Muted );
            Close( );
        }
        Close( );
    }
    Close( );

    BeginCard( "Rounded clipping", "The rows are clipped by the list's rounded corners while it scrolls." );
    {
        DenOfIz_UIElementDesc list = Column( );
        list.Id                    = m_listId;
        list.Layout.Height         = DenOfIz_UIValue_Points( 220 );
        list.Scroll.OverflowX      = DENOFIZ_UI_OVERFLOW_CLIP;
        list.Scroll.OverflowY      = DENOFIZ_UI_OVERFLOW_SCROLL;
        list.Style.BorderColor     = Theme::Border;
        list.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
        list.Style.CornerRadius    = DenOfIz_UICorners_All( 14 );
        Open( list );
        for ( uint32_t i = 0; i < NumRows; ++i )
        {
            const uint32_t              id          = DenOfIz_UI_CombineId( m_listId, i );
            const DenOfIz_UIInteraction interaction = Interaction( id );
            if ( Has( interaction.Flags, DENOFIZ_UI_INTERACTION_CLICKED_BIT ) )
            {
                m_selectedRow = static_cast<int32_t>( i );
            }
            const bool selected = m_selectedRow == static_cast<int32_t>( i );
            const bool hovered  = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );

            DenOfIz_UIElementDesc row = Row( );
            row.Id                    = id;
            row.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
            row.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 16, 10 );
            row.Style.BackgroundColor = selected ? Theme::Accent : hovered ? Theme::Hover : i % 2 == 0 ? Theme::Surface : Theme::RowAlt;
            row.Transition            = ColorTransition( );
            Open( row );
            Label( Format( "Row %u", i + 1 ), 14, selected ? Theme::OnAccent : Theme::Text );
            Close( );
        }
        Close( );
    }
    Close( );

    Close( );
}

bool UIExample::Button( const char *label, const uint32_t id, const bool primary, const bool disabled )
{
    const DenOfIz_UIInteraction interaction = Interaction( id );
    const bool                  hovered     = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );
    const bool                  pressed     = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_PRESSED_BIT );

    DenOfIz_UIColor background = primary ? ( pressed ? Theme::AccentDown : hovered ? Theme::AccentHover : Theme::Accent ) : ( pressed ? Theme::Pressed : hovered ? Theme::Hover : Theme::Surface );
    DenOfIz_UIColor border     = primary ? Theme::Accent : Theme::Border;
    DenOfIz_UIColor text       = primary ? Theme::OnAccent : Theme::Text;
    if ( disabled )
    {
        background = border = Theme::Disabled;
        text                = Theme::DisabledText;
    }

    DenOfIz_UIElementDesc desc = Row( );
    desc.Id                    = id;
    desc.Flags                 = DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT | ( disabled ? DENOFIZ_UI_ELEMENT_DISABLED_BIT : DENOFIZ_UI_ELEMENT_NONE_BIT );
    desc.Cursor                = disabled ? DENOFIZ_SYSTEM_CURSOR_NOT_ALLOWED : DENOFIZ_SYSTEM_CURSOR_POINTER;
    desc.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 14, 7 );
    desc.Style.BackgroundColor = background;
    desc.Style.BorderColor     = border;
    desc.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    desc.Style.CornerRadius    = DenOfIz_UICorners_All( 6 );
    desc.Transition            = ColorTransition( );
    FocusRing( desc );
    Open( desc );
    Label( label, 14, text );
    Close( );
    return !disabled && Activated( id, interaction );
}

bool UIExample::Toggle( const uint32_t id, bool &value, const char *label )
{
    const bool changed = Activated( id, Interaction( id ) );
    if ( changed )
    {
        value = !value;
    }

    DenOfIz_UIElementDesc row = Row( 10 );
    row.Id                    = id;
    row.Flags                 = DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT;
    row.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
    row.Layout.AlignSelf      = DENOFIZ_UI_ALIGN_SELF_START;
    row.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 4, 2 );
    row.Style.BorderColor     = Theme::Clear;
    row.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    row.Style.CornerRadius    = DenOfIz_UICorners_All( 6 );
    FocusRing( row );
    Open( row );
    {
        DenOfIz_UIElementDesc track = Row( );
        track.Layout.Width          = DenOfIz_UIValue_Points( 40 );
        track.Layout.Height         = DenOfIz_UIValue_Points( 22 );
        track.Layout.Padding        = DenOfIz_UILengthEdges_All( 2 );
        track.Layout.Justify        = value ? DENOFIZ_UI_JUSTIFY_END : DENOFIZ_UI_JUSTIFY_START;
        track.Style.BackgroundColor = value ? Theme::Accent : Theme::BorderStrong;
        track.Style.CornerRadius    = DenOfIz_UICorners_All( 11 );
        track.Transition            = ColorTransition( );
        Open( track );
        {
            DenOfIz_UIElementDesc knob = { };
            knob.Layout.Width          = DenOfIz_UIValue_Points( 18 );
            knob.Layout.Height         = DenOfIz_UIValue_Points( 18 );
            knob.Style.BackgroundColor = Theme::Surface;
            knob.Style.CornerRadius    = DenOfIz_UICorners_All( 9 );
            knob.Transition.Duration   = 0.18f;
            knob.Transition.Easing     = DENOFIZ_UI_EASING_EASE_IN_OUT;
            knob.Transition.Properties = DENOFIZ_UI_TRANSITION_PROPERTY_POSITION_BIT;
            Open( knob );
            Close( );
        }
        Close( );
        Label( label, 14, Theme::Text );
    }
    Close( );
    return changed;
}

void UIExample::Slider( const uint32_t id, float &value )
{
    const DenOfIz_UIInteraction interaction = Interaction( id );
    const float                 track       = interaction.Rect.Width - KnobSize;
    if ( Has( interaction.Flags, DENOFIZ_UI_INTERACTION_PRESSED_BIT ) && track > 0.0f )
    {
        value = std::clamp( ( interaction.PointerPosition.X - KnobSize * 0.5f ) / track, 0.0f, 1.0f );
    }
    const bool active = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_PRESSED_BIT | DENOFIZ_UI_INTERACTION_HOVERED_BIT );

    DenOfIz_UIElementDesc slider = Row( );
    slider.Id                    = id;
    slider.Pointer               = DENOFIZ_UI_POINTER_MODE_BLOCK;
    slider.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
    slider.Layout.Height         = DenOfIz_UIValue_Points( 20 );
    slider.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( KnobSize * 0.5f, 0 );
    Fill( slider.Layout );
    Open( slider );
    {
        DenOfIz_UIElementDesc rail = { };
        rail.Layout.Height         = DenOfIz_UIValue_Points( 6 );
        rail.Style.BackgroundColor = Theme::Border;
        rail.Style.CornerRadius    = DenOfIz_UICorners_All( 3 );
        Fill( rail.Layout );
        Open( rail );
        {
            DenOfIz_UIElementDesc fill = { };
            fill.Layout.Width          = DenOfIz_UIValue_Percent( value );
            fill.Style.BackgroundColor = Theme::Accent;
            fill.Style.CornerRadius    = DenOfIz_UICorners_All( 3 );
            Open( fill );
            Close( );

            DenOfIz_UIElementDesc knob = { };
            knob.Layout.Position       = DENOFIZ_UI_POSITION_ABSOLUTE;
            knob.Layout.Insets.Left    = DenOfIz_UIValue_Percent( value );
            knob.Layout.Insets.Top     = DenOfIz_UIValue_Points( ( 6 - KnobSize ) * 0.5f );
            knob.Layout.Margin.Left    = DenOfIz_UILength_Points( -KnobSize * 0.5f );
            knob.Layout.Width          = DenOfIz_UIValue_Points( KnobSize );
            knob.Layout.Height         = DenOfIz_UIValue_Points( KnobSize );
            knob.Style.BackgroundColor = Theme::Surface;
            knob.Style.BorderColor     = active ? Theme::AccentDown : Theme::Accent;
            knob.Style.BorderWidth     = DenOfIz_UIEdges_All( 2 );
            knob.Style.CornerRadius    = DenOfIz_UICorners_All( KnobSize * 0.5f );
            knob.Transition            = ColorTransition( );
            Open( knob );
            Close( );
        }
        Close( );
    }
    Close( );
}

void UIExample::Dropdown( )
{
    const DenOfIz_UIInteraction interaction = Interaction( m_qualityId );
    const uint32_t              popupId     = DenOfIz_UI_CombineId( m_qualityId, 1 );
    if ( m_qualityOpen )
    {
        for ( uint32_t i = 0; i < NumQualities; ++i )
        {
            if ( Has( Interaction( DenOfIz_UI_CombineId( popupId, i ) ).Flags, DENOFIZ_UI_INTERACTION_CLICKED_BIT ) )
            {
                m_quality     = i;
                m_qualityOpen = false;
            }
        }
        if ( Has( Interaction( popupId ).Flags, DENOFIZ_UI_INTERACTION_PRESSED_OUTSIDE_BIT ) )
        {
            m_qualityOpen = false;
        }
    }
    if ( Activated( m_qualityId, interaction ) )
    {
        m_qualityOpen = !m_qualityOpen;
    }
    const bool hovered = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );

    DenOfIz_UIElementDesc button = Row( 8 );
    button.Id                    = m_qualityId;
    button.Flags                 = DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT;
    button.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
    button.Layout.Width          = DenOfIz_UIValue_Points( 140 );
    button.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 10, 5 );
    button.Layout.Justify        = DENOFIZ_UI_JUSTIFY_SPACE_BETWEEN;
    button.Style.BackgroundColor = hovered || m_qualityOpen ? Theme::Background : Theme::Surface;
    button.Style.BorderColor     = m_qualityOpen ? Theme::Accent : Theme::Border;
    button.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    button.Style.CornerRadius    = DenOfIz_UICorners_All( 6 );
    button.Transition            = ColorTransition( );
    FocusRing( button );
    Open( button );
    Label( Qualities[ m_quality ], 14, Theme::Text );
    Label( m_qualityOpen ? "^" : "v", 12, Theme::Muted );
    Close( );
}

void UIExample::QualityPopup( )
{
    if ( !m_qualityOpen )
    {
        return;
    }
    DenOfIz_UIElementDesc popup = Column( 2 );
    popup.Id                    = DenOfIz_UI_CombineId( m_qualityId, 1 );
    popup.Layout.Width          = DenOfIz_UIValue_Percent( 1.0f );
    popup.Layout.MaxHeight      = DenOfIz_UIValue_Points( 200 );
    popup.Layout.Padding        = DenOfIz_UILengthEdges_All( 4 );
    popup.Scroll.OverflowX      = DENOFIZ_UI_OVERFLOW_CLIP;
    popup.Scroll.OverflowY      = DENOFIZ_UI_OVERFLOW_SCROLL;
    popup.Style.BackgroundColor = Theme::Surface;
    popup.Style.BorderColor     = Theme::Border;
    popup.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    popup.Style.CornerRadius    = DenOfIz_UICorners_All( 8 );
    popup.Floating.Target       = DENOFIZ_UI_FLOATING_TARGET_ELEMENT;
    popup.Floating.TargetId     = m_qualityId;
    popup.Floating.Anchor       = DENOFIZ_UI_ANCHOR_TOP_LEFT;
    popup.Floating.TargetAnchor = DENOFIZ_UI_ANCHOR_BOTTOM_LEFT;
    popup.Floating.Offset       = DenOfIz_Float2{ 0.0f, 4.0f };
    popup.Floating.Flags        = DENOFIZ_UI_FLOATING_KEEP_IN_VIEWPORT_BIT;
    popup.Transition            = FadeIn( 0.12f, DenOfIz_Float2{ 0.0f, -6.0f } );
    Open( popup );
    for ( uint32_t i = 0; i < NumQualities; ++i )
    {
        const uint32_t id       = DenOfIz_UI_CombineId( popup.Id, i );
        const bool     selected = i == m_quality;
        const bool     hovered  = Has( Interaction( id ).Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );

        DenOfIz_UIElementDesc option = Row( );
        option.Id                    = id;
        option.Cursor                = DENOFIZ_SYSTEM_CURSOR_POINTER;
        option.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 8, 6 );
        option.Style.BackgroundColor = selected ? Theme::AccentSoft : hovered ? Theme::Hover : Theme::Clear;
        option.Style.CornerRadius    = DenOfIz_UICorners_All( 4 );
        Open( option );
        Label( Qualities[ i ], 14, selected ? Theme::Accent : Theme::Text );
        Close( );
    }
    Close( );
}

void UIExample::Tooltip( ) const
{
    if ( !m_showTooltip )
    {
        return;
    }
    DenOfIz_UIElementDesc tip = Row( );
    tip.Id                    = DenOfIz_UI_CombineId( m_clickId, 1 );
    tip.Pointer               = DENOFIZ_UI_POINTER_MODE_PASSTHROUGH;
    tip.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 8, 5 );
    tip.Style.BackgroundColor = Theme::Dark;
    tip.Style.CornerRadius    = DenOfIz_UICorners_All( 5 );
    tip.Floating.Target       = DENOFIZ_UI_FLOATING_TARGET_ELEMENT;
    tip.Floating.TargetId     = m_clickId;
    tip.Floating.Anchor       = DENOFIZ_UI_ANCHOR_TOP_CENTER;
    tip.Floating.TargetAnchor = DENOFIZ_UI_ANCHOR_BOTTOM_CENTER;
    tip.Floating.Offset       = DenOfIz_Float2{ 0.0f, 6.0f };
    tip.Floating.Flags        = DENOFIZ_UI_FLOATING_KEEP_IN_VIEWPORT_BIT;
    tip.Transition            = FadeIn( 0.12f, DenOfIz_Float2{ 0.0f, -4.0f } );
    Open( tip );
    Label( "Anchored to the button, drawn above the page", 12, Theme::OnAccent );
    Close( );
}

void UIExample::Dialog( )
{
    if ( m_dialogOpen && Has( Interaction( m_dialogId ).Flags, DENOFIZ_UI_INTERACTION_PRESSED_OUTSIDE_BIT ) )
    {
        m_dialogOpen   = false;
        m_dialogResult = "dismissed";
    }
    if ( !m_dialogOpen )
    {
        return;
    }

    DenOfIz_UIElementDesc backdrop    = Column( );
    backdrop.Pointer                  = DENOFIZ_UI_POINTER_MODE_BLOCK;
    backdrop.Layout.Width             = DenOfIz_UIValue_Percent( 1.0f );
    backdrop.Layout.Height            = DenOfIz_UIValue_Percent( 1.0f );
    backdrop.Layout.Justify           = DENOFIZ_UI_JUSTIFY_CENTER;
    backdrop.Layout.AlignItems        = DENOFIZ_UI_ALIGN_CENTER;
    backdrop.Style.BackgroundColor    = Theme::Backdrop;
    backdrop.Floating.Target          = DENOFIZ_UI_FLOATING_TARGET_VIEWPORT;
    backdrop.Floating.ZIndex          = 100;
    backdrop.Transition.Duration      = 0.2f;
    backdrop.Transition.Enter.Flags   = DENOFIZ_UI_TRANSITION_STATE_OPACITY_BIT;
    backdrop.Transition.Enter.Opacity = 0.0f;
    backdrop.Transition.Exit          = backdrop.Transition.Enter;
    Open( backdrop );
    {
        DenOfIz_UIElementDesc dialog   = Column( 12 );
        dialog.Id                      = m_dialogId;
        dialog.Layout.Width            = DenOfIz_UIValue_Points( 400 );
        dialog.Layout.Padding          = DenOfIz_UILengthEdges_All( 20 );
        dialog.Style.BackgroundColor   = Theme::Surface;
        dialog.Style.BorderColor       = Theme::Border;
        dialog.Style.BorderWidth       = DenOfIz_UIEdges_All( 1 );
        dialog.Style.CornerRadius      = DenOfIz_UICorners_All( 12 );
        dialog.Transition              = FadeIn( 0.22f, DenOfIz_Float2{ 0.0f, 32.0f } );
        dialog.Transition.Flags        = DENOFIZ_UI_TRANSITION_ENTER_WITH_PARENT_BIT | DENOFIZ_UI_TRANSITION_EXIT_WITH_PARENT_BIT;
        dialog.Transition.Exit.Flags  |= DENOFIZ_UI_TRANSITION_STATE_SCALE_BIT;
        dialog.Transition.Exit.Scale   = 0.9f;
        Open( dialog );
        {
            Label( "Discard unsaved changes?", 18, Theme::Text );
            Label( "The dialog slides in, the backdrop fades on its own and both animate out after they stop being declared.", 14, Theme::Muted );

            DenOfIz_UIElementDesc actions = Row( 8 );
            actions.Layout.Justify        = DENOFIZ_UI_JUSTIFY_END;
            Open( actions );
            if ( Button( "Cancel", Id( "Cancel" ), false ) )
            {
                m_dialogOpen   = false;
                m_dialogResult = "cancelled";
            }
            if ( Button( "Discard", Id( "Discard" ) ) )
            {
                m_dialogOpen   = false;
                m_dialogResult = "discarded";
            }
            Close( );
        }
        Close( );
    }
    Close( );
}

void UIExample::TextField( TextEdit &edit )
{
    const uint32_t              textId      = DenOfIz_UI_CombineId( edit.Id, 2 );
    const DenOfIz_UIInteraction interaction = Interaction( edit.Id );
    if ( Has( interaction.Flags, DENOFIZ_UI_INTERACTION_PRESS_STARTED_BIT ) )
    {
        edit.Selecting = interaction.ClickCount == 1;
        if ( interaction.ClickCount >= 2 )
        {
            edit.Anchor = 0;
            MoveCaret( edit, static_cast<uint32_t>( edit.Text.size( ) ), true );
        }
        else
        {
            MoveCaret( edit, DenOfIz_UI_GetTextOffsetAtPoint( m_ui, textId, DenOfIz_UI_GetPointerPosition( m_ui ) ), Shift( DenOfIz_InputSystem_GetModState( ) ) );
        }
    }
    else if ( edit.Selecting && Has( interaction.Flags, DENOFIZ_UI_INTERACTION_PRESSED_BIT ) )
    {
        MoveCaret( edit, DenOfIz_UI_GetTextOffsetAtPoint( m_ui, textId, DenOfIz_UI_GetPointerPosition( m_ui ) ), true );
    }
    const bool focused = m_focusId == edit.Id;
    const bool hovered = Has( interaction.Flags, DENOFIZ_UI_INTERACTION_HOVERED_BIT );

    DenOfIz_UIElementDesc field = Row( );
    field.Id                    = edit.Id;
    field.Flags                 = DENOFIZ_UI_ELEMENT_FOCUSABLE_BIT;
    field.Cursor                = DENOFIZ_SYSTEM_CURSOR_TEXT;
    field.Layout.Padding        = DenOfIz_UILengthEdges_Symmetric( 10, 7 );
    field.Style.BackgroundColor = Theme::Surface;
    field.Style.BorderColor     = focused ? Theme::Accent : hovered ? Theme::BorderStrong : Theme::Border;
    field.Style.BorderWidth     = DenOfIz_UIEdges_All( 1 );
    field.Style.CornerRadius    = DenOfIz_UICorners_All( 6 );
    field.Transition            = ColorTransition( DENOFIZ_UI_TRANSITION_PROPERTY_BORDER_COLOR_BIT );
    Open( field );
    {
        DenOfIz_UIElementDesc viewport = Row( );
        viewport.Id                    = DenOfIz_UI_CombineId( edit.Id, 1 );
        viewport.Layout.Padding.Right  = DenOfIz_UILength_Points( 2 );
        viewport.Scroll.OverflowX      = DENOFIZ_UI_OVERFLOW_SCROLL;
        viewport.Scroll.Scrollbar      = DENOFIZ_UI_SCROLLBAR_MODE_HIDDEN;
        Fill( viewport.Layout );
        Open( viewport );

        DenOfIz_UITextDesc text{ };
        text.Id              = textId;
        text.FontId          = edit.FontId;
        text.Color           = Theme::Text;
        text.Wrap            = DENOFIZ_UI_TEXT_WRAP_NONE;
        text.Selection.Start = std::min( edit.Caret, edit.Anchor );
        text.Selection.End   = std::max( edit.Caret, edit.Anchor );
        text.Selection.Color = Theme::Selection;
        text.Caret           = edit.Caret;
        text.CaretColor      = focused && std::fmod( m_blinkTimer, 1.06f ) < 0.53f ? Theme::Text : Theme::Clear;
        DenOfIz_UI_Text( m_ui, DenOfIz_StringView{ edit.Text.data( ), edit.Text.size( ) }, &text );
        if ( edit.Reveal )
        {
            DenOfIz_UI_ScrollCaretIntoView( m_ui, textId, edit.Caret );
            edit.Reveal = false;
        }
        Close( );
    }
    Close( );
}

void UIExample::Render( const uint32_t frameIndex, DenOfIz_CommandList commandList )
{
    DenOfIz_CommandList_Begin( commandList );
    DenOfIz_Texture backBuffer = m_frame.BackBuffer;
    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, backBuffer, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingAttachmentDesc attachment{ };
    attachment.Resource   = backBuffer;
    attachment.LoadOp     = DENOFIZ_LOAD_OP_CLEAR;
    attachment.StoreOp    = DENOFIZ_STORE_OP_STORE;
    attachment.ClearColor = DenOfIz_Float4{ Theme::Background.R / 255.0f, Theme::Background.G / 255.0f, Theme::Background.B / 255.0f, 1.0f };

    const auto            width  = static_cast<float>( m_frame.Width );
    const auto            height = static_cast<float>( m_frame.Height );
    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &attachment;
    renderingDesc.RTAttachments.NumElements = 1;
    renderingDesc.RenderAreaWidth           = width;
    renderingDesc.RenderAreaHeight          = height;
    renderingDesc.NumLayers                 = 1;

    DenOfIz_CommandList_BeginRendering( commandList, &renderingDesc );
    DenOfIz_CommandList_BindViewport( commandList, 0.0f, 0.0f, width, height );
    DenOfIz_CommandList_BindScissorRect( commandList, 0.0f, 0.0f, width, height );
    DenOfIz_UI_Render( m_ui, frameIndex, commandList );
    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( m_resourceTracking, commandList, backBuffer, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_CommandList_End( commandList );
}

void UIExample::HandleEvent( DenOfIz_Event &event )
{
    DenOfIz_UI_HandleEvent( m_ui, &event );

    TextEdit *edit = Editing( );
    switch ( event.Type )
    {
    case DENOFIZ_EVENT_TYPE_MOUSE_BUTTON_DOWN:
        m_focusVisible = false;
        break;
    case DENOFIZ_EVENT_TYPE_TEXT_INPUT:
        if ( edit != nullptr )
        {
            Insert( *edit, event.Text.Text );
            return;
        }
        break;
    case DENOFIZ_EVENT_TYPE_KEY_DOWN:
        if ( HandleKeyDown( event.Key, edit ) )
        {
            return;
        }
        break;
    default:
        break;
    }
    IExample::HandleEvent( event );
}

UIExample::TextEdit *UIExample::Editing( )
{
    const uint32_t focus = DenOfIz_UI_GetFocus( m_ui );
    for ( TextEdit *edit : { &m_field, &m_family, &m_bidi } )
    {
        if ( focus == edit->Id )
        {
            return edit;
        }
    }
    return nullptr;
}

bool UIExample::HandleKeyDown( const DenOfIz_KeyboardEventData &key, TextEdit *edit )
{
    switch ( key.KeyCode )
    {
    case DENOFIZ_KEY_CODE_TAB:
        DenOfIz_UI_FocusNext( m_ui, Shift( key.Mod ) );
        m_focusVisible = true;
        return true;
    case DENOFIZ_KEY_CODE_F11:
        ToggleDebug( );
        return true;
    case DENOFIZ_KEY_CODE_ESCAPE:
        if ( m_dialogOpen )
        {
            m_dialogOpen   = false;
            m_dialogResult = "dismissed";
            return true;
        }
        if ( m_qualityOpen )
        {
            m_qualityOpen = false;
            return true;
        }
        if ( DenOfIz_UI_GetFocus( m_ui ) != 0 )
        {
            DenOfIz_UI_SetFocus( m_ui, 0 );
            return true;
        }
        return false;
    case DENOFIZ_KEY_CODE_RETURN:
    case DENOFIZ_KEY_CODE_KEYPAD_ENTER:
        if ( edit == &m_family )
        {
            m_submitFamily = true;
            return true;
        }
        [[fallthrough]];
    case DENOFIZ_KEY_CODE_SPACE:
        if ( edit == nullptr )
        {
            m_keyActivate = true;
            return true;
        }
        break;
    default:
        break;
    }
    return edit != nullptr && EditKey( *edit, key );
}

bool UIExample::EditKey( TextEdit &edit, const DenOfIz_KeyboardEventData &key )
{
    const auto               size   = static_cast<uint32_t>( edit.Text.size( ) );
    const bool               extend = Shift( key.Mod );
    const uint32_t           start  = std::min( edit.Caret, edit.Anchor );
    const uint32_t           end    = std::max( edit.Caret, edit.Anchor );
    const DenOfIz_StringView text{ edit.Text.data( ), edit.Text.size( ) };
    switch ( key.KeyCode )
    {
    case DENOFIZ_KEY_CODE_BACKSPACE:
        if ( !DeleteSelection( edit ) && edit.Caret > 0 )
        {
            const uint32_t previous = DenOfIz_Text_PreviousGrapheme( text, edit.Caret );
            edit.Text.erase( previous, edit.Caret - previous );
            MoveCaret( edit, previous, false );
        }
        return true;
    case DENOFIZ_KEY_CODE_DELETE:
        if ( !DeleteSelection( edit ) && edit.Caret < size )
        {
            edit.Text.erase( edit.Caret, DenOfIz_Text_NextGrapheme( text, edit.Caret ) - edit.Caret );
            MoveCaret( edit, edit.Caret, false );
        }
        return true;
    case DENOFIZ_KEY_CODE_LEFT:
        MoveCaret( edit, !extend && start != end ? start : DenOfIz_Text_PreviousGrapheme( text, edit.Caret ), extend );
        return true;
    case DENOFIZ_KEY_CODE_RIGHT:
        MoveCaret( edit, !extend && start != end ? end : DenOfIz_Text_NextGrapheme( text, edit.Caret ), extend );
        return true;
    case DENOFIZ_KEY_CODE_HOME:
    case DENOFIZ_KEY_CODE_END:
        {
            const uint32_t     textId = DenOfIz_UI_CombineId( edit.Id, 2 );
            DenOfIz_UITextLine line{ 0, size, {} };
            for ( uint32_t i = 0, count = DenOfIz_UI_GetTextLineCount( m_ui, textId ); i < count; ++i )
            {
                if ( DenOfIz_UI_GetTextLine( m_ui, textId, i, &line ) && edit.Caret <= line.EndOffset )
                {
                    break;
                }
            }
            MoveCaret( edit, key.KeyCode == DENOFIZ_KEY_CODE_HOME ? line.StartOffset : line.EndOffset, extend );
            return true;
        }
    case DENOFIZ_KEY_CODE_A:
        if ( Shortcut( key.Mod ) )
        {
            edit.Anchor = 0;
            MoveCaret( edit, size, true );
            return true;
        }
        return false;
    default:
        return false;
    }
}

void UIExample::Insert( TextEdit &edit, const DenOfIz_StringView text )
{
    if ( text.Chars == nullptr || text.NumChars == 0 )
    {
        return;
    }
    DeleteSelection( edit );
    edit.Text.insert( edit.Caret, text.Chars, text.NumChars );
    std::replace_if( edit.Text.begin( ) + edit.Caret, edit.Text.begin( ) + edit.Caret + text.NumChars, []( const char c ) { return c == '\n' || c == '\r'; }, ' ' );
    MoveCaret( edit, edit.Caret + static_cast<uint32_t>( text.NumChars ), false );
}

bool UIExample::DeleteSelection( TextEdit &edit )
{
    if ( edit.Caret == edit.Anchor )
    {
        return false;
    }
    const uint32_t start = std::min( edit.Caret, edit.Anchor );
    edit.Text.erase( start, std::max( edit.Caret, edit.Anchor ) - start );
    MoveCaret( edit, start, false );
    return true;
}

void UIExample::MoveCaret( TextEdit &edit, const uint32_t offset, const bool extend )
{
    edit.Caret = std::min( offset, static_cast<uint32_t>( edit.Text.size( ) ) );
    if ( !extend )
    {
        edit.Anchor = edit.Caret;
    }
    edit.Reveal  = true;
    m_blinkTimer = 0.0f;
}

uint32_t UIExample::Id( const char *name ) const
{
    return DenOfIz_UI_HashId( View( name ), 0 );
}

DenOfIz_UIInteraction UIExample::Interaction( const uint32_t id ) const
{
    DenOfIz_UIInteraction interaction{ };
    DenOfIz_UI_GetInteraction( m_ui, id, &interaction );
    return interaction;
}

bool UIExample::Activated( const uint32_t id, const DenOfIz_UIInteraction &interaction ) const
{
    return Has( interaction.Flags, DENOFIZ_UI_INTERACTION_CLICKED_BIT ) || ( m_keyActivate && m_focusId == id );
}

void UIExample::FocusRing( DenOfIz_UIElementDesc &desc ) const
{
    if ( m_focusVisible && desc.Id == m_focusId )
    {
        desc.Style.BorderColor = Theme::Dark;
    }
}

void UIExample::Open( const DenOfIz_UIElementDesc &desc ) const
{
    DenOfIz_UI_OpenElement( m_ui, &desc );
}

void UIExample::Close( ) const
{
    DenOfIz_UI_CloseElement( m_ui );
}

void UIExample::Label( const DenOfIz_StringView text, const float size, const DenOfIz_UIColor color, const uint16_t fontId ) const
{
    DenOfIz_UITextDesc desc{ };
    desc.FontId   = fontId;
    desc.Color    = color;
    desc.FontSize = size;
    DenOfIz_UI_Text( m_ui, text, &desc );
}

void UIExample::Label( const char *text, const float size, const DenOfIz_UIColor color, const uint16_t fontId ) const
{
    Label( View( text ), size, color, fontId );
}

DenOfIz_StringView UIExample::Format( const char *format, ... )
{
    va_list args;
    va_start( args, format );
    const int length = std::vsnprintf( m_format.data( ), m_format.size( ), format, args );
    va_end( args );
    return DenOfIz_StringView{ m_format.data( ), length < 0 ? 0 : std::min( static_cast<size_t>( length ), m_format.size( ) - 1 ) };
}

void UIExample::Quit( )
{
    DenOfIz_FrameSync_WaitIdle( m_frameSync );
    if ( m_textInput )
    {
        DenOfIz_InputSystem_StopTextInput( );
    }
    m_cube.reset( );
    DenOfIz_UI_Destroy( m_ui );
    m_ui = DENOFIZ_NULL_HANDLE;
    DenOfIz_Font_Destroy( m_interFont );
    DenOfIz_Font_Destroy( m_iconFont );
    DenOfIz_Font_Destroy( m_bidiFont );
    for ( const auto &font : m_webFonts )
    {
        DenOfIz_Font_Destroy( font->Font );
    }
    IExample::Quit( );
}
