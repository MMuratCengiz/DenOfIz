/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphics/Utilities/Common_Arrays.h>
#include <string>

namespace DenOfIz
{
    [[nodiscard]] inline std::string StringViewToStdString( const DenOfIz_StringView &view )
    {
        if ( view.Chars == nullptr || view.NumChars == 0 )
        {
            return std::string( );
        }

        return std::string( view.Chars, view.NumChars );
    }
} // namespace DenOfIz
