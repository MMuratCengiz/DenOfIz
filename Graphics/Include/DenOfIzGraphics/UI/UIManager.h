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

#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzGraphics/UI/ClayInternal.h>
#include <memory>

#include "DenOfIzGraphics/Input/Event.h"

namespace DenOfIz
{
	struct UIManagerDesc
	{
		ILogicalDevice *LogicalDevice           = nullptr;
		Format          RenderTargetFormat      = Format::B8G8R8A8Unorm;
		uint32_t        NumFrames               = 3;
		uint32_t        MaxElementCount         = 8192;
		uint32_t        MaxTextMeasureCacheCount = 16384;
		uint32_t        MaxNumQuads             = 2048;
		uint32_t        MaxNumMaterials         = 128;
		uint32_t        Width       = 1024;
		uint32_t        Height      = 1024;
	};

	class UIManager
	{
	    ClayPointerState m_pointerState;

	    struct Impl;
	    std::unique_ptr<Impl> m_impl;
	    UIManagerDesc         m_desc;
	public:
		DZ_API explicit UIManager( const UIManagerDesc &desc );
		DZ_API ~UIManager( );

		DZ_API void BeginFrame( float width, float height ) const;
		DZ_API void EndFrame( ICommandList* commandList, uint32_t frameIndex ) const;

		DZ_API void OpenElement( const ClayElementDeclaration &declaration ) const;
		DZ_API void CloseElement( ) const;
		DZ_API void Text( const InteropString &text, const ClayTextDesc &desc ) const;

		DZ_API void SetPointerState( Float_2 position, ClayPointerState state ) const;
		DZ_API void UpdateScrollContainers( bool enableDragScrolling, Float_2 scrollDelta, float deltaTime ) const;

		DZ_API bool            PointerOver( uint32_t id ) const;
		DZ_API ClayBoundingBox GetElementBoundingBox( uint32_t id ) const;
		DZ_API uint32_t        HashString( const InteropString &str, uint32_t index = 0, uint32_t baseId = 0 ) const;

	    DZ_API void HandleEvent( const Event& event );
		DZ_API void SetViewportSize( float width, float height ) const;
	};

} // namespace DenOfIz