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

#include <DenOfIzGraphics/Assets/Bundle/Bundle.h>
#include <filesystem>

using namespace DenOfIz;

Bundle::Bundle( const BundleDesc &desc ) : m_desc( desc ), m_bundleFile( nullptr ), m_isDirty( false )
{
    // Try to open the bundle file
    bool bundleExists = std::filesystem::exists( desc.Path.Get( ) );

    if ( bundleExists )
    {
        m_bundleFile = new std::fstream( desc.Path.Get( ), std::ios::binary | std::ios::in | std::ios::out );
        LoadTableOfContents( );
    }
    else if ( desc.CreateIfNotExists )
    {
        // Create new bundle
        m_bundleFile = new std::fstream( desc.Path.Get( ), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc );
        WriteEmptyHeader( );
    }
}

BinaryReader *Bundle::OpenReader( const AssetPath &path )
{
    const std::string pathStr = path.ToString( ).Get( );

    if ( const auto it = m_assetEntries.find( pathStr ); it != m_assetEntries.end( ) )
    {
        BinaryReaderDesc desc{ };
        desc.Offset     = it->second.Offset;
        desc.NumBytes   = it->second.NumBytes;
        desc.ForceRange = true;
        return new BinaryReader( m_bundleFile, it->second.Offset, it->second.NumBytes );
    }

    const std::string fsPath = path.Path.Get( );
    if ( std::filesystem::exists( fsPath ) )
    {
        return new BinaryReader( InteropString( fsPath.c_str( ) ) );
    }

    return nullptr;
}
