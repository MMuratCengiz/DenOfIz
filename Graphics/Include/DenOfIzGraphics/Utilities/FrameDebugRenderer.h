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

#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphics/Assets/Font/TextRenderer.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAsset.h"
#include "DenOfIzGraphics/Backends/GraphicsApi.h"
#include "DenOfIzGraphics/Backends/Interface/CommandList.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/SwapChain.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_FrameDebugRenderer )

    typedef struct DenOfIz_FrameDebugRendererDesc
    {
        DenOfIz_GraphicsApi   GraphicsApi;
        DenOfIz_LogicalDevice LogicalDevice;
        uint32_t              Width;
        uint32_t              Height;
        DenOfIz_FontAsset     FontAsset;
        DenOfIz_Float4        TextColor;
        float                 RefreshRate;
        uint32_t              FontSize;
        DenOfIz_TextDirection Direction;
        bool                  Enabled;
    } DenOfIz_FrameDebugRendererDesc;

    DZ_API DenOfIz_FrameDebugRenderer DenOfIz_FrameDebugRenderer_Create( const DenOfIz_FrameDebugRendererDesc *desc );
    DZ_API void                       DenOfIz_FrameDebugRenderer_Destroy( DenOfIz_FrameDebugRenderer renderer );

    DZ_API void DenOfIz_FrameDebugRenderer_Render( DenOfIz_FrameDebugRenderer renderer, DenOfIz_CommandList commandList );
    DZ_API void DenOfIz_FrameDebugRenderer_SetViewport( DenOfIz_FrameDebugRenderer renderer, const DenOfIz_Viewport *viewport );
    DZ_API void DenOfIz_FrameDebugRenderer_SetProjectionMatrix( DenOfIz_FrameDebugRenderer renderer, const DenOfIz_Float4x4 *projectionMatrix );
    DZ_API void DenOfIz_FrameDebugRenderer_SetViewportSize( DenOfIz_FrameDebugRenderer renderer, uint32_t width, uint32_t height );

    DZ_API void DenOfIz_FrameDebugRenderer_AddDebugLine( DenOfIz_FrameDebugRenderer renderer, DenOfIz_StringView text, const DenOfIz_Float4 *color );
    DZ_API void DenOfIz_FrameDebugRenderer_ClearCustomDebugLines( DenOfIz_FrameDebugRenderer renderer );

    DZ_API void DenOfIz_FrameDebugRenderer_SetEnabled( DenOfIz_FrameDebugRenderer renderer, bool enabled );
    DZ_API bool DenOfIz_FrameDebugRenderer_IsEnabled( DenOfIz_FrameDebugRenderer renderer );
    DZ_API void DenOfIz_FrameDebugRenderer_ToggleVisibility( DenOfIz_FrameDebugRenderer renderer );

#ifdef __cplusplus
}
#endif
