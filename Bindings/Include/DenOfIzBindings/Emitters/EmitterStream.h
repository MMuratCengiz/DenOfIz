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

#pragma once

#include <cstddef>
#include <filesystem>
#include <fmt/base.h>
#include <fmt/format.h>
#include <fstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace DenOfIz
{
    struct EmitterStreamFormattingDesc
    {
        // Line ending indicator (e.g., ";" for C++, "" for Python)
        std::string LineEndIndicator = ";";

        // Include format
        // C++:  "#include <{}>"
        std::string IncludeFormat;

        // Include format variant (language-specific alternative)
        // C++:  "#include \"{}\""
        std::string IncludeFormatVariant;

        // Assignment format: {} = {}; (left, right)
        // C++/C#:  "{} = {}";
        std::string AssignmentFormat = "{} = {}";

        // Namespace formats
        // C++:  "namespace {} {"
        // C#:   "namespace {} {"
        // Rust: "mod {} {"
        std::string NamespaceOpenFormat;
        std::string NamespaceCloseFormat = "}} // namespace";

        // Class/Struct formats (single argument: name)
        // C++:  "class {} {"
        std::string ClassOpenFormat;
        std::string ClassCloseFormat = "};";

        // Function declaration format (arguments: return_type, function_name, parameters)
        // C++:  "{} {}({});"
        std::string FunctionDeclFormat;

        // Function definition opening (arguments: return_type, function_name, parameters)
        // C++:  "{} {}({})"
        std::string FunctionDefOpenFormat;

        // Block comment format
        std::string BlockCommentOpenFormat  = "/*";
        std::string BlockCommentLineFormat  = " * {}";
        std::string BlockCommentCloseFormat = " */";

        // Inline comment format
        // C++:  "// {}"
        // Lua:  "-- {}"
        std::string InlineCommentFormat = "// {}";
    };

    struct EmitterStreamDesc
    {
        std::filesystem::path              FilePath;
        std::size_t                        IndentWidth = 4;
        EmitterStreamFormattingDesc const *Formatting  = nullptr;
    };

    class EmitterStream
    {
        std::filesystem::path              m_filePath;
        std::ofstream                      m_stream;
        std::size_t                        m_indentWidth;
        std::size_t                        m_currentIndent;
        bool                               m_commentBlockOpen = false;
        EmitterStreamDesc                  m_desc;
        EmitterStreamFormattingDesc const *m_formatting;

    public:
        explicit EmitterStream( EmitterStreamDesc desc );
        ~EmitterStream( );

        EmitterStream( const EmitterStream & )            = delete;
        EmitterStream &operator=( const EmitterStream & ) = delete;

        EmitterStream( EmitterStream && )            = default;
        EmitterStream &operator=( EmitterStream && ) = default;

        void WriteLine( std::string_view line );
        void WriteLine( );

        template <typename... Args>
        void WriteLine( fmt::format_string<Args...> format, Args &&...args )
        {
            WriteIndent( );
            m_stream << fmt::format( format, std::forward<Args>( args )... ) << "\n";
        }

        template <typename... Args>
        void WriteLineRuntime( const std::string &format, Args &&...args )
        {
            WriteIndent( );
            m_stream << fmt::vformat( format, fmt::make_format_args( args... ) ) << "\n";
        }

        void Write( std::string_view text );

        template <typename... Args>
        void Write( fmt::format_string<Args...> format, Args &&...args )
        {
            m_stream << fmt::format( format, std::forward<Args>( args )... );
        }

        void Newline( );

        void Indent( );
        void Dedent( );

        void OpenBlock( );
        void OpenBlock( std::string_view prefix );

        void CloseBlock( bool addSemicolon = false );

        void OpenComment( );
        void WriteComment( std::string_view comment );
        void CloseComment( );

        void WriteList( const std::vector<std::string> &items, std::string_view separator = ", " );
        void WriteSeparatedList( const std::vector<std::string> &items, std::string_view separator = ", ", std::string_view prefix = "", std::string_view suffix = "" );

        void WriteInclude( std::string_view includePath );

        template <typename... Args>
        void WriteInclude( fmt::format_string<Args...> format, Args &&...args )
        {
            if ( m_formatting && !m_formatting->IncludeFormat.empty( ) )
            {
                std::string formattedContent = fmt::format( format, std::forward<Args>( args )... );
                WriteIndent( );
                m_stream << fmt::vformat( m_formatting->IncludeFormat, fmt::make_format_args( formattedContent ) ) << "\n";
            }
        }

        void WriteIncludeVariant( std::string_view includePath );

        template <typename... Args>
        void WriteIncludeVariant( fmt::format_string<Args...> format, Args &&...args )
        {
            if ( m_formatting && !m_formatting->IncludeFormatVariant.empty( ) )
            {
                std::string formattedContent = fmt::format( format, std::forward<Args>( args )... );
                WriteIndent( );
                m_stream << fmt::vformat( m_formatting->IncludeFormatVariant, fmt::make_format_args( formattedContent ) ) << "\n";
            }
        }

        template <typename... Args>
        void WriteNamespaceOpen( fmt::format_string<Args...> format, Args &&...args )
        {
            if ( m_formatting && !m_formatting->NamespaceOpenFormat.empty( ) )
            {
                WriteLine( m_formatting->NamespaceOpenFormat, fmt::format( format, std::forward<Args>( args )... ) );
                Indent( );
            }
        }

        void WriteNamespaceClose( );

        template <typename... Args>
        void WriteAssignment( fmt::format_string<Args...> left, fmt::format_string<Args...> right, Args &&...leftArgs, Args &&...rightArgs )
        {
            if ( m_formatting && !m_formatting->AssignmentFormat.empty( ) )
            {
                WriteLine( m_formatting->AssignmentFormat, fmt::format( left, std::forward<Args>( leftArgs )... ), fmt::format( right, std::forward<Args>( rightArgs )... ) );
            }
        }

        template <typename LArgs, typename RArgs>
        void WriteAssignment( const std::string &left, const std::string &right )
        {
            if ( m_formatting && !m_formatting->AssignmentFormat.empty( ) )
            {
                WriteLineRuntime( m_formatting->AssignmentFormat, left, right );
            }
        }

        template <typename... Args>
        void WriteClassOpen( fmt::format_string<Args...> format, Args &&...args )
        {
            if ( m_formatting && !m_formatting->ClassOpenFormat.empty( ) )
            {
                WriteLine( m_formatting->ClassOpenFormat, fmt::format( format, std::forward<Args>( args )... ) );
                Indent( );
            }
        }

        void WriteClassClose( );

        template <typename... Args>
        void WriteFunctionDecl( fmt::format_string<Args...> returnType, fmt::format_string<Args...> name, fmt::format_string<Args...> params, Args &&...args )
        {
            if ( m_formatting && !m_formatting->FunctionDeclFormat.empty( ) )
            {
                WriteLine( m_formatting->FunctionDeclFormat, fmt::format( returnType, std::forward<Args>( args )... ), fmt::format( name, std::forward<Args>( args )... ),
                           fmt::format( params, std::forward<Args>( args )... ) );
            }
        }

        template <typename... Args>
        void WriteFunctionDeclSimple( const std::string &returnType, const std::string &name, const std::string &params )
        {
            if ( m_formatting && !m_formatting->FunctionDeclFormat.empty( ) )
            {
                WriteLineRuntime( m_formatting->FunctionDeclFormat, returnType, name, params );
            }
        }

        template <typename... Args>
        void WriteFunctionDefOpen( fmt::format_string<Args...> returnType, fmt::format_string<Args...> name, fmt::format_string<Args...> params, Args &&...args )
        {
            if ( m_formatting && !m_formatting->FunctionDefOpenFormat.empty( ) )
            {
                WriteLine( m_formatting->FunctionDefOpenFormat, fmt::format( returnType, std::forward<Args>( args )... ), fmt::format( name, std::forward<Args>( args )... ),
                           fmt::format( params, std::forward<Args>( args )... ) );
                Indent( );
            }
        }

        void WriteFunctionDefOpenSimple( const std::string &returnType, const std::string &name, const std::string &params )
        {
            if ( m_formatting && !m_formatting->FunctionDefOpenFormat.empty( ) )
            {
                WriteIndent( );
                m_stream << fmt::vformat( m_formatting->FunctionDefOpenFormat, fmt::make_format_args( returnType, name, params ) ) << "\n";
                Indent( );
            }
        }

        void WriteFunctionClose( );

        std::string Escape( std::string_view str ) const;
        void        WriteStringLiteral( std::string_view str );

        std::size_t                  GetCurrentIndent( ) const;
        const std::filesystem::path &GetFilePath( ) const;

    private:
        void WriteIndent( );
    };
} // namespace DenOfIz
