/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <stddef.h>
#include <stdint.h>
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int32_t Width;
        int32_t Height;
    } DenOfIz_DisplaySize;

    typedef struct
    {
        uint32_t            ID;
        DenOfIz_DisplaySize Size;
        float               DisplayScale;
        bool                IsPrimary;
    } DenOfIz_Display;

    typedef struct DenOfIz_DisplayArray
    {
        DenOfIz_Display *Elements;
        size_t           NumElements;
    } DenOfIz_DisplayArray;

    DZ_API DenOfIz_DisplayArray DenOfIz_Display_GetDisplays( );
    DZ_API DenOfIz_Display      DenOfIz_Display_GetPrimaryDisplay( );
    DZ_API void                 DenOfIz_Display_RefreshDisplays( );

#ifdef __cplusplus
}
#endif
