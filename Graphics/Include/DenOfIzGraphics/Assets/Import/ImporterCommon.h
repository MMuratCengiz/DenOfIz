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
