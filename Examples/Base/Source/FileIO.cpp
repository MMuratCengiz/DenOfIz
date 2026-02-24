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

#include "DenOfIzExamples/FileIO.h"
#include <filesystem>
#include <string>

using namespace DenOfIz;

namespace
{
    std::string ToStdString( const DenOfIz_StringView &view )
    {
        if ( view.Chars == nullptr || view.NumChars == 0 )
        {
            return { };
        }
        return std::string( view.Chars, view.Chars + view.NumChars );
    }
} // namespace

bool DenOfIz::FileIO::FileExists( const DenOfIz_StringView &path )
{
    const std::string pathStr = ToStdString( path );
    return std::filesystem::exists( pathStr );
}
