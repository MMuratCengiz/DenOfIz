/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
