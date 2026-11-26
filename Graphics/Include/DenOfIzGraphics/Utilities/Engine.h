/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
