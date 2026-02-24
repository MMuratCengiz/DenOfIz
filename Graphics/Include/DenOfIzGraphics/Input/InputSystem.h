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
#include "DenOfIzGraphics/Input/Controller.h"
#include "DenOfIzGraphics/Input/Event.h"
#include "DenOfIzGraphics/Input/Window.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        float X;
        float Y;
    } DenOfIz_MouseCoords;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_InputSystem )

    DZ_API DenOfIz_InputSystem DenOfIz_InputSystem_Create( );
    DZ_API void                DenOfIz_InputSystem_Destroy( DenOfIz_InputSystem inputSystem );

    DZ_API bool DenOfIz_InputSystem_PollEvent( DenOfIz_Event *outEvent );
    DZ_API bool DenOfIz_InputSystem_WaitEvent( DenOfIz_Event *outEvent );
    DZ_API bool DenOfIz_InputSystem_WaitEventTimeout( DenOfIz_Event *outEvent, int32_t timeout );
    DZ_API void DenOfIz_InputSystem_PumpEvents( );
    DZ_API void DenOfIz_InputSystem_FlushEvents( DenOfIz_EventType minType, DenOfIz_EventType maxType );
    DZ_API void DenOfIz_InputSystem_PushEvent( const DenOfIz_Event *ev );

    DZ_API DenOfIz_KeyState   DenOfIz_InputSystem_GetKeyState( DenOfIz_KeyCode key );
    DZ_API bool               DenOfIz_InputSystem_IsKeyPressed( DenOfIz_KeyCode key );
    DZ_API uint32_t           DenOfIz_InputSystem_GetModState( );
    DZ_API void               DenOfIz_InputSystem_SetModState( uint32_t modifiers );
    DZ_API DenOfIz_KeyCode    DenOfIz_InputSystem_GetKeyFromName( DenOfIz_StringView name );
    DZ_API DenOfIz_StringView DenOfIz_InputSystem_GetKeyName( DenOfIz_KeyCode key );
    DZ_API DenOfIz_StringView DenOfIz_InputSystem_GetScancodeName( uint32_t scancode );

    DZ_API DenOfIz_MouseCoords      DenOfIz_InputSystem_GetMouseState( );
    DZ_API DenOfIz_MouseCoords      DenOfIz_InputSystem_GetGlobalMouseState( );
    DZ_API DenOfIz_MouseButtonState DenOfIz_InputSystem_GetMouseButtonState( );
    DZ_API bool                     DenOfIz_InputSystem_IsMouseButtonPressed( DenOfIz_MouseButton button );
    DZ_API uint32_t                 DenOfIz_InputSystem_GetMouseButtons( );
    DZ_API DenOfIz_MouseCoords      DenOfIz_InputSystem_GetRelativeMouseState( );
    DZ_API void                     DenOfIz_InputSystem_WarpMouseInWindow( DenOfIz_Window window, float x, float y );
    DZ_API bool                     DenOfIz_InputSystem_GetRelativeMouseMode( uint32_t windowId );
    DZ_API void                     DenOfIz_InputSystem_SetRelativeMouseMode( uint32_t windowId, bool enabled );
    DZ_API void                     DenOfIz_InputSystem_CaptureMouse( bool enabled );
    DZ_API bool                     DenOfIz_InputSystem_GetMouseFocus( uint32_t windowID );

    DZ_API void DenOfIz_InputSystem_ShowCursor( bool show );
    DZ_API bool DenOfIz_InputSystem_IsCursorShown( );
    DZ_API void DenOfIz_InputSystem_SetCursor( DenOfIz_SystemCursor cursor );
    DZ_API void DenOfIz_InputSystem_ResetCursor( );

    DZ_API void DenOfIz_InputSystem_StartTextInput( );
    DZ_API void DenOfIz_InputSystem_StopTextInput( );
    DZ_API bool DenOfIz_InputSystem_IsTextInputActive( );
    DZ_API void DenOfIz_InputSystem_SetTextInputRect( DenOfIz_Window window, int32_t x, int32_t y, int32_t w, int32_t h );

    DZ_API DenOfIz_Int32Array DenOfIz_InputSystem_GetConnectedControllerIndices( );
    DZ_API int32_t            DenOfIz_InputSystem_GetNumControllers( );
    DZ_API bool               DenOfIz_InputSystem_OpenController( DenOfIz_InputSystem inputSystem, int32_t playerIndex, int32_t controllerIndex );
    DZ_API void               DenOfIz_InputSystem_CloseController( DenOfIz_InputSystem inputSystem, int32_t playerIndex );
    DZ_API bool               DenOfIz_InputSystem_IsControllerConnected( DenOfIz_InputSystem inputSystem, int32_t playerIndex );
    DZ_API DenOfIz_Controller DenOfIz_InputSystem_GetController( DenOfIz_InputSystem inputSystem, int32_t playerIndex );
    DZ_API bool               DenOfIz_InputSystem_IsControllerButtonPressed( DenOfIz_InputSystem inputSystem, int32_t playerIndex, DenOfIz_ControllerButton button );
    DZ_API int16_t            DenOfIz_InputSystem_GetControllerAxisValue( DenOfIz_InputSystem inputSystem, int32_t playerIndex, DenOfIz_ControllerAxis axis );
    DZ_API DenOfIz_StringView DenOfIz_InputSystem_GetControllerName( DenOfIz_InputSystem inputSystem, int32_t playerIndex );
    DZ_API bool DenOfIz_InputSystem_SetControllerRumble( DenOfIz_InputSystem inputSystem, int32_t playerIndex, uint16_t lowFrequency, uint16_t highFrequency, uint32_t durationMs );

#ifdef __cplusplus
}
#endif
