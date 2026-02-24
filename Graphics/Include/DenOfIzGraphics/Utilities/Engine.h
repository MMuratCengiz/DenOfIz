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

#ifdef __linux__
#undef __valid
#include <chrono>
#endif

#include "DenOfIzGraphics/Assets/FileSystem/FSConfig.h"
#include "DenOfIzGraphics/Utilities/Common_Windows.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum DenOfIz_LogLevel
    {
        DENOFIZ_LOG_LEVEL_INFO    = 0,
        DENOFIZ_LOG_LEVEL_WARNING = 1,
        DENOFIZ_LOG_LEVEL_ERROR   = 2,
        DENOFIZ_LOG_LEVEL_FATAL   = 3
    } DenOfIz_LogLevel;

    typedef struct DenOfIz_EngineDesc
    {
        DenOfIz_LogLevel   LogLevel;
        DenOfIz_StringView LogFile;
        DenOfIz_FSDesc     FS;
    } DenOfIz_EngineDesc;

    DZ_API void DenOfIz_Engine_Init( const DenOfIz_EngineDesc *desc );
    DZ_API void DenOfIz_Engine_Shutdown( );

#ifdef __cplusplus
}
#endif
