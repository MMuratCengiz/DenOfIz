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

// Resolve an odd Swig error
#ifdef __linux__
#undef __valid
#include <chrono>
#endif

#include "Common_Apple.h"
#include "Common_Windows.h"
#include "DenOfIzGraphics/Assets/FileSystem/FSConfig.h"

namespace DenOfIz
{
    enum class LogLevel
    {
        Info,
        Warning,
        Error,
        Fatal
    };

    struct DZ_API EngineDesc
    {
        LogLevel      LogLevel  = LogLevel::Info;
        InteropString LogFile   = "DenOfIz.log";
        FSDesc        FS = { };
    };

    class DZ_API Engine
    {
    public:
        static void Init( const EngineDesc &desc = { } );
        static void Shutdown( );
    };
} // namespace DenOfIz
