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

#include "DenOfIzBindings/Emitters/EmitterStream.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

using namespace DenOfIz;

EmitterStream::EmitterStream( EmitterStreamDesc desc ) :
    m_filePath( std::move( desc.FilePath ) ), m_indentWidth( desc.IndentWidth ), m_currentIndent( 0 ), m_desc( std::move( desc ) ), m_formatting( m_desc.Formatting )
{
    m_filePath = m_filePath.lexically_normal( );

    std::filesystem::path parentDir = m_filePath.parent_path( );
    if ( !parentDir.empty( ) && !std::filesystem::exists( parentDir ) )
    {
        std::error_code ec;
        std::filesystem::create_directories( parentDir, ec );
        if ( ec )
        {
            spdlog::critical( "Failed to create directories for '{}': {}", parentDir.string( ), ec.message( ) );
        }
    }

    m_stream.open( m_filePath, std::ios::trunc );
    if ( !m_stream.is_open( ) )
    {
        spdlog::critical( "Failed to open file '{}'", m_filePath.string( ) );
    }
}

EmitterStream::~EmitterStream( )
{
    if ( m_stream.is_open( ) )
    {
        m_stream.flush( );
        if ( m_stream.fail( ) )
        {
            spdlog::error( "Failed to write to file '{}'", m_filePath.string( ) );
        }
        m_stream.close( );
    }
}

void EmitterStream::WriteLine( const std::string_view line )
{
    WriteIndent( );
    m_stream << line << "\n";
}

void EmitterStream::WriteLine( )
{
    m_stream << "\n";
}

void EmitterStream::Write( const std::string_view text )
{
    m_stream << text;
}

void EmitterStream::Newline( )
{
    m_stream << "\n";
}

void EmitterStream::Indent( )
{
    m_currentIndent += m_indentWidth;
}

void EmitterStream::Dedent( )
{
    if ( m_currentIndent >= m_indentWidth )
    {
        m_currentIndent -= m_indentWidth;
    }
}

void EmitterStream::OpenBlock( )
{
    WriteIndent( );
    m_stream << "{\n";
    Indent( );
}

void EmitterStream::OpenBlock( const std::string_view prefix )
{
    WriteIndent( );
    m_stream << prefix << "\n";
    WriteIndent( );
    m_stream << "{\n";
    Indent( );
}

void EmitterStream::CloseBlock( const bool addSemicolon )
{
    Dedent( );
    WriteIndent( );
    m_stream << "}";
    if ( addSemicolon )
    {
        m_stream << ";";
    }
    m_stream << "\n";
}

void EmitterStream::OpenComment( )
{
    WriteLine( "/*" );
    m_commentBlockOpen = true;
}

void EmitterStream::WriteComment( std::string_view comment )
{
    if ( m_commentBlockOpen )
    {
        WriteLine( fmt::format( " * {}", comment ) );
    }
    else
    {
        WriteLine( fmt::format( "// {}", comment ) );
    }
}

void EmitterStream::CloseComment( )
{
    if ( m_commentBlockOpen )
    {
        WriteLine( " */" );
        m_commentBlockOpen = false;
    }
}

void EmitterStream::WriteList( const std::vector<std::string> &items, const std::string_view separator )
{
    for ( std::size_t i = 0; i < items.size( ); ++i )
    {
        if ( i > 0 )
        {
            m_stream << separator;
        }
        m_stream << items[ i ];
    }
}

void EmitterStream::WriteSeparatedList( const std::vector<std::string> &items, const std::string_view separator, const std::string_view prefix, const std::string_view suffix )
{
    if ( !prefix.empty( ) )
    {
        m_stream << prefix;
    }
    WriteList( items, separator );
    if ( !suffix.empty( ) )
    {
        m_stream << suffix;
    }
}

void EmitterStream::WriteInclude( std::string_view includePath )
{
    if ( m_formatting && !m_formatting->IncludeFormat.empty( ) )
    {
        WriteIndent( );
        m_stream << fmt::vformat( m_formatting->IncludeFormat, fmt::make_format_args( includePath ) ) << "\n";
    }
}

void EmitterStream::WriteIncludeVariant( std::string_view includePath )
{
    if ( m_formatting && !m_formatting->IncludeFormatVariant.empty( ) )
    {
        WriteIndent( );
        m_stream << fmt::vformat( m_formatting->IncludeFormatVariant, fmt::make_format_args( includePath ) ) << "\n";
    }
}

std::string EmitterStream::Escape( const std::string_view str ) const
{
    std::string result;
    result.reserve( str.size( ) + 10 );
    for ( char c : str )
    {
        switch ( c )
        {
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        case '"':
            result += "\\\"";
            break;
        case '\\':
            result += "\\\\";
            break;
        default:
            result += c;
            break;
        }
    }
    return result;
}

void EmitterStream::WriteStringLiteral( const std::string_view str )
{
    m_stream << "\"" << Escape( str ) << "\"";
}

std::size_t EmitterStream::GetCurrentIndent( ) const
{
    return m_currentIndent;
}

const std::filesystem::path &EmitterStream::GetFilePath( ) const
{
    return m_filePath;
}

void EmitterStream::WriteNamespaceClose( )
{
    Dedent( );
    if ( m_formatting && !m_formatting->NamespaceCloseFormat.empty( ) )
    {
        WriteLine( m_formatting->NamespaceCloseFormat );
    }
}

void EmitterStream::WriteClassClose( )
{
    Dedent( );
    if ( m_formatting && !m_formatting->ClassCloseFormat.empty( ) )
    {
        WriteLine( m_formatting->ClassCloseFormat );
    }
}

void EmitterStream::WriteFunctionClose( )
{
    Dedent( );
    CloseBlock( );
}

void EmitterStream::WriteIndent( )
{
    for ( std::size_t i = 0; i < m_currentIndent; ++i )
    {
        m_stream << " ";
    }
}
