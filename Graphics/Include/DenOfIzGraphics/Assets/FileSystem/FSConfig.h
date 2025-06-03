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

#include "DenOfIzGraphics/Utilities/Common.h"
#include "DenOfIzGraphics/Utilities/Interop.h"

namespace DenOfIz
{
    struct DZ_API FSDesc
    {
        InteropString Name;
        InteropString AssetPath;
    };

    /// This class configures FileIO::GetResourcePath, AssetPath configured (via FSProfileConfig) will dictate what the root asset directory is.
    class DZ_API FSConfig
    {
        static FSDesc m_profileConfig;

    public:
        /// Defaults to AssetPath = BundleResourcePath( )
        static void InitDefaults( );
        /// It is up to the developer to specify a different mode for development/production
        static void Init( const FSDesc &config );
        // On Osx this will return the resource directory, otherwise the exe directory
        static InteropString AssetPath( );
        static InteropString BundleResourcePath( );
    };
} // namespace DenOfIz
