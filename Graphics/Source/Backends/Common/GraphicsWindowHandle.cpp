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

#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "DenOfIzGraphicsInternal/Backends/Common/SDLInclude.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

struct GraphicsWindowHandle::Impl
{
	SDL_Window *m_sdlWindow = nullptr;
	uint32_t m_windowId = 0;

	void InitSDL( ) const
    {
		if ( SDL_WasInit( SDL_INIT_VIDEO ) == 0 )
		{
            spdlog::info( "Initializing SDL" );
            if ( SDL_Init( SDL_INIT_VIDEO ) != 0 )
			{
                spdlog::critical( "Failed to initialize SDL: {}", SDL_GetError( ) );
            }
			atexit( SDL_Quit );
		}
	}
};

GraphicsWindowHandle::GraphicsWindowHandle( ) 
	: m_impl( std::make_unique<Impl>( ) )
{
}

GraphicsWindowHandle::~GraphicsWindowHandle( ) = default;

void GraphicsWindowHandle::CreateFromSDLWindowId( const uint32_t windowId ) const
{
	m_impl->InitSDL( );
	m_impl->m_windowId = windowId;
	
	SDL_Window *window = SDL_GetWindowFromID( windowId );
	if ( window == nullptr )
	{
        spdlog::critical( "Failed to get window from SDL ID: {} - {}", windowId, SDL_GetError( ) );
    }
	m_impl->m_sdlWindow = window;
}

uint32_t GraphicsWindowHandle::GetSDLWindowId( ) const
{
	return m_impl->m_windowId;
}

GraphicsWindowSurface GraphicsWindowHandle::GetSurface( ) const
{
	GraphicsWindowSurface result{ };
	if ( m_impl->m_sdlWindow )
	{
        if ( const SDL_Surface *surface = SDL_GetWindowSurface( m_impl->m_sdlWindow ) )
		{
			result.Width  = surface->w;
			result.Height = surface->h;
		}
	}

	return result;
}

bool GraphicsWindowHandle::IsValid( ) const
{
	return m_impl->m_windowId != 0 && m_impl->m_sdlWindow != nullptr;
}

SDL_Window* GraphicsWindowHandle::GetSDLWindow( ) const
{
	return m_impl->m_sdlWindow;
}