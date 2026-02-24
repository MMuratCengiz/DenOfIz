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

#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <webgpu/webgpu.h>
#include "WebGPUContext.h"

namespace DenOfIz
{
    class WebGPUShadowState
    {
    public:
        struct BufferState
        {
            bool        IsMapped   = false;
            void       *MappedData = nullptr;
            size_t      MappedSize = 0;
            WGPUMapMode MapMode    = static_cast<WGPUMapMode>( 0 );
        };

        struct TextureState
        {
            uint32_t CurrentUsage = 0;
        };

    private:
        std::unordered_map<WGPUBuffer, BufferState>   m_bufferStates;
        std::unordered_map<WGPUTexture, TextureState> m_textureStates;
        mutable std::mutex                            m_mutex;

    public:
        void        SetBufferMapped( const WGPUBuffer &buffer, void *data, size_t size, WGPUMapMode mode );
        void        SetBufferUnmapped( const WGPUBuffer &buffer );
        bool        IsBufferMapped( const WGPUBuffer &buffer ) const;
        BufferState GetBufferState( const WGPUBuffer &buffer ) const;
        void        RemoveBuffer( const WGPUBuffer &buffer );
        void        SetTextureUsage( const WGPUTexture &texture, uint32_t usage );
        uint32_t    GetTextureUsage( const WGPUTexture &texture ) const;
        void        RemoveTexture( const WGPUTexture &texture );

        template <typename T>
        struct AsyncOperation
        {
            bool                    Completed = false;
            T                       Result;
            std::mutex              Mutex;
            std::condition_variable CV;

            void Complete( T result )
            {
                {
                    std::lock_guard lock( Mutex );
                    Result    = result;
                    Completed = true;
                }
                CV.notify_all( );
            }

            T Wait( )
            {
                std::unique_lock lock( Mutex );
                CV.wait( lock, [ this ] { return Completed; } );
                return Result;
            }
        };
    };

    extern WebGPUShadowState g_webGPUShadowState;
} // namespace DenOfIz
