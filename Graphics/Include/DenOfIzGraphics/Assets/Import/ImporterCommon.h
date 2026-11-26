/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum DenOfIz_ImporterResultCode
    {
        DENOFIZ_IMPORTER_RESULT_SUCCESS,
        DENOFIZ_IMPORTER_RESULT_FILE_NOT_FOUND,
        DENOFIZ_IMPORTER_RESULT_UNSUPPORTED_FORMAT,
        DENOFIZ_IMPORTER_RESULT_IMPORT_FAILED,
        DENOFIZ_IMPORTER_RESULT_WRITE_FAILED,
        DENOFIZ_IMPORTER_RESULT_INVALID_PARAMETERS,
        DENOFIZ_IMPORTER_RESULT_RESOURCE_UNAVAILABLE
    } DenOfIz_ImporterResultCode;

    typedef struct DenOfIz_ImporterResult
    {
        DenOfIz_ImporterResultCode ResultCode;
        DenOfIz_StringView         ErrorMessage;
        DenOfIz_StringViewArray    CreatedAssets;
    } DenOfIz_ImporterResult;

    DZ_API void DenOfIz_ImporterResult_Free( DenOfIz_ImporterResult *result );

#ifdef __cplusplus
}
#endif
