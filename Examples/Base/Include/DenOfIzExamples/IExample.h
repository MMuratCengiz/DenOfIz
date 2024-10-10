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

#include <DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h>
#include <DenOfIzGraphics/Backends/GraphicsApi.h>
#include "WorldData.h"

namespace DenOfIz
{
    struct WindowDesc
    {
        std::string Title     = "DenOfIzExample";
        uint32_t    Width     = 800;
        uint32_t    Height    = 600;
        bool        Resizable = false;
    };
    class IExample
    {
    protected:
        GraphicsWindowHandle       *m_windowHandle  = nullptr;
        GraphicsApi                *m_graphicsApi   = nullptr;
        ILogicalDevice             *m_logicalDevice = nullptr;
        std::unique_ptr<ISwapChain> m_swapChain     = nullptr;
        WindowDesc                  m_windowDesc;
        std::unique_ptr<Camera>     m_camera;
        WorldData                   m_worldData{ };
        bool                        m_isRunning = true;

    public:
        virtual ~IExample( ) = default;
        void Init( GraphicsWindowHandle *window, GraphicsApi *graphicsApi, ILogicalDevice *device )
        {
            m_windowHandle  = window;
            m_graphicsApi   = graphicsApi;
            m_logicalDevice = device;
            m_windowDesc    = WindowDesc( );

            SwapChainDesc swapChainDesc{ };
            swapChainDesc.Width        = m_windowDesc.Width;
            swapChainDesc.Height       = m_windowDesc.Height;
            swapChainDesc.WindowHandle = m_windowHandle;

            m_swapChain        = m_logicalDevice->CreateSwapChain( swapChainDesc );
            m_camera           = std::make_unique<Camera>( static_cast<float>( m_windowDesc.Width ) / m_windowDesc.Height );
            m_worldData.Camera = m_camera.get( );

            Init( );
        }

        virtual void Init( ) = 0;
        virtual void ModifyApiPreferences( APIPreference &defaultApiPreference )
        {
        }
        virtual void HandleEvent( SDL_Event &event )
        {
            if ( event.type == SDL_KEYDOWN )
            {
                switch ( event.key.keysym.sym )
                {
                case SDLK_ESCAPE:
                    m_isRunning = false;
                    break;
                default:
                    break;
                }
            }
        }
        virtual void Update( ) = 0;
        virtual void Quit( )
        {
            m_logicalDevice->WaitIdle( );
            m_logicalDevice = nullptr;
        }
        virtual WindowDesc WindowDesc( )
        {
            struct WindowDesc windowDesc;
            return windowDesc;
        }
        [[nodiscard]] bool IsRunning( ) const
        {
            return m_isRunning;
        }
    };
} // namespace DenOfIz
