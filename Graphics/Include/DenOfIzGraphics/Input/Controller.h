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

#include <stdbool.h>
#include <stdint.h>
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Input/InputData.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_ControllerDeviceInfo
    {
        uint32_t           InstanceID;
        DenOfIz_StringView Name;
        bool               IsConnected;
        uint32_t           VendorID;
        uint32_t           ProductID;
        uint16_t           Version;
        uint32_t           PlayerIndex;
    } DenOfIz_ControllerDeviceInfo;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_Controller )

    DZ_API DenOfIz_Controller DenOfIz_Controller_Create( );
    DZ_API DenOfIz_Controller DenOfIz_Controller_CreateWithIndex( int32_t controllerIndex );
    DZ_API void               DenOfIz_Controller_Destroy( DenOfIz_Controller controller );

    DZ_API bool DenOfIz_Controller_Open( DenOfIz_Controller controller, int32_t controllerIndex );
    DZ_API void DenOfIz_Controller_Close( DenOfIz_Controller controller );

    DZ_API bool    DenOfIz_Controller_IsButtonPressed( DenOfIz_Controller controller, DenOfIz_ControllerButton button );
    DZ_API int16_t DenOfIz_Controller_GetAxisValue( DenOfIz_Controller controller, DenOfIz_ControllerAxis axis );

    DZ_API bool DenOfIz_Controller_HasRumble( DenOfIz_Controller controller );
    DZ_API bool DenOfIz_Controller_SetRumble( DenOfIz_Controller controller, uint16_t lowFrequencyRumble, uint16_t highFrequencyRumble, uint32_t durationMs );
    DZ_API bool DenOfIz_Controller_SetTriggerRumble( DenOfIz_Controller controller, bool leftTrigger, bool rightTrigger, uint16_t strength, uint32_t durationMs );

    DZ_API DenOfIz_StringView DenOfIz_Controller_GetButtonName( DenOfIz_Controller controller, DenOfIz_ControllerButton button );
    DZ_API DenOfIz_StringView DenOfIz_Controller_GetAxisName( DenOfIz_Controller controller, DenOfIz_ControllerAxis axis );
    DZ_API bool               DenOfIz_Controller_HasButton( DenOfIz_Controller controller, DenOfIz_ControllerButton button );
    DZ_API bool               DenOfIz_Controller_HasAxis( DenOfIz_Controller controller, DenOfIz_ControllerAxis axis );
    DZ_API DenOfIz_StringView DenOfIz_Controller_GetMapping( DenOfIz_Controller controller );

    DZ_API bool                         DenOfIz_Controller_IsConnected( DenOfIz_Controller controller );
    DZ_API DenOfIz_StringView           DenOfIz_Controller_GetName( DenOfIz_Controller controller );
    DZ_API uint32_t                     DenOfIz_Controller_GetInstanceID( DenOfIz_Controller controller );
    DZ_API DenOfIz_ControllerDeviceInfo DenOfIz_Controller_GetDeviceInfo( DenOfIz_Controller controller );
    DZ_API bool                         DenOfIz_Controller_SetPlayerIndex( DenOfIz_Controller controller, int32_t playerIndex );
    DZ_API int32_t                      DenOfIz_Controller_GetPlayerIndex( DenOfIz_Controller controller );

    DZ_API void               DenOfIz_Controller_InitializeSDL( );
    DZ_API DenOfIz_Int32Array DenOfIz_Controller_GetConnectedControllerIndices( );
    DZ_API bool               DenOfIz_Controller_IsGameController( int32_t joystickIndex );
    DZ_API int32_t            DenOfIz_Controller_GetControllerCount( );
    DZ_API DenOfIz_StringView DenOfIz_Controller_GetControllerNameForIndex( int32_t joystickIndex );

#ifdef __cplusplus
}
#endif
