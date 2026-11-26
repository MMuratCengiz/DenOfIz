/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include "DenOfIzExamples/IExample.h"
#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphics/UI/UI.h"
#include "Spinning3DCubeWidget.h"

namespace DenOfIz
{
    class UIExample final : public IExample
    {
        enum class Page : uint32_t
        {
            Controls,
            Text,
            Fonts,
            Scripts,
            Media,
            Count
        };

        struct TextEdit
        {
            std::string Text;
            uint16_t    FontId    = 0;
            uint32_t    Id        = 0;
            uint32_t    Caret     = 0;
            uint32_t    Anchor    = 0;
            bool        Selecting = false;
            bool        Reveal    = false;
        };

        struct WebFont
        {
            std::string          Family;
            std::vector<uint8_t> Bytes;
            std::string          Error;
            size_t               Size       = 0;
            double               DownloadMs = 0.0;
            std::atomic<bool>    Done{ false };
            DenOfIz_Font         Font         = DENOFIZ_NULL_HANDLE;
            uint16_t             FontId       = 0;
            double               CreateMs     = 0.0;
            double               FirstFrameMs = -1.0;
            bool                 Listed       = false;
            bool                 Drawn        = false;
        };

        DenOfIz_UI                            m_ui        = DENOFIZ_NULL_HANDLE;
        DenOfIz_Font                          m_interFont = DENOFIZ_NULL_HANDLE;
        DenOfIz_Font                          m_iconFont  = DENOFIZ_NULL_HANDLE;
        DenOfIz_Font                          m_bidiFont  = DENOFIZ_NULL_HANDLE;
        std::unique_ptr<Spinning3DCubeWidget> m_cube;
        DenOfIz_SystemCursor                  m_cursor = DENOFIZ_SYSTEM_CURSOR_DEFAULT;
        std::array<char, 256>                 m_format{ };

        Page     m_page         = Page::Controls;
        float    m_deltaTime    = 0.0f;
        float    m_blinkTimer   = 0.0f;
        uint32_t m_focusId      = 0;
        bool     m_focusVisible = false;
        bool     m_keyActivate  = false;
        bool     m_textInput    = false;
        bool     m_debug        = false;

        uint32_t    m_clicks        = 0;
        bool        m_showTooltip   = false;
        bool        m_notifications = true;
        float       m_volume        = 0.65f;
        uint32_t    m_quality       = 1;
        bool        m_qualityOpen   = false;
        bool        m_dialogOpen    = false;
        const char *m_dialogResult  = "none";

        TextEdit m_field{ "Click to edit, drag to select" };
        TextEdit m_family{ "Open Sans" };
        TextEdit m_bidi{ "Edit me: שלום means peace, سلام too (123)" };
        bool     m_submitFamily = false;

        std::vector<std::shared_ptr<WebFont>> m_webFonts;
        std::vector<WebFont *>                m_scriptFonts;
        std::vector<WebFont *>                m_colourFonts;
        bool                                  m_notoChain  = false;
        WebFont                              *m_sizeFont   = nullptr;
        uint16_t                              m_nextFontId = 4;
        float                                 m_sizeValue  = 0.45f;
        double                                m_frameMs    = 0.0;
        std::string                           m_fontsStatus;

        int32_t m_selectedRow = -1;

        uint32_t m_tabsId    = 0;
        uint32_t m_pagesId   = 0;
        uint32_t m_clickId   = 0;
        uint32_t m_qualityId = 0;
        uint32_t m_listId    = 0;
        uint32_t m_dialogId  = 0;

    public:
        ~UIExample( ) override = default;
        void Init( ) override;
        void HandleEvent( DenOfIz_Event &event ) override;
        void Update( ) override;
        void Render( uint32_t frameIndex, DenOfIz_CommandList commandList ) override;
        void Quit( ) override;

        ExampleWindowDesc WindowDesc( ) override
        {
            auto windowDesc  = ExampleWindowDesc( );
            windowDesc.Title = "DenOfIz UI Example";
            return windowDesc;
        }

    private:
        void BuildUI( uint32_t frameIndex );
        void TopBar( );
        void Sidebar( );
        void BeginPage( Page page ) const;
        void ControlsPage( );
        void TextPage( );
        void FontsPage( );
        void FontCard( WebFont &font, uint32_t index );
        void ScriptsPage( );
        void FallbackCard( );
        void EmojiCard( );
        void BidiCard( );
        void EdgeCaseCard( );
        void MediaPage( uint32_t frameIndex );
        void QualityPopup( );
        void Tooltip( ) const;
        void Dialog( );
        void ToggleDebug( );

        bool Button( const char *label, uint32_t id, bool primary = true, bool disabled = false );
        bool Toggle( uint32_t id, bool &value, const char *label );
        void Slider( uint32_t id, float &value );
        void Dropdown( );
        void TextField( TextEdit &edit );

        WebFont &RequestFont( const std::string &family );
        void     LoadDownloadedFont( );
        void     FontStatus( const WebFont &font );
        uint16_t Use( WebFont &font, Page page ) const;

        TextEdit *Editing( );
        bool      HandleKeyDown( const DenOfIz_KeyboardEventData &key, TextEdit *edit );
        bool      EditKey( TextEdit &edit, const DenOfIz_KeyboardEventData &key );
        void      Insert( TextEdit &edit, DenOfIz_StringView text );
        bool      DeleteSelection( TextEdit &edit );
        void      MoveCaret( TextEdit &edit, uint32_t offset, bool extend );

        uint32_t              Id( const char *name ) const;
        DenOfIz_UIInteraction Interaction( uint32_t id ) const;
        bool                  Activated( uint32_t id, const DenOfIz_UIInteraction &interaction ) const;
        void                  FocusRing( DenOfIz_UIElementDesc &desc ) const;
        void                  Open( const DenOfIz_UIElementDesc &desc ) const;
        void                  Close( ) const;
        void                  Label( DenOfIz_StringView text, float size, DenOfIz_UIColor color, uint16_t fontId = 0 ) const;
        void                  Label( const char *text, float size, DenOfIz_UIColor color, uint16_t fontId = 0 ) const;
        void                  BeginCard( const char *title, const char *description, float basis = 0.0f ) const;
        DenOfIz_StringView    Format( const char *format, ... );
    };
} // namespace DenOfIz
