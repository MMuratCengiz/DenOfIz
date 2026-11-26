/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum DenOfIz_Result
    {
        DENOFIZ_SUCCESS    = 0,
        DENOFIZ_NOT_READY  = 1,
        DENOFIZ_TIMEOUT    = 2,
        DENOFIZ_INCOMPLETE = 3,

        DENOFIZ_ERROR_UNKNOWN              = -1,
        DENOFIZ_ERROR_OUT_OF_HOST_MEMORY   = -2,
        DENOFIZ_ERROR_OUT_OF_DEVICE_MEMORY = -3,
        DENOFIZ_ERROR_INITIALIZATION       = -4,
        DENOFIZ_ERROR_DEVICE_LOST          = -5,
        DENOFIZ_ERROR_FEATURE_NOT_PRESENT  = -6,
        DENOFIZ_ERROR_INVALID_HANDLE       = -7,
        DENOFIZ_ERROR_INVALID_ARGUMENT     = -8,
        DENOFIZ_ERROR_NOT_SUPPORTED        = -9,
        DENOFIZ_ERROR_TOO_MANY_OBJECTS     = -10,
        DENOFIZ_ERROR_FORMAT_NOT_SUPPORTED = -11,
        DENOFIZ_ERROR_SURFACE_LOST         = -12,
        DENOFIZ_ERROR_OUT_OF_DATE          = -13,
    } DenOfIz_Result;

#define DENOFIZ_SUCCEEDED( result ) ( ( result ) >= 0 )
#define DENOFIZ_FAILED( result ) ( ( result ) < 0 )

#ifdef __cplusplus
}
#endif
